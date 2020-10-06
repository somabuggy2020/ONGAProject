#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QtGlobal>

int main(int argc, char *argv[])
{
	qSetMessagePattern("[%{function}](%{line}) %{message}");

	QApplication a(argc, argv);
	MainWindow w;
	w.setWindowState(Qt::WindowState::WindowMaximized);

//	QDesktopWidget dw;
//	QRect mainScreen = dw.availableGeometry(dw.primaryScreen());
//	w.resize(mainScreen.width()*0.5,mainScreen.height());
//	w.move(mainScreen.width()*0.5,0);
	w.show();

	return a.exec();
}
