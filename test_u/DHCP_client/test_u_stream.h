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

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_target.h"

#include "test_u_common_modules.h"
//#include "test_u_message.h"
//#include "test_u_session_message.h"

#include "test_u_dhcp_client_common.h"

// forward declarations
class Stream_IAllocator;
class Test_U_Message;
class Test_U_SessionMessage;

template <typename ConnectorType>
class Test_U_Stream_T
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        Test_U_StreamConfiguration,
                        Test_U_RuntimeStatistic_t,
                        Stream_ModuleConfiguration,
                        Test_U_StreamModuleHandlerConfiguration,
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        DHCPClient_ControlMessage_t,
                        Test_U_Message,
                        Test_U_SessionMessage>
{
 public:
  Test_U_Stream_T ();
  virtual ~Test_U_Stream_T ();

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
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        Test_U_StreamConfiguration,
                        Test_U_RuntimeStatistic_t,
                        Stream_ModuleConfiguration,
                        Test_U_StreamModuleHandlerConfiguration,
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Test_U_Message,
                        Test_U_SessionMessage> inherited;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     Test_U_StreamModuleHandlerConfiguration,
                                     ACE_Message_Block,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     Test_U_StreamSessionData_t,
                                     Test_U_ConnectionManager_t,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                            // task synch type
                                         Common_TimePolicy_t,                     // time policy
                                         Stream_SessionId_t,                      // session id type
                                         Test_U_StreamSessionData,                // session data type
                                         Stream_SessionMessageType,               // session event type
                                         Stream_ModuleConfiguration,              // module configuration type
                                         Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                                         Test_U_IStreamNotify_t,                  // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;               // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream_T (const Test_U_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream_T& operator= (const Test_U_Stream_T&))

  // modules
  Test_U_Module_DHCPDiscoverH_Module    DHCPDiscover_;
  Test_U_Module_RuntimeStatistic_Module runtimeStatistic_;
  Test_U_Module_Marshal_Module          marshal_;
  TARGET_MODULE_T                       netTarget_;
};

// include template definition
#include "test_u_stream.inl"

#endif
