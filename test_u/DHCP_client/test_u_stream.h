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

#ifndef TEST_I_HTTP_GET_STREAM_H
#define TEST_I_HTTP_GET_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "stream_module_source.h"

#include "net_connection_manager.h"

#include "test_u_common.h"
#include "test_u_common_modules.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

// forward declarations
class Stream_IAllocator;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_U_Configuration,
                                 Test_U_ConnectionState,
                                 Test_U_RuntimeStatistic_t,
                                 ////////
                                 Test_U_UserData> Test_U_ConnectionManager_t;

template <typename ConnectorType>
class Test_U_DHCPClient_Stream_T
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        /////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        Stream_State,
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
  Test_U_DHCPClient_Stream_T ();
  virtual ~Test_U_DHCPClient_Stream_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_U_StreamConfiguration&); // configuration

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
                        Stream_State,
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
  typedef Stream_Module_Net_Source_T<ACE_SYNCH_MUTEX,
                                     ////
                                     Test_U_SessionMessage,
                                     Test_U_Message,
                                     ////
                                     Test_U_StreamModuleHandlerConfiguration,
                                     ////
                                     Stream_State,
                                     ////
                                     Test_U_StreamSessionData,
                                     Test_U_StreamSessionData_t,
                                     ////
                                     Test_U_RuntimeStatistic_t,
                                     ////
                                     Test_U_ConnectionManager_t,
                                     ConnectorType> SOURCE_WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                            // task synch type
                                         Common_TimePolicy_t,                     // time policy
                                         Stream_ModuleConfiguration,              // module configuration type
                                         Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                                         SOURCE_WRITER_T> SOURCE_MODULE_T;        // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_U_DHCPClient_Stream_T (const Test_U_DHCPClient_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_DHCPClient_Stream_T& operator= (const Test_U_DHCPClient_Stream_T&))

  // modules
  SOURCE_MODULE_T                       netSource_;
  Test_U_Module_Marshal_Module          marshal_;
  Test_U_Module_RuntimeStatistic_Module runtimeStatistic_;
  Test_U_Module_DHCPDiscover_Module     DHCPDiscover_;
  //Test_U_Module_Dump_Module           dump_;
};

// include template implementation
#include "test_u_stream.inl"

/////////////////////////////////////////

typedef Test_U_DHCPClient_Stream_T<Test_U_TCPConnector_t> Test_U_DHCPClient_Stream_t;
typedef Test_U_DHCPClient_Stream_T<Test_U_TCPAsynchConnector_t> Test_U_DHCPClient_AsynchStream_t;

#endif
