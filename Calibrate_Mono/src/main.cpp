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

#include "cv_camera_calib.h"

/**
 * Write settings to an xml file
 * @param void
 */
void write_settings_to_xml();

int main(int argc, char **argv){

	std::cout << "settings writte" << std::endl;
	// Software usage
	if(argc<4 || argc>4){ 
		std::cout << "Usage:\t CV_Calib_V1 infile outfile	\n"
				  << "\t	infile: input configuratoin file \n" 
				  << "\t	outfile: name of the file to write calibration matrices\n"
				  << "\t	prefix: camera prefix (L/R)" 
				  << std::endl;
		return 0;
	}

	std::string settings_file_name(argv[1]), output_file_name(argv[2]);
	std::string cam_prefix(argv[3]); // L or R typically
	// Read the settings file.
	cv::FileStorage file;	
	if(!file.open(settings_file_name, cv::FileStorage::READ)){
		std::cout << "Could not open the configuration file" << std::endl;
		return 0;
	}

	cv::FileNode n = file["Calibration_Images"];
	// file prefix
	std::string prefix = (std::string)n["folder_name"] +
						"/" + (std::string)n["prefix"];
	// file extention
	std::string ext   = (std::string)n["extension"];
	// number of files
	int n_images      = (int)n["image_count"];

	n = file["Calibration_Params"];
	int distortion_model = (int)n["distortion_model_param"];

	// Checkerboard params
	n = file["Checkerboard_Specs"];
	int w_corners((int)n["width_count"]);
	int h_corners((int)n["height_count"]);
	float sq_size((int)n["square_size"]);
	
	//Read image folder and detect corners
	std::string filename;
	char img_no[3];

	CvCameraCalib CameraCalibrator; 
	CvCameraCalib::CalibParams params;
	params.distortion_model = distortion_model;
	params.h_corners = h_corners;
	params.w_corners = w_corners;
	params.sq_size = sq_size;

	std::cout << "Calibration parameters:\n"
		      << "\t Distortion Model: " << params.distortion_model
			  << " parameter model\n"
			  << "\t Width Corner Count: " << params.w_corners
			  << "\n"
			  << "\t Height Corner Count: " << params.h_corners
			  << "\n"
			  << "\t Square size: " << params.sq_size << "mm"
			  << "\n" << std::endl << std::endl;

	cv::Mat corners;
	std::vector<std::vector<cv::Point2f>> all_corners;

	char key;

	std::cout << "Reading " << n_images
			  << " images from " << prefix << std::endl;
	for(int i=0; i<n_images; i++){

		itoa(i, img_no, 10);
		filename = prefix + img_no + cam_prefix + ".png"; 

		std::cout << "Reading image: " << filename << std::endl;
		cv::Mat frame = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
		if(frame.empty()){
			std::cout << filename << " was not found." << std::endl;
			continue;
		}

		// convert image to grayscale. 
		cv::Mat frame_gry(frame.rows, frame.cols, CV_32F);
		cv::cvtColor(frame, frame_gry, CV_BGR2GRAY);
		
		if(frame.empty()){
			std::cerr << "Bad image!" << std::endl;
			continue; // read the next image
		}
				
		params.img_width = frame.cols;
		params.img_height = frame.rows;

		if(!cv::findChessboardCorners(frame, cvSize(w_corners, h_corners),
									corners,
									cv::CALIB_CB_ADAPTIVE_THRESH+
									cv::CALIB_CB_FILTER_QUADS)){

			std::cerr << "Failed to find the checkerboard" << std::endl;
		}
		else{
			//draw checkerboard corners
			cv::drawChessboardCorners(frame, cvSize(w_corners, h_corners),
										corners, true);			

			// show image
			cv::namedWindow( filename, CV_WINDOW_KEEPRATIO);			
			cv::imshow(filename, frame);

			// wait for any key
			key = cv::waitKey(-1);	

			// Refine corner sub pix
			cv::cornerSubPix(frame_gry, corners, cvSize(5, 5), 
								cvSize(-1, -1), 
								cv::TermCriteria::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER,
												30, 0.1));

			//copy corner points into a std::vector
			std::vector<cv::Point2f> corners_per_image;
			for(int j=0; j<corners.rows; j++){
				cv::Point2f point(corners.at<float>(j, 0), 
									corners.at<float>(j, 1));
				corners_per_image.push_back(point);
			}
			all_corners.push_back(corners_per_image);

			cv::destroyWindow( filename );
		}

		frame_gry.release();
	}

	std::cout << "No. of calibration images: " << all_corners.size() << std::endl;
	std::cout << "Proceed with calibration?" << std::endl;
	char input;
	std::cin >> input;

	// Proceed to calibration.
	if(toupper(input) == 'Y'){

 		CameraCalibrator.calibrateCamera(params, all_corners);
		if(!CameraCalibrator.saveCalibrationParams(output_file_name)){
			std::cout << "Unable to write calibration parameters to "
					  << output_file_name << std::endl;
			return 0;
		}

		std::cout << "Parameters were written to " 
				  << output_file_name << std::endl;

	//std::vector<std::vector<float>> intrinsics;
	//std::vector<float> distortion_params;

	//CameraCalibrator.getCameraIntrinsics(&intrinsics);
	//CameraCalibrator.getCameraDistortionParams(&distortion_params);
	}
	else
		return 0; 
}


void write_settings_to_xml(){

	// xml read write (build setting file)
	cv::FileStorage fs("settings.xml", cv::FileStorage::WRITE);

	if(!fs.isOpened())
		std::cout << "could not open file" << std::endl;

	fs << "Checkerboard_Specs";
	fs << "{" << "width_count" << 9;
	fs << "height_count" << 6;
	fs << "square_size" << 5.0 <<  "}";

	fs << "Calibration_Images";
	fs << "{" << "folder_name" << "./captures";
	fs << "prefix" << "CAP_";
	fs << "extension" << ".png";
	fs << "image_count" << 7 << "}";

	fs << "Calibration_Params";
	fs << "{" << "distortion_model_param" << 5 << "}";

	std::cout << "Settings were written to settings.xml file" << std::endl;

	fs.release();
}