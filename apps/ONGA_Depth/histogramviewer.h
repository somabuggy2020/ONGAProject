#ifndef HISTOGRAMVIEWER_H
#define HISTOGRAMVIEWER_H

#include <QWidget>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtCharts>

namespace Ui {
	class HistogramViewer;
}

class HistogramViewer : public QWidget
{
	Q_OBJECT

public:
	explicit HistogramViewer(QWidget *parent = nullptr);
	~HistogramViewer();

	void clear();
	void addHistogramData(QList<float> data);
	void updateHistograms(int div_n);

private:
	Ui::HistogramViewer *ui;
	QList<QChartView*> chartViews;
};

#endif // HISTOGRAMVIEWER_H
