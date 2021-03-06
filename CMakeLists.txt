cmake_minimum_required(VERSION 3.5)

project(ONGAProject LANGUAGES CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets Charts REQUIRED)

#include common classes for intellisense
include_directories(.\\src)

#librealsense
include_directories(C:\\librealsense1.12.4\\include)
link_directories(C:\\librealsense1.12.4\\lib\\x64)
include_directories(C:\\librealsense\\include)
link_directories(C:\\librealsense\\lib)

#librealsense2
include_directories(C:\\librealsense2\\include)
link_directories(C:\\librealsense2\\lib)

#OpenCV
set(OpenCV_DIR $ENV{OpenCV_ROOT})
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#PCL
#find_package(PCL 1.8.1 REQUIRED EXACT)
#include_directories(${PCL_INCLUDE_DIRS})

#for resource files
QT5_ADD_RESOURCES(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/ONGA_Depth.qrc)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR/resources/config.ini} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)


add_executable(Sample_D455
	apps/Samples/Sample_D455.cpp
	src/D400s/d400s.cpp
	src/D400s/d400s.h
	)


add_executable(ONGA_Depth
	apps/ONGA_Depth/ONGA_Depth.cpp
	apps/ONGA_Depth/ONGA_Depth_mainwindow.cpp
	apps/ONGA_Depth/ONGA_Depth_mainwindow.h
	apps/ONGA_Depth/ONGA_Depth_mainwindow.ui
	apps/ONGA_Depth/common.h
	apps/ONGA_Depth/controlpanel.cpp
	apps/ONGA_Depth/controlpanel.h
	apps/ONGA_Depth/controlpanel.ui
	apps/ONGA_Depth/histogramviewer.cpp
	apps/ONGA_Depth/histogramviewer.h
	apps/ONGA_Depth/histogramviewer.ui
	src/R200/r200.cpp
	src/R200/r200.h
	src/D400s/d400s.cpp
	src/D400s/d400s.h
	src/R200/difinition.h
	src/ui/CameraParamControl/cameraparamcontrol.cpp
	src/ui/CameraParamControl/cameraparamcontrol.h
	src/ui/CameraParamControl/cameraparamcontrol.ui
	src/ui/ImageViewer/imageviewer.cpp
	src/ui/ImageViewer/imageviewer.h
	src/ui/ImageViewer/imageviewer.ui
	${RESOURCES}
	)

add_executable(ONGA_BlueDoctor
	apps/ONGA_BlueDoctor/ONGA_BlueDoctor.cpp
	apps/ONGA_BlueDoctor/ONGA_BlueDoctor_mainwindow.cpp
	apps/ONGA_BlueDoctor/ONGA_BlueDoctor_mainwindow.h
	apps/ONGA_BlueDoctor/ONGA_BlueDoctor_mainwindow.ui
	apps/ONGA_BlueDoctor/common.h
	src/R200/r200.cpp
	src/R200/r200.h
	src/R200/difinition.h
	src/ui/CameraParamControl/cameraparamcontrol.cpp
	src/ui/CameraParamControl/cameraparamcontrol.h
	src/ui/CameraParamControl/cameraparamcontrol.ui
	src/ui/ImageViewer/imageviewer.cpp
	src/ui/ImageViewer/imageviewer.h
	src/ui/ImageViewer/imageviewer.ui
	src/ui/HSVRangeControlPanel/hsvrangecontrolpanel.cpp
	src/ui/HSVRangeControlPanel/hsvrangecontrolpanel.h
	src/ui/HSVRangeControlPanel/hsvrangecontrolpanel.ui
	)

target_link_libraries(Sample_D455 PRIVATE
	Qt5::Core
	${OpenCV_LIBRARIES}
	realsense2)

target_link_libraries(ONGA_Depth PRIVATE
	Qt5::Widgets
	Qt5::Charts
	${OpenCV_LIBRARIES}
	realsense
	realsense2)

target_link_libraries(ONGA_BlueDoctor PRIVATE
	Qt5::Widgets
	${OpenCV_LIBRARIES}
	realsense)

# for install
install(TARGETS ONGA_Depth RUNTIME
	DESTINATION ${PROJECT_BINARY_DIR}/../bin)
