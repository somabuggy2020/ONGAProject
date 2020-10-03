#ifndef CONTROL_H
#define CONTROL_H

#include <QWidget>
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QDateTime>
#include <QSettings>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui {
	class Control;
}
QT_END_NAMESPACE

class Control : public QWidget
{
	Q_OBJECT

public:
	explicit Control(QWidget *parent = 0);
	~Control();

	void init();
	QString getPath(QString dirnam); //保存先のパスを取得

signals:
	void save_rgbimage();
	void setdivnum(int);
	void setgridnum(QVector<int>);
	void clickradiobotton(bool);
	void set_depth(double);
	void plotdata(double,double);//計算値と真値
	void remeasure();
	void PCVopen();

public slots:
	void countup();
	void clearcount();
	void Getaverage(QString);
	//	void set_property(double, double);
	void setLogDepthDatapath(QString);

private slots:
	void on_rgbsave_clicked();
	void on_dividednum_valueChanged(int arg1);
	void on_gridms_clicked(bool checked);
	void on_allms_clicked(bool checked);
	//void on_set_clicked();
	void on_depth_valueChanged(double arg1);
	void on_finish_clicked();

private:
	Ui::Control *ui;
	bool allbool;
	double tvalue;//test用真値
	//double inclination;	//傾き
	//double intercept;		//切片
	QString logDepthDataPath; // /build/LogDepthData/data/time/time.csv のパスを保存
	float depth_average;

	//ヒストグラムの度数を格納するベクトル(1d)を定義
	//vector<int> colmListNum;

	//ファイル保存用
	QDir buildDir;
	QString orgDirPath;
	QDateTime currentDateTime;
	QString savePath;

};

#endif // CONTROL_H
