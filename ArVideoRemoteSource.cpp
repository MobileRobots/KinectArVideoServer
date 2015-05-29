
#include "ArVideo.h"
#include "ArVideoRemoteSource.h"
#include <assert.h>


/**
  @param name Must match name (prefix/suffix) of remote camera 
*/
ArVideoRemoteSource::ArVideoRemoteSource(ArServerBase *myserver, ArClientBase
*remoteclient, const char *name, int interval) : 
  myRemoteName(name), myName(name), myOpen(false), myUpdated(false), 
  myWidth(0), myHeight(0), 
  myJpegData(NULL), myDataLen(0),
  myVideoDataCallbacks("ArVideoRemoteSource RGB Video Data Callbacks"),
  myJpegVideoDataCallbacks("ArVideoRemoteSource JPEG Video Data Callbacks", ArLog::Normal),
  myRemotePictureCB(this, &ArVideoRemoteSource::remotePictureCB),
  myServePictureCB(this, &ArVideoRemoteSource::servePictureCB)
{
  myName += "_from_";
  myName += remoteclient->getHost();

  printf("myRemoteName %s\n", myRemoteName.c_str());
  char reqname[256];
  memset(reqname, 0, 256);
  snprintf(reqname, 256, "getPicture%s", myRemoteName.c_str());
  remoteclient->addHandler(reqname, &myRemotePictureCB);
  remoteclient->request(reqname, interval);
  ArLog::log(ArLog::Normal, "ArVideoRemoteSourcee: requesting %s at interval %d\n", reqname, interval);

  snprintf(reqname, 256, "getPicture%s", myName.c_str());
  myserver->addData(reqname, "forwarded image data from remote source", &myServePictureCB, "see source", "see source");
  
  // TODO check camera list for our camera, and start/stop forwarding if it
  // appears/disappears in list:
  //client->addHandler("getCameraList", &myCameraListCB);
  //client->addHandler("cameraListUpdated", &myCameraListChangedCB);
  //client->request("getCameraList");
}

ArVideoRemoteSource::~ArVideoRemoteSource() {
  lock();
  if(myJpegData)
    free(myJpegData);
  unlock();
}



/*
void ArVideoRemoteSource::cameraListChanged(ArNetPacket *pkt)
{
  client->request("getCameraList");
}

*/


void ArVideoRemoteSource::remotePictureCB(ArNetPacket *pkt)
{
  ArLog::log(ArLog::Normal, "ArVideoRemoteSource: got picture from remote");
  lock();
  myWidth = pkt->bufToUByte2();
  myHeight = pkt->bufToUByte2();
  myDataLen = pkt->getLength() - pkt->getReadLength();
  ArLog::log(ArLog::Normal, "ArVideoRemoteSource: got picture from remote %dx%d pixels, %d bytes of jpeg data.", myWidth, myHeight, myDataLen);
  myJpegData = (unsigned char*)realloc(myJpegData, myDataLen);
  pkt->bufToData(myJpegData, myDataLen);
  updated();
  unlock();
}

void ArVideoRemoteSource::updated() { 
  puts("remote source updated");
  myUpdateTime.setToNow();
  myUpdated = true; 
  /// TODO decode JPEG data and provide RGB (if there are any RGB callbacks):
  //myVideoDataCallbacks.invoke(myJpegData, myWidth, myHeight, myUpdateTime);
  myJpegVideoDataCallbacks.setLogging(true);
  myJpegVideoDataCallbacks.invoke(myJpegData, myWidth, myHeight, myDataLen);
}

void ArVideoRemoteSource::servePictureCB(ArServerClient *client, ArNetPacket *pkt)
{
puts("servePictureCB");
  lock();
  pkt->uByte2ToBuf(myWidth);
  pkt->uByte2ToBuf(myHeight);
  pkt->dataToBuf(myJpegData, myDataLen);
  unlock();
}



ArVideoRemoteForwarder::ArVideoRemoteForwarder(ArServerBase *serverbase, const char *remotehost, int remoteport) :
    myServer(serverbase),
    myGetCamerasCB(this, &ArVideoRemoteForwarder::getCamerasCB)
{
  if(!myRemoteClient.blockingConnect(remotehost, remoteport)) {
    ArLog::log(ArLog::Normal, "ArVideoRemoteForwarder: Error connecting to %s:%d", remotehost, remoteport);
    return;
  }
  myRemoteClient.runAsync();
  init();
}

ArVideoRemoteForwarder::~ArVideoRemoteForwarder()
{
  for(std::list<ArVideoRemoteSource*>::iterator i = myRemoteSources.begin(); i != myRemoteSources.end(); ++i)
    if(*i)
      delete(*i);
}

void ArVideoRemoteForwarder::init()
{
  myRemoteClient.addHandler("getCameraList", &myGetCamerasCB);
  assert(myRemoteClient.requestOnce("getCameraList"));
}

// receive from remote client
void ArVideoRemoteForwarder::getCamerasCB(ArNetPacket *pkt)
{
  int n = pkt->bufToByte2();
  printf("remote has %d cameras\n", n);
  char remotename[128];
  for(int i = 0; i < n; ++i)
  {
    memset(remotename, 0, 128);
    pkt->bufToStr(remotename, 128); // name

    // consume rest of fields
    // todo skip in netpacket instead of copying
    char s[128];
    pkt->bufToStr(s, 128); // type
    pkt->bufToStr(s, 128); // displayName
    pkt->bufToStr(s, 128); // displayType
    int ncmds = pkt->bufToByte2();
    int interval = -1;
    for(int c = 0; c < ncmds; ++c)
    {
      pkt->bufToStr(s, 128); // command
      if(strcmp(s, "getPicture") == 0)
      {
        pkt->bufToStr(s, 128); // consume this camera's commandName
        //printf("commandName=%s ", s);
        interval = pkt->bufToByte4();
        //printf("interval=%d\n", interval);
      }
    }

    ArVideoRemoteSource *source = new ArVideoRemoteSource(myServer, &myRemoteClient, remotename, interval);
    char forwardername[128];
    snprintf(forwardername, 128, "%s--remote", remotename);
    if(!ArVideo::createVideoServer(myServer, source, forwardername, "forwarder"))
    {
      puts("Error creating new video server for remote source!");
      delete(source);
      continue;
    }
    myRemoteSources.push_back(source);
    printf("created a video server with remote source %s\n", remotename);
  }
}


    
      

