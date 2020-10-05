#include "mainwindow.h"
#include "src/ui_mainwindow.h"

#define THREAD_T 33

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	//保存ディレクトリ作成
	QDir dir;
	dir.mkdir("Log");
	dir.mkdir("Log/DepthData");
	path = QString("./Log/DepthData/");

	//configファイル読み込み
	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("MAIN");
	count_n = cfg.value("CountN", 100).toInt();
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
	 * GUIウィジェットの追加
	 ***/

	//CameraParmeter UI
	camparam = new Cameraparameter(this);
	camparam->setWindowFlag(Qt::Window);
	camparam->hide();

	//Control Panel UI
	controlPanel = new ControlPanel(this);
	ui->dwControlPanel->setWidget(controlPanel);

	//Control UI
	//	control = new Control(this);
	//	QDockWidget *dwControl = new QDockWidget();
	//	dwControl->setWidget(control);
	//	this->addDockWidget(Qt::RightDockWidgetArea, dwControl);

	//ProcessingProgress UI
	//	procProg = new ProcessingProgress("", this);
	//	QDockWidget *dwProcProg = new QDockWidget();
	//	dwProcProg->setWidget(procProg);
	//	this->addDockWidget(Qt::RightDockWidgetArea, dwProcProg);

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
	connect(controlPanel, &ControlPanel::On_start_clicked, this,
					[=](){
		mode = Mode::Measure;
		emit startMeasurement();
	});

	connect(controlPanel, &ControlPanel::On_clear_clicked, this,
					[=](){
		grid_depth_averages_T.clear();
	});

	connect(controlPanel, &ControlPanel::On_DivN_changed, this,
					[=](int arg){
		div_n = arg;
	});

	connect(controlPanel, &ControlPanel::On_CountMax_changed, this,
					[=](int arg){
		count_n = arg;
	});

	connect(this, &MainWindow::updateTime, this,
					[=](){
		QDateTime ct = QDateTime::currentDateTime();
		unsigned long dt = t.msecsTo(ct);
		QString str = ct.toString("hh:mm:ss.zzz");
		str += QString(" / %1 Hz").arg(QString::number(1/(dt/1000.0),'f',1));
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
		bar->setRange(0, count_n);
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
}

/*!
 * \brief MainWindow::main
 */
void MainWindow::main()
{
	qDebug() << "Do work ...";
	QMetaObject::invokeMethod(timer, "stop");
	emit updateTime();

	int ret = 0;
	CamParams_t camparams;

	//カメラパラメータの取得
	//	camparam->get(camparams);

	//カメラパラメータのセット
	//	r200->setParams(camparams);

	//フレームデータの更新，新規フレームを取得
	ret = r200->getFrames(*frames);
	if(ret == -1){
		qCritical() << "Check Realsense Device Connection";
		exit(255);
	}

	//処理, モード変数modeにしたがって処理内容を切替
	qDebug() << "Mode:" << Mode::str[mode];

	switch (mode) {
		case Mode::Wait:
			break;
		case Mode::Measure:
			measure(*frames);
			break;
		case Mode::Save:
			save();
			break;
		case Mode::Calc:
			calc();
			break;
		default:
			break;
	}

	//描画処理
	draw(*frames);
	emit updateFrames(frames);

	//ループ終了,タイマを再スタートしない感じで
	if(!isThread){
		return;
	}

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

			//depth値の平均値を出す
			//cv::Scalar(4次元配列)で受け取るけど，実質チャンネルは1なので値は一つしかないはず
			//depth値の取れていないピクセルに関しては計算に含めないようにしないと平均値の誤差につながる
			//なのでmaskをかけてある maskにおける値が1のところだけ計算している
			cv::Scalar _ave = cv::mean(out, mask);
			double ave = _ave.val[0];			//多分1チャンネル目の平均値
			grid_depth_averages_t << ave;	//配列に格納
		}
	}

	grid_depth_averages_T << grid_depth_averages_t;

	//計測回数がNUM_MEASUREに達したら計測終了,Saveモードに移行する
	if(grid_depth_averages_T.length() >= count_n){
		mode = Mode::Save; //モード遷移
		//		mode = Mode::Wait;
		emit finishedMeasurement();
	}

	emit progressMeasurement(grid_depth_averages_T.count());
	return;
}

/*!
 * \brief MainWindow::save
 */
void MainWindow::save()
{
	//	qDebug() << grid_depth_averages_T;

	//現在時刻取得
	QDateTime currentTime = QDateTime::currentDateTime();

	//csvファイル名作成
	QString fname = currentTime.toString("yyyy-MM-dd-hh-mm-ss");
	fname += QString(".csv");

	//csvファイル作成
	QFile f;
	f.setFileName(path + "/" + fname);
	if(!f.open(QFile::WriteOnly | QFile::Text)){
		qCritical() << f.errorString();
		mode = Mode::Wait;
		return;
	}

	//クローズ
	f.close();

	//保存終了したのでCalcモードへ遷移
	mode = Mode::Calc;
	return;
}

/*!
 * \brief MainWindow::calc
 * 各グリッドごとのdepth値のヒストグラムを作成する
 */
void MainWindow::calc()
{
	qInfo() << "Calculate histgram";

	Histgrams.clear();

	//頑張ってcv::Mat型にdepth平均値の時系列データを挿入する
	for(int i = 0; i < div_n*div_n; i++){ //i番目のgrid

		float *averages = new float[grid_depth_averages_T.length()];
		for(int t = 0; t < grid_depth_averages_T.length(); t++){ //時刻tのi番目のグリッド
			float val = (float)grid_depth_averages_T[t][i];
			averages[t] = val;
		}

		//ヒストグラム作成のためのMat型作成
		cv::Mat m1 = cv::Mat(1, grid_depth_averages_T.length(), CV_32FC1, averages);

		cv::Mat hist;
		float range[] = {0.0, h_max}; //ヒストグラムのレンジ設定
		const float *hrange = {range};
		int histSize = (int)(range[1]/h_b); //ヒストグラムのビン数(整数でよろしく)

		//ヒストグラム計算(OpenCV任せ)
		cv::calcHist(&m1, 1, 0, cv::Mat(), hist, 1, &histSize, &hrange, true, false);

		//集合に追加
		Histgrams << hist;
	}

	//モード遷移,待機状態
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
	cfg.setValue("CountN", QVariant::fromValue(count_n));
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
	//メイン処理スレッドインスタンス作成
	isThread = true;
	th = new QThread();

	//	vision->moveToThread(thMainProc);

	//タイマー作成
	timer = new QTimer();
	timer->setInterval(THREAD_T);	//処理周期T[mse]の設定
	timer->moveToThread(th);			//Timerを丸ごとスレッドに譲渡

	//timerのtimeoutシグナルとスロット関数の接続
	//Qt::DirectConnectionオプションによって,timerが所属するスレッド上でスロット関数が処理されるはず,,,
	connect(timer, SIGNAL(timeout()),
					this, SLOT(main()),
					Qt::DirectConnection);

	//スレッド起動,イベントループ開始
	th->start();

	QMetaObject::invokeMethod(timer, "start");	//タイマースタート
	qInfo() << "Start main thread";
	//これで、周期T[msec]でスロット関数が実行される
}

void MainWindow::on_actOpenCamParamCont_triggered()
{
	camparam->show();
}
