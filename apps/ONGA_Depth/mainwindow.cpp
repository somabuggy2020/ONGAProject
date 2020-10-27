#include "mainwindow.h"
#include "src/ui_mainwindow.h"

#define THREAD_T 33

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	//Make log directry
	QDir dir;
	dir.mkdir("Log");
	dir.mkdir("Log/DepthData");
	DataPath = QString("./Log");
	DepthDataPath = QString("./Log/DepthData/");

	//Load config
	QSettings cfg("config.ini", QSettings::IniFormat);

	//Refers to the values of the "MAIN" group
	cfg.beginGroup("MAIN");
	count_max = cfg.value("CountN", 100).toInt(); //Number of maximum total frame
	div_n = cfg.value("DivN", 7).toInt();					//Number of image divisions
	h_max = cfg.value("HistMax", 6.0).toFloat();	//Max value of histgram range [m]
	h_b = cfg.value("HistBin", 0.1).toFloat();		//Bin size of histgram [m]
	cfg.endGroup();

	qRegisterMetaType<cv::Mat>();
	qRegisterMetaType<Frames_t>();

	setup(); //UI setup
	setup_signals_slots(); //Signal and Slot connections


	//
	// Try to open R200
	//
	r200 = new R200();

	if(r200->init() == -1){
		qWarning() << "Check Realsense Device Connection";
		QMessageBox::critical(this,
													"R200 connection error",
													"Check Realsense Device connection");
		exit(-1);
	}


	mode = Mode::Wait;
	isCanceled = false;
	t = QDateTime::currentDateTime();
	camparams = camParamControl->camparams;
	isCamParamChanged = false;
	frames = new Frames_t();


	//Try to start the process
	start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

/*!
 * \brief MainWindow::setup
 */
void MainWindow::setup()
{
	/***
	 * setup UI widgets
	 ***/

	//CameraParmeter UI
	camParamControl = new CameraParameterControlPanel(this);
	camParamControl->setWindowFlag(Qt::Window);
	camParamControl->hide();

	//Control Panel UI (add to right dock widget area)
	controlPanel = new ControlPanel(this);
	ui->dwControlPanel->setWidget(controlPanel);

	//Image UIs (add to left dock widget area)
	imgvwrRGB = new ImageViewer("RGB", this);
	imgvwrRGB->initialize(CV_8UC3, QImage::Format::Format_BGR888);
	ui->dwImgRGB->setWidget(imgvwrRGB);

	imgvwrAlignedRGB = new ImageViewer("Aligned RGB", this);
	imgvwrAlignedRGB->initialize(CV_8UC3, QImage::Format::Format_BGR888);
	ui->dwImgAlignedRGB->setWidget(imgvwrAlignedRGB);

	imgvwrAlignedDepth = new ImageViewer("Aligned Depth", this);
	imgvwrAlignedDepth->initialize(CV_16UC1, QImage::Format::Format_Grayscale8);
	ui->dwImgDepth->setWidget(imgvwrAlignedDepth);

	imgvwrHistograms = new ImageViewer("Histograms", this);
	imgvwrHistograms->setWindowFlag(Qt::Window);
	imgvwrHistograms->initialize(CV_8UC3, QImage::Format_BGR888);
	imgvwrHistograms->hide();

	histogramvwr = new HistogramViewer(this);
	histogramvwr->setWindowFlag(Qt::Window);
	histogramvwr->show();

	//Time show label UI (add to statusbar)
	lblStatus = new QLabel(this);
	lblStatus->setText("start ...");
	ui->statusbar->addWidget(lblStatus);
}

/*!
 * \brief MainWindow::setup_signals_slots
 */
