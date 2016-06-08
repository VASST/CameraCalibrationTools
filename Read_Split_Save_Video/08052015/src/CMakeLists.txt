cmake_minimum_required(VERSION 2.6)
project(ReadVid)

find_package(OpenCV REQUIRED)
find_package(Qt4 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIR})
include(${QT_USE_FILE})

add_executable( ${PROJECT_NAME} main.cpp 
				cv_includes.h
				offlineVideo.h offlineVideo.cpp)

target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} ${QT_LIBRARIES})

