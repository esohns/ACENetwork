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

#ifndef BITTORRENT_NETWORK_H
#define BITTORRENT_NETWORK_H

#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch_Traits.h>

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpsockethandler.h"
#include "net_tcpconnection_base.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "bittorrent_common.h"
//#include "bittorrent_configuration.h"
//#include "bittorrent_stream.h"
#include "bittorrent_stream_common.h"

//////////////////////////////////////////

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Net_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct BitTorrent_Configuration,
                                  struct BitTorrent_ConnectionState,
                                  BitTorrent_RuntimeStatistic_t,
                                  BitTorrent_Stream_t,
                                  struct Stream_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_ModuleHandlerConfiguration> BitTorrent_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Net_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_Configuration,
                                        struct BitTorrent_ConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_Stream_t,
                                        struct Stream_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_ModuleHandlerConfiguration> BitTorrent_AsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_TCPHandler_t,
                                struct BitTorrent_Configuration,
                                struct BitTorrent_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                BitTorrent_Stream_t,
                                struct Net_SocketHandlerConfiguration,
                                struct Stream_UserData> BitTorrent_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_AsynchTCPHandler_t,
                                      struct BitTorrent_Configuration,
                                      struct BitTorrent_ConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      BitTorrent_Stream_t,
                                      struct Net_SocketHandlerConfiguration,
                                      struct Stream_UserData> BitTorrent_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct BitTorrent_Configuration,
                          struct BitTorrent_ConnectionState,
                          BitTorrent_RuntimeStatistic_t> BitTorrent_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                struct BitTorrent_Configuration,
                                struct BitTorrent_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                BitTorrent_Stream_t,
                                enum Stream_StateMachine_ControlState,
                                struct Net_SocketConfiguration,
                                struct Net_SocketHandlerConfiguration> BitTorrent_ISocketConnection_t;
//typedef Net_ISession_T<ACE_INET_Addr,
//                       struct Net_SocketConfiguration,
//                       struct BitTorrent_Configuration,
//                       struct BitTorrent_ConnectionState,
//                       BitTorrent_RuntimeStatistic_t,
//                       BitTorrent_Stream_t> BitTorrent_ISession_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Net_SocketHandlerConfiguration> BitTorrent_IConnector_t;
//typedef Net_IConnector_T<ACE_INET_Addr,
//                         truct Net_ConnectorConfiguration> BitTorrent_IConnector_t;
typedef Net_Client_Connector_T<BitTorrent_TCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_Configuration,
                               struct BitTorrent_ConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               BitTorrent_Stream_t,
                               struct Net_SocketHandlerConfiguration,
                               struct Stream_UserData> BitTorrent__Connector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_Configuration,
                                     struct BitTorrent_ConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     BitTorrent_Stream_t,
                                     struct Net_SocketHandlerConfiguration,
                                     struct Stream_UserData> BitTorrent_AsynchConnector_t;

//////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct BitTorrent_Configuration,
                                 struct BitTorrent_ConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct Stream_UserData> BitTorrent_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct BitTorrent_Configuration,
                                 struct BitTorrent_ConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct Stream_UserData> BitTorrent_Connection_Manager_t;

typedef ACE_Singleton<BitTorrent_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> BITTORRENT_CONNECTIONMANAGER_SINGLETON;

#endif
