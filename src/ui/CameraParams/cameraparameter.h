#ifndef CAMERAPARAMETER_H
#define CAMERAPARAMETER_H

#include <QWidget>
#include <QDebug>
#include <QString>

#include "../../common.h"

namespace Ui {
	class Cameraparameter;
}

class Cameraparameter : public QWidget
{
	Q_OBJECT

public:
	explicit Cameraparameter(QWidget *parent = 0);
	~Cameraparameter();

	void init(CamParam_t);

signals:
	void changedcamerapara(CamParam_t);

private slots:
	void on_Saturation_valueChanged(int value);
	void on_Whitebalance_valueChanged(int value);
	void on_R200lr_exposure_valueChanged(int value);
	void on_R200lr_gain_valueChanged(int value);
	void on_Color_gain_valueChanged(int value);
	void on_Color_exposure_valueChanged(int value);

private:
	Ui::Cameraparameter *ui;

public:
	CamParam_t cameraparameter;
};

#endif // CAMERAPARAMETER_H
