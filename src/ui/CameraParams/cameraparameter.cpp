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

void Cameraparameter::init(CamParam_t camerapara)
{
	ui->R200lr_exposure->setValue((int)camerapara.lrexposure);
	ui->R200lr_gain->setValue((int)camerapara.lrgain);
	ui->Saturation->setValue((int)camerapara.saturation);
	ui->Whitebalance->setValue((int)camerapara.whitebalance);
	ui->Color_gain->setValue((int)camerapara.colorgain);
	ui->Color_exposure->setValue((int)camerapara.colorexposure);

	ui->r200lr_exposure->setNum(camerapara.lrexposure);
	ui->r200lr_gain->setNum(camerapara.lrgain);
	ui->saturation->setNum(camerapara.saturation);
	ui->whitebalance->setNum(camerapara.whitebalance);
	ui->color_gain->setNum(camerapara.colorgain);
	ui->color_exposure->setNum(camerapara.colorexposure);

	cameraparameter.lrexposure = camerapara.lrexposure;
	cameraparameter.lrgain = camerapara.lrgain;
	cameraparameter.saturation = camerapara.saturation;
	cameraparameter.whitebalance = camerapara.whitebalance;
	cameraparameter.colorgain = camerapara.colorgain;
	cameraparameter.colorexposure = camerapara.colorexposure;
}

void Cameraparameter::on_Saturation_valueChanged(int value)
{
	ui->saturation->setNum(value);
	cameraparameter.saturation = (double)value;
	emit changedcamerapara(cameraparameter);
}

void Cameraparameter::on_Whitebalance_valueChanged(int value)
{
	ui->whitebalance->setNum(value);
	cameraparameter.whitebalance = (double)value;
	emit changedcamerapara(cameraparameter);
}

void Cameraparameter::on_R200lr_exposure_valueChanged(int value)
{
	ui->r200lr_exposure->setNum(value);
	cameraparameter.lrexposure = (double)value;
	emit changedcamerapara(cameraparameter);
}

void Cameraparameter::on_R200lr_gain_valueChanged(int value)
{
	ui->r200lr_gain->setNum(value);
	cameraparameter.lrgain = (double)value;
	emit changedcamerapara(cameraparameter);
}

void Cameraparameter::on_Color_gain_valueChanged(int value)
{
	ui->color_gain->setNum(value);
	cameraparameter.colorgain = (double)value;
	emit changedcamerapara(cameraparameter);
}

void Cameraparameter::on_Color_exposure_valueChanged(int value)
{
	ui->color_exposure->setNum(value);
	cameraparameter.colorexposure = (double)value;
	emit changedcamerapara(cameraparameter);
}
