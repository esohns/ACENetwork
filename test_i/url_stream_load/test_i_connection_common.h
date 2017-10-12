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

#include <map>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"
#include "ace/SSL/SSL_SOCK_Connector.h"
#include "ace/SSL/SSL_SOCK_Stream.h"

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

#include "test_i_common.h"

// forward declarations
struct Test_I_ConnectionConfiguration;
class Test_I_ConnectionStream;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct HTTP_Stream_UserData> Test_I_IConnectionManager_t;

//struct Test_I_SocketHandlerConfiguration
//  : HTTP_SocketHandlerConfiguration
//{
//  inline Test_I_SocketHandlerConfiguration ()
//    : HTTP_SocketHandlerConfiguration ()
//    ///////////////////////////////////////
//    , userData (NULL)
//  {};
//
//  struct Test_I_UserData* userData;
//};

//extern const char stream_name_string_[];
struct Test_I_URLStreamLoad_StreamConfiguration;
struct Test_I_URLStreamLoad_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AllocatorConfiguration,
                               struct Test_I_URLStreamLoad_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration> Test_I_URLStreamLoad_StreamConfiguration_t;
struct Test_I_URLStreamLoad_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  Test_I_URLStreamLoad_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , streamConfiguration (NULL)
   , userData (NULL)
  {
    PDUSize = NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
  };

  Test_I_IConnectionManager_t*                connectionManager;
  struct HTTP_SocketHandlerConfiguration      socketHandlerConfiguration;
  Test_I_URLStreamLoad_StreamConfiguration_t* streamConfiguration;

  struct HTTP_Stream_UserData*                userData;
};
typedef std::map<std::string,
                 struct Test_I_URLStreamLoad_ConnectionConfiguration> Test_I_URLStreamLoad_ConnectionConfigurations_t;
typedef Test_I_URLStreamLoad_ConnectionConfigurations_t::iterator Test_I_URLStreamLoad_ConnectionConfigurationIterator_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_URLStreamLoad_ConnectionConfiguration,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> Test_I_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct HTTP_SocketHandlerConfiguration,
                                Test_I_ConnectionStream,
                                enum Stream_StateMachine_ControlState> Test_I_IStreamConnection_t;

//////////////////////////////////////////

typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
                               ACE_SOCK_STREAM,
                               struct HTTP_SocketHandlerConfiguration> Test_I_TCPSocketHandler_t;
typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
                               ACE_SSL_SOCK_Stream,
                               struct HTTP_SocketHandlerConfiguration> Test_I_SSLTCPSocketHandler_t;
typedef Net_AsynchTCPSocketHandler_T<struct HTTP_SocketHandlerConfiguration> Test_I_AsynchTCPSocketHandler_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Test_I_TCPSocketHandler_t,
                                struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                struct HTTP_SocketHandlerConfiguration,
                                struct Net_ListenerConfiguration,
                                Test_I_ConnectionStream,
                                Common_Timer_Manager_t,
                                struct HTTP_Stream_UserData> Test_I_TCPConnection_t;
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Test_I_SSLTCPSocketHandler_t,
                                struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                struct HTTP_SocketHandlerConfiguration,
                                struct Net_ListenerConfiguration,
                                Test_I_ConnectionStream,
                                Common_Timer_Manager_t,
                                struct HTTP_Stream_UserData> Test_I_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_AsynchTCPSocketHandler_t,
                                      struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      struct HTTP_SocketHandlerConfiguration,
                                      struct Net_ListenerConfiguration,
                                      Test_I_ConnectionStream,
                                      Common_Timer_Manager_t,
                                      struct HTTP_Stream_UserData> Test_I_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_I_URLStreamLoad_ConnectionConfiguration> Test_I_IConnector_t;

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_I_URLStreamLoad_ConnectionConfiguration,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               struct Net_TCPSocketConfiguration,
                               struct HTTP_SocketHandlerConfiguration,
                               Test_I_ConnectionStream,
                               struct HTTP_Stream_UserData> Test_I_TCPConnector_t;
typedef Net_Client_SSL_Connector_T<Test_I_SSLTCPConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                   struct HTTP_ConnectionState,
                                   HTTP_Statistic_t,
                                   struct HTTP_SocketHandlerConfiguration,
                                   Test_I_ConnectionStream,
                                   struct HTTP_Stream_UserData> Test_I_SSLTCPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     struct Net_TCPSocketConfiguration,
                                     struct HTTP_SocketHandlerConfiguration,
                                     Test_I_ConnectionStream,
                                     struct HTTP_Stream_UserData> Test_I_AsynchTCPConnector_t;

#endif
