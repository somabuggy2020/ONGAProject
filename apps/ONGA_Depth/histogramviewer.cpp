#include "histogramviewer.h"
#include "ui_histogramviewer.h"

HistogramViewer::HistogramViewer(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HistogramViewer)
{
	ui->setupUi(this);
	this->setWindowTitle("Histogram Viewer");

	QBarSet *set0 = new QBarSet("Jane");
	*set0 << 1 << 2 << 3 << 4 << 5 << 6;

	QBarSeries *series = new QBarSeries();
	series->append(set0);

	QChart *chart = new QChart();
	chart->addSeries(series);

	QChartView *tmp = new QChartView(this);
	tmp->setChart(chart);
	tmp->setRenderHint(QPainter::Antialiasing);

	ui->gridLayout->addWidget(tmp,0,0);
}

HistogramViewer::~HistogramViewer()
{
	delete ui;
}

void HistogramViewer::clear()
{
	chartViews.clear();
}

void HistogramViewer::addHistogramData(QList<float> data)
{
	QBarSet *set = new QBarSet("Histogram");
	for(int i = 0; i < data.count(); i++) *set << data[i];

	QBarSeries *series = new QBarSeries();
	series->append(set);

	QChart *chart = new QChart();
	chart->addSeries(series);
	chart->setTheme(QChart::ChartTheme::ChartThemeDark);
	chart->legend()->setVisible(false);

	QChartView *tmp = new QChartView(this);
	tmp->setChart(chart);
	tmp->setRenderHint(QPainter::Antialiasing);
	tmp->chart()->createDefaultAxes();
	tmp->chart()->axisX()->hide();

	chartViews.append(tmp);
}

void HistogramViewer::updateHistograms(int div_n)
{
	QLayoutItem *child;
	while((child = ui->gridLayout->takeAt(0)) != nullptr){
		delete child->widget();
		delete child;
	}

	for(int i = 0; i < chartViews.count(); i++){
		int x = (int)(i%div_n);
		int y = (int)(i/div_n);
		ui->gridLayout->addWidget(chartViews[i],y,x);
	}
}
