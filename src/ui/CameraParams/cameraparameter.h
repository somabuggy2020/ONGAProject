#ifndef CAMERAPARAMETER_H
#define CAMERAPARAMETER_H

#include <QWidget>
#include <QDebug>
#include <QString>

#include "../../common.h"

QT_BEGIN_NAMESPACE
namespace Ui {
	class Cameraparameter;
}
QT_END_NAMESPACE

class Cameraparameter : public QWidget
{
	Q_OBJECT

public:
	explicit Cameraparameter(QWidget *parent = 0);
	~Cameraparameter();

	void init(CamParams_t);
	void get(CamParams_t &camparam);

signals:
	void changedcamerapara(CamParams_t);

private slots:
	void on_sldrSaturation_valueChanged(int value);
	void on_sldrWhitebalance_valueChanged(int value);
	void on_sldrGain_valueChanged(int value);
	void on_sldrExposure_valueChanged(int value);
	void on_sldrIrExposure_valueChanged(int value);
	void on_sldrIrGain_valueChanged(int value);

private:
	Ui::Cameraparameter *ui;

public:
	CamParams_t camparam;
};

#endif // CAMERAPARAMETER_H
