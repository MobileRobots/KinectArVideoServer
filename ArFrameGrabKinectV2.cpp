/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2014 Adept Technology

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Aria.h"
#include "ArExport.h"
#include "ArFrameGrabKinectV2.h"
#include "ArJpegUtil.h"
#include "ArCurlUtil.h"

#include "libfreenect2/libfreenect2.hpp"
#include "libfreenect2/frame_listener_impl.h"

//#define AR_FRAMEGRAB_KINECTV2_DEBUG 1

#ifdef AR_FRAMEGRAB_KINECTV2_DEBUG
#define PRINTDEBUG(s) { ArLog::log(ArLog::Normal, "DEBUG ArFrameGrabKinectV2: " s ); }
#define PRINTDEBUG1(s, a) { ArLog::log(ArLog::Normal, "DEBUG ArFrameGrabKinectV2: " s, a); }
//#define PRINTDEBUG2(s, a, b) { ArLog::log(ArLog::Normal, "DEBUG ArFrameGrabKinectV2: " s " (%s:%s)", a, b, __FILE__, __LINE__); }
//#define PRINTDEBUG3(s, a, b, c) { ArLog::log(ArLog::Normal, "DEBUG ArFrameGrabKinectV2: " s " (%s:%s)", a, b, c, __FILE__, __LINE__); }
#else
#define PRINTDEBUG(s) { }
#define PRINTDEBUG1(s, a) {  }
#define PRINTDEBUG2(s, a, b) { }
#define PRINTDEBUG3(s, a, b, c) { }
#endif


bool ArFrameGrabKinectV2::ourFreenectInitialized = false;
libfreenect2::Freenect2Device *ArFrameGrabKinectV2::ourFreenectDevice = NULL;

ArFrameGrabAbstract *createRGBKinect(size_t i, ArVideoParams params, ArArgumentParser *args)
{
  return new ArFrameGrabKinectV2("kinectv2_rgb");
}
ArVideoConnector::GlobalFrameGrabCreateFunc createeRGBKinectFunc(&createRGBKinect);

ArFrameGrabAbstract * createDepthKinect(size_t i, ArVideoParams params, ArArgumentParser *args)
{
  return new ArFrameGrabKinectV2("kinectv2_depth");
}
ArVideoConnector::GlobalFrameGrabCreateFunc createeRGBKinectFunc(&createRGBKinect);

ArFrameGrabAbstract *createIRKinect(size_t i, ArVideoParams params, ArArgumentParser *args)
{
  return new ArFrameGrabKinectV2("kinectv2_ir");
}
ArVideoConnector::GlobalFrameGrabCreateFunc createeIRKinectFunc(&createIRKinect);


AREXPORT void ArFrameGrabKinectV2::init()
{
  ArVideoConnector::registerFrameGrabberType("kinectv2_rgb", &createRGBKinectFunc, "Color image stream from KinectV2 camera");
  ArVideoConnector::registerFrameGrabberType("kinectv2_depth", &createDepthKinectFunc, "Greyscale depthmap stream from KinectV2");
  ArVideoConnector::registerFrameGrabberType("kinectv2_ir", &createIRKinectFunc, "Greyscale image stream from KinectV2 infrared camera");
}

//=============================================================================
// Constructor
AREXPORT ArFrameGrabKinectV2::ArFrameGrabKinectV2(const char* imageType) :
  myTypeName(imageType),
  myFrameType(0),
  myHeight(0), myWidth(0),
  myRgbData(NULL),
  myFrameListenerPtr(NULL),
  myFramesPtr(NULL)
  
{

  myRgbData = new unsigned char[MAX_WIDTH*MAX_HEIGHT*3];

  myDataMutex.setLogName("ArFrameGrabKinectV2");

  if(myTypeName == "kinectv2_rgb")
    myFrameType = libfreenect2::Frame::Color;
  else if(myTypeName == "kinectv2_depth")
    myFrameType = libfreenect2::Frame::Depth;
  else if(myTypeName == "kinectv2_ir")
    myFrameType = libfreenect2::Frame::Ir;
  else
  {
    ArLog::log(ArLog::Terse, "ArFrameGrabKinectV2: Error: invalid image type \"%s\".  Must be one of: kinectv2_depth, kinectv2_rgb, kinectv2_ir", myTypeName.c_str());
    myFrameType = 0;
    return;
  }

  myFrameListenerPtr = new libfreenect2::SyncMultiFrameListener(myFrameType);
  myFramesPtr = new libfreenect2::FrameMap;
  
  ArLog::log(ArLog::Normal, "ArFrameGrabKinectV2: Created %s video driver", myTypeName.c_str());
}

