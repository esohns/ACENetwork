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

#ifndef NET_SERVER_LISTENER_COMMON_H
#define NET_SERVER_LISTENER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#if defined (SSL_USE)
#include "ace/SSL/SSL_SOCK_Acceptor.h"
#endif // SSL_USE

#include "net_common.h"

#include "net_client_asynchconnector.h"
#include "net_server_asynchlistener.h"
#include "net_server_common.h"
#include "net_client_connector.h"
#include "net_server_listener.h"

#include "test_u_connection_common.h"
#include "test_u_stream.h"
#include "net_client_stream_common.h"
#include "test_u_tcpconnection.h"
#include "test_u_udpconnection.h"

typedef Net_Server_AsynchListener_T<Test_U_AsynchTCPConnection,
                                    ACE_INET_Addr,
                                    Test_U_TCPConnectionConfiguration,
                                    struct Net_StreamConnectionState,
                                    Test_U_Stream,
                                    struct Net_UserData> Server_Asynch_TCP_Listener_t;
typedef Net_Server_Listener_T<Test_U_TCPConnection,
                              ACE_SOCK_ACCEPTOR,
                              ACE_INET_Addr,
                              Test_U_TCPConnectionConfiguration,
                              struct Net_StreamConnectionState,
                              Test_U_Stream,
                              struct Net_UserData> Server_TCP_Listener_t;
#if defined (SSL_SUPPORT)
typedef Net_Server_Listener_T<Test_U_SSLConnection,
                              ACE_SSL_SOCK_Acceptor,
                              ACE_INET_Addr,
                              Test_U_TCPConnectionConfiguration,
                              struct Net_StreamConnectionState,
                              Test_U_Stream,
                              struct Net_UserData> Server_SSL_Listener_t;
#endif // SSL_SUPPORT

typedef Net_Client_AsynchConnector_T<Test_U_AsynchUDPConnection,
                                     ACE_INET_Addr,
                                     Test_U_UDPConnectionConfiguration,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_Stream,
                                     struct Net_UserData> Server_UDP_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_UDPConnection,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_U_UDPConnectionConfiguration,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_Stream,
                               struct Net_UserData> Server_UDP_Connector_t;

typedef ACE_Singleton<Server_Asynch_TCP_Listener_t,
                      ACE_SYNCH_MUTEX> SERVER_ASYNCH_TCP_LISTENER_SINGLETON;
typedef ACE_Singleton<Server_TCP_Listener_t,
                      ACE_SYNCH_MUTEX> SERVER_TCP_LISTENER_SINGLETON;
#if defined (SSL_SUPPORT)
typedef ACE_Singleton<Server_SSL_Listener_t,
                      ACE_SYNCH_MUTEX> SERVER_SSL_LISTENER_SINGLETON;
#endif // SSL_SUPPORT

#endif
