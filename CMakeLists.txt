cmake_minimum_required(VERSION 3.5)

project(ONGAProject LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

#include common classes for intellisense
include_directories(.\\src)

#librealsense
include_directories(C:\\librealsense1.12.4\\include)
link_directories(C:\\librealsense1.12.4\\lib\\x64)
include_directories(C:\\librealsense\\include)
link_directories(C:\\librealsense\\lib)

#OpenCV
set(OpenCV_DIR $ENV{OpenCV_ROOT})
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#PCL
#find_package(PCL 1.8.1 REQUIRED EXACT)
#include_directories(${PCL_INCLUDE_DIRS})

#for resource files
file(GLOB RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.ini)
file(COPY ${RESOURCES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)

add_executable(ONGA_Depth
	apps/ONGA_Depth/main.cpp
	apps/ONGA_Depth/mainwindow.cpp
	apps/ONGA_Depth/mainwindow.h
	apps/ONGA_Depth/mainwindow.ui
	apps/ONGA_Depth/common.h
	apps/ONGA_Depth/controlpanel.cpp
	apps/ONGA_Depth/controlpanel.h
	apps/ONGA_Depth/controlpanel.ui
	src/R200/r200.cpp
	src/R200/r200.h
	src/R200/difinition.h
	src/ui/CameraParamControl/cameraparamcontrol.cpp
	src/ui/CameraParamControl/cameraparamcontrol.h
	src/ui/CameraParamControl/cameraparamcontrol.ui
	src/ui/ImageViewer/imageviewer.cpp
	src/ui/ImageViewer/imageviewer.h
	src/ui/ImageViewer/imageviewer.ui
	)

add_executable(ONGA_BlueDoctor
	apps/ONGA_BlueDoctor/main.cpp
	apps/ONGA_BlueDoctor/mainwindow.cpp
	apps/ONGA_BlueDoctor/mainwindow.h
	apps/ONGA_BlueDoctor/mainwindow.ui
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

target_link_libraries(ONGA_Depth PRIVATE
	Qt5::Widgets
	${OpenCV_LIBRARIES}
	realsense)

target_link_libraries(ONGA_BlueDoctor PRIVATE
	Qt5::Widgets
	${OpenCV_LIBRARIES}
	realsense)
