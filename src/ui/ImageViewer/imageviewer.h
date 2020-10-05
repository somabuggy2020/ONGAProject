#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QPainter>
#include <QPaintEvent>

#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
	class ImageViewer;
}
QT_END_NAMESPACE

class ImageViewer : public QWidget
{
	Q_OBJECT

public:
	explicit ImageViewer(QString windowTitle, QWidget *parent = 0);
	~ImageViewer();

	int initialize(int ImageType = CV_8UC3, QImage::Format qImageFormat = QImage::Format_RGB888);	//初期化処理

public slots:
//	void setImage(cv::Mat *img);
	void setImage(cv::Mat img);

	// QWidget interface
protected:
	void paintEvent(QPaintEvent *event);

private:
	Ui::ImageViewer *ui;
	cv::Mat *img;
	int cvMatFormat;
	QImage::Format qImageFormat;
};

#endif // VIEWERIMAGE_H
