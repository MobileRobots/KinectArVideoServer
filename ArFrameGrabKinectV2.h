/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2015 Adept Technology

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
#ifndef _ArFrameGrabKinectV2_H_
#define _ArFrameGrabKinectV2_H_

#include "Aria.h"
#include "ArFrameGrabAbstract.h"

#include "libfreenect2/libfreenect2.hpp"
#include "libfreenect2/frame_listener_impl.h"

//namespace libfreenect2 {
//  class Freenect2Device;
//  class SyncMultiFrameListener;
//  class FrameMap;
//  struct Frame {
//    enum Type;  
//  };
//}

/** Serve images taken directly from a Kinect Version 2 camera by libfreenect2.
 *
 * DON'T USE THIS, IT WON'T WORK AS WELL AS READING KINECT DATA SEPERATELY, AND 
 * PROVIDING IT TO ARVIDEO VIA ArVideoOpenCV OR ArVideoExternalData.
 *
 *  An instance of this class can grab either
 *  color images, depthmap images, or the IR camera images, and provide them
 *  through the ArFrameGrabAbstract interface, which can be used by
 *  ArVideoServer to provide them to remote clients.   The three image types
 *  are "kinectv2_rgb", "kinectv2_depth" and "kinectv2_ir".  Specify the image
 *  type in the constructor.   Static methods can be used to initialize the
 *  connection to the Kinect device in libfreenect2.  All instances of this class
 *  will share a static reference to the device through libfreenect2 but have
 *  separate "listeners".
 */
class ArFrameGrabKinectV2 : public ArFrameGrabAbstract {
public:

  // register this type with ArVideoConnector.
  AREXPORT static void init();

  /** Constructor
      @param imageType specify either "kinectv2_depth", "kinectv2_rgb", or "kinectv2_ir".
  */
  
  AREXPORT ArFrameGrabKinectV2(const char* imageType);

  AREXPORT virtual ~ArFrameGrabKinectV2();

  const char* getTypeName() { return myTypeName.c_str(); }

  /// Open the framegrabber.  Determine the parameters later
  AREXPORT bool open();

  /// Close the framegrabber
  AREXPORT void close();

  /// Grab a single frame
  AREXPORT bool capture();

  /// Generic parameter-setting routine
  AREXPORT bool setParameter(const char * name, float value);

  /// Get the value of a parameter
  AREXPORT bool getParameter(const char * name, float & value);

  /// Return the number of allowed parameters
  AREXPORT unsigned int numParameters();

  /// Return a character pointer to a specific parameter
  AREXPORT const char * parameterName(unsigned int idx);

  /// Return a pointer to the RGB data. May be NULL if there was an error capturing or decoding the image.
  AREXPORT unsigned char * dataPtr();

  /// A parameter help function that just prints out the information
  static void help();

  virtual int getWidth() { return myWidth; }
  virtual int getHeight() { return myHeight; }


protected:
  /// Image source identifier
  std::string myTypeName;
  libfreenect2::Frame::Type myFrameType;

  /// Height of the image in myRgbData
  int myHeight;

  /// Width of the image in myRgbData
  int myWidth;

  static ArMutex ourDeviceMutex;

  static bool openFreenect();
  static bool closeFreenect();
  static bool ourNumInstances;
  static libfreenect2::Freenect2Device *ourFreenectDevice;

  /// A local copy of the image that is formatted in an RGB byte order
  unsigned char *myRgbData;

  libfreenect2::SyncMultiFrameListener *myFrameListenerPtr;
  libfreenect2::FrameMap *myFramesPtr;


};


#endif
