#ifndef D400S_H
#define D400S_H

#include <QObject>
#include <QDebug>
#include <QString>

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

class D400s
{
public:
	D400s();

	int init();


private:
	rs2::pipeline *pipe;
	rs2::config *rscfg;
	rs2::align *align_to_depth, *align_to_color;
};

#endif // D400S_H
