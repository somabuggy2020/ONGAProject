#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QSettings>

namespace Ui {
	class ControlPanel;
}

class ControlPanel : public QWidget
{
	Q_OBJECT

public:
	explicit ControlPanel(QWidget *parent = nullptr);
	~ControlPanel();

private slots:
	void on_btnStart_clicked();
	void on_btnClear_clicked();
	void on_spnCountMax_valueChanged(int arg1);
	void on_spnDivN_valueChanged(int arg1);
	void on_spnHistMax_valueChanged(double arg1);
	void on_spnHistBin_valueChanged(double arg1);

signals:
	void On_start_clicked();
	void On_clear_clicked();
	void On_CountMax_changed(int arg);
	void On_DivN_changed(int arg);
	void On_HistMax_changed(double arg);
	void On_HistBin_changed(double arg);

private:
	Ui::ControlPanel *ui;
};

#endif // CONTROLPANEL_H
