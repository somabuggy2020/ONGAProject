#ifndef HSVRANGECONTROLPANEL_H
#define HSVRANGECONTROLPANEL_H

#include <QWidget>
#include <QString>
#include <QDebug>


namespace Range
{
	const int Min = 0;
	const int Max = 255;
}

struct Range_t
{
	int min, max;

	Range_t():min(Range::Min),max(Range::Max) {}
};

struct HSVRange_t
{
	Range_t H, S, V;
};

namespace Ui {
	class HSVRangeControlPanel;
}

class HSVRangeControlPanel : public QWidget
{
	Q_OBJECT

public:
	explicit HSVRangeControlPanel(QWidget *parent = nullptr);
	~HSVRangeControlPanel();

private slots:
	void on_pushButton_clicked();
	void on_sldrHMin_valueChanged(int value);
	void on_sldrHMax_valueChanged(int value);
	void on_sldrSMin_valueChanged(int value);
	void on_sldrSMax_valueChanged(int value);
	void on_sldrVMin_valueChanged(int value);
	void on_sldrVMax_valueChanged(int value);

signals:
	void On_HSVRange_changed(HSVRange_t hsvRange);

private:
	Ui::HSVRangeControlPanel *ui;

	HSVRange_t hsvRange;
};

#endif // HSVRANGECONTROLPANEL_H
