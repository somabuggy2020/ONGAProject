#include "cameraparameter.h"
#include "ui_cameraparameter.h"

Cameraparameter::Cameraparameter(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Cameraparameter)
{
	ui->setupUi(this);
	setWindowTitle("CameraParameter");
}

Cameraparameter::~Cameraparameter()
{
	delete ui;
}

void Cameraparameter::init(CamParam_t camparam)
{
	ui->sldrSaturation->setValue((int)camparam.saturation);
	ui->sldrWhitebalance->setValue((int)camparam.whitebalance);
	ui->sldrGain->setValue((int)camparam.colorgain);
	ui->sldrExposure->setValue((int)camparam.colorexposure);
	ui->sldrIrExposure->setValue((int)camparam.lrexposure);
	ui->sldrGain->setValue((int)camparam.lrgain);

	ui->lblSaturation->setNum(camparam.saturation);
	ui->lblWhiteBalance->setNum(camparam.whitebalance);
	ui->lblGain->setNum(camparam.colorgain);
	ui->lblExposure->setNum(camparam.colorexposure);
	ui->lblIrExposure->setNum(camparam.lrexposure);
	ui->lblIrGain->setNum(camparam.lrgain);

	//store to local instance
	this->camparam.lrexposure = camparam.lrexposure;
	this->camparam.lrgain = camparam.lrgain;
	this->camparam.saturation = camparam.saturation;
	this->camparam.whitebalance = camparam.whitebalance;
	this->camparam.colorgain = camparam.colorgain;
	this->camparam.colorexposure = camparam.colorexposure;

}




void Cameraparameter::on_sldrSaturation_valueChanged(int value)
{
	camparam.saturation = (double)value;
	emit changedcamerapara(camparam);
}

void Cameraparameter::on_sldrWhitebalance_valueChanged(int value)
{
	camparam.whitebalance = (double)value;
	emit changedcamerapara(camparam);
}

void Cameraparameter::on_sldrGain_valueChanged(int value)
{
	camparam.lrexposure = (double)value;
	emit changedcamerapara(camparam);
}

void Cameraparameter::on_sldrExposure_valueChanged(int value)
{
	camparam.lrgain = (double)value;
	emit changedcamerapara(camparam);
}

void Cameraparameter::on_sldrIrExposure_valueChanged(int value)
{
	camparam.colorgain = (double)value;
	emit changedcamerapara(camparam);
}

void Cameraparameter::on_sldrIrGain_valueChanged(int value)
{
	camparam.colorexposure = (double)value;
	emit changedcamerapara(camparam);
}













