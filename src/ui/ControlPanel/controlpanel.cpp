#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ControlPanel)
{
	ui->setupUi(this);

	QSettings cfg("config.ini", QSettings::IniFormat);
	cfg.beginGroup("MAIN");
	ui->spnCountMax->setValue(cfg.value("CountN").toUInt());
	ui->spnDivN->setValue(cfg.value("DivN").toInt());
	ui->spnHistMax->setValue(cfg.value("HistMax").toFloat());
	ui->spnHistBin->setValue(cfg.value("HistBin").toFloat());
	cfg.endGroup();
}

ControlPanel::~ControlPanel()
{
	delete ui;
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

/*!
 */
void ControlPanel::on_btnStart_clicked(){emit On_start_clicked();}
void ControlPanel::on_btnClear_clicked(){emit On_clear_clicked(); }

void ControlPanel::on_spnCountMax_valueChanged(int arg1){emit On_CountMax_changed(arg1);}
void ControlPanel::on_spnDivN_valueChanged(int arg1){emit On_DivN_changed(arg1);}
void ControlPanel::on_spnHistMax_valueChanged(double arg1){emit On_HistMax_changed(arg1); }
void ControlPanel::on_spnHistBin_valueChanged(double arg1){emit On_HistBin_changed(arg1);}
