#include "mainwindow.h"
#include "src/ui_mainwindow.h"

#define THREAD_T 1

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
	}


	mode = Mode::Wait;
	t = QDateTime::currentDateTime();
	camparams = camParamControl->camparams;
	isCamParamChanged = false;
	frames = new Frames_t();
	imgResAves = cv::Mat();


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

	imgvwrHistgrams = new ImageViewer("Histgrams", this);
	imgvwrHistgrams->initialize(CV_8UC3, QImage::Format_BGR888);
	imgvwrHistgrams->setWindowFlag(Qt::WindowType::Window);
	imgvwrHistgrams->hide();

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
		mode = Mode::Measure;
		imgvwrHistgrams->hide();
		emit startMeasurement();
	});

	connect(controlPanel, &ControlPanel::On_clear_clicked, this,
					[=](){
		grid_depth_averages_T.clear();
	});

	connect(controlPanel, &ControlPanel::On_CountMax_changed, this,
					[=](int arg){
		count_max = arg;
	});

	connect(controlPanel, &ControlPanel::On_DivN_changed, this,
					[=](int arg){
		div_n = arg;
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
		t = ct;
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
		bar = new QProgressBar(this);
		ui->statusbar->addWidget(bar);
		bar->setRange(0, count_max);
	});

	connect(this, &MainWindow::progressMeasurement, this,
					[=](int count){
		bar->setValue(count);
	});

	connect(this, &MainWindow::finishedMeasurement, this,
					[=](){
		ui->statusbar->removeWidget(bar);
		bar->deleteLater();
	});

	connect(this, &MainWindow::finishedCalculate, this,
					[=](){

		QLayoutItem *child;
		while((child = ui->gl->takeAt(0)) != nullptr){
			delete child->widget();
			delete child;
		}
		for(int i = 0; i < maximums.count(); i++){
			int x = i%div_n;
			int y = i/div_n;
			QLabel *lbl = new QLabel();
			lbl->setText(QString::number(maximums[i],'f',3));
			lbl->setAlignment(Qt::AlignHCenter);
			lbl->setAlignment(Qt::AlignVCenter);
			lbl->setFrameShape(QFrame::Shape::WinPanel);
			ui->gl->addWidget(lbl, y, x);
		}


		imgvwrHistgrams->setImage(imgTotalHistgram);
		imgvwrHistgrams->show();
	});






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

	if(r200->getFrames(*frames) == -1){
		qCritical() << "Check Realsense Device Connection";
		exit(255);
	}

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
	cv::Mat _Depth = frames.imgDepth.clone();
	QList<double> grid_depth_averages_t;

	for(int j = 0; j < div_n; j++){
		for(int i = 0; i < div_n; i++){
			//set clipping area as rectangle
			cv::Rect rect = cv::Rect(_Depth.cols/div_n*i, //origin x
															 _Depth.rows/div_n*j, //origin y
															 _Depth.cols/div_n,   //width
															 _Depth.rows/div_n);  //height

			//carry-out clipping
			cv::Mat _out(_Depth, rect);
			//calculate 16bit depth value to 32bit float meter value
			cv::Mat out;
			_out.convertTo(out, CV_32FC1, frames.scale);


			//make mask for error value
			cv::Mat mask_low, mask_high;
			cv::Mat mask;
			cv::threshold(out, mask_low, 0.05, 255, cv::THRESH_BINARY);
			cv::threshold(out, mask_high, 10.0, 255, cv::THRESH_BINARY_INV);

			cv::bitwise_and(mask_low, mask_high, mask);
			mask.convertTo(mask, CV_8UC1);

			//calculate mean value in clipped depth values
			cv::Scalar _ave = cv::mean(out, mask);
			double ave = _ave.val[0];
			grid_depth_averages_t << ave; //append to array
		}
	}

	grid_depth_averages_T << grid_depth_averages_t;



	//if the number of current total measured frame reached "count_max"
	//mode will be Mode::Calc
	if(grid_depth_averages_T.length() >= count_max){
		mode = Mode::Calc;
		emit finishedMeasurement();
	}

	//else
	//continue the measure mode
	//and update progressbar
	emit progressMeasurement(grid_depth_averages_T.count());
	return;
}

/*!
 * \brief MainWindow::calc
 */
