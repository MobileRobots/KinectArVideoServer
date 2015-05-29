
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

#ifndef ARVIDEOCBLIST4_H
#define ARVIDEOCBLIST4_H

#include "Aria.h"

template<class P1, class P2, class P3, class P4>
class ArCallbackList4 : public ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4>*>
{
public:
  ArCallbackList4(const char *name = "", 
			  ArLog::LogLevel logLevel = ArLog::Verbose,
			  bool singleShot = false) : 
    ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>(name, logLevel, singleShot)
    {
    }
  virtual ~ArCallbackList4()
    {
    }
  void invoke(P1 p1, P2 p2, P3 p3, P4 p4)
    {
      // references to members of parent class for clarity below
      ArMutex &mutex = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myDataMutex;
      ArLog::LogLevel &loglevel = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myLogLevel;
      const char *name = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myName.c_str();
	    std::multimap< int, ArFunctor4<P1, P2, P3, P4>* > &list = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myList; 
      bool &singleshot = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::mySingleShot;
      bool &logging = ArGenericCallbackList<ArFunctor4<P1, P2, P3, P4> *>::myLogging;
      
      mutex.lock();
      
      typename std::multimap<int, ArFunctor4<P1, P2, P3, P4> *>::iterator it;
      ArFunctor4<P1, P2, P3, P4> *functor;
      
      if(logging)
        ArLog::log( loglevel,  "%s: Starting calls", name);
      
      for (it = list.begin();  it != list.end(); ++it)
      {
        functor = (*it).second;
        if (functor == NULL) 
          continue;
	
        if(logging)
        {
          if (functor->getName() != NULL && functor->getName()[0] != '\0')
            ArLog::log(loglevel, "%s: Calling functor '%s' (0x%x) at %d", name, functor->getName(), functor, -(*it).first);
          else
            ArLog::log(loglevel, "%s: Calling unnamed functor (0x%x) at %d", name, functor, -(*it).first);
        }
        functor->invoke(p1, p2, p3, p4);
      }
      
      if(logging)
        ArLog::log(loglevel, "%s: Ended calls", name);
      
      if (singleshot)
      {
        if(logging)
          ArLog::log(loglevel, "%s: Clearing callbacks", name);
        list.clear();
      }

      mutex.unlock();
    }
};

#endif
