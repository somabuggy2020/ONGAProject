#include "mainwindow.h"
#include "src/ui_mainwindow.h"

#define THREAD_T 33

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);


	control = new Control();
	QDockWidget *dwControl = new QDockWidget();
	dwControl->setWidget(control);
	this->addDockWidget(Qt::RightDockWidgetArea, dwControl);

	camparam = new Cameraparameter();
	QDockWidget *dwCamParam = new QDockWidget();
	dwCamParam->setWidget(camparam);
	this->addDockWidget(Qt::RightDockWidgetArea, dwCamParam);

	start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::main()
{
	qDebug() << "do work ...";
	QMetaObject::invokeMethod(timer, "stop");

	QMetaObject::invokeMethod(timer, "start");
	return;
}

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


