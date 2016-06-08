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

/**
 * Write settings to an xml file
 * @param void
 */
void write_settings_to_xml();

int main(int argc, char** argv){

	// Software usage
	if(argc<3 || argc>3){ 
		std::cout << "Usage:\t CV_Calib_V1 infile outfile	\n"
				  << "\t	infile: input configuratoin file \n" 
				  << "\t	outfile: name of the file to write calibration matrices"
				  << std::endl;
		return 0;
	}

	// Display software usage
	std::cout << "Stereo Calibration:\n"
			  << "\t Press 'd' to detect the checkerboard\n"
			  << "\t Press 's' to select the images for calibration\n" 
			  << std::endl;


	std::string settings_file_name(argv[1]), output_file_name(argv[2]);
	
	// Read the settings file.
	cv::FileStorage file(settings_file_name, cv::FileStorage::READ);
	if(!file.isOpened()){
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

	// Calibration params
	cv::Mat left_intrinsics(3, 3, CV_32F);
	cv::Mat right_intrinsics(3, 3, CV_32F);

	// Distortion params
	cv::Mat left_distortion_params(distortion_model, distortion_model, 
										CV_32F);
	cv::Mat right_distortion_params(distortion_model, distortion_model, 
										CV_32F);

	// Read in calibration params from files.
	n = file["Calibration_File_Locations"];
	std::string left_calibration_file_name((std::string)n["left_calib_file"]), 
				right_calibration_file_name((std::string)n["right_calib_file"]);
	
	cv::FileStorage calib_file(left_calibration_file_name,
								cv::FileStorage::READ);
	calib_file["Intrinsics"] >> left_intrinsics;
	calib_file["Distortion_Parameters"] >> left_distortion_params;

	calib_file.release();

	calib_file = cv::FileStorage(right_calibration_file_name, 
									cv::FileStorage::READ);
	calib_file["Intrinsics"] >> right_intrinsics;
	calib_file["Distortion_Parameters"] >> right_distortion_params;
	calib_file.release();

	
	//Read image folder and detect corners
	std::string filename;
	char img_no[3];	
	
	cv::Mat left_corners, right_corners;
	std::vector<std::vector<cv::Point2f>> all_left_corners, all_right_corners;
	std::vector<std::vector<cv::Point3f>> all_object_points;

	std::vector<cv::Point3f> object_points;
	for(int y=0; y<h_corners; y++){
		for(int x=0; x<w_corners; x++){
			cv::Point3f point(x*sq_size, y*sq_size, 0);
			object_points.push_back(point);
		}		
	}
	all_object_points.push_back(object_points);

		

	// Output matrics
	cv::Mat R(3, 3, CV_32F); // Rotational matrix between cameras
	cv::Mat T(3, 1, CV_32F); // Translational vector between cameras
	cv::Mat E(3, 3, CV_32F); // Essential matrix between the cameras
	cv::Mat F(3, 3, CV_32F); // Fundamental matrix between the cameras
	cv::Mat R1(3, 3, CV_32F); // 1st rectification transform
	cv::Mat R2(3, 3, CV_32F); // 2nd rectification transform
	cv::Mat P1(3, 4, CV_32F); // 1st projection matrix
	cv::Mat P2(3, 4, CV_32F); // 2nd projection matrix
	cv::Mat Q(4, 4, CV_32F); // disparity-to-depth mapping matrix

	char key;

	std::cout << "Reading " << n_images
			  << " images from " << prefix << std::endl;
	for(int i=0; i<n_images; i++){

		itoa(i, img_no, 10);
		filename = prefix + img_no + "L.png"; 

		// reading left frame
		std::cout << "Reading image: " << filename << std::endl;
		cv::Mat left_frame = cv::imread(filename, CV_LOAD_IMAGE_COLOR);

		// reading right frame
		filename = prefix + img_no + "R.png";
		std::cout << "Reading image: " << filename << std::endl;
		cv::Mat right_frame = cv::imread(filename, CV_LOAD_IMAGE_COLOR);

		if(left_frame.empty() || right_frame.empty()){
			std::cerr << "Bad image!" << std::endl;
			continue; // read the next image
		}

		// convert image to grayscale. 
		cv::Mat left_frame_gry(left_frame.rows, left_frame.cols, CV_32F);
		cv::Mat right_frame_gry(right_frame.rows, right_frame.cols, CV_32F);
		cv::cvtColor(left_frame, left_frame_gry, CV_BGR2GRAY);
		cv::cvtColor(right_frame, right_frame_gry, CV_BGR2GRAY);

		// Show images
		cv::namedWindow("Left_Image", CV_WINDOW_KEEPRATIO);
		cv::namedWindow("Right_Image", CV_WINDOW_KEEPRATIO);

		cv::imshow("Left_Image", left_frame);
		cv::imshow("Right_Image", right_frame);

		// wait for key stroke. 
		key = cv::waitKey(-1);

		if(key == 100 ){ // if the key is 'd'
			// Detect the checkerboards
			
			if(!cv::findChessboardCorners(left_frame, cvSize(w_corners, h_corners),
									left_corners,
									cv::CALIB_CB_ADAPTIVE_THRESH+
									cv::CALIB_CB_NORMALIZE_IMAGE) || 
					!cv::findChessboardCorners(right_frame, cvSize(w_corners, h_corners),
									right_corners,
									cv::CALIB_CB_ADAPTIVE_THRESH+
									cv::CALIB_CB_NORMALIZE_IMAGE)){

			std::cerr << "Failed to find the checkerboard at least in one image" << std::endl;
			continue; 
			}
			else{ // if the checkerboards were detected.
				// draw checkerboard corners
				cv::drawChessboardCorners(left_frame, 
											cvSize(w_corners, h_corners), 
											left_corners, true);

				cv::drawChessboardCorners(right_frame, 
											cvSize(w_corners, h_corners), 
											right_corners, true);

				// Show image
				cv::imshow("Left_Image", left_frame);
				cv::imshow("Right_Image", right_frame);

				key = cv::waitKey(-1);

				if( key == 115){ // if the key is 's' -- SELECT
					// Find corner subpix
					cv::cornerSubPix(left_frame_gry, left_corners, 
										cvSize(5, 5), 
										cvSize(-1, -1), 
										cv::TermCriteria::TermCriteria(cv::TermCriteria::EPS+
													cv::TermCriteria::MAX_ITER,30, 0.1));
					cv::cornerSubPix(right_frame_gry, right_corners, 
										cvSize(5, 5), 
										cvSize(-1, -1), 
										cv::TermCriteria::TermCriteria(cv::TermCriteria::EPS+
													cv::TermCriteria::MAX_ITER,30, 0.1));

					std::vector<std::vector<cv::Point2f>> all_left_corners, all_right_corners;
					std::vector<cv::Point2f> l_temp, r_temp;
					
					for(int j=0; j<left_corners.rows; j++){
						cv::Point2f l_point(left_corners.at<float>( j, 0), 
												left_corners.at<float>(j, 1)), 
									r_point(right_corners.at<float>(j, 0), 
												right_corners.at<float>(j, 1));
						l_temp.push_back(l_point);
						r_temp.push_back(r_point);
					}
					all_left_corners.push_back(l_temp);
					all_right_corners.push_back(r_temp);
					
					//Do the calibration
					cv::stereoCalibrate(all_object_points, 
										all_left_corners, all_right_corners, 
										left_intrinsics, left_distortion_params, 
										right_intrinsics, right_distortion_params, 
										cvSize(left_frame.rows, left_frame.cols), 
										R, T, 
										E, F, 
										cv::TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 30, 1e-6));

					// Do stereo calibrated rectifications
					cv::stereoRectify(left_intrinsics, left_distortion_params, 
										right_intrinsics, right_distortion_params, 
										cvSize(left_frame.rows, left_frame.cols), 
										R, T, 
										R1, R2, 
										P1, P2, 
										Q, 
										0);
					
					//Save Results
					cv::FileStorage outputFile(output_file_name, cv::FileStorage::WRITE);
					outputFile << "R" << R;
					outputFile << "T" << T;
					outputFile << "E" << E;
					outputFile << "F" << F;
					outputFile << "R1"<< R1;
					outputFile << "R2"<< R2;
					outputFile << "P1"<< P1;
					outputFile << "P2"<< P2;
					outputFile << "Q" << Q;

					std::cout << "Stereo Calibration done."
							  << "Results were written to the output file" << std::endl;


					//Release resources
					outputFile.release();

					R.release();
					T.release();
					E.release();
					F.release();
					left_corners.release();
					right_corners.release();

					return 0;
				}
			}
		}
		else
			--i; // loop until the right key stroke	
	}
 	
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

	fs << "Calibration_File_Locations";
	fs << "{" << "left_calib_file" << "./left_calibration.xml";
	fs << "right_calib_file" << "./right_calibration.xml" << "}";

	fs.release();
}