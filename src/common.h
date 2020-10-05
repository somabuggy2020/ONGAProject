#pragma once

#include <QString>
#include <QMap>
#include <opencv2/opencv.hpp>

/*!
 * \brief The CamParam_t struct
 * Structure for camera parameters
 */
struct CamParams_t
{
	double saturation;
	double whitebalance;
	double gain;
	double exposure;
	double lrgain;
	double lrexposure;
};

/*!
 * \brief The Frames_t struct
 */
struct Frames_t
{
	cv::Mat imgRGB;
	cv::Mat imgDepth;
	cv::Mat imgAlignedDepth;
	cv::Mat imgAlignedRGB;
	//		cv::Mat *imgInfrared;
	//    PointCloudT::Ptr pointCloud;
	double scale;
};

/*!
 *
 */
namespace Mode
{
	const int Wait = 0;
	const int Measure = 1;
	const int Save = 2;
	const int Calc = 3;

	const QMap<int,QString> str = {
		{Wait, QString("Wait")},
		{Measure, QString("Measure")},
		{Save, QString("Save")},
		{Calc, QString("Calc")}
	};
}
