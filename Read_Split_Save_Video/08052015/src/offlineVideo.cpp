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

#include "offlineVideo.h"

/** 
Constructor for the OfflineVideo class 
*/
OfflineVideo::OfflineVideo(){

}

/**
Destructor for the OfflineVideo class 
*/
OfflineVideo::~OfflineVideo(){

}

/**
This function sets video file name 

@param FileName
@return void
*/
void OfflineVideo::setFileName(char* val){
	specs_.file_name = val;
}

/**
This function initializes the video 

@param void
@return true on success false if failed
*/ 
bool OfflineVideo::init(){

	if ((capture_handle_ = cvCreateFileCapture( specs_.file_name.c_str() )) != NULL ){
	
		// Set video specs.
		specs_.n_frames	= (int)cvGetCaptureProperty( capture_handle_, 
											CV_CAP_PROP_FRAME_COUNT );
		specs_.frame_width = (int)cvGetCaptureProperty( capture_handle_,
											CV_CAP_PROP_FRAME_WIDTH );
		specs_.frame_height = (int)cvGetCaptureProperty( capture_handle_,
											CV_CAP_PROP_FRAME_HEIGHT);
		specs_.frame_rate  = (int)cvGetCaptureProperty( capture_handle_, 
											CV_CAP_PROP_FPS );

		slider_position_ = 0; 
		current_frame_   = 0; 
		n_captures_		 = 0; 

		return true; // Success
	}
	else
		return false;  // Fail
}

/**
This function gets the video specs. 

@param pointer to a VideoSpecs structure
@return void
*/
void OfflineVideo::getVideoSpecs(VideoSpecs* s){
	memcpy( s, &specs_, sizeof(specs_) );
}

/**
This function plays the initialized video.

@param void
@return void
*/
void OfflineVideo::playVideo(){
	
	// create the window
	cvNamedWindow( specs_.file_name.c_str(), CV_WINDOW_KEEPRATIO );
	// create and attach a trackbar to the window
	if( specs_.n_frames != 0 ){
		cvCreateTrackbar("Frames", specs_.file_name.c_str(),
			&slider_position_, specs_.n_frames, on_trackerBar_slide);
		current_frame_ = slider_position_; 
	}
	
	//Read video and playback
	while(current_frame_ < specs_.n_frames ){
		frame_ = cvQueryFrame( capture_handle_ );
		if( !frame_ ){
			std::cerr << "OfflineVideo: Bad frame" << std::endl;
			break; 
		}

		//display frame
		cvShowImage( specs_.file_name.c_str(), frame_ );
		cvSetTrackbarPos("Frames", specs_.file_name.c_str(), ++current_frame_); 
		key_ = cvWaitKey(((int)(1000/specs_.frame_rate)));

	}

}


/**
The callback for trackbar position change

@param position
@return void
*/
//void OfflineVideo::on_trackerBar_slide(int pos){
/*	if( pos > current_frame_+2 || pos < current_frame_-2){
		cvSetCaptureProperty( capture_handle_, CV_CAP_PROP_POS_FRAMES, pos );	
		current_frame_ = pos;
	}*/
//}

