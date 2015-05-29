
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

#ifndef ARVIDEOREMOTESOURCE_H
#define ARVIDEOREMOTESOURCE_H

#include <map>

#include "Aria.h"
#include "ArNetworking.h"
#include "ArClientBase.h"
#include "ArVideoInterface.h"
#include "ArCallbackList4.h"


/** Forwards images from a camera source on another ArVideo server */
class ArVideoRemoteSource : public virtual ArVideoInterface {
protected:
  std::string myRemoteName;
  std::string myName;
  bool myOpen;
  bool myUpdated;
  int myWidth;
  int myHeight;
  ArTime myUpdateTime;
  bool myDataAllocated;
  unsigned char *myJpegData;
  unsigned int myDataLen;
  ArMutex myDataMutex;
  ArCallbackList4<unsigned char* , int, int, ArTime> myVideoDataCallbacks;
  ArCallbackList4<unsigned char* , int, int, unsigned int> myJpegVideoDataCallbacks;
  ArFunctor1C<ArVideoRemoteSource, ArNetPacket*> myRemotePictureCB;
  ArFunctor2C<ArVideoRemoteSource, ArServerClient* , ArNetPacket*> myServePictureCB;
public:
  /**
    @param name Must match name (prefix/suffix) of remote camera 
  */
  ArVideoRemoteSource(ArServerBase *myserver, ArClientBase *remoteclient, const
char *name, int requestInterval = -1);  
  virtual ~ArVideoRemoteSource() ;

protected:
  void updated(); 

  void lock() {
    myDataMutex.lock();
  }
  void unlock() { 
    myDataMutex.unlock();
  }
public:
  /// @internal
  void remotePictureCB(ArNetPacket *pkt);
  /// @internal
  void servePictureCB(ArServerClient *client, ArNetPacket *pkt);
public:
  virtual VideoFormat videoFormat() { return VIDEO_RGB24; }
  virtual int bytesPerPixel() { return 3; }
  virtual bool updateVideo() { return true; } 
  virtual bool updateVideoNow() { return myUpdated; } // todo could wait on a condition broadcast by updated
  virtual void addDataCallback(ArVideoDataCallback *functor) {
    myVideoDataCallbacks.addCallback(functor); // todo move to default implementation in ArVideoInterface?
  }
  virtual void remDataCallback(ArVideoDataCallback *functor) {
    myVideoDataCallbacks.remCallback(functor); // todo move to default implementation in ArVideoInterface?
  }
  virtual void addJpegDataCallback(ArJpegVideoDataCallback *functor) {
    printf("jpeg callback %s added\n", functor->getName());
    myJpegVideoDataCallbacks.addCallback(functor); // todo move to default implementation in ArVideoInterface?
  }
  virtual void remJpegDataCallback(ArJpegVideoDataCallback *functor) {
    printf("jpeg callback %s removed\n", functor->getName());
    myJpegVideoDataCallbacks.remCallback(functor); // todo move to default implementation in ArVideoInterface?
  }
  void open() {
    myOpen = true;
  }
  void close() {
    myOpen = false;
  }
  virtual bool isOpen() {
    return myOpen;
  }
  virtual int getWidth() {
    return myWidth;
  }
  virtual int getHeight() {
    return myHeight;
  }
  virtual unsigned char *getData() {
    return NULL;
  }
  virtual unsigned char *getJpegData() {
    // TODO add to ArVideoInterface?
    return myJpegData;
  }
  virtual bool hasJpegData() { return true; }
  virtual const char *getConfigSectionName() {
    return myName.c_str();
  }
  virtual const char *getVideoSize() {
    return "default";
  }
  virtual int getCaptureTimeSubtrahendMsecs() {
    // todo
    return 0;
  }
  virtual bool getCaptureTime(ArTime *t) {
    if(t) { 
      *t = myUpdateTime;
      return true;
    }
    return false;
  }
  virtual std::list<std::string> getCameraParameterNames() {
    return std::list<std::string>();
  }   
  virtual unsigned int getCameraParameterValue(const std::string& param) {
    return 0;
  }
  virtual ArPriority::Priority getMaxConfigParamPriority() {
    return ArPriority::NORMAL;
  }
};


/// Forward all images from cameras/sources one remote server
class ArVideoRemoteForwarder
{
public:
  ArVideoRemoteForwarder(ArServerBase *serverbase, const char* remotehost, int remoteport = 7070);
  // TODO ArVideoRemoteForwarder(ArServerBase *serverbase, ArClientBase *remoteclient);
  ~ArVideoRemoteForwarder();
  bool connected() { return myRemoteClient.isConnected(); }
  bool isConnected() { return myRemoteClient.isConnected(); }
protected:
  ArClientBase myRemoteClient;
  ArServerBase *myServer;
  ArFunctor1C<ArVideoRemoteForwarder, ArNetPacket*> myGetCamerasCB;
  void init();
  std::list<ArVideoRemoteSource*> myRemoteSources;
public:
  /// @internal
  void getCamerasCB(ArNetPacket *pkt);
};

#endif
 


