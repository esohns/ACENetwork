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

#ifndef BITTORRENT_CLIENT_NETWORK_H
#define BITTORRENT_CLIENT_NETWORK_H

#include <map>
#include <string>

#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch_Traits.h>

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

#include "bittorrent_stream_common.h"

#include "bittorrent_client_common.h"
#include "bittorrent_client_stream.h"
#include "bittorrent_client_stream_common.h"

// forward declarations
struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_UserData;
template <typename SessionStateType>
class BitTorrent_ISession_T;

//////////////////////////////////////////

struct BitTorrent_Client_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct BitTorrent_Client_Configuration,
                          struct BitTorrent_Client_ConnectionState,
                          BitTorrent_RuntimeStatistic_t> BitTorrent_Client_IConnection_t;

struct BitTorrent_Client_SocketHandlerConfiguration;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                struct BitTorrent_Client_Configuration,
                                struct BitTorrent_Client_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                BitTorrent_Client_Stream,
                                enum Stream_StateMachine_ControlState,
                                struct Net_SocketConfiguration,
                                struct BitTorrent_Client_SocketHandlerConfiguration> BitTorrent_Client_ISocketConnection_t;
//typedef Net_ISession_T<ACE_INET_Addr,
//                       Net_SocketConfiguration,
//                       BitTorrent_Client_Configuration,
//                       BitTorrent_Client_ConnectionState,
//                       BitTorrent_Client_RuntimeStatistic_t,
//                       BitTorrent_Client_Stream> BitTorrent_Client_ISession_t;

//////////////////////////////////////////

struct BitTorrent_Client_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline BitTorrent_Client_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct BitTorrent_Client_UserData* userData;
};

struct BitTorrent_Client_SessionState;
typedef BitTorrent_ISession_T<BitTorrent_Client_SessionState> BitTorrent_Client_ISession_t;
struct BitTorrent_Client_ConnectionState
 : BitTorrent_ConnectionState
{
  inline BitTorrent_Client_ConnectionState ()
   : BitTorrent_ConnectionState ()
   , configuration (NULL)
   , connection (NULL)
   , session (NULL)
  {};

  struct BitTorrent_Client_Configuration* configuration;
  BitTorrent_Client_IConnection_t*        connection;
  BitTorrent_Client_ISession_t*           session;
};

typedef std::map<Stream_SessionId_t, BitTorrent_Client_IConnection_t*> BitTorrent_Client_Connections_t;
typedef BitTorrent_Client_Connections_t::iterator BitTorrent_Client_ConnectionsIterator_t;
typedef std::map<std::string, BitTorrent_Client_Connections_t> BitTorrent_Client_SessionConnections_t;
typedef BitTorrent_Client_SessionConnections_t::iterator BitTorrent_Client_SessionConnectionsIterator_t;

struct BitTorrent_Client_SessionState
 : BitTorrent_SessionState
{
  inline BitTorrent_Client_SessionState ()
   : BitTorrent_SessionState ()
   , configuration (NULL)
   , connections ()
   , session (NULL)
   , userData (NULL)
  {};

  struct BitTorrent_Client_Configuration* configuration;
  BitTorrent_Client_SessionConnections_t  connections;
  BitTorrent_Client_ISession_t*           session;
  struct BitTorrent_Client_UserData*      userData;
};

//////////////////////////////////////////

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct BitTorrent_Client_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct BitTorrent_Client_Configuration,
                                  struct BitTorrent_Client_ConnectionState,
                                  BitTorrent_RuntimeStatistic_t,
                                  BitTorrent_Client_Stream,
                                  struct BitTorrent_Client_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_Client_ModuleHandlerConfiguration> BitTorrent_Client_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct BitTorrent_Client_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_Client_Configuration,
                                        struct BitTorrent_Client_ConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_Client_Stream,
                                        struct BitTorrent_Client_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_Client_ModuleHandlerConfiguration> BitTorrent_Client_AsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_Client_TCPHandler_t,
                                struct BitTorrent_Client_Configuration,
                                struct BitTorrent_Client_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                BitTorrent_Client_Stream,
                                struct BitTorrent_Client_SocketHandlerConfiguration,
                                struct BitTorrent_Client_UserData> BitTorrent_Client_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_Client_AsynchTCPHandler_t,
                                      struct BitTorrent_Client_Configuration,
                                      struct BitTorrent_Client_ConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      BitTorrent_Client_Stream,
                                      struct BitTorrent_Client_SocketHandlerConfiguration,
                                      struct BitTorrent_Client_UserData> BitTorrent_Client_AsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct BitTorrent_Client_SocketHandlerConfiguration> BitTorrent_Client_IConnector_t;
//typedef Net_IConnector_T<ACE_INET_Addr,
//                         BitTorrent_Client_ConnectorConfiguration> BitTorrent_Client_IConnector_t;
typedef Net_Client_Connector_T<BitTorrent_Client_TCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_Client_Configuration,
                               struct BitTorrent_Client_ConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               BitTorrent_Client_Stream,
                               struct BitTorrent_Client_SocketHandlerConfiguration,
                               struct BitTorrent_Client_UserData> BitTorrent_Client_Connector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_Client_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_Client_Configuration,
                                     struct BitTorrent_Client_ConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     BitTorrent_Client_Stream,
                                     struct BitTorrent_Client_SocketHandlerConfiguration,
                                     struct BitTorrent_Client_UserData> BitTorrent_Client_AsynchConnector_t;

//////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct BitTorrent_Client_Configuration,
                                 struct BitTorrent_Client_ConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct BitTorrent_Client_UserData> BitTorrent_Client_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct BitTorrent_Client_Configuration,
                                 struct BitTorrent_Client_ConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct BitTorrent_Client_UserData> BitTorrent_Client_Connection_Manager_t;

typedef ACE_Singleton<BitTorrent_Client_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> BITTORRENT_CLIENT_PEER_CONNECTIONMANAGER_SINGLETON;

#endif
