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

	if(initDevice() == -1) return -1;
	if(initStreams() == -1) return -1;

	try{
		rsdev->start();

		//ストリームがちゃんと動くまで待機
		while(!rsdev->is_streaming()){
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

int R200::getFrames(RGBD::Data_t &data)
{
	try{
		if(rsdev->is_streaming())	rsdev->wait_for_frames();
		else	return -1; //failure get new frames

		data.imgRGB = new cv::Mat(szRGB, CV_8UC3, (uchar*)rsdev->get_frame_data(rs::stream::color));
		data.imgDepth = new cv::Mat(szDepth, CV_16UC1, (uchar*)rsdev->get_frame_data(rs::stream::depth));
		data.imgAlignedRGB = new cv::Mat(szRGB2Depth, CV_8UC3, (uchar*)rsdev->get_frame_data(rs::stream::color_aligned_to_depth));
		data.imgAlignedDepth = new cv::Mat(szDepth2RGB, CV_16UC1, (uchar*)rsdev->get_frame_data(rs::stream::depth_aligned_to_color));
	}
	catch(const rs::error &e){
		qWarning() << e.get_failed_args().c_str();
		qWarning() << e.get_failed_function().c_str();
		qWarning() << e.what();
		return -1;
	}


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
		rsdev->enable_stream(rs::stream::color, rs::preset::best_quality);
		//		rsDev->enable_stream(rs::stream::color,1920,1080,rs::format::rgb8,15);
		//		rsDev->enable_stream(rs::stream::color,320,240,rs::format::rgb8,30);

		//depthストリーム起動
		rsdev->enable_stream(rs::stream::depth, rs::preset::best_quality);
		//rsDev->enable_stream(rs::stream::depth,640,480, rs::format::z16,60);

		//depth_aligned_to_colorストリーム起動
		rsdev->enable_stream(rs::stream::depth_aligned_to_color, rs::preset::best_quality);

		//color_alined_to_depthストリーム起動
		rsdev->enable_stream(rs::stream::color_aligned_to_depth, rs::preset::best_quality);

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
