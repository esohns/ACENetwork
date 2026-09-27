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

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_data.h"

#include "stream_net_io_stream.h"

#include "net_common.h"
#include "net_connection_manager.h"
#include "net_socket_common.h"
#include "net_sock_connector.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "ws_common.h"
#include "ws_network.h"

#include "test_u_connection_stream.h"

// forward declarations
class Test_U_InboundSSDPConnectionStream;
class Test_U_OutboundSSDPConnectionStream;
class Test_U_WebSocket_ConnectionStream;
class Test_U_Message;
class WebSocket_Client_ConnectionConfiguration;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 WebSocket_Client_ConnectionConfiguration,
                                 struct WebSocket_ConnectionState,
                                 WebSocket_Statistic_t,
                                 struct Net_UserData> WebSocket_Client_ConnectionManager_t;

struct WebSocket_Client_StreamConfiguration;
struct WebSocket_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<struct WebSocket_Client_StreamConfiguration,
                               struct WebSocket_Client_ModuleHandlerConfiguration> WebSocket_Client_StreamConfiguration_t;
class WebSocket_Client_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<WebSocket_Client_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  WebSocket_Client_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {}
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct WebSocket_ConnectionState,
                          WebSocket_Statistic_t> WebSocket_Client_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                WebSocket_Client_ConnectionConfiguration,
                                struct WebSocket_ConnectionState,
                                WebSocket_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                WebSocket_Client_ConnectionStream,
                                enum Stream_StateMachine_ControlState> WebSocket_Client_IStreamConnection_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                WebSocket_Client_ConnectionConfiguration,
                                struct WebSocket_ConnectionState,
                                WebSocket_Statistic_t,
                                WebSocket_Client_ConnectionStream,
                                struct Net_UserData> WebSocket_Client_Connection_t;
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      WebSocket_Client_ConnectionConfiguration,
                                      struct WebSocket_ConnectionState,
                                      WebSocket_Statistic_t,
                                      WebSocket_Client_ConnectionStream,
                                      struct Net_UserData> WebSocket_Client_AsynchConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                WebSocket_Client_ConnectionConfiguration,
                                struct WebSocket_ConnectionState,
                                WebSocket_Statistic_t,
                                WebSocket_Client_ConnectionStream,
                                struct Net_UserData> WebSocket_Client_SSLConnection_t;
#endif // SSL_SUPPORT

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         WebSocket_Client_ConnectionConfiguration> WebSocket_Client_IConnector_t;

//////////////////////////////////////////

typedef Net_Client_AsynchConnector_T<WebSocket_Client_AsynchConnection_t,
                                     ACE_INET_Addr,
                                     WebSocket_Client_ConnectionConfiguration,
                                     struct WebSocket_ConnectionState,
                                     WebSocket_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_U_WebSocket_ConnectionStream,
                                     struct Net_UserData> WebSocket_Client_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               WebSocket_Client_Connection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               WebSocket_Client_ConnectionConfiguration,
                               struct WebSocket_ConnectionState,
                               WebSocket_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_U_WebSocket_ConnectionStream,
                               struct Net_UserData> WebSocket_Client_Connector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<WebSocket_Client_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   WebSocket_Client_ConnectionConfiguration,
                                   struct WebSocket_ConnectionState,
                                   WebSocket_Statistic_t,
                                   Test_U_WebSocket_ConnectionStream,
                                   struct Net_UserData> WebSocket_Client_SSLConnector_t;
#endif // SSL_SUPPORT

#endif
