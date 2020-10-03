#include "mainwindow.h"
#include "src/ui_mainwindow.h"

#define THREAD_T 33

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	/***
	 * GUIウィジェットの追加
	 ***/
	//Control UI
	control = new Control();
	QDockWidget *dwControl = new QDockWidget();
	dwControl->setWidget(control);
	this->addDockWidget(Qt::RightDockWidgetArea, dwControl);
	//CameraParmeter UI
	camparam = new Cameraparameter();
	QDockWidget *dwCamParam = new QDockWidget();
	dwCamParam->setWidget(camparam);
	this->addDockWidget(Qt::RightDockWidgetArea, dwCamParam);



	r200 = new R200();
	if(r200->init() == -1){
		qWarning() << "Check Realsense Device Connection";
	}

	start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::main()
{
	qDebug() << "Do work ...";
	QMetaObject::invokeMethod(timer, "stop");

	int ret = 0;
	RGBD::Data_t frames;
	ret = r200->getFrames(frames);
	if(ret == -1){
		qCritical() << "Check Realsense Device Connection";
	}







	QMetaObject::invokeMethod(timer, "start");
	return;
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
