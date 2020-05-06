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

#ifndef NET_CLIENT_CONNECTOR_COMMON_H
#define NET_CLIENT_CONNECTOR_COMMON_H

#include "ace/INET_Addr.h"
#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Connector.h"
#endif // SSL_SUPPORT

#include "net_connection_configuration.h"
#include "net_iconnector.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_stream.h"
#include "test_u_tcpconnection.h"
#include "test_u_udpconnection.h"

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_TCPConnectionConfiguration> Test_U_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_U_UDPConnectionConfiguration> Test_U_IUDPConnector_t;

typedef Net_Client_AsynchConnector_T<Test_U_AsynchTCPConnection,
                                     ACE_INET_Addr,
                                     Test_U_TCPConnectionConfiguration,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_U_Stream,
                                     struct Net_UserData> Client_TCP_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_TCPConnection,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_U_TCPConnectionConfiguration,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_U_Stream,
                               struct Net_UserData> Client_TCP_Connector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_U_SSLConnection,
                                   ACE_SSL_SOCK_Connector,
                                   Test_U_TCPConnectionConfiguration,
                                   struct Net_StreamConnectionState,
                                   Net_StreamStatistic_t,
                                   Test_U_Stream,
                                   struct Net_UserData> Client_SSL_Connector_t;
#endif // SSL_SUPPORT

typedef Net_Client_AsynchConnector_T<Test_U_AsynchUDPConnection,
                                     ACE_INET_Addr,
                                     Test_U_UDPConnectionConfiguration,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_U_Stream,
                                     struct Net_UserData> Client_UDP_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_U_UDPConnection,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_U_UDPConnectionConfiguration,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_U_Stream,
                               struct Net_UserData> Client_UDP_Connector_t;

#endif
