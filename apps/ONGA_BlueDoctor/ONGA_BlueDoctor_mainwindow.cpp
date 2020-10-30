#include "ONGA_BlueDoctor_mainwindow.h"
#include "ui_ONGA_BlueDoctor_mainwindow.h"

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
	//"MAIN" group values
	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("MAIN");
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

	//
	hsvRangeControl = new HSVRangeControlPanel(this);
	ui->dwHSVRangeControlPanel->setWidget(hsvRangeControl);

	//CameraParmeter UI
	camParamControl = new CameraParameterControlPanel(this);
	camParamControl->setWindowFlag(Qt::Window);
	camParamControl->hide();

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

	connect(hsvRangeControl, &HSVRangeControlPanel::On_HSVRange_changed, this,
					[=](HSVRange_t hsvRange){
		this->hsvRange = hsvRange;
	});

	connect(camParamControl, &CameraParameterControlPanel::On_CameraParams_changed, this,
					[=](CamParams_t &camparams){
		this->camparams = camparams;
		isCamParamChanged = true;
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
		//		exit(255);
	}

	tracking(*frames);
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
	cv::Mat imgDepth = frames.imgDepth.clone();
	//	emit progressMeasurement(grid_depth_averages_T.count());
	return;
}

/*!
 * \brief MainWindow::calc
 */
void MainWindow::calc(Frames_t &frames)
{
	emit finishedCalculate();
	mode = Mode::Save;
	return;
}

/*!
 * \brief MainWindow::save
 */
void MainWindow::save(Frames_t &frames)
{

	mode = Mode::Wait;
	return;
}

/*!
 * \brief MainWindow::tracking
 * \param frames
 */
void MainWindow::tracking(Frames_t &frames)
{
	return;
}

/*!
 * \brief MainWindow::draw
 * \param frames
 */
void MainWindow::draw(Frames_t &frames)
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	isThread = false;
	th->exit();
	r200->close();

	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("MAIN");
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
