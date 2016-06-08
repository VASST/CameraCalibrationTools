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

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkNDITracker.h>
#include <vtkTrackerTool.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

#include "opencv_internals.h"

#define STEREO	115
#define MONO	109


/** 
 * Get local time and write to a string
 * @return time string
 */
void captureFrame(cv::Mat *frameL, cv::Mat *frameR, 
							OpenCVInternals *captureL,
								OpenCVInternals *captureR, bool stereo){
	
	try{
		if(stereo){// if stereo
			// Loop infinitely
			for(;;){
				// capture from the Endoscope
				*frameL = cv::cvarrToMat(captureL->grab_frame());
				*frameR = cv::cvarrToMat(captureR->grab_frame());
			}
		}
		else{ // if mono
			for(;;){ 
				//capture from the Endoscope
				*frameL = cv::cvarrToMat(captureL->grab_frame());
			}
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

	if(argc < 6 || argc > 6){
		//print usage message
		std::cout << "Usage:\tCapture_Video.exe framerate port1 port2\n"
			      << "\tframerate: video framerate\n"
				  << "\tport1: port for the left camera\n"
				  << "\tport2: port for the right camera\n"
				  << "\tcamera ROM path: path to the ROM file for the camera\n"
				  << "\tobject ROM path: path to the ROM file for the object"<< std::endl;
		return 0;
	}

	std::cout << "Video capture tool" << std::endl;
	std::cout << "Key strokes:" << std::endl;
	std::cout << "\tSpace\t- start\\stop saving video\n";
	std::cout << "\ts\t- select stereo stream\n";
	std::cout << "\tm\t- select mono stream\n";
	std::cout << std::endl;	

	char* filename;// = "output.mp4";
	float frame_rate = atof(argv[1]); // frame rate 
	int port1(atoi(argv[2])), port2(atoi(argv[3]));

	int codec = CV_FOURCC('D','I','V','X'); 

	cv::Mat frame1, frame2, side_by_side; // video frames.
	//Matrox Capture devices
	OpenCVInternals capture0 = OpenCVInternals(0, 0, port1); // left camera
	OpenCVInternals capture1 = OpenCVInternals(capture0.get_sysID(), capture0.get_appID(), port2);  // right camera
	cv::VideoWriter video_writer;

	//Tracker
	vtkSmartPointer< vtkNDITracker > tracker = vtkSmartPointer< vtkNDITracker >::New();
	vtkSmartPointer< vtkTrackerTool> cameraDRB = vtkSmartPointer< vtkTrackerTool >::New();
	vtkSmartPointer< vtkTrackerTool> objDRB = vtkSmartPointer< vtkTrackerTool >::New();
	vtkSmartPointer< vtkTransform > cameraTransform = vtkSmartPointer< vtkTransform >::New();
	vtkSmartPointer< vtkTransform > objectTransform = vtkSmartPointer< vtkTransform >::New();

	// Use 4 as the camera port and 5 as the object port
	int cameraPort(4), objPort(5);
	bool isTrackerInit(false);
	ofstream posesfile; // pose outputs
	long int frame_count(0);

	// init the tracker
	tracker->LoadVirtualSROM(cameraPort, argv[4]);
	tracker->LoadVirtualSROM(objPort, argv[5]);

	cameraDRB = tracker->GetTool( cameraPort );
	objDRB    = tracker->GetTool( objPort );

	if(tracker->Probe()){
		isTrackerInit = true;
		tracker->StartTracking();
	}
	else{
		isTrackerInit = false;
		return -1; // return if the tracker is not initialized.
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

	int img_width = cv::cvarrToMat(capture0.grab_frame()).cols;
	int img_height= cv::cvarrToMat(capture0.grab_frame()).rows;

	cv::namedWindow("Input Stream", CV_WINDOW_KEEPRATIO);

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
									&capture0, &capture1, true);

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
				frame2.copyTo(side_by_side(cv::Rect(img_width, 0, 
														img_width, img_height)));
				// Flip the image to account for the VTK->OpenCV axis change
				cv::flip(side_by_side, side_by_side, 0);
				cv::cvtColor(side_by_side, side_by_side, CV_RGB2BGR);
				// Show image
				cv::imshow("Input Stream", side_by_side);

				if(saving){
					
					// Save OTS measurements
					tracker->Update();
					if(!cameraDRB->IsMissing() && !cameraDRB->IsOutOfView() && !cameraDRB->IsOutOfVolume() &&
						!objDRB->IsMissing() && !objDRB->IsOutOfView() && !objDRB->IsOutOfVolume()){
						   
						   cameraTransform->DeepCopy( cameraDRB->GetTransform() );
						   objectTransform->DeepCopy( objDRB->GetTransform() );
						   video_writer << side_by_side;
						   
						   posesfile << frame_count << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(0,0) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(0,1) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(0,2) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(0,3) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(1,0) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(1,1) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(1,2) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(1,3) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(2,0) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(2,1) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(2,2) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(2,3) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(3,0) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(3,1) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(3,2) << ",";
						   posesfile << cameraTransform->GetMatrix()->GetElement(3,3) << ",";
	
						   posesfile << objectTransform->GetMatrix()->GetElement(0,0) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(0,1) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(0,2) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(0,3) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(1,0) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(1,1) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(1,2) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(1,3) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(2,0) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(2,1) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(2,2) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(2,3) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(3,0) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(3,1) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(3,2) << ",";
						   posesfile << objectTransform->GetMatrix()->GetElement(3,3) << "\n";

						   frame_count++;

				   }
				   else
					   std::cout << "Tool occlusion!" << std::endl;

				}
				//Release resources
				side_by_side.release();
			}			

			// check key stroke
			key = cv::waitKey(5);

			if ( key == 27 ){ // Quit
				std::cout << "Quiting" << std::endl;
				// stop acquiring data
				captureThread.interrupt();
				tracker->StopTracking();
				posesfile.close();
				break;
			}
			else if(key == 32 ){ // start/stop saving

				if(!saving){
					std::string out_file_name, prefix("CAP_"), ext(".avi");
					out_file_name = prefix + getTime() + ext;

					if(s == MONO)
						video_writer = cv::VideoWriter(out_file_name, 
														codec, 
														frame_rate, 
														cvSize(img_width, img_height), 
														true);
					else if(s == STEREO)
						video_writer = cv::VideoWriter(out_file_name, 
														codec, 
														frame_rate, 
														cvSize(img_width*2, img_height), 
														true);

					// Open the video writer
					if(!video_writer.isOpened()){
						std::cout << "Failed to open the file for writing" << std::endl;
						continue;
					}

					std::string poses_file_name, poses_file_ext(".csv");
					poses_file_name = prefix + getTime() + poses_file_ext;
					posesfile.open(poses_file_name.c_str());

					posesfile	 << "Frame No"
								 << ",e00, e01, e02, e03"
								 << ",e10, e11, e12, e13"
								 << ",e20, e21, e23, e23"
								 << ",e30, e31, e32, e33"
								 << ",p00, p01, p02, p03"
								 << ",p10, p11, p12, p13"
								 << ",p20, p21, p23, p23"
								 << ",p30, p31, p32, p33\n"; 

					std::cout << "Saving video to " 
							  << out_file_name << std::endl;
					std::cout << "Saving tracker data to " 
							  << poses_file_name << std::endl;
					saving = true;
					}
					else{
						std::cout << "Saving stopped" << std::endl;
						saving = false;
						frame_count = 0;
						// Call the destructor explicitly because there's no
						// .release() method for the cv::VideoWriter. 
						video_writer.~VideoWriter();
					
					}
			}
			
			nextFrameTimeStamp = nextFrameTimeStamp + boost::posix_time::microsec(1000000.0/frame_rate);
			td = (currentFrameTimeStamp - nextFrameTimeStamp);

			finalLoopTimeStamp = boost::posix_time::microsec_clock::local_time();
			td1 = (finalLoopTimeStamp - initialLoopTimeStamp );
			delayFound = td1.total_milliseconds();
	}

	// Release resources
	frame1.release();
	frame2.release();

	return 0;
}
	

	
	