#include "cameraparamcontrol.h"
#include "ui_cameraparamcontrol.h"

CameraParameterControlPanel::CameraParameterControlPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CameraParamControl)
{
	ui->setupUi(this);
	setWindowTitle("Camera Parameter Controller");

	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("VISION");
	camparams.exposure = cfg.value("Exposure").toFloat();
	camparams.gain = cfg.value("Gain").toFloat();
	camparams.whitebalance = cfg.value("WhiteBalance").toFloat();
	camparams.lr_exposure = cfg.value("lr_Exposure").toFloat();
	camparams.lr_gain = cfg.value("lr_Gain").toFloat();
	cfg.endGroup();

	ui->lblExposure->setNum(camparams.exposure);
	ui->lblGain->setNum(camparams.gain);
	ui->lblWhiteBalance->setNum(camparams.whitebalance);
	ui->lblIrExposure->setNum(camparams.lr_exposure);
	ui->lblIrGain->setNum(camparams.lr_gain);

	ui->sldrExposure->setValue(camparams.exposure);
	ui->sldrGain->setValue(camparams.gain);
	ui->sldrWhitebalance->setValue(camparams.whitebalance);
	ui->sldrIrExposure->setValue(camparams.lr_exposure);
	ui->sldrIrGain->setValue(camparams.lr_gain);
}

CameraParameterControlPanel::~CameraParameterControlPanel()
{
	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("VISION");
	cfg.setValue("Exposure", QVariant::fromValue(camparams.exposure));
	cfg.setValue("Gain", QVariant::fromValue(camparams.gain));
	cfg.setValue("WhiteBalance", QVariant::fromValue(camparams.whitebalance));
	cfg.setValue("lr_Exposure", QVariant::fromValue(camparams.lr_exposure));
	cfg.setValue("lr_Gain", QVariant::fromValue(camparams.lr_gain));
	cfg.endGroup();
	cfg.sync();

	delete ui;
}

void CameraParameterControlPanel::init(CamParams_t camparam)
{
	ui->sldrSaturation->setValue((int)camparam.saturation);
	ui->sldrWhitebalance->setValue((int)camparam.whitebalance);
	ui->sldrGain->setValue((int)camparam.gain);
	ui->sldrExposure->setValue((int)camparam.exposure);
	ui->sldrIrExposure->setValue((int)camparam.lr_exposure);
	ui->sldrIrGain->setValue((int)camparam.lr_gain);

	//	ui->lblSaturation->setNum(camparam.saturation);
	//	ui->lblWhiteBalance->setNum(camparam.whitebalance);
	//	ui->lblGain->setNum(camparam.gain);
	//	ui->lblExposure->setNum(camparam.exposure);
	//	ui->lblIrExposure->setNum(camparam.lrexposure);
	//	ui->lblIrGain->setNum(camparam.lrgain);

	//store to local instance
	this->camparams.lr_exposure = camparam.lr_exposure;
	this->camparams.lr_gain = camparam.lr_gain;
	this->camparams.saturation = camparam.saturation;
	this->camparams.whitebalance = camparam.whitebalance;
	this->camparams.gain = camparam.gain;
	this->camparams.exposure = camparam.exposure;
}

void CameraParameterControlPanel::on_sldrExposure_valueChanged(int value)
{
	camparams.exposure = value;
	emit On_CameraParams_changed(camparams);
}

void CameraParameterControlPanel::on_sldrGain_valueChanged(int value)
{
	camparams.gain = value;
	emit On_CameraParams_changed(camparams);
}

void CameraParameterControlPanel::on_sldrWhitebalance_valueChanged(int value)
{
	camparams.whitebalance = value;
	emit On_CameraParams_changed(camparams);
}

void CameraParameterControlPanel::on_sldrIrExposure_valueChanged(int value)
{
	camparams.lr_exposure = value;
	emit On_CameraParams_changed(camparams);
}

void CameraParameterControlPanel::on_sldrIrGain_valueChanged(int value)
{
	camparams.lr_gain = value;
	emit On_CameraParams_changed(camparams);
}
