
#include <iostream>
#include <signal.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "Aria.h"
#include "ArVideo.h"
#include "ArVideoOpenCV.h"
#include "ArSystemStatus.h"


int main(int argc, char *argv[])
{

  Aria::init();
  ArVideo::init();


  ArArgumentParser argParser(&argc, argv);

  ArServerBase server;
  ArServerSimpleOpener openServer(&argParser);

  argParser.loadDefaultArguments();
  if(!Aria::parseArgs() || !argParser.checkHelpAndWarnUnparsed()) {
    Aria::logOptions();
    Aria::exit(-1);
  }


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



  ArVideoOpenCV slideshow("OpenCV_Slideshow");
  ArVideo::createVideoServer(&server, &slideshow, "OpenCV_Slideshow", "OpenCV_Slideshow");

  if(!openServer.open(&server))
  {
    std::cout << "error opening ArNetworking server" << std::endl;
    Aria::exit(5);
    return 5;
  }
  server.runAsync();
  std::cout << "ArNetworking server running on port " << server.getTcpPort() << std::endl;


  /* Main loop, load image, copy to ArVideo source */

  while(true)
  {
    for(int i = 1; i <= 10; ++i)
    {
      char filename[128];
      snprintf(filename, 128, "image%d.jpg", i);
      std::cout << "loading " << filename << std::endl;
      cv::Mat img = cv::imread(filename);
      printf("img size %dx%d\n", img.cols, img.rows);
//      cv::imshow("rgb", img);
//      cv::waitKey(100);

      if(!slideshow.updateVideoDataCopy(img, 1, CV_BGR2RGB))
        std::cout << "Warning error copying rgb data to ArVideo source" << std::endl;
  
    

      ArUtil::sleep(5000);
    }
  }


  Aria::exit(0);
  return 0;
}
