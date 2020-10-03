#ifndef PROCESSINGPROGRESS_H
#define PROCESSINGPROGRESS_H

#include <QWidget>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui {
	class ProcessingProgress;
}
QT_END_NAMESPACE

class ProcessingProgress : public QWidget
{
	Q_OBJECT

public:
	explicit ProcessingProgress(QString windowTitle, QWidget *parent = 0);
	~ProcessingProgress();

private:
	Ui::ProcessingProgress *ui;

signals:
	void start();
	void sendStop();
	void dataclear();//ui表示のデータ，バッファデータを削除用のシグナル

public slots:
	void showData(QString showData);
	void showDicision(int i);

private slots:
	void on_btnStart_clicked();
	void on_stopButton_clicked();
	void on_clearButton_clicked();
	//	void on_startButton_clicked();
	void on_calcButton_clicked();
};

#endif // PROCESSINGPROGRESS_H
