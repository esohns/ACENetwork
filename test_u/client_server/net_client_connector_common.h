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

#include "net_iconnector.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_tcpconnection.h"
#include "test_u_udpconnection.h"

// forward declarations
class Test_U_Stream;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         ClientServer_ConnectionConfiguration_t> Client_IConnector_t;

typedef Net_Client_AsynchConnector_T<ClientServer_AsynchTCPConnection,
                                     ACE_INET_Addr,
                                     ClientServer_ConnectionConfiguration_t,
                                     struct ClientServer_ConnectionState,
                                     Test_U_Statistic_t,
                                     struct Net_TCPSocketConfiguration,
                                     struct ClientServer_SocketHandlerConfiguration,
                                     Test_U_Stream,
                                     struct Test_U_UserData> Client_TCP_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               ClientServer_TCPConnection,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               ClientServer_ConnectionConfiguration_t,
                               struct ClientServer_ConnectionState,
                               Test_U_Statistic_t,
                               struct Net_TCPSocketConfiguration,
                               struct ClientServer_SocketHandlerConfiguration,
                               Test_U_Stream,
                               struct Test_U_UserData> Client_TCP_Connector_t;

typedef Net_Client_AsynchConnector_T<ClientServer_AsynchUDPConnection,
                                     ACE_INET_Addr,
                                     ClientServer_ConnectionConfiguration_t,
                                     struct ClientServer_ConnectionState,
                                     Test_U_Statistic_t,
                                     struct Net_UDPSocketConfiguration,
                                     struct ClientServer_SocketHandlerConfiguration,
                                     Test_U_Stream,
                                     struct Test_U_UserData> Client_UDP_AsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               ClientServer_UDPConnection,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               ClientServer_ConnectionConfiguration_t,
                               struct ClientServer_ConnectionState,
                               Test_U_Statistic_t,
                               struct Net_UDPSocketConfiguration,
                               struct ClientServer_SocketHandlerConfiguration,
                               Test_U_Stream,
                               struct Test_U_UserData> Client_UDP_Connector_t;

#endif
