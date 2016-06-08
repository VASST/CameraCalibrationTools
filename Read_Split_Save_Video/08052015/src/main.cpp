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
#include <string>
#include <list> 
#include <fstream>
#include "cv_includes.h" 

using namespace std;

#define PAUSE  32 //Space
#define STOP   27 //Esc
#define LEFT   114 //r
#define RIGHT  102 //f
#define SELECT 115 //s
#define SPLIT  120 //x

CvCapture* g_capture = NULL;
int g_slider_position= 0;
int current_frame = 0; 
int n_captured = 0; 
int n_split_captures = 0; 

void onTrackerbarSlide( int );
/*
 ReadVid <filename> <framerate> 
 Sample: ReadVid.exe ../capture-20140417T190127Z.avi 30
*/ 
int main(int argc, char** argv){

	if(argc < 2 || argc > 3){
		//print usage message
		std::cout << "Usage:\tReadVid.exe infile posefile\n"
				  << "Usage:\tReadVid.exe infile\n"
			      << "\tinfile: input video file\n"
				  << "\tposefile: pose file" << std::endl;
		return 0;
	}

	std::cout << "Video playback tool" << std::endl;
	std::cout << "Key strokes:" << std::endl;
	std::cout << "\tSpace\t-- pause video "<< std::endl;
	std::cout << "\tEsc\t-- stop video  " << std::endl;
	std::cout << "\tf\t-- advance one frame " << std::endl;
	std::cout << "\ts\t-- select and save frame " << std::endl;
	std::cout << "\tx\t-- split and save frame " << std::endl;
	std::cout << std::endl;

	char* filename = argv[1];
	char pose_line[256];
	bool hasPoses = false;
	fstream in_file, out_file;

	if( argv[2] != NULL ){
		hasPoses = true;
		
		in_file.open(argv[2], std::fstream::in);
		out_file.open("poses.csv", std::fstream::out);

		// copy the first line
		in_file.getline(pose_line, 256, '\n');
		out_file << pose_line << "\n";
	}

	//Create a window with a fixed aspect ratio
	cvNamedWindow(filename, CV_WINDOW_KEEPRATIO);
	g_capture = cvCreateFileCapture(filename);
	IplImage *frame, *left_img, *right_img; 
	int split_width, split_height;

	// Query capture properties
	int n_frames	   = (int)cvGetCaptureProperty( g_capture, CV_CAP_PROP_FRAME_COUNT );
	int frame_width    = (int)cvGetCaptureProperty( g_capture, CV_CAP_PROP_FRAME_WIDTH );
	int frame_height   = (int)cvGetCaptureProperty( g_capture, CV_CAP_PROP_FRAME_HEIGHT);
	int frame_rate     = (int)cvGetCaptureProperty( g_capture, CV_CAP_PROP_FPS );
	std::cout << " Playing back " << filename << " at " << frame_rate << "fps" 
			  << " [" << n_frames << ", " << frame_width << "x" << frame_width
			  << " frames]" << std::endl;

	if( n_frames != 0 ){
		cvCreateTrackbar("Position", filename, &g_slider_position, 
							n_frames, onTrackerbarSlide);
		current_frame = g_slider_position; 
	}

	//Read video and playback
	while( current_frame < n_frames ){

		if( !(frame = cvQueryFrame( g_capture ))){
			std::cout << "Bad frame" << std::endl;
			break;
		}

		if( current_frame == 0 ){
			split_width = frame->width/2;
			split_height= frame->height;
			//left and the right images. 
			left_img = cvCreateImage(cvSize(split_width, split_height), 
												frame->depth, frame->nChannels);
			right_img = cvCreateImage(cvSize(split_width, split_height), 
												frame->depth, frame->nChannels);
		}

		cvShowImage(filename, frame);
		//std::cout << "updating position 1: " << g_slider_position << std::endl;
		cvSetTrackbarPos( "Position", filename, ++current_frame);
		char c = cvWaitKey((int)(1000/frame_rate));

		if( c == STOP ){
				std::cout << "Video stopped" << std::endl;
				break;
		}
		else if( c == PAUSE ){
			std::cout << "Video paused" << std::endl;
			c = cvWaitKey(-1);

			do{
				if( c == LEFT ){ //navigate to the left
					cvSetTrackbarPos( "Position", filename, --current_frame);
					//std::cout << "updating position 3: " << g_slider_position << std::endl;
					//cvSetCaptureProperty( g_capture, CV_CAP_PROP_POS_FRAMES, current_frame );	

					frame = cvQueryFrame( g_capture );
					cvShowImage( filename, frame );
				}
				else if( c == RIGHT ){ // navigate to the right
					cvSetTrackbarPos( "Position", filename, ++current_frame);
					//std::cout << "updating position 4: " << g_slider_position << std::endl;
					//cvSetCaptureProperty( g_capture, CV_CAP_PROP_POS_FRAMES, current_frame );
					frame = cvQueryFrame( g_capture );
					cvShowImage( filename, frame );
				}
				else if( c == SPLIT ){ //split the frame and save

					std::string left_img_name, right_img_name, prefix("./captures/CAP_");
					char img_num[3];
					itoa(n_split_captures, img_num, 10);
					left_img_name = prefix + img_num + "L.png";
					right_img_name = prefix + img_num + "R.png";

					// Split image and copy. 
					cvSetImageROI(frame, cvRect(0, 0, split_width, split_height));
					cvCopyImage(frame, left_img);
					cvResetImageROI(frame);
					cvSetImageROI(frame, cvRect(split_width, 0, split_width, split_height));
					cvCopyImage(frame, right_img);
					cvResetImageROI(frame);

					cvNamedWindow("Left", CV_WINDOW_KEEPRATIO);
					cvNamedWindow("Right", CV_WINDOW_KEEPRATIO);

					cvShowImage("Left", left_img);
					cvShowImage("Right", right_img);

					// save the images to the location in prefix variable
					cvSaveImage( left_img_name.c_str(), left_img );
					cvSaveImage( right_img_name.c_str(), right_img );

					std::cout << "Saved " << (n_split_captures+1) << " pairs of split images" << std::endl;
					n_split_captures++;
				}
				else if( c == SELECT ){
					std::string img_name, prefix("./captures/CAP_IMG");
					char img_num[3];
					itoa(n_captured, img_num, 10);
					img_name = prefix + img_num + ".png";

					cvSaveImage( img_name.c_str() , frame);

					// Get the pose corresponding to this frame
					if( hasPoses ){
						in_file.clear();
						in_file.seekg(0, ios::beg);
						for(int i=0; i<=current_frame+1; i++)
							in_file.getline(pose_line, 256, '\n');

						out_file << pose_line << "\n";
					}

					std::cout << "Captured " << (n_captured+1) << " frame" << std::endl;
					n_captured++;
				}
			}while( (c=cvWaitKey( -1 )) != PAUSE );
			//std::cout << "Frame no. " << g_slider_position << std::endl;
		}
	}

	//Release resources
	cvReleaseImage(&left_img );
	cvReleaseImage(&right_img );
	cvReleaseCapture( &g_capture ); 
	cvDestroyWindow( filename );
	
	return 0; 
}

/* call back for the trackerbar */
void onTrackerbarSlide(int pos){

	if( pos > current_frame+2 || pos < current_frame-2){
		cvSetCaptureProperty( g_capture, CV_CAP_PROP_POS_FRAMES, pos );	
		current_frame = pos;
		std::cout << pos << std::endl;
	}
}