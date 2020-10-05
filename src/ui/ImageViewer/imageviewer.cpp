#include "imageviewer.h"
#include "ui_imageviewer.h"

ImageViewer::ImageViewer(QString windowTitle, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ImageViewer)
{
	ui->setupUi(this);
	this->setWindowTitle(windowTitle);
}

ImageViewer::~ImageViewer()
{
	delete ui;
}

int ImageViewer::initialize(int ImageType, QImage::Format qImageFormat)
{
	//メンバ初期化
	img = new cv::Mat(this->height(), this->width(), ImageType, cv::Scalar::all(0));
	this->cvMatFormat = ImageType;
	this->qImageFormat = qImageFormat;
	return 0;
}

//void ImageViewer::setImage(cv::Mat *img)
//{
//	//メンバに複製
//	//OpenCVはBGRフォーマット, QImageはRGBフォーマット
//	if(cvMatFormat == CV_8UC3) *(this->img) = img->clone();
//	if(cvMatFormat == CV_16UC1) img->convertTo(*(this->img), CV_8UC1);
//	if(cvMatFormat == CV_8UC1) img->copyTo(*(this->img));
//	update();	//paintEvent関数の呼び出しシグナル
//	return;
//}

void ImageViewer::setImage(cv::Mat img)
{
	if(cvMatFormat == CV_8UC3) *(this->img) = img.clone();
	if(cvMatFormat == CV_16UC1) img.convertTo(*(this->img), CV_8UC1, 0.08);
	//		if(cvMatFormat == CV_16UC1) img.copyTo(*(this->img));
	if(cvMatFormat == CV_8UC1) img.copyTo(*(this->img));
	update();	//paintEvent関数の呼び出しシグナル
	return;
}

void ImageViewer::paintEvent(QPaintEvent *event)
{
	QPainter *painter = new QPainter(this);

	QImage qImg;

	qImg = QImage(this->img->data,
								this->img->cols,
								this->img->rows,
								this->qImageFormat);

	//リサイズ -> ウィジェットのサイズに合わせる
	QImage qImgResized = qImg.scaled(this->width(),
																	 this->height(),
																	 Qt::KeepAspectRatio);

	//描画処理
	painter->drawImage(QPoint(0, 0), qImgResized);
	delete painter;
	return;
}
