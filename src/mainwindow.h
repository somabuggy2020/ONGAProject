#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QDockWidget>

#include "ui/Control/control.h"
#include "ui/CameraParams/cameraparameter.h"

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
	Ui::MainWindow *ui;
	Control *control;
	Cameraparameter *camparam;

	QThread *th;
	QTimer *timer;
};
#endif // MAINWINDOW_H
