#ifndef DIFINITION_H
#define DIFINITION_H

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
	double lr_gain;
	double lr_exposure;
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


#endif // DIFINITION_H
