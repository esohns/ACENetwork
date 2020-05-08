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

#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Connector.h"
#endif

#include "common_configuration.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_session_data.h"

#include "stream_net_io_stream.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_sock_connector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "http_common.h"
#include "http_network.h"

#include "test_u_stream.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;

//////////////////////////////////////////

struct Test_U_StreamConfiguration;
struct Test_U_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_HTTPDecoder_StreamConfiguration,
                               struct Test_U_HTTPDecoder_ModuleHandlerConfiguration> Test_U_HTTPDecoder_StreamConfiguration_t;

typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_U_ConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> Test_U_IConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_U_ConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> Test_U_ConnectionManager_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_U_ConnectionConfiguration,
                          struct Net_StreamConnectionState,
                          HTTP_Statistic_t> Test_U_IConnection_t;

//////////////////////////////////////////

typedef Test_U_Stream_T<Common_Timer_Manager_t> Test_U_Stream_t;

//////////////////////////////////////////

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                Test_U_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                HTTP_Statistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_U_Stream_t,
                                enum Stream_StateMachine_ControlState> Test_U_IStreamConnection_t;
//typedef Net_IStreamConnection_T<ACE_INET_Addr,
//                                Test_U_ConnectionConfiguration_t,
//                                struct Net_StreamConnectionState,
//                                HTTP_Statistic_t,
//                                Net_TCPSocketConfiguration_t,
//                                Net_TCPSocketConfiguration_t,
//                                Test_U_AsynchStream_t,
//                                enum Stream_StateMachine_ControlState> Test_U_IAsynchStreamConnection_t;

//////////////////////////////////////////

//typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
//                               ACE_SOCK_STREAM,
//                               Net_TCPSocketConfiguration_t> Test_U_TCPSocketHandler_t;
//#if defined (SSL_USE)
//typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
//                               ACE_SSL_SOCK_Stream,
//                               Net_TCPSocketConfiguration_t> Test_U_SSLTCPSocketHandler_t;
//#endif // SSL_USE
//typedef Net_AsynchTCPSocketHandler_T<Net_TCPSocketConfiguration_t> Test_U_AsynchTCPSocketHandler_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_U_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                HTTP_Statistic_t,
                                Test_U_Stream_t,
                                struct Net_UserData> Test_U_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_U_ConnectionConfiguration,
                                struct Net_StreamConnectionState,
                                HTTP_Statistic_t,
                                Test_U_Stream_t,
                                struct Net_UserData> Test_U_SSLConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_U_ConnectionConfiguration,
                                      struct Net_StreamConnectionState,
                                      HTTP_Statistic_t,
                                      Test_U_Stream_t,
                                      struct Net_UserData> Test_U_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_ConnectionConfiguration> Test_U_IConnector_t;

//////////////////////////////////////////

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_U_ConnectionConfiguration,
                               struct Net_StreamConnectionState,
                               HTTP_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_U_Stream_t,
                               struct Net_UserData> Test_U_TCPConnector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_U_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_U_ConnectionConfiguration,
                                   struct Net_StreamConnectionState,
                                   HTTP_Statistic_t,
                                   Test_U_Stream_t,
                                   struct Net_UserData> Test_U_SSLConnector_t;
#endif // SSL_SUPPORT
typedef Net_Client_AsynchConnector_T<Test_U_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_U_ConnectionConfiguration,
                                     struct Net_StreamConnectionState,
                                     HTTP_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_U_Stream_t,
                                     struct Net_UserData> Test_U_TCPAsynchConnector_t;

#endif
