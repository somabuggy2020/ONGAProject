#ifndef DIFINITION_H
#define DIFINITION_H

#include <opencv2/opencv.hpp>

/*!
 * \brief The CamParam_t struct
 * Structure for camera parameters
 */
#ifndef CAMPARAM_STRUCT
#define CAMPARAM_STRUCT
struct CamParams_t
{
	double saturation;
	double whitebalance;
	double gain;
	double exposure;
	double lr_gain;
	double lr_exposure;
};
#endif

/*!
 * \brief The Frames_t struct
 */
#ifndef FRAMES_STRUCT
#define FRAMES_STRUCT
struct Frames_t
{
	cv::Mat imgRGB;
	cv::Mat imgDepth;
	cv::Mat imgAlignedDepth;
	cv::Mat imgAlignedRGB;
	double scale;
};
#endif

#endif // DIFINITION_H
