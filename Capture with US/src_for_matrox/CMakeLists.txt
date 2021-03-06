# Capture_Video configuration file
# 22 Sept. 2014
# Uditha Jayarathne <ujayarat@robarts.ca>
# All Rights Reserved

cmake_minimum_required(VERSION 2.6)
project(Capture_VideoPlusUS)

################## FindBoost #######################################################
# configure boost libs.
set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Boost no cmake")
set(Boost_DEBUG ON CACHE INTERNAL "Boost debug on")
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Don't search in system path")
set(BOOST_ROOT C:/Program\ Files\ \(x86\)/Boost CACHE PATH "Boost Root")

add_definitions("-DBOOST_ALL_NO_LIB")

# options for Boost
set(Boost_USE_STATIC_LIBS OFF CACHE BOOL "Boost use static libs")
set(Boost_USE_MULTITHREADED ON CACHE BOOL "Boost use multithreaded")
set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "Boost use static runtime")
set(Boost_USE_DEBUG_RUNTIME ON CACHE BOOL "Boost use debug runtime")

# force dynamic linking for all libraries
if(${Boost_USE_STATIC_LIBS})
message(WARNING "Setting stating linking in all libraries")
else(${Boost_USE_STATIC_LIBS})
message(WARNING "Setting dynamic linking in all libraries")
add_definitions("-DBOOST_ALL_DYN_LINK")
endif()

#find Boost libs
set(BOOST_MIN_VERSION "1.47.0")
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS system thread date_time)

include_directories(${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIRS ${BOOST_ROOT}/lib CACHE PATH "Boost Libs")

SET( CMAKE_FIND_LIBRARY_SUFFIXES ".lib" )
SET (ULTERIUS_ROOT "./ulterius-2.0")
FIND_PATH(ULTERIUS_INCLUDE_DIR "ulterius.h" "${ULTERIUS_ROOT}/inc")
FIND_LIBRARY(ULTERIUS_LIBRARY ulterius "${ULTERIUS_ROOT}/lib")
INCLUDE_DIRECTORIES(${ULTERIUS_INCLUDE_DIR})
##########################################################################################

#find and include Opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIR})

#find and include Qt4
#find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})

#find and include VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

#find and include AIGS
find_package(AIGS REQUIRED)
include(${AIGS_USE_FILE})

add_executable( ${PROJECT_NAME} main.cpp
				opencv_internals.h opencv_internals.cpp
				video_source_wrapper.h video_source_wrapper.cpp
				vtkSonixVideoSource.cxx)

target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} 	
					   ${Boost_LIBRARIES}
					   ${ULTERIUS_LIBRARY}
					   vtkndicapi
					   vtkTracking
					   QVTK
						vtkCommon
						vtkFiltering
						vtkGraphics
						vtkHybrid
						vtkImaging
						vtkIO
						vtkParallel
						vtkRendering
						vtkVolumeRendering
						vtkWidgets
						vtksys)

