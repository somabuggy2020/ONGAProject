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
#include <QLabel>
#include <QProgressBar>

#include "common.h"
#include "../../src/ui/CameraParamControl/cameraparamcontrol.h"
#include "../../src/ui/ControlPanel/controlpanel.h"
#include "../../src/ui/ImageViewer/imageviewer.h"
#include "../../src/R200/r200.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void setup();
	void setup_signals_slots();
	void start();

private slots:
	void main();

	void on_actOpenCamParamCont_triggered();

private:
	void measure(Frames_t &frames);
	void save(Frames_t &frames);
	void calc(Frames_t &frames);
	void draw(Frames_t &frames);
	void closeEvent(QCloseEvent *event);

signals:
	void updateTime();
	void updateFrames(Frames_t *frames);

	void startMeasurement();
	void progressMeasurement(int count);
	void finishedMeasurement();

	void finishedCalculate();

private:
	Ui::MainWindow *ui;
	CameraParamControl *camParamControl;
	ControlPanel *controlPanel;
	ImageViewer *imgvwrRGB;
	ImageViewer *imgvwrAlignedRGB;
	ImageViewer *imgvwrAlignedDepth;
	QLabel *lblStatus;
	QProgressBar *bar;

	QThread *th;
	QTimer *timer;
	bool isThread;

	QString DepthDataPath;
	QString DataPath;

	R200 *r200;

	int count_max;
	int div_n;
	float h_max;
	float h_b;

	int mode;
	QDateTime t;
	CamParams_t camparams;
	bool isCamParamChanged;
	Frames_t *frames;
	QList<QList<double>> grid_depth_averages_T;
	cv::Mat imgResAves;
	QList<cv::Mat> matHistgram;
	QList<double> maximums;
};

Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(Frames_t)
#endif // MAINWINDOW_H