void MainWindow::setup_signals_slots()
{
	/***
	 * signals-slots
	 ***/

	/***
	 * call from CameraParamControl UI class
	 ***/
	connect(camParamControl, &CameraParameterControlPanel::On_CameraParams_changed, this,
					[=](CamParams_t &camparams){
		this->camparams = camparams;
		isCamParamChanged = true;
	});


	/***
	 * call from ControlPanel UI class
	 ***/
	connect(controlPanel, &ControlPanel::On_start_clicked, this,
					[=](){

		imgvwrHistograms->hide();

		//release memory
		Q_FOREACH(QList<cv::Mat> mats, I_div_T){
			for(int i = 0; i < mats.count(); i++){
				mats[i].release();
			}
			mats.clear();
		}
		I_div_T.clear();

		//allocate divided depth images memory
		for(int i = 0; i < (div_n*div_n); i++){
			QList<cv::Mat> tmp;
			for(int t = 0; t < count_max; t++){
				cv::Mat mat_tmp = cv::Mat();
				tmp.append(mat_tmp);
			}
			I_div_T.append(tmp);
		}

		frame_counter = 0;
		//		imgHistograms.release();
		//		imgHistograms = cv::Mat();

		emit startMeasurement();
		mode = Mode::Measure; //Mode transition
	});

	connect(controlPanel, &ControlPanel::On_clear_clicked, this,
					[=](){
	});

	connect(controlPanel, &ControlPanel::On_CountMax_changed, this,
					[=](int arg){
		count_max = arg; //maximum number of frame count
	});

	connect(controlPanel, &ControlPanel::On_DivN_changed, this,
					[=](int arg){
		div_n = arg; //change the number of division
	});

	connect(controlPanel, &ControlPanel::On_HistMax_changed, this,
					[=](double arg){
		h_max = arg; //change max value of histgram range
	});

	connect(controlPanel, &ControlPanel::On_HistBin_changed, this,
					[=](double arg){
		h_b = arg; //change bin size of histgram
	});




	/***
	 * call from local signals
	 ***/
	connect(this, &MainWindow::updateTime, this,
					[=](){
		QDateTime ct = QDateTime::currentDateTime();
		unsigned long dt = t.msecsTo(ct);

		QString str = ct.toString("hh:mm:ss.zzz");
		str += QString(" / %1 Hz").arg(QString::number(1/(dt/1000.0),'f',1));
		str += QString(" Mode:%1").arg(Mode::str[mode]);
		lblStatus->setText(str);
		t = ct; //current time shift
	});

	connect(this, &MainWindow::updateFrames, this,
					[=](Frames_t *frames){
		imgvwrRGB->setImage(frames->imgRGB);
	},Qt::BlockingQueuedConnection);

	connect(this, &MainWindow::updateFrames, this,
					[=](Frames_t *frames){
		imgvwrAlignedRGB->setImage(frames->imgAlignedRGB);
	},Qt::BlockingQueuedConnection);

	connect(this, &MainWindow::updateFrames, this,
					[=](Frames_t *frames){
		imgvwrAlignedDepth->setImage(frames->imgDepth);
	}, Qt::BlockingQueuedConnection);

	connect(this, &MainWindow::startMeasurement, this,
					[=](){
		progdialog = new QProgressDialog(this);
		progdialog->setRange(0, count_max);

		connect(progdialog, &QProgressDialog::canceled, this,
						[=]{
			isCanceled = true;
			progdialog->deleteLater();
		});

	});

	connect(this, &MainWindow::progressMeasurement, this,
					[=](int count){
		progdialog->setValue(count);
	});

	connect(this, &MainWindow::finishedMeasurement, this,
					[=](){
		progdialog->deleteLater();
	});

	connect(this, &MainWindow::startCalculate, this,
					[=](){

		QMessageBox *msgbx;
		msgbx = new QMessageBox(this);
		msgbx->setStandardButtons(QMessageBox::NoButton);
		msgbx->setText("Please wait a minute ...");
		connect(this, &MainWindow::finishedCalculate,
						msgbx, &QMessageBox::deleteLater);
		msgbx->show();

	}, Qt::QueuedConnection);

	connect(this, &MainWindow::finishedCalculate, this,
					[=](){

		QLayoutItem *child;
		while((child = ui->gl->takeAt(0)) != nullptr){
			delete child->widget();
			delete child;
		}

		for(int i = 0; i < class_values.count(); i++){
			int x = i%div_n;
			int y = i/div_n;
			QLabel *lbl = new QLabel();
			lbl->setText(QString::number(class_values[i],'f',3));
			lbl->setAlignment(Qt::AlignHCenter);
			lbl->setAlignment(Qt::AlignVCenter);
			lbl->setFrameShape(QFrame::Shape::WinPanel);
			ui->gl->addWidget(lbl, y, x);
		}

		//		imgvwrHistograms->setImage(imgHistograms);
		//		imgvwrHistograms->show();


		histogramvwr->clear();
		Q_FOREACH(QList<float> set, histogram_values){
			histogramvwr->addHistogramData(set);
		}
		histogramvwr->updateHistograms(div_n);
		histogramvwr->update();

	}, Qt::BlockingQueuedConnection);

}

