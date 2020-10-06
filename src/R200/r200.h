#ifndef R200_H
#define R200_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QThread>

#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>

#include "difinition.h"

class R200 : public QObject
{
	Q_OBJECT
public:
	explicit R200(QObject *parent = nullptr);

	int init();
	int getFrames(Frames_t &frames);
	int setParams(CamParams_t &camparams);
	void close();

private:
	int initDevice(QString qsSerial="none");
	int initStreams();

private:
	rs::context *rscxt;
	rs::device *rsdev;

	//画像サイズ取得用
	cv::Size szRGB, szDepth, szDepth2RGB, szRGB2Depth;
};

#endif // R200_H
