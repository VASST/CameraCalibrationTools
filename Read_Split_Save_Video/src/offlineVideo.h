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

#ifndef __OFFLINE_VIDEO_H__
#define __OFFLINE_VIDEO_H__

#include <iostream>
#include <list>
#include "cv_includes.h"

/* Defines for Keystrokes */
#define PAUSE  32
#define STOP   27
#define LEFT   114
#define RIGHT  102
#define SELECT 115

/* Video Specs */ 
struct VideoSpecs{
	std::string file_name;
	int	n_frames;
	int	frame_rate;
	int	frame_height;
	int	frame_width;
};

/* OfflineVideo class definition */
class OfflineVideo{

public:


	OfflineVideo();
	~OfflineVideo();
	
	//public metods

	/* Set video file name */
	void setFileName(char* );

	/* Initialize */ 
	bool init();

	/* Get video specs */ 
	void getVideoSpecs(VideoSpecs* );

	/* Play video */ 
	void playVideo();

	/* Pause the video */ 
	void pauseVideo();

	/* Stop playing the video */
	void stopVideo();

	/* Is the video paused */ 
	void isPaused();

	/* Is the video stopped */ 
	void isStopped();

	/* Is the video playing */
	void isPlaying();

	/* Capture current frame */ 
	void captureFrame();

	/* Clear last captured frame from the list */
	void clearLastCaptureFrame();

	/* Number of frames captured */
	int getCaptureCount();

	/* Copy captured frames to IplImage Array */
	void copyCaptureList(IplImage* arr );

	/* Clear capture list */ 
	void clearCaptureList();


private:
	//private members
	CvCapture* capture_handle_;
	IplImage*  frame_;
	int		   current_frame_;
	int		   n_captures_;
	int		   slider_position_;
	char	   key_;
	std::list<IplImage> capture_img_list_;

	VideoSpecs specs_;

	// state variables
	bool	   is_playing_;

	//private methods
	static void on_trackerBar_slide( int pos)
	{

	}
};

#endif //__OFFLINE_VIDEO_H__