/*!
 * \brief MainWindow::main
 */
void MainWindow::main()
{
	QMetaObject::invokeMethod(timer, "stop");
	emit updateTime();

	if(isCamParamChanged){
		r200->setParams(camparams);
		isCamParamChanged = false;
	}
	if(isCamParamAuto){
		r200->autosetParams();
		isCamParamAuto = false;
	}

	if(r200->getFrames(*frames) == -1){
		qCritical() << "Check Realsense Device Connection";
		exit(255);
	}

	//Drawing
	draw(*frames);

	switch (mode) {
		case Mode::Wait:
			break;
		case Mode::Measure:
			measure(*frames); //call measure method
			break;
		case Mode::Calc: //call calc method
			calc(*frames);
			break;
		case Mode::Save:
			save(*frames); //call save method
			break;
		default:
			break;
	}

	emit updateFrames(frames);

	//if got close signal
	if(!isThread)	return;

	//restart timer
	QMetaObject::invokeMethod(timer, "start");
	return;
}

/*!
 * \brief MainWindow::measure
 * \param frames
 */
void MainWindow::measure(Frames_t &frames)
{
	//clone depth image
	cv::Mat I_depth_t = frames.imgDepth.clone();

	for(int i = 0; i < (div_n*div_n); i++){
		//grid position
		int x = i%div_n; //x position
		int y = i/div_n; //y position

		//set clipping area as rectangle
		cv::Rect rect = cv::Rect(cvRound(I_depth_t.cols/div_n*x), //origin x
														 cvRound(I_depth_t.rows/div_n*y), //origin y
														 cvRound(I_depth_t.cols/div_n),   //width
														 cvRound(I_depth_t.rows/div_n));  //height

		//carry-out clipping
		cv::Mat I_div_i_t_16u(I_depth_t, rect); //1 channel 16bit unsigned int depth

		//calculate 16bit depth value to 32bit float meter value
		cv::Mat I_div_i_t_32f;
		I_div_i_t_16u.convertTo(I_div_i_t_32f,
														CV_32FC1,
														frames.scale);

		I_div_T[i][frame_counter] = I_div_i_t_32f.clone();
	}

	if(isCanceled){
		mode = Mode::Wait;
		frame_counter = 0;
		isCanceled = false;
		return;
	}

	//increment frame counter
	frame_counter++;

	//	qInfo() << "scale factor:" << frames.scale << "/"
	//					<< "frame counter:" << frame_counter;

	//if the number of current total measured frame reached "count_max"
	//mode will be Mode::Calc
	if(frame_counter >= count_max){
		mode = Mode::Calc;
		emit finishedMeasurement();
	}
	else{
		mode = Mode::Measure; //continue
		emit progressMeasurement(frame_counter);
	}
	return;
}

/*!
 * \brief MainWindow::calc
 */