AREXPORT ArFrameGrabKinectV2::~ArFrameGrabKinectV2()
{
  close();
  if(myRgbData) delete[] myRgbData;
  if(myFrameListenerPtr) delete myFrameListenerPtr;
  if(myFramesPtr) delete myFramesPtr;
}
  


//=============================================================================
// Open the framegrabber.  Determine the parameters later
AREXPORT bool ArFrameGrabKinectV2::open() 
{
  PRINTDEBUG1("open called myTypeName.c_str()=%s", myTypeName.c_str());

  if(!openFreenect()) 
  {
    ArLog::log(ArLog::Terse, "ArFrameGrabKinectV2: Error initializing libfreenect2!");
    return false;
  }

  return true;
}

AREXPORT void ArFrameGrabKinectV2::close() 
{
  // TODO remove listeners
  closeFreenect();
}

AREXPORT bool ArFrameGrabKinectV2::openFreenect()
{
  ourDeviceMutex.lock();
  ourFreenectLib = new libfreenect2::Freenect2;
  ourFreenectDevice = ourFreenectLib.openDefaultDevice(); // todo make device interface type (opencl, opengl, cpu) configurable
  if(!ourFreenectDevice)
    return false;
  ourFreenectDevice->start();
  ++ourNumInstances;
  ourDeviceMutex.unlock();
}

AREXPORT bool ArFrameGrabKinect::closeFreenect()
{
  ourDeviceMutex.lock();
  if(--ourNumInstances == 0)
  {
    ourFreenectDevice->stop();
    ourFreenectDevice->close();
    delete ourFreenectDevice;
    delete ourFreenectLib;
  }
  if(ourNumInstances < 0)
    ourNumInstances = 0; // fix accidental double shutdown
  ourDeviceMutex.unlock();
  return true;
}

AREXPORT bool ArFrameGrabKinectV2::capture() 
{
  if(!myFrameListenerPtr) return false;

  // PROBLEM if three instances each call waitForNewFrame but only take one of
  // the three channels, then each instance will only get every third (or
  // whatever) frame.  Instead this needs to be done asynchronously and stored
  // for each ArFrameGrab instance to get. That thread will need to recreate the
  // listener if the set of desired image types changes.

  ourDeviceMutex.lock();

  myFrameListenerPtr->waitForNewFrame(*myFramesPtr);

  libfreenect2::Frame *frame = (*myFramesPtr)[myFrameType];
  myHeight = frame->height;
  myWidth = frame->width;
  myRgbData = rgb->data; 
  // todo copy and convert depending on type. rgb will be 8UC3. ir is 32FC1.
  // depth is 32FC1.    Or maybe we need to use opencv to convert (and maybe
  // downscale as well.)

  ourDeviceMutex.unlock();
}

// Generic parameter-setting routine
AREXPORT bool ArFrameGrabKinectV2::setParameter( const char * name, float value) 
{
  return false;
}

// Generic parameter-setting routine
AREXPORT bool ArFrameGrabKinectV2::getParameter( const char * name, float & value) 
{
  return false;
}

//=============================================================================
// Return a pointer to the data
AREXPORT unsigned char * ArFrameGrabKinectV2::dataPtr() 
{
  return myRgbData;
}

//=============================================================================
void ArFrameGrabKinectV2::help() 
{
  /*
  std::cout << "Kinect parameters:" << std::endl << std::endl;
  std::cout << "The following parameters accept values from 0-1 inclusive"
	    << std::endl << std::endl;
  std::cout << "  brightness" << std::endl;
  std::cout << "  contrast" << std::endl;
  std::cout << "  hue" << std::endl;
  std::cout << "  color" << std::endl << std::endl;
  std::cout << "All parameters keep their values after ACTS has quit." 
	    << std::endl << std::endl;
  */
}
