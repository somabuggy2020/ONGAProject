#include "mainwindow.h"
#include "src/ui_mainwindow.h"

#define THREAD_T 33
#define NUM_MEASURE 30

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






	/***
	 * GUIウィジェットの追加
	 ***/

	//CameraParmeter UI
	camparam = new Cameraparameter(this);
	QDockWidget *dwCamParam = new QDockWidget();
	dwCamParam->setWidget(camparam);
	this->addDockWidget(Qt::RightDockWidgetArea, dwCamParam);

	//Control UI
	control = new Control(this);
	QDockWidget *dwControl = new QDockWidget();
	dwControl->setWidget(control);
	this->addDockWidget(Qt::RightDockWidgetArea, dwControl);

	//ProcessingProgress UI
	procProg = new ProcessingProgress("", this);
	QDockWidget *dwProcProg = new QDockWidget();
	dwProcProg->setWidget(procProg);
	this->addDockWidget(Qt::LeftDockWidgetArea, dwProcProg);


	/***
	 * signals-slots
	 ***/
	connect(control, &Control::setdivnum, this,
					[=](int divnum){
		this->divnum = divnum;
	}, Qt::QueuedConnection);

	connect(procProg, &ProcessingProgress::start, this,
					[=](){
		mode = Mode::Measure;
	}, Qt::QueuedConnection);


	/***
 *
 ***/

	r200 = new R200();
	if(r200->init() == -1){
		qWarning() << "Check Realsense Device Connection";
	}

	mode = Mode::Wait;

	start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

/*!
 * \brief MainWindow::main
 */
void MainWindow::main()
{
	qDebug() << "Do work ...";
	QMetaObject::invokeMethod(timer, "stop");

	int ret = 0;
	CamParams_t camparams;
	Frames_t frames;

	//カメラパラメータの取得
	camparam->get(camparams);

	//カメラパラメータのセット
	r200->setParams(camparams);

	//フレームデータの更新，新規フレームを取得
	ret = r200->getFrames(frames);
	if(ret == -1)	qCritical() << "Check Realsense Device Connection";

	//処理
	//モード変数modeにしたがって処理内容を切替
	qInfo() << "Mode:" << Mode::str[mode];

	switch (mode) {
		case Mode::Wait:
			break;
		case Mode::Measure:
			measure(frames); //update grid_depth_averages
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
	draw(frames);

	//画像用メモリ空間はは必ずリリース
	frames.imgRGB->release();
	frames.imgDepth->release();
	frames.imgAlignedRGB->release();
	frames.imgAlignedDepth->release();

	QMetaObject::invokeMethod(timer, "start");
	return;
}

/*!
 * \brief MainWindow::measure
 * \param frames
 */
void MainWindow::measure(Frames_t &frames)
{
	//(0)Aligned Depth画像を複製
	cv::Mat imgDepth = frames.imgAlignedDepth->clone();

	//(1)Depth画像格子分割しながら，各格子における平均depth値を格納
	//格納先はローカルメンバ変数
	grid_depth_averages_t.clear();
	for(int j = 0; j < divnum; j++){
		for(int i = 0; i < divnum; i++){
			//切り出し範囲計算
			cv::Rect rect = cv::Rect(imgDepth.cols/divnum*i,
															 imgDepth.rows/divnum*j,
															 imgDepth.cols/divnum,
															 imgDepth.rows/divnum);

			//切り出し処理
			cv::Mat _out(imgDepth, rect);
			cv::Mat out;
			_out.convertTo(out, CV_32FC1, frames.scale); //16bit value to float[m]

			//depth値の平均値を出す
			//cv::Scalarで受け取るけど，実質チャンネルは1なので値は一つしかないはず
			cv::Scalar _ave = cv::mean(out);
			double ave = _ave.val[0]; //多分1チャンネル目の平均値
			grid_depth_averages_t << ave;		//配列に格納
		}
	}

	grid_depth_averages_T << grid_depth_averages_t;

	//計測回数がNUM_MEASUREに達したら計測終了
	if(grid_depth_averages_T.length() >= (int)NUM_MEASURE){
		mode = Mode::Save; //モード遷移
	}

	return;
}

/*!
 * \brief MainWindow::save
 */
void MainWindow::save()
{
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
	for(int i = 0; i < divnum*divnum; i++){ //i番目のgrid

		float *averages = new float[grid_depth_averages_T.length()];
		for(int t = 0; t < grid_depth_averages_T.length(); t++){ //時刻tのi番目のグリッド
			float val = (float)grid_depth_averages_T[t][i];
			averages[t] = val;
		}

		//ヒストグラム作成のためのMat型作成
		cv::Mat m1 = cv::Mat(1, grid_depth_averages_T.length(), CV_32FC1, averages);

		cv::Mat hist;
		float range[] = {0.0, 5.0};
		const float *hrange = {range};
		float b = 0.1; //ヒストグラムのビン幅[m]
		int histSize = (int)(range[1]/b); //ヒストグラムのビン数(整数でよろしく)
		cv::calcHist(&m1, 1, 0, cv::Mat(), hist, 1, &histSize, &hrange, true, false);

		//集合に追加
		Histgrams << hist;
	}

	qInfo() << "H" << Histgrams.count();
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
	for(int i = 0; i <= divnum; i++){
		//緯線
		cv::line(*frames.imgAlignedRGB,
						 cv::Point(frames.imgAlignedRGB->cols/divnum*i, 0), //上座標
						 cv::Point(frames.imgAlignedRGB->cols/divnum*i, frames.imgAlignedRGB->rows),
						 cv::Scalar(0,0,255), //Red
						 2);
		//罫線
		cv::line(*frames.imgAlignedRGB,
						 cv::Point(0, frames.imgAlignedRGB->rows/divnum*i),
						 cv::Point(frames.imgAlignedRGB->cols, frames.imgAlignedRGB->rows/divnum*i),
						 cv::Scalar(0,0,255),
						 2);
	}
}

/*!
 * \brief MainWindow::start
 */
void MainWindow::start()
{
	//メイン処理スレッドインスタンス作成
	th = new QThread();

	//	vision->moveToThread(thMainProc);

	//タイマー作成
	timer = new QTimer();
	timer->setInterval(THREAD_T);	//処理周期T[mse]の設定
	timer->moveToThread(th);			//Timerを丸ごとスレッドに譲渡

	//timerのtimeoutシグナルとスロット関数の接続
	//Qt::DirectConnectionオプションによって,timerが所属するスレッド上でスロット関数が処理されるはず,,,
	connect(timer, SIGNAL(timeout()),this,
					SLOT(main()),Qt::DirectConnection);

	//スレッド起動,イベントループ開始
	th->start();
	QMetaObject::invokeMethod(timer, "start");	//タイマースタート
	qInfo() << "Start main thread";
	//これで、周期T[msec]でスロット関数が実行される
}