void MainWindow::calc(Frames_t &frames)
{
	emit startCalculate(); //emit signal

	//calculate mean value t~T of each grid
	set_mean_T.clear();
	for(int i = 0; i < I_div_T.count(); i++){ //grid(i)

		float *averages = new float[I_div_T[i].count()];

		for(int _t = 0; _t < I_div_T[i].count(); _t++){ //time(t)
			//make mask for error value
			cv::Mat mask_low, mask_high;
			cv::Mat mask;
			cv::threshold(I_div_T[i][_t], mask_low, 0.05, 255, cv::THRESH_BINARY);
			cv::threshold(I_div_T[i][_t], mask_high, 10.0, 255, cv::THRESH_BINARY_INV);


			mask_low.convertTo(mask_low, CV_8UC1);
			mask_high.convertTo(mask_high, CV_8UC1);
			cv::bitwise_and(mask_low, mask_high, mask);

			//			cv::imshow("mask low", mask_low);
			//			cv::imshow("mask high", mask_high);
			//			cv::imshow("mask", mask);
			//			cv::waitKey(-1);

			//Compute mean value in grid(i) at time(t) without error depth value's pixels
			cv::Scalar _ave = cv::mean(I_div_T[i][_t], mask);
			double ave = _ave.val[0];
			averages[_t] = ave;

			//			qDebug() << QString("[%1] at %2 ave:%3[m]").arg(i).arg(_t).arg(ave);
		}

		cv::Mat tmp(1, I_div_T[i].count(), CV_32FC1, averages);
		set_mean_T.append(tmp);
	}


	//Make histgrams
	QList<cv::Mat> _set_histograms;

	float range[] = {0.0, h_max};
	const float *hrange = {range};
	int histSize = (int)(h_max/h_b); //number of bin

	for(int i = 0; i < set_mean_T.count(); i++){
		cv::Mat hist;
		cv::calcHist(&set_mean_T[i],
								 1, 0, cv::Mat(),
								 hist, 1, &histSize, &hrange,
								 true, false);
		_set_histograms.append(hist);
	}


	/***
	 * Compute the class values
	 * that will be set the maximum frequency value in 0~T frames
	 ***/
	class_values.clear();
	for(int i = 0; i < _set_histograms.count(); i++){
		double min, max;
		cv::Point minLoc, maxLoc;
		min = max = -1;

		//search value of maximum degree class
		cv::minMaxLoc(_set_histograms[i], &min, &max, &minLoc, &maxLoc);

		//get value[m]
		class_values << maxLoc.y*(h_b);
	}



	//Make histgram images for debug
	QList<cv::Mat> _imgHistograms;

	int hist_w = cvRound(h_max/h_b); //[pixel] one bin to one pixel width
	int hist_h = 100; //[pixel] constant value
	cv::Size sz(hist_w, hist_h); //image size of one histgram image

	int bin_w = cvRound((double)hist_w/(h_max/h_b));

	for(int i = 0; i < _set_histograms.count(); i++){
		_imgHistograms.append(cv::Mat(sz, CV_8UC3, cv::Scalar::all(0)));

		//normalization
		cv::Mat tmp;
		cv::normalize(_set_histograms[i], //input
									tmp, //output
									0, //minimum value
									sz.height, //maximum value
									cv::NORM_MINMAX,
									-1, cv::Mat());

		for(int j = 0; j < (int)(h_max/h_b); j++){
			cv::line(_imgHistograms.back(),
							 cv::Point(bin_w*j, hist_h-1),
							 cv::Point(bin_w*j, hist_h - cvRound(tmp.at<float>(j))),
							 cv::Scalar::all(255),
							 1, 8, 0);
		}
	}

	//	int line_width = 3;
	//	cv::Mat vline(hist_h, line_width, CV_8UC3, cv::Scalar(0,0,255));
	//	imgHistograms.release();
	//	imgHistograms = cv::Mat(1, (hist_w+vline.cols)*div_n+vline.cols,
	//													CV_8UC3, cv::Scalar(0,0,255));

	//	for(int y = 0; y < div_n; y++){
	//		cv::Mat matH = vline.clone();
	//		for(int x = 0; x < div_n; x++){
	//			int idx = x + y*div_n;
	//			cv::hconcat(matH, _imgHistograms[idx], matH); //horizontal concat
	//			cv::hconcat(matH, vline, matH);
	//		}
	//		cv::vconcat(imgHistograms, matH, imgHistograms);
	//		cv::Mat hline(line_width, matH.cols, CV_8UC3, cv::Scalar(0,0,255));
	//		cv::vconcat(imgHistograms, hline, imgHistograms);
	//	}


	//--------------------------------------------------
	//--------------------------------------------------
	//--------------------------------------------------
	histogram_values.clear();

	for(int i = 0; i < _set_histograms.count(); i++){
		cv::Mat tmp = _set_histograms[i].t(); //1-D matrix

		QList<float> tmp_vec;
		for(int j = 0; j < tmp.cols; j++) tmp_vec << tmp.at<float>(j);

		histogram_values.append(tmp_vec);
	}

	emit finishedCalculate();
	mode = Mode::Save;
	return;
}

