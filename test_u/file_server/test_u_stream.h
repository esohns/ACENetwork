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

#ifndef TEST_U_STREAM_H
#define TEST_U_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_statemachine_control.h"

#include "stream_module_io_stream.h"

#include "test_u_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_configuration.h"

#include "file_server_stream_common.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_U_ControlMessage_t;

extern const char stream_name_string_[];

class Test_U_Stream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct FileServer_StreamState,
                                        struct FileServer_StreamConfiguration,
                                        Net_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Stream_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct FileServer_SessionData,
                                        FileServer_SessionData_t,
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        FileServer_InetConnectionManager_t,
                                        struct FileServer_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct FileServer_StreamState,
                                        struct FileServer_StreamConfiguration,
                                        Net_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Stream_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct FileServer_SessionData,
                                        FileServer_SessionData_t,
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        FileServer_InetConnectionManager_t,
                                        struct FileServer_UserData> inherited;

 public:
  Test_U_Stream ();
  virtual ~Test_U_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to the statistic report module
  virtual bool collect (Net_Statistic_t&); // return value: statistic data
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream (const Test_U_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream& operator= (const Test_U_Stream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

//////////////////////////////////////////

class Test_U_UDPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct FileServer_StreamState,
                                        struct FileServer_StreamConfiguration,
                                        Net_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Stream_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct FileServer_SessionData,
                                        FileServer_SessionData_t,
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        FileServer_InetConnectionManager_t,
                                        struct FileServer_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct FileServer_StreamState,
                                        struct FileServer_StreamConfiguration,
                                        Net_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Stream_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct FileServer_SessionData,
                                        FileServer_SessionData_t,
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        FileServer_InetConnectionManager_t,
                                        struct FileServer_UserData> inherited;

 public:
  Test_U_UDPStream ();
  virtual ~Test_U_UDPStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to the statistic report module
  virtual bool collect (Net_Statistic_t&); // return value: statistic data
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_UDPStream (const Test_U_UDPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_UDPStream& operator= (const Test_U_UDPStream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

#endif
