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

#ifndef NET_MODULE_SOCKETHANDLER_H
#define NET_MODULE_SOCKETHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"
#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_istatistic.h"

#include "stream_iallocator.h"
#include "stream_headmoduletask_base.h"
#include "stream_streammodule_base.h"

#include "net_stream_common.h"
#include "net_stream_configuration.h"
#include "net_sessionmessage.h"
#include "net_message.h"

class Net_Module_SocketHandler
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Net_StreamProtocolConfigurationState_t,
                                      Net_StreamConfiguration,
                                      Net_SessionMessage,
                                      Net_Message>
   // callback to trigger statistics collection...
 , public Common_IStatistic_T<Net_RuntimeStatistic_t>
{
 public:
  Net_Module_SocketHandler ();
  virtual ~Net_Module_SocketHandler ();

  // configuration / initialization
  bool init (Stream_IAllocator*, // message allocator
             unsigned int,       // session id
             bool = false,       // active object ?
             unsigned int = 0);  // statistics collecting interval (second(s))
                                 // 0 --> DON'T collect statistics

  // user interface
  // info
  bool isInitialized () const;
  unsigned int getSessionID () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Net_Message*&, // data message handle
                                  bool&);        // return value: pass message downstream ?

  // catch the session ID...
  virtual void handleSessionMessage (Net_SessionMessage*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistics collection
  virtual bool collect (Net_RuntimeStatistic_t&) const; // return value: (currently unused !)
  virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Net_StreamProtocolConfigurationState_t,
                                      Net_StreamConfiguration,
                                      Net_SessionMessage,
                                      Net_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Module_SocketHandler (const Net_Module_SocketHandler&));
  ACE_UNIMPLEMENTED_FUNC (Net_Module_SocketHandler& operator= (const Net_Module_SocketHandler&));

  // helper methods
  bool bisectMessages (Net_Message*&); // return value: complete message (chain)
//   Net_Message* allocateMessage(const unsigned int&); // requested size
  bool putStatisticsMessage (const Net_RuntimeStatistic_t&, // statistics info
                             const ACE_Time_Value&) const;  // statistics generation time

  bool                           isInitialized_;

  // timer stuff
  unsigned int                   statCollectionInterval_; // seconds
  Net_StatisticHandler_Reactor_t statCollectHandler_;
  long                           statCollectHandlerID_;

  // protocol stuff
  unsigned int                   currentMessageLength_;
  Net_Message*                   currentMessage_;
  Net_Message*                   currentBuffer_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,              // task synch type
                              Common_TimePolicy_t,       // time policy type
                              Net_Module_SocketHandler); // writer type

#endif
