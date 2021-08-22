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

#include "common_configuration.h"
#include "common_statistic_handler.h"
#include "common_time_common.h"

#include "common_timer_manager_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_data.h"
#include "stream_statemachine_control.h"

#include "stream_net_io.h"
#include "stream_net_io_stream.h"

#include "net_connection_manager.h"

#include "http_common.h"

#include "test_u_common.h"

//#include "test_u_connection_common.h"
#include "test_u_HTTP_decoder_stream_common.h"

// forward declarations
struct Test_U_HTTPDecoder_StreamState;
struct Test_U_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
//struct Test_U_HTTPDecoder_SessionData;
//typedef Stream_SessionData_T<struct Test_U_HTTPDecoder_SessionData> Test_U_HTTPDecoder_SessionData_t;
class Test_U_Message;
class Test_U_SessionMessage;
struct Test_U_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_HTTPDecoder_StreamConfiguration,
                               struct Test_U_HTTPDecoder_ModuleHandlerConfiguration> Test_U_HTTPDecoder_StreamConfiguration_t;
//class Test_U_ConnectionConfiguration
// : public Net_ConnectionConfiguration_T<struct Common_Parser_FlexAllocatorConfiguration,
//                                        Test_U_StreamConfiguration_t,
//                                        NET_TRANSPORTLAYER_TCP>
//{
// public:
//  Test_U_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration_T ()
//  {}
//};
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_U_ConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> Test_U_ConnectionManager_t;

extern const char stream_name_string_[];

template <typename TimerManagerType> // implements Common_ITimer
class Test_U_Stream_T
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_HTTPDecoder_StreamState,
                                        struct Test_U_HTTPDecoder_StreamConfiguration,
                                        struct Stream_Statistic,
                                        TimerManagerType,
                                        struct Test_U_HTTPDecoder_ModuleHandlerConfiguration,
                                        struct Test_U_HTTPDecoder_SessionData,
                                        Test_U_HTTPDecoder_SessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_U_ConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_HTTPDecoder_StreamState,
                                        struct Test_U_HTTPDecoder_StreamConfiguration,
                                        struct Stream_Statistic,
                                        TimerManagerType,
                                        struct Test_U_HTTPDecoder_ModuleHandlerConfiguration,
                                        struct Test_U_HTTPDecoder_SessionData,
                                        Test_U_HTTPDecoder_SessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_U_ConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_U_Stream_T ();
  virtual ~Test_U_Stream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE);

  // override (part of) Common_IStatistic_T
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       struct Test_U_HTTPDecoder_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_U_HTTPDecoder_StreamState,
                                       struct Test_U_HTTPDecoder_SessionData,
                                       Test_U_HTTPDecoder_SessionData_t,
                                       struct Stream_Statistic,
                                       TimerManagerType,
                                       ACE_INET_Addr,
                                       Test_U_ConnectionManager_t,
                                       struct Stream_UserData> WRITER_T;
  typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       struct Test_U_HTTPDecoder_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_U_HTTPDecoder_StreamState,
                                       struct Test_U_HTTPDecoder_SessionData,
                                       Test_U_HTTPDecoder_SessionData_t,
                                       struct Stream_Statistic,
                                       TimerManagerType,
                                       ACE_INET_Addr,
                                       Test_U_ConnectionManager_t,
                                       struct Stream_UserData> READER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,                             // task synch type
                                Common_TimePolicy_t,                      // time policy
                                struct Test_U_HTTPDecoder_SessionData,    // session data type
                                enum Stream_SessionMessageType,           // session event type
                                struct Stream_ModuleConfiguration,        // module configuration type
                                struct Test_U_HTTPDecoder_ModuleHandlerConfiguration, // module handler configuration type
                                libacestream_default_net_io_module_name_string,
                                Stream_INotify_t,                         // stream notification interface type
                                READER_T,                                 // reader type
                                WRITER_T> IO_MODULE_T;                    // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream_T (const Test_U_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream_T& operator= (const Test_U_Stream_T&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

// include template definition
#include "test_u_stream.inl"

#endif
