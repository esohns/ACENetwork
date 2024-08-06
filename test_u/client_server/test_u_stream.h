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

#ifndef TEST_U_STREAM_T_H
#define TEST_U_STREAM_T_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_statemachine_control.h"

#include "stream_net_io_stream.h"

#include "net_common.h"

#include "test_u_common.h"
#include "test_u_configuration.h"
#include "test_u_connection_manager_common.h"
#include "test_u_message.h"
#include "net_client_stream_common.h"

// forward declarations
//typedef Stream_ControlMessage_T<enum Stream_ControlType,
//                                enum Stream_ControlMessageType,
//                                struct Common_Parser_FlexAllocatorConfiguration> Stream_ControlMessage_t;
class Test_U_SessionMessage;

extern const char stream_name_string_[];

template <typename ConnectionManagerType>
class Test_U_Stream_T
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_StreamState,
                                        struct Test_U_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct ClientServer_ModuleHandlerConfiguration,
                                        struct Test_U_StreamSessionData, // session data
                                        Test_U_StreamSessionData_t,      // session data container (reference counted)
                                        Stream_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        ConnectionManagerType,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_StreamState,
                                        struct Test_U_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct ClientServer_ModuleHandlerConfiguration,
                                        struct Test_U_StreamSessionData,
                                        Test_U_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        ConnectionManagerType,
                                        struct Stream_UserData> inherited;

 public:
  Test_U_Stream_T ();
  inline virtual ~Test_U_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // *TODO*: re-consider this API
  void ping ();

  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE); // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream_T (const Test_U_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream_T& operator= (const Test_U_Stream_T&))
};

// include template definition
#include "test_u_stream.inl"

#endif
