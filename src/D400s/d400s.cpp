#include "d400s.h"

D400s::D400s()
{
	rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);

	pipe = nullptr;
	rscfg = nullptr;
}

int D400s::init()
{
	try {
		qInfo() << "Try to connect RealSense device ...";
		pipe = new rs2::pipeline();
		rscfg = new rs2::config();
		rscfg->enable_stream(RS2_STREAM_COLOR);
		rscfg->enable_stream(RS2_STREAM_DEPTH);
		pipe->start(*rscfg);

		align_to_depth = new rs2::align(RS2_STREAM_DEPTH);
		align_to_color = new rs2::align(RS2_STREAM_COLOR);
	}
	catch (const rs2::error &e) {
		qCritical() << "[RS Error]:" << e.get_failed_function().c_str();
		qCritical() << "[RS Error]:" << e.get_failed_args().c_str();
		qCritical() << "[RS Error]:" << e.what();
		return -1;
	}
	return 0;
}
