# KinectArVideoServer

kinectArVideo server reads RGB and Depth images from a version 2 Kinect RGBD sensor using libfreenect2.  It then 
uses OpenCV to resize and format conversion.  The resulting images are then provided to an ArVideo ArNetworking 
server (using ArVideo) for remote access (for example, they can be viewed in MobileEyes).

Use this as an example or basis for further work using the Kinect sensor.  Any intermediate or diagnostic images you 
generate in OpenCV can also be provided to the ArVideo server in the same manner as the (resized, converted) original
Kinect RGB and depth images currently are. 

ArVideo, ARIA, ArNetworking and MobileEyes: http://robots.mobilerobots.com
libfreenect2: http://github.com/OpenKinect/libfreenect2
OpenCV: http://opencv.org

More information about building and running libfreenect2 using OpenCL on Intel hardware can be found at
http://robots.mobilerobots.com/wiki/Kinect_version_2_Notes


