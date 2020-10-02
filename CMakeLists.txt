cmake_minimum_required(VERSION 3.5)

project(ONGAProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

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

if(ANDROID)
  add_library(ONGAProject SHARED
    main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    ${TS_FILES}
  )
else()
	add_executable(ONGA_Depth
    main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
		src/R200/r200.cpp
		src/R200/r200.h
    ${TS_FILES}
  )
endif()

target_link_libraries(ONGA_Depth PRIVATE Qt5::Widgets
	${OpenCV_LIBRARIES}
	realsense)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