void MainWindow::calc(Frames_t &frames)
{
	matHistgrams.clear();

	for(int i = 0; i < div_n*div_n; i++){

		float *averages = new float[grid_depth_averages_T.length()];

		for(int t = 0; t < grid_depth_averages_T.length(); t++){
			float val = (float)grid_depth_averages_T[t][i];
			averages[t] = val;
		}

		cv::Mat m1 = cv::Mat(1, grid_depth_averages_T.length(), CV_32FC1, averages);

		//histgram settings
		cv::Mat hist;
		float range[] = {0.0, h_max};
		const float *hrange = {range};
		int histSize = (int)(h_max/h_b);

		//calculate histgram by OpenCV
		cv::calcHist(&m1, 1, 0, cv::Mat(), hist, 1, &histSize, &hrange, true, false);

		matHistgrams << hist;
	}

	maximums.clear();
	for(int i = 0; i < matHistgrams.count(); i++){
		double min, max;
		cv::Point minLoc, maxLoc;
		min = max = -1;
		//search value of maximum degree position
		cv::minMaxLoc(matHistgrams[i], &min, &max, &minLoc, &maxLoc);
		maximums << maxLoc.y*(h_b); //get value [m]
	}


	//Make histgram images for debug
	imgHistgrams.clear();
	int hist_w = cvRound(h_max/h_b)*3;
	int hist_h = cvRound(h_max/h_b)*3;
	int bin_w = cvRound((double)hist_w/(h_max/h_b));

	cv::Size sz(hist_w, hist_h); //image size

	for(int i = 0; i < matHistgrams.count(); i++){
		imgHistgrams.append(cv::Mat(sz, CV_8UC3, cv::Scalar::all(0)));

		cv::normalize(matHistgrams[i],
									matHistgrams[i],
									0,
									sz.height,
									cv::NORM_MINMAX,
									-1,
									cv::Mat());

		for(int j = 0; j < (int)(h_max/h_b); j++){
			cv::line(imgHistgrams.back(),
							 cv::Point(bin_w*j, hist_h-1),
							 cv::Point(bin_w*j, hist_h - cvRound(matHistgrams[i].at<float>(j))),
							 cv::Scalar::all(255),
							 2, 8, 0);
		}
	}

	cv::Mat vline(hist_h,3,CV_8UC3,cv::Scalar(0,0,255));

	for(int y = 0; y < div_n; y++){
		cv::Mat matHorizontal(hist_h,3,CV_8UC3,cv::Scalar(0,0,255));
		for(int x = 0; x < div_n; x++){
			int idx = x + y*div_n;
			cv::hconcat(matHorizontal, imgHistgrams[idx], matHorizontal);
			cv::hconcat(matHorizontal, vline, matHorizontal);
		}

		if(imgTotalHistgram.empty()) imgTotalHistgram = matHorizontal.clone();
		else cv::vconcat(imgTotalHistgram, matHorizontal, imgTotalHistgram);

		cv::vconcat(imgTotalHistgram,
								cv::Mat(3, matHorizontal.cols, CV_8UC3, cv::Scalar(0,0,255)),
								imgTotalHistgram);
	}

//	cv::imshow("test", imgTotalHistgram);
//	cv::waitKey(1);

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
		header += QString("%1,").arg(i);
	}
	header += QString("\n");
	f.write(header.toStdString().c_str());

	for(int i = 0; i < grid_depth_averages_T.count(); i++){
		QString line;
		line += QString("%1,").arg(i);

		QList<double> depth_t = grid_depth_averages_T[i];
		Q_FOREACH(double depth, depth_t){
			line += QString("%1,").arg(depth);
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

	for(int i = 0; i < maximums.count(); i++){
		QString line;
		line += QString("%1,%2\n").arg(i).arg(maximums[i]);
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
						 2);
		cv::line(frames.imgAlignedRGB,
						 cv::Point(0, frames.imgAlignedRGB.rows/div_n*i),
						 cv::Point(frames.imgAlignedRGB.cols, frames.imgAlignedRGB.rows/div_n*i),
						 cv::Scalar(0,0,255), //Red
						 2);
	}


	//Make image of Histgram
	//	if(matHistgrams.empty()) return;

	//	cv::Size sz; //Histgram image size
	//	sz.width = 512;
	//	sz.height = 400;
	//	int bin_w = cvRound((double)sz.width/(h_max/h_b));
	//	cv::Mat img(sz, CV_8UC3, cv::Scalar::all(0));

	//	cv::normalize(matHistgrams[0], matHistgrams[0], 0, img.rows, cv::NORM_MINMAX, -1, cv::Mat());

	//	for(int i = 1; i < (int)(h_max/h_b); i++){
	//		line(img,
	//				 cv::Point(bin_w*(i-1), sz.height - cvRound(matHistgrams[0].at<float>(i-1))),
	//				cv::Point(bin_w*(i), sz.height - cvRound(matHistgrams[0].at<float>(i))),
	//				cv::Scalar(255, 0, 0),
	//				2, 8, 0
	//				);
	//	}

	//	cv::imshow("hist", img);
	//	cv::waitKey(-1);

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
}
