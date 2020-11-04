#ifndef D400S_H
#define D400S_H

#include <QObject>
#include <QDebug>
#include <QString>

#include <librealsense2/rs.hpp>
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

class D400s
{
public:
	D400s();

	int init();
	int getFrames(Frames_t &frames);

private:

private:
	rs2::pipeline *rsPipe;
	rs2::config *rsCfg;
	rs2::align *align_to_depth, *align_to_color;
};

#endif // D400S_H
