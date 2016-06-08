/*==========================================================================

  Copyright (c) 2014 Uditha L. Jayarathne, ujayarat@robarts.ca

  Use, modification and redistribution of the software, in source or
  binary forms, are permitted provided that the following terms and
  conditions are met:

  1) Redistribution of the source code, in verbatim or modified
  form, must retain the above copyright notice, this license,
  the following disclaimer, and any notices that refer to this
  license and/or the following disclaimer.  

  2) Redistribution in binary form must include the above copyright
  notice, a copy of this license and the following disclaimer
  in the documentation or with other materials provided with the
  distribution.

  3) Modified copies of the source code must be clearly marked as such,
  and must not be misrepresented as verbatim copies of the source code.

  THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE SOFTWARE "AS IS"
  WITHOUT EXPRESSED OR IMPLIED WARRANTY INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE.  IN NO EVENT SHALL ANY COPYRIGHT HOLDER OR OTHER PARTY WHO MAY
  MODIFY AND/OR REDISTRIBUTE THE SOFTWARE UNDER THE TERMS OF THIS LICENSE
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA OR DATA BECOMING INACCURATE
  OR LOSS OF PROFIT OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF
  THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES.
  =========================================================================*/
#include <iostream> 
#include <vector>
#include <stdlib.h>
#include <time.h>

// Opencv includes
#include "cv.h"
#include "highgui.h"


int main(int argc, char** argv){

	// Software usage
	if(argc<5 || argc>5){ 
		std::cout << "Usage:\t CV_Undistort_Stereo infile outfile	\n"
				  << "\t	infile: input video file \n" 
				  << "\t    left_calib_file: left calibration.xml \n"
				  << "\t    right_calib_file: right calibration.xml \n"
				  << "\t	outfile: output video filename"
				  << std::endl;
		return 0;
	}
	
	std::string in_file_name(argv[1]), out_file_name(argv[2]), left_calib_name(argv[3]), right_calib_name(argv[4]);
	
	// Read calibration file.
	cv::FileStorage left_calib_file(left_calib_name, cv::FileStorage::READ);
	if(!left_calib_file.isOpened()){
		std::cout << "Could not open the left calibration file" << std::endl;
		return 0;
	}

	cv::FileStorage right_calib_file(right_calib_name, cv::FileStorage::READ);
	if(!right_calib_file.isOpened()){
		std::cout << "Could not open the left calibration file" << std::endl;
		return 0;
	}

	// camera matrices.
	cv::Mat left_intrinsics;
	cv::Mat right_intrinsics;

	// Distortion params
	cv::Mat left_distortion_params;
	cv::Mat right_distortion_params;

	left_calib_file["Intrinsics"] >> left_intrinsics;
	left_calib_file["Distortion_Parameters"] >> left_distortion_params;
	right_calib_file["Intrinsics"] >> right_intrinsics;
	right_calib_file["Distortion_Parameters"] >> right_distortion_params;

	left_calib_file.release();
	right_calib_file.release();

	std::cout << "Calibration params:" << std::endl;
	std::cout << "Left Intrinsics: " << std::endl;
	std::cout << left_intrinsics << std::endl;
	std::cout << "Left Distortions: " << std::endl;
	std::cout << left_distortion_params << std::endl;
	std::cout << "Right Intrinsics: " << std::endl;
	std::cout << right_intrinsics << std::endl;
	std::cout << "Right Distortions: " << std::endl;
	std::cout << right_distortion_params << std::endl;

	//Create a window with a fixed aspect ratio
	cvNamedWindow(in_file_name.c_str(), CV_WINDOW_KEEPRATIO);
	cvNamedWindow("Undistorted", CV_WINDOW_KEEPRATIO);

	//CvCapture* g_capture = cvCreateFileCapture(in_file_name.c_str());
	cv::VideoCapture g_capture;
	cv::VideoWriter video_writer;
	int codec = CV_FOURCC('D','I','V','X'); 

	cv::Mat frame; 

	if(!g_capture.open(in_file_name.c_str())){
		std::cerr << "Failed to open the video file. " << std::endl;
		return 0;
	}
 	
	// Query capture properties
	int n_frames	   = (int)g_capture.get(CV_CAP_PROP_FRAME_COUNT);
	int frame_width    = (int)g_capture.get(CV_CAP_PROP_FRAME_WIDTH );
	int frame_height   = (int)g_capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	int frame_rate     = (int)g_capture.get(CV_CAP_PROP_FPS );

	std::cout << " Playing back " << in_file_name.c_str() << " at " << frame_rate << "fps" 
			  << " [" << n_frames << ", " << frame_width << "x" << frame_height
			  << " frames]" << std::endl;

	// Open the video writer
	video_writer = cv::VideoWriter(out_file_name, 
									codec, 
									frame_rate, 
									cvSize(frame_width, frame_height), 
									true);

	if(!video_writer.isOpened()){
		std::cerr << "Failed to open the video writer." << std::endl;
		return 0;
	}

	std::cout << "Undistorting video ..";
	while( n_frames-- > 0 ){

		if( !g_capture.read(frame)){
			std::cout << "Bad frame" << std::endl;
			break;
		}

		if( n_frames%100 == 0)
			std::cout << ".";

		// Split frame into left and right parts
    	int split_width = frame_width/2;
		int split_height = frame_height;

		cv::Mat left(split_height, split_width, frame.type());
		cv::Mat uleft(split_height, split_width, frame.type());
		cv::Mat right(split_height, split_width, frame.type());
		cv::Mat uright(split_height, split_width, frame.type());
		cv::Mat undistorted_frame(frame_height, frame_width, frame.type());

		// Split, Undistort and Stich together
		cv::Rect roi(0, 0, split_width, split_height);
		left = frame(roi);
		cv::undistort(left, uleft, left_intrinsics, left_distortion_params);
		uleft.copyTo(undistorted_frame(roi));

		roi.x = split_width;
		right = frame(roi);
		cv::undistort(right, uright, right_intrinsics, right_distortion_params);
		uright.copyTo(undistorted_frame(roi));

		cv::imshow(in_file_name.c_str(), frame);
		cv::imshow("Undistorted", undistorted_frame);

		video_writer << undistorted_frame;

		char c = cvWaitKey((int)(1000/frame_rate));
	}
	
	return 0; 
}


