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
	//"MAIN" group values
	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("MAIN");
	count_max = cfg.value("CountN", 100).toInt();
	div_n = cfg.value("DivN", 7).toInt();
	h_max = cfg.value("HistMax").toFloat();
	h_b = cfg.value("HistBin").toFloat();
	cfg.endGroup();

	qRegisterMetaType<cv::Mat>();
	qRegisterMetaType<Frames_t>();

	setup();
	setup_signals_slots();

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
	camParamControl = new CameraParamControl(this);
	camParamControl->setWindowFlag(Qt::Window);
	camParamControl->hide();

	//Control Panel UI
	controlPanel = new ControlPanel(this);
	ui->dwControlPanel->setWidget(controlPanel);

	//Image UI
	imgvwrRGB = new ImageViewer("RGB", this);
	imgvwrRGB->initialize(CV_8UC3, QImage::Format::Format_BGR888);
	ui->dwImgRGB->setWidget(imgvwrRGB);

	imgvwrAlignedRGB = new ImageViewer("Aligned RGB", this);
	imgvwrAlignedRGB->initialize(CV_8UC3, QImage::Format::Format_BGR888);
	ui->dwImgAlignedRGB->setWidget(imgvwrAlignedRGB);

	imgvwrAlignedDepth = new ImageViewer("Aligned Depth", this);
	imgvwrAlignedDepth->initialize(CV_16UC1, QImage::Format::Format_Grayscale8);
	ui->dwImgDepth->setWidget(imgvwrAlignedDepth);

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

	connect(camParamControl, &CameraParamControl::On_CameraParams_changed, this,
					[=](CamParams_t &camparams){
		this->camparams = camparams;
		isCamParamChanged = true;
	});

	connect(controlPanel, &ControlPanel::On_start_clicked, this,
					[=](){
		mode = Mode::Measure;
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
		h_max = arg;
	});

	connect(controlPanel, &ControlPanel::On_HistMax_changed, this,
					[=](double arg){
		h_max = arg;
	});

	connect(controlPanel, &ControlPanel::On_HistBin_changed, this,
					[=](double arg){
		h_b = arg;
	});

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

	QMetaObject::invokeMethod(timer, "start");
	return;
}

/*!
 * \brief MainWindow::measure
 * \param frames
 */
void MainWindow::measure(Frames_t &frames)
{
	//Depth画像を複製
	cv::Mat imgDepth = frames.imgDepth.clone();

	//Depth画像格子分割しながら，各格子における平均depth値を格納
	//格納先はローカルメンバ変数
	QList<double> grid_depth_averages_t;

	for(int j = 0; j < div_n; j++){
		for(int i = 0; i < div_n; i++){
			//切り出し範囲計算
			cv::Rect rect = cv::Rect(imgDepth.cols/div_n*i,
															 imgDepth.rows/div_n*j,
															 imgDepth.cols/div_n,
															 imgDepth.rows/div_n);

			//切り出し処理
			cv::Mat _out(imgDepth, rect); //また16UC1
			cv::Mat out;
			_out.convertTo(out, CV_32FC1, frames.scale); //16bit value to 32bit-float[m]

			//マスク作成
			cv::Mat mask;
			cv::threshold(out, mask, 0.01, 1, cv::THRESH_BINARY);
			mask.convertTo(mask, CV_8UC1);

			cv::Scalar _ave = cv::mean(out, mask);
			double ave = _ave.val[0];			//多分1チャンネル目の平均値
			grid_depth_averages_t << ave;	//配列に格納
		}
	}

	grid_depth_averages_T << grid_depth_averages_t;

	if(grid_depth_averages_T.length() >= count_max){
		mode = Mode::Calc;
		emit finishedMeasurement();
	}

	emit progressMeasurement(grid_depth_averages_T.count());
	return;
}

/*!
 * \brief MainWindow::calc
 */
void MainWindow::calc(Frames_t &frames)
{
	matHistgram.clear();

	for(int i = 0; i < div_n*div_n; i++){
		float *averages = new float[grid_depth_averages_T.length()];
		for(int t = 0; t < grid_depth_averages_T.length(); t++){ //時刻tのi番目のグリッド
			float val = (float)grid_depth_averages_T[t][i];
			averages[t] = val;
		}

		cv::Mat m1 = cv::Mat(1, grid_depth_averages_T.length(), CV_32FC1, averages);

		cv::Mat hist;
		float range[] = {0.0, h_max};
		const float *hrange = {range};
		int histSize = (int)(range[1]/h_b);

		//calculate histgram by OpenCV
		cv::calcHist(&m1, 1, 0, cv::Mat(), hist, 1, &histSize, &hrange, true, false);

		matHistgram << hist;
	}

	maximums.clear();
	for(int i = 0; i < matHistgram.count(); i++){
		double min, max;
		cv::Point minLoc, maxLoc;
		min=max=-1;
		//search value of maximum degree position
		cv::minMaxLoc(matHistgram[i], &min, &max, &minLoc, &maxLoc);
		maximums << maxLoc.y*(h_b)/2.0; //get value [m]
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
	//格子の描画
	for(int i = 0; i <= div_n; i++){
		//緯線
		cv::line(frames.imgAlignedRGB,
						 cv::Point(frames.imgAlignedRGB.cols/div_n*i, 0),
						 cv::Point(frames.imgAlignedRGB.cols/div_n*i, frames.imgAlignedRGB.rows),
						 cv::Scalar(0,0,255), //Red
						 2);
		//		//罫線
		cv::line(frames.imgAlignedRGB,
						 cv::Point(0, frames.imgAlignedRGB.rows/div_n*i),
						 cv::Point(frames.imgAlignedRGB.cols, frames.imgAlignedRGB.rows/div_n*i),
						 cv::Scalar(0,0,255), //Red
						 2);
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	isThread = false;
	th->exit();
	r200->close();

	//設定値書き込み
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
