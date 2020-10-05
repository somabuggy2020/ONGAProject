#include "r200.h"

R200::R200(QObject *parent) : QObject(parent)
{
	rscxt = nullptr;
	rsdev = nullptr;
}

/*!
 * \brief R200::init
 * \return
 */
int R200::init()
{
	qInfo() << "";
	if(rscxt == nullptr) rscxt = new rs::context();

	rs::log_to_file(rs::log_severity::debug, "librealsense.log");

	if(initDevice("none") == -1) return -1;
	if(initStreams() == -1) return -1;

	try{
		if(!rsdev->is_streaming()) rsdev->start();

		//ストリームがちゃんと動くまで待機
		int i = 0;
		while(!rsdev->is_streaming()){
			qDebug() << i++;
			QThread::sleep(33);
		}
		qInfo() << "Success device start";
	}
	catch(const rs::error &e){
		qCritical() << e.get_failed_args().c_str();
		qCritical() << e.get_failed_function().c_str();
		qCritical() << e.what();
		return -1;
	}
}

int R200::getFrames(Frames_t &frames)
{
	if(rsdev == nullptr){
		//		frames.imgRGB = new cv::Mat(480, 640, CV_8UC3, cv::Scalar::all(255));
		//		frames.imgDepth = new cv::Mat();
		//		frames.imgAlignedRGB = new cv::Mat(480, 640, CV_8UC3, cv::Scalar::all(255));
		//		frames.imgAlignedDepth = new cv::Mat(480, 640, CV_16UC1, cv::Scalar::all(0));
	}

	try{
		rsdev->wait_for_frames();

		frames.imgRGB = cv::Mat(szRGB, CV_8UC3, (uchar*)rsdev->get_frame_data(rs::stream::color)).clone();
		frames.imgDepth = cv::Mat(szDepth, CV_16UC1, (uchar*)rsdev->get_frame_data(rs::stream::depth)).clone();
		frames.imgAlignedRGB = cv::Mat(szRGB2Depth, CV_8UC3, (uchar*)rsdev->get_frame_data(rs::stream::color_aligned_to_depth)).clone();
		frames.imgAlignedDepth = cv::Mat(szDepth2RGB, CV_16UC1, (uchar*)rsdev->get_frame_data(rs::stream::depth_aligned_to_color)).clone();
		frames.scale = rsdev->get_depth_scale();
	}
	catch(const rs::error &e){
		qWarning() << e.get_failed_args().c_str();
		qWarning() << e.get_failed_function().c_str();
		qWarning() << e.what();
		return -1;
	}

	return 0;
}

int R200::setParams(CamParams_t &camparams)
{
	return 0;
}

/*!
 * \brief R200::initDevice
 * \param qsSerial
 * \return
 */
int R200::initDevice(QString qsSerial)
{
	qInfo() << "";

	try{
		if(rscxt->get_device_count() <= 0){
			qCritical() << "Not found RealSense device";
			return -1;
		}
		else{
			qInfo() << "Found" << rscxt->get_device_count() << "devices";
			for(int i = 0; i < rscxt->get_device_count(); i++){
				qInfo() << "Device" << i
								<< ": Serial number" << rscxt->get_device(i)->get_serial();
			}
		}
	}
	catch(const rs::error &e){
		qCritical() << e.get_failed_args().c_str();
		qCritical() << e.get_failed_function().c_str();
		qCritical() << e.what();
		return -1;
	}


	//rs::deviceインスタンス作成
	rsdev = nullptr;

	try{
		if(qsSerial == QString("none")){
			//シリアル番号で指定されていない場合は0番デバイスをオープン
			rsdev = rscxt->get_device(0);
		}
		else{
			for(int i = 0; i < rscxt->get_device_count(); i++){
				QString qs = QString(rscxt->get_device(i)->get_serial());
				if(qs == qsSerial){
					rsdev = rscxt->get_device(i);
					break;
				}
			} //end for

			if(rsdev == nullptr){
				qCritical() << "Cannot open device at" << qsSerial;
				return -1;
			}
		}
	}
	catch(const rs::error &e){
		qCritical() << e.get_failed_args().c_str();
		qCritical() << e.get_failed_function().c_str();
		qCritical() << e.what();
		return -1;
	}

	qInfo() << "Opened Device :" << rsdev->get_name()
					<< ":" << rsdev->get_serial();

	return 0;
}

int R200::initStreams()
{
	try{
		//RGB画像ストリーム起動
		//		rsdev->enable_stream(rs::stream::color, rs::preset::best_quality);
		rsdev->enable_stream(rs::stream::color, 640, 480, rs::format::bgr8, 15);
		//		rsDev->enable_stream(rs::stream::color,320,240,rs::format::rgb8,30);

		//depthストリーム起動
		rsdev->enable_stream(rs::stream::depth, rs::preset::best_quality);
		//		rsDev->enable_stream(rs::stream::depth,640,480, rs::format::z16,60);

		//depth_aligned_to_colorストリーム起動
		//		rsdev->enable_stream(rs::stream::depth_aligned_to_color, rs::preset::best_quality);

		//color_alined_to_depthストリーム起動
		//		rsdev->enable_stream(rs::stream::color_aligned_to_depth, rs::preset::best_quality);

		//画像サイズ保存
		szRGB = cv::Size(rsdev->get_stream_width(rs::stream::color),
										 rsdev->get_stream_height(rs::stream::color));
		szDepth = cv::Size(rsdev->get_stream_width(rs::stream::depth),
											 rsdev->get_stream_height(rs::stream::depth));
		szDepth2RGB = cv::Size(rsdev->get_stream_width(rs::stream::depth_aligned_to_color),
													 rsdev->get_stream_height(rs::stream::depth_aligned_to_color));
		szRGB2Depth = cv::Size(rsdev->get_stream_width(rs::stream::color_aligned_to_depth),
													 rsdev->get_stream_height(rs::stream::color_aligned_to_depth));

		qInfo() << "Color(WxH) :" << szRGB.width << "x" << szRGB.height;
		qInfo() << "Depth(WxH) :" << szDepth.width << "x" << szDepth.height;
		qInfo() << "D2C(WxH) :" << szDepth2RGB.width << "x" << szDepth2RGB.height;
		qInfo() << "C2D(WxH) :" << szRGB2Depth.width << "x" << szRGB2Depth.height;
	}
	catch(const rs::error &e){
		qCritical() << e.get_failed_args().c_str();
		qCritical() << e.get_failed_function().c_str();
		qCritical() << e.what();
		return -1;
	}

	qInfo() << "Enabled all streams";
	return 0;
}
