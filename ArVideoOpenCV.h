#ifndef ARVIDEOOPENCV_H
#define ARVIDEOOPENCV_H

#include "ArVideoExternalSource.h"
#include "opencv2/opencv.hpp"
#include <assert.h>

/** Receives image data from OpenCV data structure (namely cv::Mat).
 * Use setVideoDataCopi() to copy the data.  It will be converted to 
 * 24-bit RGB byte array.
 * Uses OpenCV version 2.  Link code using this class to libopencv_core.
  
 * @todo XXX does ArVideoServer make a copy? If so does it do that during
 * update()? If so we can avoid copying here, and lock during update().
 */
class ArVideoOpenCV : public virtual ArVideoExternalSource
{
  cv::Mat myCvData;
public:

  ArVideoOpenCV(const char* name) : ArVideoExternalSource(name)
  {
  }

  /** The cv::Mat::convertTo() method is used. @a scale is passed as @a alpha.
    Normally a cv::Mat object is provided for @a input but cv::InputArray type
    allows other OpenCV types to also be used.
  */
  virtual bool updateVideoDataCopy(cv::InputArray input, double scale = 1)
  {
    lock();
    input.getMat().convertTo(myCvData, CV_8UC3, scale);
    myWidth = myCvData.cols;
    myHeight = myCvData.rows;
    assert(input.getMat().cols == myCvData.cols);
    assert(input.getMat().rows == myCvData.rows);
    if(myWidth == -1 || myHeight == -1)
      return false;
    unlock();
    updated();
    return true;
  }

  virtual unsigned char *getData() {
    return myCvData.ptr();
  }

  virtual bool setVideoDataPtr(unsigned char* ptr, int width, int height)
  {
    // TODO
    return false;
  }
  
  virtual bool updateVideoDataCopy(unsigned char *ptr, int width, int height)
  {
    // TODO
    return false;
  }
};

#endif
