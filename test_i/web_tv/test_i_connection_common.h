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

#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Connector.h"
#include "ace/SSL/SSL_SOCK_Stream.h"
#endif // SSL_SUPPORT

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_data.h"

#include "stream_net_io_stream.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_socket_common.h"
#include "net_tcpconnection_base.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "http_common.h"
#include "http_network.h"

#include "test_i_common.h"

// forward declarations
class Test_I_ConnectionStream;
class Test_I_ConnectionStream_3;

struct Test_I_WebTV_StreamConfiguration;
struct Test_I_WebTV_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<struct Test_I_WebTV_StreamConfiguration,
                               struct Test_I_WebTV_ModuleHandlerConfiguration> Test_I_WebTV_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_WebTV_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_WebTV_ConnectionConfiguration_t;

//////////////////////////////////////////

struct Test_I_WebTV_StreamConfiguration_3;
struct Test_I_WebTV_ModuleHandlerConfiguration_3;
typedef Stream_Configuration_T<struct Test_I_WebTV_StreamConfiguration_3,
                               struct Test_I_WebTV_ModuleHandlerConfiguration_3> Test_I_WebTV_StreamConfiguration_3_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_WebTV_StreamConfiguration_3_t,
                                           NET_TRANSPORTLAYER_TCP> Test_I_WebTV_ConnectionConfiguration_3_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> Test_I_IConnection_t;

//////////////////////////////////////////

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                Test_I_WebTV_ConnectionConfiguration_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_I_ConnectionStream,
                                enum Stream_StateMachine_ControlState> Test_I_IStreamConnection_t;

//////////////////////////////////////////

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                Test_I_WebTV_ConnectionConfiguration_3_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_I_ConnectionStream_3,
                                enum Stream_StateMachine_ControlState> Test_I_IStreamConnection_3;

//////////////////////////////////////////

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_WebTV_ConnectionConfiguration_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_I_ConnectionStream,
                                struct Net_UserData> Test_I_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_WebTV_ConnectionConfiguration_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_I_ConnectionStream,
                                struct Net_UserData> Test_I_SSLConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_WebTV_ConnectionConfiguration_t,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      Test_I_ConnectionStream,
                                      struct Net_UserData> Test_I_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_WebTV_ConnectionConfiguration_t> Test_I_IConnector_t;

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_WebTV_ConnectionConfiguration_t,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_ConnectionStream,
                               struct Net_UserData> Test_I_TCPConnector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_I_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_I_WebTV_ConnectionConfiguration_t,
                                   struct HTTP_ConnectionState,
                                   HTTP_Statistic_t,
                                   Test_I_ConnectionStream,
                                   struct Net_UserData> Test_I_SSLConnector_t;
#endif // SSL_SUPPORT
typedef Net_Client_AsynchConnector_T<Test_I_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_WebTV_ConnectionConfiguration_t,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_ConnectionStream,
                                     struct Net_UserData> Test_I_AsynchTCPConnector_t;

//////////////////////////////////////////

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_WebTV_ConnectionConfiguration_3_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_I_ConnectionStream_3,
                                struct Net_UserData> Test_I_TCPConnection_3_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_WebTV_ConnectionConfiguration_3_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_I_ConnectionStream_3,
                                struct Net_UserData> Test_I_SSLConnection_3_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_WebTV_ConnectionConfiguration_3_t,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      Test_I_ConnectionStream_3,
                                      struct Net_UserData> Test_I_AsynchTCPConnection_3_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_WebTV_ConnectionConfiguration_3_t> Test_I_IConnector_3_t;

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_TCPConnection_3_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_WebTV_ConnectionConfiguration_3_t,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_ConnectionStream_3,
                               struct Net_UserData> Test_I_TCPConnector_3_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_I_SSLConnection_3_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_I_WebTV_ConnectionConfiguration_3_t,
                                   struct HTTP_ConnectionState,
                                   HTTP_Statistic_t,
                                   Test_I_ConnectionStream_3,
                                   struct Net_UserData> Test_I_SSLConnector_3_t;
#endif // SSL_SUPPORT
typedef Net_Client_AsynchConnector_T<Test_I_AsynchTCPConnection_3_t,
                                     ACE_INET_Addr,
                                     Test_I_WebTV_ConnectionConfiguration_3_t,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_ConnectionStream_3,
                                     struct Net_UserData> Test_I_AsynchTCPConnector_3_t;

#endif
