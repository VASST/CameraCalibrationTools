''' Python scripts to calibrate a single camera
    All Rights Resrved (c) Uditha Jayarathne 2014
'''

import os
import sys
#from termcolor import colored

## Use this function to invove Capture_Video.exe
# @param framerate
# @param port1 left camera port
# @param port2 right camera port
def capture_video(input1, input2, input3):
    print("Executing Capture_Video.exe")
    cmd = 'Capture_Video.exe' + ' ' + input1 + ' ' + input2 + ' ' + input3
    os.system(cmd)

    return


## Use this function to invoke ReadVid.exe
#  @param input video_to_be_playedback
#  @param output director to write the images
def read_and_split_video(input, output):
    # ceate directory to save captured images
    cmd = 'mkdir' + ' ' + output
    os.system(cmd)

    print("Executing ReavVid.exe")
    cmd = 'ReadVid.exe' + ' ' + input
    os.system(cmd)

    return


## Use this function to invoke CV_Calib_V1.exe
#  @param input name of the settings file
#  @param output name of the output file
def calibrate_camera(input, output, prefix):

    print("Executing CV_Calib_V1.exe for calibration")
    cmd = 'CV_Calib_V1.exe' + ' ' + input + ' ' + output + ' ' + prefix
    os.system(cmd)

    return
	
## Use this function to invoke CV_Stereo_Calib.exe
#  @param input name of the settings file
#  @param output file
def stereo_calibrate_cameras(input, output):

    print("Executing CV_Stereo_Calib.exe for stereo calibration")
    cmd = 'CV_Stereo_Calib.exe' + ' ' + input + ' ' + output
    os.system(cmd)
	
    return


# Script
os.system('cls')
while True:
    print("\n")
    print("Camera Calibration Tools")
    print("Key strokes:")
    print("\t1 - capture video from camera(s)")
    print("\t2 - read and split frames")
    print("\t3 - calibrate camera")
    print("\t4 - stereo calibrate")
    print("\t0 - quit")
    # Get user input
    key = int(input("-->"))

    if key == 1:
        capture_video('30', '1', '0')
        
    if key == 2:
        read_and_split_video('videos/CAP_2014923T184648.avi', 'captures')

    if key == 3:
		print("Select which camera to calibrate (L - 0/R - 1)")
		s = int(input("-->"))
		# Select which camera to calibrate
		if s == 0:
			calibrate_camera('settings.xml', 'left_calibration.xml', 'L')
		if s == 1:
			calibrate_camera('settings.xml', 'right_calibration.xml', 'R')
			
    if key == 4:
	stereo_calibrate_cameras('stereo_settings.xml', 'stereo_calibration.xml')
    
    if key == 0:
        print("Exiting the calibration tool.")
        break;
    




    


