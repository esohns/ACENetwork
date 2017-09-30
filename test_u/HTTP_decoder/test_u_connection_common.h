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

#ifndef TEST_U_CONNECTION_COMMON_H
#define TEST_U_CONNECTION_COMMON_H

#include <map>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/SSL/SSL_SOCK_Connector.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"

#include "stream_module_io_stream.h"
#include "stream_session_data.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#include "net_client_ssl_connector.h"

#include "http_common.h"
#include "http_network.h"

#include "test_u_stream.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;
struct Test_U_ConnectionConfiguration;
struct Test_U_ConnectionState;
struct Test_U_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
struct Test_U_HTTPDecoder_SessionData;
typedef Stream_SessionData_T<struct Test_U_HTTPDecoder_SessionData> Test_U_HTTPDecoder_SessionData_t;
struct Test_U_StreamState;
struct Test_U_UserData;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_U_ConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Test_U_UserData> Test_U_ConnectionManager_t;
struct Test_U_SocketHandlerConfiguration;

//////////////////////////////////////////

struct Test_U_ConnectionConfiguration;
struct Test_U_SocketHandlerConfiguration
 : HTTP_SocketHandlerConfiguration
{
  inline Test_U_SocketHandlerConfiguration ()
   : HTTP_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_U_ConnectionConfiguration* connectionConfiguration;

  struct Test_U_UserData*                userData;
};

struct Test_U_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline Test_U_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , socketHandlerConfiguration ()
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_U_SocketHandlerConfiguration socketHandlerConfiguration;
  Test_U_StreamConfiguration_t*            streamConfiguration;

  struct Test_U_UserData*                  userData;
};
typedef std::map<std::string,
                 struct Test_U_ConnectionConfiguration> Test_U_ConnectionConfigurations_t;
typedef Test_U_ConnectionConfigurations_t::iterator Test_U_ConnectionConfigurationIterator_t;

struct Test_U_ConnectionState
 : HTTP_ConnectionState
{
  inline Test_U_ConnectionState ()
   : HTTP_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  struct Test_U_ConnectionConfiguration* configuration;

  struct Test_U_UserData*                userData;
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_U_ConnectionConfiguration,
                          struct Test_U_ConnectionState,
                          HTTP_Statistic_t> Test_U_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                HTTP_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct Test_U_SocketHandlerConfiguration,
                                Test_U_Stream_t,
                                enum Stream_StateMachine_ControlState> Test_U_IStreamConnection_t;
//typedef Net_IStreamConnection_T<ACE_INET_Addr,
//                                struct Test_U_ConnectionConfiguration,
//                                struct Test_U_ConnectionState,
//                                HTTP_Statistic_t,
//                                struct Net_SocketConfiguration,
//                                struct Test_U_SocketHandlerConfiguration,
//                                Test_U_AsynchStream_t,
//                                enum Stream_StateMachine_ControlState> Test_U_IAsynchStreamConnection_t;

//////////////////////////////////////////

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_U_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  HTTP_Statistic_t,
                                  Common_Timer_Manager_t,
                                  Test_U_Stream_t,
                                  struct Test_U_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_TCPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_U_SocketHandlerConfiguration,
                                                         ACE_SSL_SOCK_Stream>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  HTTP_Statistic_t,
                                  Common_Timer_Manager_t,
                                  Test_U_Stream_t,
                                  struct Test_U_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_SSLTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        HTTP_Statistic_t,
                                        Common_Timer_Manager_t,
                                        Test_U_Stream_t,
                                        struct Test_U_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_AsynchTCPHandler_t;

typedef Net_TCPConnectionBase_T<Test_U_TCPHandler_t,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                HTTP_Statistic_t,
                                struct Test_U_SocketHandlerConfiguration,
                                struct Test_U_ListenerConfiguration,
                                Test_U_Stream_t,
                                struct Test_U_UserData> Test_U_TCPConnection_t;
typedef Net_TCPConnectionBase_T<Test_U_SSLTCPHandler_t,
                                struct Test_U_ConnectionConfiguration,
                                struct Test_U_ConnectionState,
                                HTTP_Statistic_t,
                                struct Test_U_SocketHandlerConfiguration,
                                struct Test_U_ListenerConfiguration,
                                Test_U_Stream_t,
                                struct Test_U_UserData> Test_U_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_U_AsynchTCPHandler_t,
                                      struct Test_U_ConnectionConfiguration,
                                      struct Test_U_ConnectionState,
                                      HTTP_Statistic_t,
                                      struct Test_U_SocketHandlerConfiguration,
                                      struct Test_U_ListenerConfiguration,
                                      Test_U_Stream_t,
                                      struct Test_U_UserData> Test_U_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_U_ConnectionConfiguration> Test_U_IConnector_t;

//////////////////////////////////////////

typedef Net_Client_Connector_T<Test_U_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_U_ConnectionConfiguration,
                               struct Test_U_ConnectionState,
                               HTTP_Statistic_t,
                               struct Net_TCPSocketConfiguration,
                               struct Test_U_SocketHandlerConfiguration,
                               Test_U_Stream_t,
                               struct Test_U_UserData> Test_U_TCPConnector_t;
typedef Net_Client_SSL_Connector_T<Test_U_SSLTCPConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   struct Test_U_ConnectionConfiguration,
                                   struct Test_U_ConnectionState,
                                   HTTP_Statistic_t,
                                   struct Test_U_SocketHandlerConfiguration,
                                   Test_U_Stream_t,
                                   struct Test_U_UserData> Test_U_SSLTCPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_U_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_U_ConnectionConfiguration,
                                     struct Test_U_ConnectionState,
                                     HTTP_Statistic_t,
                                     struct Net_TCPSocketConfiguration,
                                     struct Test_U_SocketHandlerConfiguration,
                                     Test_U_Stream_t,
                                     struct Test_U_UserData> Test_U_TCPAsynchConnector_t;

#endif
