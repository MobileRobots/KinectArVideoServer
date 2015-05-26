
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

  /** The cv::Mat::convertTo() method is used. 
    @param input Normally a cv::Mat object is provided for @a input but cv::InputArray type
    allows other OpenCV types to also be used. 
    @param pixelscale Use to scale pixel values to unsigned char values (0, 255)
used by ArVideo.   For example, to convert floating point values (0.0, 1.0),
pass 255.0.  If your image is already in (0,255) pixel values, omit or specify
1.0.
    @param convertchannelstorgb If @a input is not in RGB channel
    format, pass a CV_???2RGB code as @a convertchannelstorgb (this is used with
    the <tt>cvtColor</tt> OpenCV function).  For example, if
    the image is in BGR channel order (common in OpenCV), then specify
    CV_BGR2RGB. IF the image is one channel (greyscale), specify CV_GRAY2RGB. If
    HSV, specify CV_HSV2RGB. See OpenCV documetation for <tt>cvtColor</tt> for full
    list of codes and further details:
    http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html#cvtcolor
  */
  virtual bool updateVideoDataCopy(cv::InputArray input, double pixelscale = 1.0, int convertchannelstorgb = 0)
  {
    lock();
    cv::Mat tmp;
    input.getMat().convertTo(tmp, CV_8U, pixelscale);
    if(convertchannelstorgb > 0)
      cvtColor(tmp, myCvData, convertchannelstorgb);
    // XXX BUG this is turning stuff blue?
    myWidth = myCvData.cols;
    myHeight = myCvData.rows;
   // printf("ArVideoOpenCV: update %dx%d\n  scaled %f\n", myWidth, myHeight, scale);
    assert(input.getMat().cols == myCvData.cols);
    assert(input.getMat().rows == myCvData.rows);
    if(myWidth == -1 || myHeight == -1)
      return false;
    myData = myCvData.ptr();
    unlock();
    updated();

    /*
    lock();
    FILE *f = fopen("test.ppm", "w");
    assert(f);
    fprintf(f, "P6\n%d %d 255\n", myWidth, myHeight);
    fwrite(myData, 3,  myWidth*myHeight, f);
    fclose(f);
    unlock();
    */

    return true;
  }

  virtual unsigned char *getData() {
    return myData;
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