/*!
 * \brief MainWindow::save
 */
void MainWindow::save(Frames_t &frames)
{
	QDateTime currentTime = QDateTime::currentDateTime();

	QString fname = currentTime.toString("yyyy-MM-dd-hh-mm-ss");
	fname += QString(".csv");

	//--------------------------------------------------
	//--------------------------------------------------
	//--------------------------------------------------
	QFile f;
	f.setFileName(DepthDataPath + "/" + fname);

	if(!f.open(QFile::WriteOnly | QFile::Text)){
		qCritical() << f.errorString();
		mode = Mode::Wait;
		return;
	}

	QString header;
	header += QString(",");
	for(int i = 0; i < div_n*div_n; i++){
		header += QString("%1,").arg(i); //grid index
	}
	header += QString("\n");
	f.write(header.toStdString().c_str());

	//	for(int i = 0; i < grid_depth_averages_T.count(); i++){
	for(int i = 0; i < set_mean_T.count(); i++){
		QString line;
		line += QString("%1,").arg(i);

		for(int _t = 0; _t < set_mean_T[i].cols; _t++){
			line += QString("%1,").arg(set_mean_T[i].at<float>(_t));
		}

		line += QString("\n");
		f.write(line.toStdString().c_str());
	}

	f.close();


	//--------------------------------------------------
	//--------------------------------------------------
	//--------------------------------------------------
	f.setFileName(DataPath + "/" + fname);
	if(!f.open(QFile::WriteOnly | QFile::Text)){
		qCritical() << f.errorString();
		mode = Mode::Wait;
		return;
	}

	header = QString("");
	header += QString("DivN,%1\n").arg(div_n);
	header += QString("CountMax,%1\n").arg(count_max);
	f.write(header.toStdString().c_str());

	for(int i = 0; i < class_values.count(); i++){
		QString line;
		line += QString("%1,%2\n").arg(i).arg(class_values[i]);
		f.write(line.toStdString().c_str());
	}
	f.close();

	mode = Mode::Wait;
	return;
}

/*!
 * \brief MainWindow::draw
 * \param frames
 */
void MainWindow::draw(Frames_t &frames)
{
	//draw grid on aligned RGB image
	for(int i = 0; i <= div_n; i++){
		cv::line(frames.imgAlignedRGB,
						 cv::Point(frames.imgAlignedRGB.cols/div_n*i, 0),
						 cv::Point(frames.imgAlignedRGB.cols/div_n*i, frames.imgAlignedRGB.rows),
						 cv::Scalar(0,0,255), //Red
						 3);
		cv::line(frames.imgAlignedRGB,
						 cv::Point(0, frames.imgAlignedRGB.rows/div_n*i),
						 cv::Point(frames.imgAlignedRGB.cols, frames.imgAlignedRGB.rows/div_n*i),
						 cv::Scalar(0,0,255), //Red
						 3);
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	isThread = false;
	th->exit();
	r200->close();

	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("MAIN");
	cfg.setValue("CountN", QVariant::fromValue(count_max));
	cfg.setValue("DivN", QVariant::fromValue(div_n));
	cfg.setValue("HistMax", QVariant::fromValue<float>(h_max));
	cfg.setValue("HistBin", QVariant::fromValue<float>(h_b));
	cfg.endGroup();
	cfg.sync();

	qInfo() << "System closed";
}

/*!
 * \brief MainWindow::start
 */
void MainWindow::start()
{
	isThread = true;
	th = new QThread();

	timer = new QTimer();
	timer->setInterval(THREAD_T);
	timer->moveToThread(th);

	connect(timer, SIGNAL(timeout()),
					this, SLOT(main()),
					Qt::DirectConnection);

	th->start();

	QMetaObject::invokeMethod(timer, "start");
	qInfo() << "Start main thread";
}

void MainWindow::on_actOpenCamParamCont_triggered()
{
	camParamControl->show();
	return;
}

void MainWindow::on_actionCameraParamAuto_triggered()
{
	isCamParamAuto = true;
	return;
}
