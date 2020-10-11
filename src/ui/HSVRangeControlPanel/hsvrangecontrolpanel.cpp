#include "hsvrangecontrolpanel.h"
#include "ui_hsvrangecontrolpanel.h"

HSVRangeControlPanel::HSVRangeControlPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HSVRangeControlPanel)
{
	ui->setupUi(this);
}

HSVRangeControlPanel::~HSVRangeControlPanel()
{
	delete ui;
}

void HSVRangeControlPanel::on_pushButton_clicked()
{
	ui->sldrHMin->setValue(Range::Min);
	ui->sldrHMax->setValue(Range::Max);
	ui->sldrSMin->setValue(Range::Min);
	ui->sldrSMax->setValue(Range::Max);
	ui->sldrVMin->setValue(Range::Min);
	ui->sldrVMax->setValue(Range::Max);
}

void HSVRangeControlPanel::on_sldrHMin_valueChanged(int value)
{
	hsvRange.H.min = value;
	emit On_HSVRange_changed(hsvRange);
}

void HSVRangeControlPanel::on_sldrHMax_valueChanged(int value)
{
	hsvRange.H.max = value;
	emit On_HSVRange_changed(hsvRange);
}

void HSVRangeControlPanel::on_sldrSMin_valueChanged(int value)
{
	hsvRange.S.min = value;
	emit On_HSVRange_changed(hsvRange);
}

void HSVRangeControlPanel::on_sldrSMax_valueChanged(int value)
{
	hsvRange.S.max = value;
	emit On_HSVRange_changed(hsvRange);
}

void HSVRangeControlPanel::on_sldrVMin_valueChanged(int value)
{
	hsvRange.V.min = value;
	emit On_HSVRange_changed(hsvRange);
}

void HSVRangeControlPanel::on_sldrVMax_valueChanged(int value)
{
	hsvRange.V.max = value;
	emit On_HSVRange_changed(hsvRange);
}
