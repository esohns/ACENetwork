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

#ifndef TEST_I_STREAM_T_H
#define TEST_I_STREAM_T_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_target.h"

#include "test_i_common.h"
#include "test_i_common_modules.h"
//#include "test_i_message.h"
//#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Message;
class Test_I_SessionMessage;

template <typename ConnectorType>
class Test_I_Stream_T
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_StreamState,
                        struct Test_I_StreamConfiguration,
                        Test_I_Statistic_t,
                        struct Net_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        struct Test_I_StreamModuleHandlerConfiguration,
                        struct Test_I_StreamSessionData, // session data
                        Test_I_StreamSessionData_t,      // session data container (reference counted)
                        ACE_Message_Block,
                        Test_I_Message,
                        Test_I_SessionMessage>
{
  typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_StreamState,
                        struct Test_I_StreamConfiguration,
                        Test_I_Statistic_t,
                        struct Net_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        struct Test_I_StreamModuleHandlerConfiguration,
                        struct Test_I_StreamSessionData, // session data
                        Test_I_StreamSessionData_t,      // session data container (reference counted)
                        ACE_Message_Block,
                        Test_I_Message,
                        Test_I_SessionMessage> inherited;

 public:
  Test_I_Stream_T ();
  virtual ~Test_I_Stream_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

  // *TODO*: re-consider this API
  void ping ();

 private:
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_StreamModuleHandlerConfiguration,
                                     ACE_Message_Block,
                                     Test_I_Message,
                                     Test_I_SessionMessage,
                                     Test_I_StreamSessionData_t,
                                     Test_I_ConnectionManager_t,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                                   // task synch type
                                         Common_TimePolicy_t,                            // time policy
                                         Stream_SessionId_t,                             // session id type
                                         struct Test_I_StreamSessionData,                // session data type
                                         enum Stream_SessionMessageType,                 // session event type
                                         struct Stream_ModuleConfiguration,              // module configuration type
                                         struct Test_I_StreamModuleHandlerConfiguration, // module handler configuration type
                                         Test_I_IStreamNotify_t,                         // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;                      // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_T (const Test_I_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_T& operator= (const Test_I_Stream_T&))

  // modules
  Test_I_Module_DHCPDiscoverH_Module    DHCPDiscover_;
  Test_I_Module_RuntimeStatistic_Module runtimeStatistic_;
  Test_I_Module_Marshal_Module          marshal_;
  TARGET_MODULE_T                       netTarget_;
};

// include template definition
#include "test_i_stream.inl"

#endif
