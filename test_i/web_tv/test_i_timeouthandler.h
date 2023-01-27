/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef TEST_I_TIMEOUTHANDLER_H
#define TEST_I_TIMEOUTHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_timer_handler.h"

// forward declarations
struct Test_I_WebTV_ChannelSegment;
struct Common_AllocatorConfiguration;
class Stream_IAllocator;

class Test_I_TimeoutHandler
 : public Common_Timer_Handler
{
  typedef Common_Timer_Handler inherited;

 public:
  Test_I_TimeoutHandler ();
  inline virtual ~Test_I_TimeoutHandler () {}

  void initialize (ACE_HANDLE,                            // connection handle
                   struct Test_I_WebTV_ChannelSegment*,   // channel segment
                   const std::string&,                    // segment URL (if segment URLs are URIs)
                   struct Common_AllocatorConfiguration*, // connection allocator configuration
                   Stream_IAllocator*);                   // message allocator

  // implement specific behaviour
  virtual void handle (const void*); // asynchronous completion token

  ACE_Time_Value                        interval_;
  long                                  timerId_;
  ACE_Thread_Mutex*                     lock_;

 private:
//  ACE_UNIMPLEMENTED_FUNC (Test_I_TimeoutHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_TimeoutHandler (const Test_I_TimeoutHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_TimeoutHandler& operator= (const Test_I_TimeoutHandler&))

  ACE_HANDLE                            handle_;
  struct Test_I_WebTV_ChannelSegment*   segment_;
  std::string                           segmentURL_;
  struct Common_AllocatorConfiguration* allocatorConfiguration_;
  Stream_IAllocator*                    messageAllocator_;
};

#endif
