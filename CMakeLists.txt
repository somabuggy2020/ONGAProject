cmake_minimum_required(VERSION 3.5)

project(ONGAProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets LinguistTools REQUIRED)

#librealsense
include_directories(C:\\librealsense1.12.4\\include)
link_directories(C:\\librealsense1.12.4\\lib\\x64)

#OpenCV
set(OpenCV_DIR $ENV{OpenCV_ROOT})
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#PCL
find_package(PCL 1.8.1 REQUIRED EXACT)
include_directories(${PCL_INCLUDE_DIRS})

set(TS_FILES ONGAProject_ja_JP.ts)

add_executable(ONGA_Depth
	src/main.cpp
	src/mainwindow.cpp
	src/mainwindow.h
	src/mainwindow.ui
	src/common.h
	src/R200/r200.cpp
	src/R200/r200.h
	src/ui/Control/control.cpp
	src/ui/Control/control.h
	src/ui/Control/control.ui
	src/ui/CameraParams/cameraparameter.cpp
	src/ui/CameraParams/cameraparameter.h
	src/ui/CameraParams/cameraparameter.ui
	src/ui/ProcessingProgress/processingprogress.cpp
	src/ui/ProcessingProgress/processingprogress.h
	src/ui/ProcessingProgress/processingprogress.ui
	${TS_FILES}
)

target_link_libraries(ONGA_Depth PRIVATE
	Qt5::Widgets
	${OpenCV_LIBRARIES}
	realsense-d)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
