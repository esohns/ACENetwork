/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef DHCP_NETWORK_H
#define DHCP_NETWORK_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpsockethandler.h"
#include "net_tcpconnection_base.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "dhcp_common.h"
#include "dhcp_configuration.h"
//#include "dhcp_stream.h"
#include "dhcp_stream_common.h"

/////////////////////////////////////////

//typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_t,
//                                  ACE_INET_Addr,
//                                  struct DHCP_ConnectionConfiguration,
//                                  struct DHCP_ConnectionState,
//                                  DHCP_Statistic_t,
//                                  DHCP_Stream_t,
//                                  struct Net_UserData,
//                                  struct Stream_ModuleConfiguration,
//                                  struct DHCP_ModuleHandlerConfiguration> DHCP_TCPHandler_t;
//typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_t,
//                                        ACE_INET_Addr,
//                                        struct DHCP_ConnectionConfiguration,
//                                        struct DHCP_ConnectionState,
//                                        DHCP_Statistic_t,
//                                        DHCP_Stream_t,
//                                        struct Net_UserData,
//                                        struct Stream_ModuleConfiguration,
//                                        struct DHCP_ModuleHandlerConfiguration> DHCP_AsynchTCPHandler_t;
//typedef Net_TCPConnectionBase_T<DHCP_TCPHandler_t,
//                                struct DHCP_ConnectionConfiguration,
//                                struct DHCP_ConnectionState,
//                                DHCP_Statistic_t,
//                                DHCP_Stream_t,
//                                struct DHCP_SocketHandlerConfiguration,
//                                struct Net_UserData> DHCP_TCPConnection_t;
//typedef Net_AsynchTCPConnectionBase_T<DHCP_AsynchTCPHandler_t,
//                                      struct DHCP_ConnectionConfiguration,
//                                      struct DHCP_ConnectionState,
//                                      DHCP_Statistic_t,
//                                      DHCP_Stream_t,
//                                      struct DHCP_SocketHandlerConfiguration,
//                                      struct Net_UserData> DHCP_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct DHCP_ConnectionState,
                          DHCP_Statistic_t> DHCP_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                struct DHCP_ConnectionConfiguration,
                                struct DHCP_ConnectionState,
                                DHCP_Statistic_t,
                                struct DHCP_SocketHandlerConfiguration> DHCP_ISocketConnection_t;
//typedef Net_ISession_T<ACE_INET_Addr,
//                       struct Net_SocketConfiguration,
//                       struct DHCP_ConnectionConfiguration,
//                       struct DHCP_ConnectionState,
//                       DHCP_Statistic_t,
//                       DHCP_Stream> DHCP_ISession_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct DHCP_SocketHandlerConfiguration> DHCP_IConnector_t;
//typedef Net_Client_Connector_T<DHCP_TCPConnection_t,
//                               ACE_SOCK_CONNECTOR,
//                               ACE_INET_Addr,
//                               struct DHCP_ConnectionConfiguration,
//                               struct DHCP_ConnectionState,
//                               DHCP_Statistic_t,
//                               DHCP_Stream_t,
//                               struct DHCP_SocketHandlerConfiguration,
//                               struct Net_UserData> DHCP_Connector_t;
//typedef Net_Client_AsynchConnector_T<DHCP_AsynchTCPConnection_t,
//                                     ACE_INET_Addr,
//                                     struct DHCP_ConnectionConfiguration,
//                                     struct DHCP_ConnectionState,
//                                     DHCP_Statistic_t,
//                                     DHCP_Stream_t,
//                                     struct DHCP_SocketHandlerConfiguration,
//                                     struct Net_UserData> DHCP_AsynchConnector_t;

/////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct DHCP_ConnectionConfiguration,
                                 struct DHCP_ConnectionState,
                                 DHCP_Statistic_t,
                                 struct Net_UserData> DHCP_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct DHCP_ConnectionConfiguration,
                                 struct DHCP_ConnectionState,
                                 DHCP_Statistic_t,
                                 struct Net_UserData> DHCP_Connection_Manager_t;

typedef ACE_Singleton<DHCP_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> DHCP_CONNECTIONMANAGER_SINGLETON;

#endif
