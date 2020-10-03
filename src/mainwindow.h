#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QString>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QDockWidget>
#include <QList>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QProgressBar>

#include "ui/Control/control.h"
#include "ui/CameraParams/cameraparameter.h"
#include "ui/ProcessingProgress/processingprogress.h"
#include "ui/ImageViewer/imageviewer.h"
#include "R200/r200.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void start();

private slots:
	void main();

private:
	void measure(Frames_t &frames);
	void save();
	void calc();
	void draw(Frames_t &frames);
	void closeEvent(QCloseEvent *event);

signals:
	void updateFrames(Frames_t *frames);
	void progressMeasurement(int count);
	void finishedMeasurement();

private:
	Ui::MainWindow *ui;
	Control *control;
	Cameraparameter *camparam;
	ProcessingProgress *procProg;
	ImageViewer *imgvwr;
	QProgressBar *bar;

	QThread *th;
	QTimer *timer;
	bool isThread;
	QMutex *mtx;

	R200 *r200;

	QString path;
	int mode;

	//config用パラメータ
	int count_n;	//計測最大数
	int div_n;		//平面分割数
	float h_max;	//ヒストグラムの最大値[m]
	float h_b;			//ヒストグラムのビン幅[m]


	QList<double> grid_depth_averages_t; //時刻tにおける各格子の平均depth値
	QList<QList<double>> grid_depth_averages_T; //時刻0～Tにおける平均depth値の集合
	QList<cv::Mat> Histgrams; //ヒストグラムの実態,OpenCV型
};
Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(Frames_t)
#endif // MAINWINDOW_H
