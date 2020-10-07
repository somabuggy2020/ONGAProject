#ifndef CAMERAPARAMETER_H
#define CAMERAPARAMETER_H

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QSettings>

#include "../../R200/difinition.h"

QT_BEGIN_NAMESPACE
namespace Ui {
	class CameraParamControl;
}
QT_END_NAMESPACE

class CameraParameterControlPanel : public QWidget
{
	Q_OBJECT

public:
	explicit CameraParameterControlPanel(QWidget *parent = 0);
	~CameraParameterControlPanel();

	void init(CamParams_t);

signals:
	void On_CameraParams_changed(CamParams_t &camparams);

private slots:

	void on_sldrExposure_valueChanged(int value);

	void on_sldrGain_valueChanged(int value);

	void on_sldrWhitebalance_valueChanged(int value);

	void on_sldrIrExposure_valueChanged(int value);

	void on_sldrIrGain_valueChanged(int value);

private:
	Ui::CameraParamControl *ui;

public:
	CamParams_t camparams;
};

#endif // CAMERAPARAMETER_H
