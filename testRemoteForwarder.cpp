
#include <iostream>
#include <signal.h>


#include "Aria.h"
#include "ArVideo.h"
#include "ArSystemStatus.h"
#include "ArVideoRemoteSource.h"

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


  if(!openServer.open(&server))
  {
    std::cout << "error opening ArNetworking server" << std::endl;
    Aria::exit(5);
    return 5;
  }
  server.runAsync();
  std::cout << "ArNetworking server running on port " << server.getTcpPort() << std::endl;

  /* Set up ArNetworking services */

  ArVideoRemoteForwarder forwarder(&server, "localhost", 7070);
  if(!forwarder.connected())
  {
    std::cout << "ERror connecting to server localhost:7070" << std::endl;
    Aria::exit(1);
  }
  std::cout << "ArNetworking server running on port " << server.getTcpPort() << std::endl;
  
  while(true) ArUtil::sleep(10000);

  Aria::exit(0);
  return 0;
}
