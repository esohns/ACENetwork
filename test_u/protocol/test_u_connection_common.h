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

#ifndef TEST_I_CONNECTION_COMMON_H
#define TEST_I_CONNECTION_COMMON_H

#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Netlink_Addr.h"
#endif
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"

#include "stream_module_io_stream.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

// forward declarations
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;
struct Test_I_Configuration;
struct Test_I_ConnectionState;
typedef Stream_Statistic Test_I_RuntimeStatistic_t;
struct Test_I_Stream_Configuration;
struct Test_I_Stream_ModuleHandlerConfiguration;
struct Test_I_Stream_SessionData;
typedef Stream_SessionDataBase_T<Test_I_Stream_SessionData> Test_I_Stream_SessionData_t;
struct Test_I_Stream_State;
struct Test_I_UserData;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Configuration,
                                 Test_I_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_UserData> Test_I_Stream_InetConnectionManager_t;
struct Test_I_Stream_SocketHandlerConfiguration;

/////////////////////////////////////////

typedef Stream_Module_Net_IO_Stream_T<ACE_SYNCH_MUTEX,
                                      ///
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ///
                                      Stream_StateMachine_ControlState,
                                      Test_I_Stream_State,
                                      ///
                                      Test_I_Stream_Configuration,
                                      ///
                                      Test_I_RuntimeStatistic_t,
                                      ///
                                      Stream_ModuleConfiguration,
                                      Test_I_Stream_ModuleHandlerConfiguration,
                                      ///
                                      Test_I_Stream_SessionData,   // session data
                                      Test_I_Stream_SessionData_t, // session data container (reference counted)
                                      Test_I_Stream_SessionMessage,
                                      Test_I_Stream_Message,
                                      ///
                                      ACE_INET_Addr,
                                      Test_I_Stream_InetConnectionManager_t> Test_I_NetStream_t;

/////////////////////////////////////////

struct Test_I_ConnectionState
{
  inline Test_I_ConnectionState ()
   : configuration (NULL)
   , status (NET_CONNECTION_STATUS_INVALID)
   , currentStatistic ()
   , userData (NULL)
  {};

  Test_I_Configuration*     configuration;

  Net_Connection_Status     status;

  Test_I_RuntimeStatistic_t currentStatistic;

  Test_I_UserData*          userData;
};

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Configuration,
                          Test_I_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_IConnection_t;

/////////////////////////////////////////

// outbound
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Stream_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_I_Configuration,
                                  Test_I_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_NetStream_t,
                                  ///////
                                  Test_I_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_I_Stream_ModuleHandlerConfiguration> Test_I_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Stream_SocketHandlerConfiguration>,

                                        ACE_INET_Addr,
                                        Test_I_Configuration,
                                        Test_I_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_NetStream_t,

                                        Test_I_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_I_Stream_ModuleHandlerConfiguration> Test_I_AsynchTCPHandler_t;

typedef Net_TCPConnectionBase_T<Test_I_TCPHandler_t,
                                /////////
                                Test_I_Configuration,
                                Test_I_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_NetStream_t,
                                /////////
                                Test_I_Stream_SocketHandlerConfiguration,
                                /////////
                                Test_I_UserData> Test_I_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_AsynchTCPHandler_t,
                                      ///
                                      Test_I_Configuration,
                                      Test_I_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_NetStream_t,
                                      ///
                                      Test_I_Stream_SocketHandlerConfiguration,
                                      ///
                                      Test_I_UserData> Test_I_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Stream_SocketHandlerConfiguration> Test_I_Stream_IInetConnector_t;

/////////////////////////////////////////

// outbound
typedef Net_Client_AsynchConnector_T<Test_I_AsynchTCPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Configuration,
                                     Test_I_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_NetStream_t,
                                     ////
                                     Test_I_Stream_SocketHandlerConfiguration,
                                     ////
                                     Test_I_UserData> Test_I_Stream_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_TCPConnection_t,
                               Net_SOCK_Connector,
                               //////////
                               ACE_INET_Addr,
                               Test_I_Configuration,
                               Test_I_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_NetStream_t,
                               //////////
                               Test_I_Stream_SocketHandlerConfiguration,
                               //////////
                               Test_I_UserData> Test_I_Stream_TCPConnector_t;

#endif
