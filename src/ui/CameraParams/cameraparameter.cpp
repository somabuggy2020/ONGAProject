#include "cameraparameter.h"
#include "ui_cameraparameter.h"

Cameraparameter::Cameraparameter(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Cameraparameter)
{
	ui->setupUi(this);
	setWindowTitle("Camera Parameter Controller");


	QSettings cfg("config.ini", QSettings::IniFormat);
}

Cameraparameter::~Cameraparameter()
{
	delete ui;
}

void Cameraparameter::init(CamParams_t camparam)
{
	ui->sldrSaturation->setValue((int)camparam.saturation);
	ui->sldrWhitebalance->setValue((int)camparam.whitebalance);
	ui->sldrGain->setValue((int)camparam.gain);
	ui->sldrExposure->setValue((int)camparam.exposure);
	ui->sldrIrExposure->setValue((int)camparam.lrexposure);
	ui->sldrIrGain->setValue((int)camparam.lrgain);

	//	ui->lblSaturation->setNum(camparam.saturation);
	//	ui->lblWhiteBalance->setNum(camparam.whitebalance);
	//	ui->lblGain->setNum(camparam.gain);
	//	ui->lblExposure->setNum(camparam.exposure);
	//	ui->lblIrExposure->setNum(camparam.lrexposure);
	//	ui->lblIrGain->setNum(camparam.lrgain);

	//store to local instance
	this->camparam.lrexposure = camparam.lrexposure;
	this->camparam.lrgain = camparam.lrgain;
	this->camparam.saturation = camparam.saturation;
	this->camparam.whitebalance = camparam.whitebalance;
	this->camparam.gain = camparam.gain;
	this->camparam.exposure = camparam.exposure;

}

void Cameraparameter::get(CamParams_t &camparam)
{
	camparam = this->camparam;
	return;
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
	camparam.gain = (double)value;
	emit changedcamerapara(camparam);
}

void Cameraparameter::on_sldrIrGain_valueChanged(int value)
{
	camparam.exposure = (double)value;
	emit changedcamerapara(camparam);
}













