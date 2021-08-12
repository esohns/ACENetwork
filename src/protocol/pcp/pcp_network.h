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

#ifndef PCP_NETWORK_H
#define PCP_NETWORK_H

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

#include "pcp_common.h"
#include "pcp_configuration.h"
//#include "pcp_stream.h"
#include "pcp_stream_common.h"

/////////////////////////////////////////

//typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_t,
//                                  ACE_INET_Addr,
//                                  struct PCP_ConnectionConfiguration,
//                                  struct PCP_ConnectionState,
//                                  PCP_Statistic_t,
//                                  PCP_Stream_t,
//                                  struct Net_UserData,
//                                  struct Stream_ModuleConfiguration,
//                                  struct PCP_ModuleHandlerConfiguration> PCP_TCPHandler_t;
//typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_t,
//                                        ACE_INET_Addr,
//                                        struct PCP_ConnectionConfiguration,
//                                        struct PCP_ConnectionState,
//                                        PCP_Statistic_t,
//                                        PCP_Stream_t,
//                                        struct Net_UserData,
//                                        struct Stream_ModuleConfiguration,
//                                        struct PCP_ModuleHandlerConfiguration> PCP_AsynchTCPHandler_t;
//typedef Net_TCPConnectionBase_T<PCP_TCPHandler_t,
//                                struct PCP_ConnectionConfiguration,
//                                struct PCP_ConnectionState,
//                                PCP_Statistic_t,
//                                PCP_Stream_t,
//                                struct PCP_SocketHandlerConfiguration,
//                                struct Net_UserData> PCP_TCPConnection_t;
//typedef Net_AsynchTCPConnectionBase_T<PCP_AsynchTCPHandler_t,
//                                      struct PCP_ConnectionConfiguration,
//                                      struct PCP_ConnectionState,
//                                      PCP_Statistic_t,
//                                      PCP_Stream_t,
//                                      struct PCP_SocketHandlerConfiguration,
//                                      struct Net_UserData> PCP_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct PCP_ConnectionState,
                          PCP_Statistic_t> PCP_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                struct PCP_ConnectionConfiguration,
                                struct PCP_ConnectionState,
                                PCP_Statistic_t,
                                struct PCP_SocketHandlerConfiguration> PCP_ISocketConnection_t;
//typedef Net_ISession_T<ACE_INET_Addr,
//                       struct Net_SocketConfiguration,
//                       struct PCP_ConnectionConfiguration,
//                       struct PCP_ConnectionState,
//                       PCP_Statistic_t,
//                       PCP_Stream> PCP_ISession_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct PCP_SocketHandlerConfiguration> PCP_IConnector_t;
//typedef Net_Client_Connector_T<PCP_TCPConnection_t,
//                               ACE_SOCK_CONNECTOR,
//                               ACE_INET_Addr,
//                               struct PCP_ConnectionConfiguration,
//                               struct PCP_ConnectionState,
//                               PCP_Statistic_t,
//                               PCP_Stream_t,
//                               struct PCP_SocketHandlerConfiguration,
//                               struct Net_UserData> PCP_Connector_t;
//typedef Net_Client_AsynchConnector_T<PCP_AsynchTCPConnection_t,
//                                     ACE_INET_Addr,
//                                     struct PCP_ConnectionConfiguration,
//                                     struct PCP_ConnectionState,
//                                     PCP_Statistic_t,
//                                     PCP_Stream_t,
//                                     struct PCP_SocketHandlerConfiguration,
//                                     struct Net_UserData> PCP_AsynchConnector_t;

/////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct PCP_ConnectionConfiguration,
                                 struct PCP_ConnectionState,
                                 PCP_Statistic_t,
                                 struct Net_UserData> PCP_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct PCP_ConnectionConfiguration,
                                 struct PCP_ConnectionState,
                                 PCP_Statistic_t,
                                 struct Net_UserData> PCP_Connection_Manager_t;

typedef ACE_Singleton<PCP_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> PCP_CONNECTIONMANAGER_SINGLETON;

#endif
