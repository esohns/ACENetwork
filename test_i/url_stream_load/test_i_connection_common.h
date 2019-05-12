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
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif

#include "http_common.h"
#include "http_network.h"

#include "test_i_common.h"

// forward declarations
struct Test_I_ConnectionConfiguration;
class Test_I_ConnectionStream;

//extern const char stream_name_string_[];
struct Test_I_URLStreamLoad_StreamConfiguration;
struct Test_I_URLStreamLoad_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Common_FlexParserAllocatorConfiguration,
                               struct Test_I_URLStreamLoad_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration> Test_I_URLStreamLoad_StreamConfiguration_t;
typedef Net_ConnectionConfiguration_T<struct Common_FlexParserAllocatorConfiguration,
                                      Test_I_URLStreamLoad_StreamConfiguration_t,
                                      NET_TRANSPORTLAYER_TCP> Test_I_URLStreamLoad_ConnectionConfiguration_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_URLStreamLoad_ConnectionConfiguration_t,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> Test_I_IConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                Net_TCPSocketConfiguration_t,
                                Test_I_ConnectionStream,
                                enum Stream_StateMachine_ControlState> Test_I_IStreamConnection_t;

//////////////////////////////////////////

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_I_ConnectionStream,
                                Common_Timer_Manager_t,
                                struct Net_UserData> Test_I_TCPConnection_t;
#if defined (SSL_USE)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                Test_I_ConnectionStream,
                                Common_Timer_Manager_t,
                                struct Net_UserData> Test_I_SSLConnection_t;
#endif // SSL_USE
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                      struct HTTP_ConnectionState,
                                      HTTP_Statistic_t,
                                      Test_I_ConnectionStream,
                                      Common_Timer_Manager_t,
                                      struct Net_UserData> Test_I_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_URLStreamLoad_ConnectionConfiguration_t> Test_I_IConnector_t;

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_URLStreamLoad_ConnectionConfiguration_t,
                               struct HTTP_ConnectionState,
                               HTTP_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_ConnectionStream,
                               struct Net_UserData> Test_I_TCPConnector_t;
#if defined (SSL_USE)
typedef Net_Client_SSL_Connector_T<Test_I_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                   struct HTTP_ConnectionState,
                                   HTTP_Statistic_t,
                                   Test_I_ConnectionStream,
                                   struct Net_UserData> Test_I_SSLConnector_t;
#endif // SSL_USE
typedef Net_Client_AsynchConnector_T<Test_I_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                     struct HTTP_ConnectionState,
                                     HTTP_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_ConnectionStream,
                                     struct Net_UserData> Test_I_AsynchTCPConnector_t;

#endif
