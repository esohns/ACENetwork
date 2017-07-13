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

// forward declarations
class Test_U_Stream;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_U_ConnectionConfiguration> Test_U_IConnector_t;

typedef Net_Client_AsynchConnector_T<Test_U_AsynchTCPConnection,
                                     ACE_INET_Addr,
                                     struct Test_U_ConnectionConfiguration,
                                     struct Test_U_ConnectionState,
                                     Net_RuntimeStatistic_t,
                                     struct Net_TCPSocketConfiguration,
                                     struct Test_U_SocketHandlerConfiguration,
                                     Test_U_Stream,
                                     struct Test_U_UserData> Test_U_Client_TCP_AsynchConnector_t;
typedef Net_Client_Connector_T<Test_U_TCPConnection,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct Test_U_ConnectionConfiguration,
                               struct Test_U_ConnectionState,
                               Net_RuntimeStatistic_t,
                               struct Net_TCPSocketConfiguration,
                               struct Test_U_SocketHandlerConfiguration,
                               Test_U_Stream,
                               struct Test_U_UserData> Test_U_Client_TCP_Connector_t;
//typedef Net_Client_AsynchConnector_T<Test_U_AsynchUDPConnection,
//                                     ACE_INET_Addr,
//                                     struct Test_U_ConnectionConfiguration,
//                                     struct Test_U_ConnectionState,
//                                     Net_RuntimeStatistic_t,
//                                     struct Net_UDPSocketConfiguration,
//                                     struct Test_U_SocketHandlerConfiguration,
//                                     Test_U_Stream,
//                                     struct Test_U_UserData> Test_U_Client_UDP_AsynchConnector_t;
//typedef Net_Client_Connector_T<Test_U_UDPConnection,
//                               ACE_SOCK_CONNECTOR,
//                               ACE_INET_Addr,
//                               struct Test_U_ConnectionConfiguration,
//                               struct Test_U_ConnectionState,
//                               Net_RuntimeStatistic_t,
//                               struct Net_UDPSocketConfiguration,
//                               struct Test_U_SocketHandlerConfiguration,
//                               Test_U_Stream,
//                               struct Test_U_UserData> Test_U_Client_UDP_Connector_t;

#endif
