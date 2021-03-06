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
#include <QMessageBox>
#include <QList>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QLabel>
#include <QProgressBar>
#include <QProgressDialog>

#include "common.h"
#include "../../src/ui/CameraParamControl/cameraparamcontrol.h"
#include "../../src/ui/ControlPanel/controlpanel.h"
#include "../../src/ui/ImageViewer/imageviewer.h"
#include "histogramviewer.h"
#include "../../src/R200/r200.h"
#include "../../src/D400s/d400s.h"

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
	void on_actionCameraParamAuto_triggered();

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

	void startCalculate();
	void finishedCalculate();

private:
	Ui::MainWindow *ui;
	CameraParameterControlPanel *camParamControl;
	ControlPanel *controlPanel;
	ImageViewer *imgvwrRGB;
	ImageViewer *imgvwrAlignedRGB;
	ImageViewer *imgvwrAlignedDepth;
	ImageViewer *imgvwrHistograms;
	HistogramViewer *histogramvwr;
	QLabel *lblStatus;
	QProgressDialog *progdialog;

	QThread *th;
	QTimer *timer;
	bool isThread;

	QString DepthDataPath;
	QString DataPath;

	R200 *r200;
	D400s *d455;

	//Measurement parameter
	int frame_counter;
	int count_max;
	int div_n;
	QList<QList<cv::Mat>> I_div_T;
	QList<cv::Mat> set_mean_T;

	float h_max;
	float h_b;
	QList<QList<float>> histogram_values;
	QList<double> class_values;

	int mode;			//{Mode:Wait, Mode::Measure,,,}
	bool isCanceled;
	QDateTime t;	//timestamp
	CamParams_t camparams;
	bool isCamParamChanged;
	bool isCamParamAuto;
	Frames_t *frames;

	//	cv::Mat imgHistograms;
};

Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(Frames_t)
#endif // MAINWINDOW_H
