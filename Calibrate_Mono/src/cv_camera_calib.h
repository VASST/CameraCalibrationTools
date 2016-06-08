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

#ifndef __CV_CAMERA_CALIB__H
#define __CV_CAMERA_CALIB__H

#include <iostream>
#include <vector> 

// Opencv Includes 
#include "cv.h"
#include "highgui.h"

//enable debuging
//#define CV_CAMERA_CALIB_DEBUG

class CvCameraCalib{

public:	

	// Utility structures
	// Calibration param structure
	typedef struct CalibParams{
		int w_corners;	// corners along width
		int h_corners;  // corners along height
		float sq_size;  // quad size

		int img_width;	// image width
		int img_height; // image height
		int distortion_model; // integer specifying the distortion model
	} CalibParams;


	CvCameraCalib();
	~CvCameraCalib();

	//public methods

	/**
	 * Calibrate camera
	 * @param calibration parameters in CalibParam Structure
	 * @param image points
	 */	
	void calibrateCamera(CalibParams, 
						 std::vector<std::vector<cv::Point2f>>);

	/** 
	 * Get camera intrinsics 
	 * @param reference to a 3x3 array 
	 */ 
	void getCameraIntrinsics(std::vector<std::vector<float>>*);

	/**
	 * Save camera instrinsics and distortions
	 * @param filename
	 * @return true on success
	 */
	bool saveCalibrationParams(std::string);

	/** 
	 * Get camera distortion params 
	 * @param reference to a 5x1 array
	 */ 
	void getCameraDistortionParams(std::vector<float>*);

	/** 
	 * Project points 
	 * @param Nx3 array containing object points
	 * @param 3x1 array rotation vector
	 * @param 3x1 array of translation vector
	 * @param reference to 2D array 
	 */
	void projectPoints(std::vector<cv::Point3f>*, 
						std::vector<float>, 
						std::vector<float>, 
						std::vector<cv::Point2f>*);

	/**
	 * Undistort image 
	 * @param src image
	 * @param dst image
	 */
	/* TODO: params (src, dst, method) */
	void undistortImage(cv::Mat *, cv::Mat *);



private:
	//private members

	// 3x3 matrix containing instrinsic params
	cv::Mat *intrinsics_;

	// 4x1 or 5x1  or 8x1 matrix containng distortion params
	cv::Mat *distortion_param_;

	// rVec of the calib object pose
	std::vector<cv::Mat> rVec;

	// tVec of the calib object pose
	std::vector<cv::Mat> tVec;

};

#endif //__CV_CAMERA_CALIB__H