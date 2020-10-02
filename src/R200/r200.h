#ifndef R200_H
#define R200_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QThread>

#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>
#include <boost/shared_ptr.hpp>

namespace RGBD
{
	struct Data_t
	{
		cv::Mat *imgRGB;
		cv::Mat *imgDepth;
		cv::Mat *imgAlignedDepth;
		cv::Mat *imgAlignedRGB;
		//		cv::Mat *imgInfrared;
		//    PointCloudT::Ptr pointCloud;
		double scale;
	};
}

class R200 : public QObject
{
	Q_OBJECT
public:
	explicit R200(QObject *parent = nullptr);

	int init();
	int getFrames(RGBD::Data_t &data);

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