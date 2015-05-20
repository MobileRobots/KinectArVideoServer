/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology

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

//#include "Aria.h"
//#include "ArSystemStatus.h"

//#include "ArVideo.h"
//#include "ArVideoConnector.h"
//#include "ArPTZConnector.h"

//#include "ArServerBase.h"
//#include "ArServerHandlerCommMonitor.h"
//#include "ArSystemStatus.h"
//#include "ArServerInfoStrings.h"
//#include "ArServerHandlerCommands.h"

//#ifndef WIN32
//#include "ArServerFileUtils.h"
//#endif

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
//#include <libfreenect2/threading.h>

//#include "ArVideoOpenCV.h"


int main(int argc, char **argv)
{
#if 0
  Aria::init();
  ArVideo::init();  

  /* To get command line arguments: */
  ArArgumentParser argParser(&argc, argv);
  argParser.loadDefaultArguments();

  /* Used To set up ArNetworking server: */
  ArServerBase server;
  ArServerSimpleOpener simpleOpener(&argParser);


  /* Parse command line arguments */
  if (!Aria::parseArgs())
  {
    ArLog::log(ArLog::Terse, "kinectArVideoServer: Error parsing program arguments.");
    Aria::exit(1);
  }

  if(!argParser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(0);
    return 0;
  }


  /* Set up ArNetworking server with various other services */
  ArServerHandlerCommands commandsServer(&server);

#ifndef WIN32
  ArServerFileLister fileLister(&server, ".");
  ArServerFileToClient fileToClient(&server, ".");
  ArServerDeleteFileOnServer deleteFileOnServer(&server, ".");
#endif

  ArServerInfoStrings stringInfoServer(&server);
  Aria::getInfoGroup()->addAddStringCallback(stringInfoServer.getAddStringFunctor());
  ArSystemStatus::startPeriodicUpdate();
  Aria::getInfoGroup()->addStringDouble(
    "CPU Use", 10, ArSystemStatus::getCPUPercentFunctor(), "% 4.0f%%");
//  Aria::getInfoGroup()->addStringUnsignedLong(
//    "Computer Uptime", 14, ArSystemStatus::getUptimeFunctor());
//  Aria::getInfoGroup()->addStringUnsignedLong(
//    "Program Uptime", 14, ArSystemStatus::getProgramUptimeFunctor());
  Aria::getInfoGroup()->addStringInt(
    "Wireless Link Quality", 9, ArSystemStatus::getWirelessLinkQualityFunctor(), "%d");
  Aria::getInfoGroup()->addStringInt(
    "Wireless Noise", 10, ArSystemStatus::getWirelessLinkNoiseFunctor(), "%d");
  Aria::getInfoGroup()->addStringInt(
    "Wireless Signal", 10, ArSystemStatus::getWirelessLinkSignalFunctor(), "%d");
  ArServerHandlerCommMonitor commMonitorServer(&server);


  /* Create ArVideo sources and servers for kinect images */
  ArVideoOpenCV kinectDepthSource("Kinect_Depth"), kinectIRSource("Kinect_IR"), kinectRGBSource("Kinect_RGB");
  ArVideo::createVideoServer(&server, &kinectDepthSource, "Kinect_Depth", "Kinect_Depth");
  ArVideo::createVideoServer(&server, &kinectRGBSource, "Kinect_RGB", "Kinect_RGB");
  ArVideo::createVideoServer(&server, &kinectIRSource, "Kinect_IR", "Kinect_IR");
#endif
  
  /* Set up kinect */
  libfreenect2::Freenect2 freenect2;
  libfreenect2::Freenect2Device *kinectDev = freenect2.openDefaultDevice();

  if(kinectDev == 0)
  {
    std::cout << "no kinectDevice connected or failure opening the default one!" << std::endl;
    //Aria::exit(5);
    exit(5);
  }

  libfreenect2::SyncMultiFrameListener kinectListener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth);
  libfreenect2::FrameMap kinectFrames;

  kinectDev->setColorFrameListener(&kinectListener);
  kinectDev->setIrAndDepthFrameListener(&kinectListener);
  kinectDev->start();

  std::cout << "kinectDevice serial: " << kinectDev->getSerialNumber() << std::endl;
  std::cout << "kinectDevice firmware: " << kinectDev->getFirmwareVersion() << std::endl;

#if 0
  kinectDepthSource.open();
  kinectIRSource.open();
  kinectRGBSource.open();
#endif

  /* Run ArNetworking server */
#if 0
  if(!simpleOpener.open(&server))
  {
    ArLog::log(ArLog::Terse, "\nkinectArVideoServer: Error opening server on port %d!", server.getTcpPort());
    Aria::exit(2);
    return 2;
  }
  server.runAsync();
  ArLog::log(ArLog::Normal, "\nkinectArVideoServer: Server running and ready for client connections on port %d.\n", server.getTcpPort());
#endif
  
  /* Loop reading Kinect frames */
  while(true)
  {
    printf("."); fflush(stdout);
    kinectListener.waitForNewFrame(kinectFrames);
    libfreenect2::Frame *rgb = kinectFrames[libfreenect2::Frame::Color];
    libfreenect2::Frame *ir = kinectFrames[libfreenect2::Frame::Ir];
    libfreenect2::Frame *depth = kinectFrames[libfreenect2::Frame::Depth];

    cv::imshow("rgb", cv::Mat(rgb->height, rgb->width, CV_8UC3, rgb->data));
    cv::imshow("ir", cv::Mat(ir->height, ir->width, CV_32FC1, ir->data) / 20000.0f);
    cv::imshow("depth", cv::Mat(depth->height, depth->width, CV_32FC1, depth->data) / 4500.0f);

#if 0
    // scale them down and provide copies to ArVideo sources 
    kinectRGBSource.updateVideoDataCopy(cv::Mat(rgb->height, rgb->width, CV_8UC3, rgb->data) / 2000.0);
    kinectIRSource.updateVideoDataCopy(cv::Mat(ir->height, ir->width, CV_32FC1, ir->data) / 2000.0);
    kinectDepthSource.updateVideoDataCopy(cv::Mat(depth->height, depth->width, CV_32FC1, depth->data) / 4500.0);
#endif
    kinectListener.release(kinectFrames);
  }

  kinectDev->stop();
  kinectDev->close();


//  Aria::exit(0);
  return 0;
}




