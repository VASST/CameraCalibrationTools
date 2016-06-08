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
#include <ctime>
#include <conio.h> 

// Opencv includes
#include "cv.h"
#include "highgui.h"

// Boost inlcudes
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#define STEREO	115
#define MONO	109

/** 
 * Get local time and write to a string
 * @return time string
 */
void captureFrame(cv::Mat *frameL, cv::Mat *frameR, 
							cv::VideoCapture *captureL,
								cv::VideoCapture *captureR){
	
	bool status[2] = {captureL->read(*frameL), 
									captureR->read(*frameR)};

	try{
		if(status[0] && status[1]){// if stereo
			// Loop infinitely
			for(;;){
				// capture from the webcam
				(*captureL) >> *frameL;
				(*captureR) >> *frameR;
			}
		}
		else if(status[0]){ // if mono
			for(;;){ 
				//capture from the wabcam
				(*captureL) >> *frameL;
			}
		}
		else{ 
			std::cerr << "Cannot read camera feed" << std::endl;
			return; 
		}
	}
	catch(boost::thread_interrupted&){
		// Stop capturing
		std::cout << "Capturing stopped" << std::endl;
		return;
	}
}

// get current time
std::string getTime(){
	time_t t = time(0); 
	struct tm* now = localtime( &t );

	std::stringstream ss;
	ss << (now->tm_year + 1900) 
         << (now->tm_mon + 1)
         <<  now->tm_mday << "T" 
		 << now->tm_hour
		 << now->tm_min 
		 << now->tm_sec;

	return ss.str();
}


int main(int argc, char** argv){

	if(argc < 4 || argc > 4){
		//print usage message
		std::cout << "Usage:\tCapture_Video.exe framerate port1 port2\n"
			      << "\tframerate: video framerate\n"
				  << "\tport1: port for the left camera\n"
				  << "\tport2: port for the right camera"<< std::endl;
		return 0;
	}

	std::cout << "Video capture tool" << std::endl;
	std::cout << "Key strokes:" << std::endl;
	std::cout << "\tSpace\t- start\\stop saving video\n";
	std::cout << "\ts\t- select stereo stream\n";
	std::cout << "\tm\t- select mono stream\n";
	std::cout << std::endl;	

	char* filename;// = "output.mp4";
	float frame_rate = (float)atof(argv[1]); // frame rate 
	int port1(atoi(argv[2])), port2(atoi(argv[3]));

	int codec = CV_FOURCC('D','I','V','X'); 

	cv::Mat frame1, frame2, side_by_side; // video frames.
	//capture devices
	cv::VideoCapture capture[2];
	capture[0].open(port1); // left camera
	capture[1].open(port2); // right camera
	cv::VideoWriter video_writer;

	// camera availability flags
	bool cam_idx[2] = {true, true};

	// check if the device is open
	if(!capture[0].isOpened()){
		std::cerr << "Camera No. 1 can not be initialized"
				  << std::endl;
		cam_idx[0] = false;
	}
	if(!capture[1].isOpened()){
		std::cerr << "Camera No. 2 can not be initialized"
				  << std::endl;
		cam_idx[1] = false;
	}

	if(!cam_idx[0] && !cam_idx[1]){
		std::cerr << "No camera was inialized" << std::endl;
		return -1;
	}

	// Select video stream 
	char s, key;
	bool saving = false;
	std::string stream;
	do{
		std::cout << "Select stream("
				  << "Stereo(s)/Mono(m) --> ";

		std::cin >> (char)s;
		
	}while( s != STEREO && s != MONO);

	int img_width = capture[0].get(CV_CAP_PROP_FRAME_WIDTH);
	int img_height= capture[0].get(CV_CAP_PROP_FRAME_HEIGHT);

	cv::namedWindow("Input Stream", CV_WINDOW_KEEPRATIO);

	// Experiemental code
	boost::posix_time::time_duration td, td1;
	boost::posix_time::ptime nextFrameTimeStamp, 
							 currentFrameTimeStamp, 
							 initialLoopTimeStamp, 
							 finalLoopTimeStamp;
	int delayFound = 0;
	int totalDelay = 0; 

	// initialize initial timestamps
	nextFrameTimeStamp = boost::posix_time::microsec_clock::local_time();
	currentFrameTimeStamp = nextFrameTimeStamp;
	td = (currentFrameTimeStamp - nextFrameTimeStamp);

	// start the thread
	boost::thread captureThread(captureFrame, &frame1, &frame2, 
									&capture[0], &capture[1]);

	// infinite loop 
	while(true){
			
			while(td.total_microseconds() < 1000000.0/frame_rate){
				// determine current elapsed time
				currentFrameTimeStamp = boost::posix_time::microsec_clock::local_time();
				td = (currentFrameTimeStamp - nextFrameTimeStamp);
			}

			//determine time at start of write
			initialLoopTimeStamp = boost::posix_time::microsec_clock::local_time();

			//write frame
			if(s == MONO){

				// Show image
				cv::imshow("Input Stream", frame1);

				if(saving)
					video_writer << frame1;

			}
			else if( s == STEREO ){

				side_by_side = cv::Mat(img_height, 2*img_width, 
										frame1.type());
				frame1.copyTo(side_by_side(cv::Rect(0, 0, img_width, img_height)));
				// Flip the right eye. Original stream is flipped for some reason. 
				// Not sure why!!! 
				cv::Mat temp(frame2.rows, frame2.cols, frame2.type()); 
				frame2.copyTo(temp);
				cv::flip(temp,temp, 1);
				temp.copyTo(side_by_side(cv::Rect(img_width, 0, 
														img_width, img_height)));

				// Show image
				cv::imshow("Input Stream", side_by_side);

				if(saving)
					video_writer << side_by_side;		
			}			

			// check key stroke
			key = cv::waitKey(5);

			if ( key == 27 ){ // Quit
				std::cout << "Quiting" << std::endl;
				captureThread.interrupt();
				break;
			}
			else if(key == 32 ){ // start/stop saving

				if(!saving){
					std::string out_file_name, prefix("CAP_"), ext(".avi");
					out_file_name = prefix + getTime() + ext;

					video_writer = cv::VideoWriter(out_file_name, 
													codec, 
													frame_rate, 
													cvSize(img_width, img_height), 
													true);

					// Open the video writer
					if(!video_writer.isOpened()){
						std::cout << "Failed to open the file for writing" << std::endl;
						continue;
					}

					std::cout << "Saving video to " 
							  << out_file_name << std::endl;
					saving = true;
					}
					else{
						std::cout << "Saving stopped" << std::endl;
						saving = false;
					
					}
			}
			
			nextFrameTimeStamp = nextFrameTimeStamp + boost::posix_time::microsec(1000000.0/frame_rate);
			td = (currentFrameTimeStamp - nextFrameTimeStamp);

			finalLoopTimeStamp = boost::posix_time::microsec_clock::local_time();
			td1 = (finalLoopTimeStamp - initialLoopTimeStamp );
			delayFound = td1.total_milliseconds();

			// show image
			cv::imshow("Input Stream", frame1);

	}

	// Release resources
	capture[0].release();
	capture[1].release();

	frame1.release();
	frame2.release();

	return 0;
}
	

	
	