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

#ifndef TEST_U_CONNECTION_STREAM_H
#define TEST_U_CONNECTION_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_common.h"
#include "test_u_common_modules.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

// forward declarations
class Stream_IAllocator;

class Test_U_InboundConnectionStream
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        /////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        /////////////////
                        Test_U_StreamConfiguration,
                        /////////////////
                        Test_U_RuntimeStatistic_t,
                        /////////////////
                        Stream_ModuleConfiguration,
                        Test_U_StreamModuleHandlerConfiguration,
                        /////////////////
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        Test_U_SessionMessage,
                        Test_U_Message>
{
 public:
  Test_U_InboundConnectionStream (const std::string&); // name
  virtual ~Test_U_InboundConnectionStream ();

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_U_StreamConfiguration&, // configuration
                           bool = true,                       // setup pipeline ?
                           bool = true);                      // reset session data ?

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_U_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                        /////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        /////////////////
                        Test_U_StreamConfiguration,
                        /////////////////
                        Test_U_RuntimeStatistic_t,
                        /////////////////
                        Stream_ModuleConfiguration,
                        Test_U_StreamModuleHandlerConfiguration,
                        /////////////////
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        Test_U_SessionMessage,
                        Test_U_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_InboundConnectionStream ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_InboundConnectionStream (const Test_U_InboundConnectionStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_InboundConnectionStream& operator= (const Test_U_InboundConnectionStream&))

  // modules
  Test_U_Module_Net_IO_Module           netIO_;
  Test_U_Module_Marshal_Module          marshal_;
  //Test_U_Module_Parser_Module           parser_;
  Test_U_Module_RuntimeStatistic_Module runtimeStatistic_;
  Test_U_Module_DHCPDiscover_Module     DHCPDiscover_;
  Test_U_Module_Dump_Module             dump_;
};

////////////////////////////////////////////////////////////////////////////////

class Test_U_OutboundConnectionStream
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        /////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        /////////////////
                        Test_U_StreamConfiguration,
                        /////////////////
                        Test_U_RuntimeStatistic_t,
                        /////////////////
                        Stream_ModuleConfiguration,
                        Test_U_StreamModuleHandlerConfiguration,
                        /////////////////
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        Test_U_SessionMessage,
                        Test_U_Message>
{
 public:
  Test_U_OutboundConnectionStream (const std::string&); // name
  virtual ~Test_U_OutboundConnectionStream ();

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_U_StreamConfiguration&, // configuration
                           bool = true,                       // setup pipeline ?
                           bool = true);                      // reset session data ?

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_U_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                        /////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        /////////////////
                        Test_U_StreamConfiguration,
                        /////////////////
                        Test_U_RuntimeStatistic_t,
                        /////////////////
                        Stream_ModuleConfiguration,
                        Test_U_StreamModuleHandlerConfiguration,
                        /////////////////
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        Test_U_SessionMessage,
                        Test_U_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_OutboundConnectionStream ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_OutboundConnectionStream (const Test_U_OutboundConnectionStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_OutboundConnectionStream& operator= (const Test_U_OutboundConnectionStream&))

  // modules
  Test_U_Module_Net_IO_Module           netIO_;
  Test_U_Module_Streamer_Module         marshal_;
  Test_U_Module_RuntimeStatistic_Module runtimeStatistic_;
};

#endif
