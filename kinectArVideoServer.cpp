/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2011 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */


#include <iostream>
#include <signal.h>

#include <opencv2/opencv.hpp>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/threading.h>

#include "Aria.h"
#include "ArVideo.h"
#include "ArVideoOpenCV.h"
#include "ArSystemStatus.h"

bool protonect_shutdown = false;

void sigint_handler(int s)
{
  protonect_shutdown = true;
}

libfreenect2::Freenect2Device *freenect_dev = NULL;

void shutdown_app()
{
  // TODO: restarting ir stream doesn't work!
  // TODO: bad things will happen, if frame listeners are freed before dev->stop() :(
  freenect_dev->stop();
  freenect_dev->close();
}

int main(int argc, char *argv[])
{

  Aria::init();
  ArVideo::init();

  Aria::addExitCallback(new ArGlobalFunctor(&shutdown_app));

  std::string program_path(argv[0]);
  size_t executable_name_idx = program_path.rfind("Protonect");

  std::string binpath = "/";

  if(executable_name_idx != std::string::npos)
  {
    binpath = program_path.substr(0, executable_name_idx);
  }


  /* Open Kinect */
  libfreenect2::Freenect2 freenect2;
  libfreenect2::Freenect2Device *dev = freenect2.openDefaultDevice();
  freenect_dev = dev;

  if(dev == 0)
  {
    std::cout << "no device connected or failure opening the default one!" << std::endl;
    return -1;
  }


  ArServerBase server;
  ArServerSimpleOpener openServer(NULL);
  



  /* Shutdown cleanly */
  signal(SIGINT,sigint_handler);
  protonect_shutdown = false;


  /* Set up libfreenect listeners, start capturing  */

  //libfreenect2::SyncMultiFrameListener listener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth);
  libfreenect2::SyncMultiFrameListener listener(libfreenect2::Frame::Color | libfreenect2::Frame::Depth);
  libfreenect2::FrameMap frames;

  dev->setColorFrameListener(&listener);
  dev->setIrAndDepthFrameListener(&listener);
  dev->start();

  std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
  std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;


  /* Set up ArNetworking services */
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



  ArVideoOpenCV kinectDepthSource("Kinect_Depth");
  ArVideo::createVideoServer(&server, &kinectDepthSource, "Kinect_Depth", "freenect2|Depth|OpenCV");
  ArVideoOpenCV kinectRGBSource("Kinect_RGB");
  ArVideo::createVideoServer(&server, &kinectRGBSource, "Kinect_RGB", "freenect2|RGB|OpenCV");

  if(!openServer.open(&server))
  {
    std::cout << "error opening ArNetworking server" << std::endl;
    Aria::exit(5);
    return 5;
  }
  server.runAsync();
  std::cout << "ArNetworking server running on port " << server.getTcpPort() << std::endl;


  /* Main loop, capture images from kinect, display, copy to ArVideo sources */

  bool first = true;
  while(!protonect_shutdown)
  {
    std::cout << "." << std::flush;
    listener.waitForNewFrame(frames);
    libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
//    libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
    libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

    // todo don't recreate Mat objects each time
    cv::Mat rgbm(rgb->height, rgb->width, CV_8UC3, rgb->data);
    cv::Mat rgbm_small(rgb->height, rgb->width, CV_8UC3);
    cv::resize (rgbm, rgbm_small, cv::Size(640, 480));

    cv::Mat depthm(depth->height, depth->width, CV_32FC1, depth->data);

    cv::imshow("rgb", rgbm_small);
//    cv::imshow("ir", cv::Mat(ir->height, ir->width, CV_32FC1, ir->data) / 20000.0f);
    cv::imshow("depth", depthm / 4500.0f);

    if(!kinectRGBSource.updateVideoDataCopy(rgbm_small, 1, CV_BGR2RGB))
      std::cout << "Warning error copying rgb data to ArVideo source" << std::endl;
    if(!kinectDepthSource.updateVideoDataCopy(depthm/4500.0f, 255,
/*(1/255.0),*/ CV_GRAY2RGB))
      std::cout << "Warning error copying depth data to ArVideo source" << std::endl;
  
    
    int key = cv::waitKey(1);

    protonect_shutdown = protonect_shutdown || (key > 0 && ((key & 0xFF) == 27)); // shutdown on escape

    listener.release(frames);
    //libfreenect2::this_thread::sleep_for(libfreenect2::chrono::milliseconds(100));

    if(first)
    {
      first = false;
      cv::moveWindow("rgb",   90, 85); 
      cv::moveWindow("depth", 90, 599);
    }
  }

  shutdown_app();

  return 0;
}
