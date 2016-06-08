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

#include "cv_camera_calib.h"

CvCameraCalib::CvCameraCalib(){

	intrinsics_ = new cv::Mat(3, 3, CV_64F);// intrinsics
	distortion_param_ = new cv::Mat(4, 1, CV_64F); // distortion params. 
}

CvCameraCalib::~CvCameraCalib(){

	//Detele matrices
	intrinsics_->release();

	if(distortion_param_)
		distortion_param_->release();
}


void CvCameraCalib::calibrateCamera(CalibParams params, 
						 std::vector<std::vector<cv::Point2f>> all_image_points){

	int n_images = all_image_points.size();

	// create distortion vector
	switch(params.distortion_model){
		case 0:
			std::cout << "Selecting non-parametric distortion model " << std::endl;
			break;
		case 4:
			std::cout << "Selecting 4 parameter distortion model" << std::endl;
			distortion_param_->release();
			distortion_param_ = new cv::Mat(4, 1, CV_64F);
			break;
		case 5:
			std::cout << "Selecting 5 parameter distortion model" << std::endl;
			distortion_param_->release();
			distortion_param_ = new cv::Mat(5, 1, CV_64F);
			break;
		case 8:
			std::cout << "Selecting 8 parameter distortion model" << std::endl;
			distortion_param_->release();
			distortion_param_ = new cv::Mat(8, 1, CV_64F);
			break;
		default:
			std::cout << "Unknown distortion model" << std::endl;
			return;
	}

	// generate object points
	std::vector<std::vector<cv::Point3f>> all_object_points;
	std::vector<cv::Point3f> object_points;

	for(int i=0; i<n_images; i++){
		object_points.clear();
		for(int y=0; y<params.h_corners; y++){
			for(int x=0; x<params.w_corners; x++){
				cv::Point3f point(x*params.sq_size,
									y*params.sq_size, 
									  0);
				object_points.push_back(point);
			}
		}
		all_object_points.push_back(object_points);
	}

	std::cout << "Calibrating camera.. " << std::endl;
	// calibrate camera. Pose per view will be saved in rVec and tVec
	double error = cv::calibrateCamera(all_object_points, 
										all_image_points, 
										cvSize(params.img_width, params.img_height),
										*intrinsics_, *distortion_param_, 
										rVec, tVec); 

	std::cout << "Calibration done. Reprojection error " << error << std::endl; 

#ifdef CV_CAMERA_CALIB_DEBUG
	std::cerr << "Printing intrinsics_ matrix" << std::endl;
	std::cerr << *intrinsics_ << std::endl;

	std::cerr << "Printing distortion params" << std::endl;
	std::cerr << *distortion_param_ << std::endl;
#endif

	return ; 						
}

void CvCameraCalib::getCameraIntrinsics(std::vector<std::vector<float>> *mat){

	for(int i=0; i<3; i++){
		std::vector<float> row;
		for(int j=0; j<3; j++){
			row.push_back((float)(intrinsics_->at<double>(i, j)));
		}
		mat->push_back(row);
	}
}

bool CvCameraCalib::saveCalibrationParams(std::string filename){
	
	cv::FileStorage file(filename, cv::FileStorage::WRITE);
	if(!file.isOpened()){
		std::cerr << "Unable to open the file" << std::endl;
		return false;
	}

	file << "Intrinsics" << *intrinsics_ ;
	file << "Distortion_Parameters" << *distortion_param_;

	file.release();
	return true;
}

void CvCameraCalib::getCameraDistortionParams(std::vector<float>* params){

	int n_params = distortion_param_->rows;

	for(int i=0; i<n_params; i++)
		params->push_back((float)(distortion_param_->at<double>(i, 0)));
}

void CvCameraCalib::projectPoints(std::vector<cv::Point3f> *points3d,
								  std::vector<float> rVec, std::vector<float> tVec, 
								  std::vector<cv::Point2f> *points2d){

	  //TODO

}

