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

#include <map>
#include <string>

#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/Synch_Traits.h>

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_connection_manager.h"
#include "net_iconnection.h"
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
//#include "bittorrent_session.h"

// forward declarations
struct BitTorrent_Configuration;
class BitTorrent_IControl;
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename SessionStateType>
class BitTorrent_ISession_T;
template <typename HandlerConfigurationType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename StateType,
          typename UserDataType>
class BitTorrent_Session_T;

//////////////////////////////////////////

struct BitTorrent_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct BitTorrent_Configuration,
                          struct BitTorrent_ConnectionState,
                          BitTorrent_RuntimeStatistic_t> BitTorrent_IConnection_t;

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct BitTorrent_Configuration,
                                struct BitTorrent_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct Net_SocketHandlerConfiguration,
                                BitTorrent_PeerStream_t,
                                enum Stream_StateMachine_ControlState> BitTorrent_IPeerStreamConnection_t;

//////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename StateType>
class BitTorrent_ISession_T;
struct BitTorrent_SessionState;
typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              struct BitTorrent_Configuration,
                              struct BitTorrent_ConnectionState,
                              BitTorrent_RuntimeStatistic_t,
                              struct Net_SocketConfiguration,
                              struct Net_SocketHandlerConfiguration,
                              BitTorrent_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_SessionState> BitTorrent_ISession_t;
struct BitTorrent_ConnectionState
 : Net_ConnectionState
{
  inline BitTorrent_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , connection (NULL)
   , handshake (NULL)
   , session (NULL)
  {};

  struct BitTorrent_Configuration* configuration;
  BitTorrent_IConnection_t*        connection;
  struct BitTorrent_PeerHandshake* handshake;
  BitTorrent_ISession_t*           session;
};

typedef std::vector<Net_ConnectionId_t> BitTorrent_Connections_t;
typedef BitTorrent_Connections_t::iterator BitTorrent_ConnectionsIterator_t;
typedef std::map<std::string, BitTorrent_Connections_t> BitTorrent_SessionConnections_t;
typedef BitTorrent_SessionConnections_t::iterator BitTorrent_SessionConnectionsIterator_t;

struct BitTorrent_SessionState
{
  inline BitTorrent_SessionState ()
   : configuration (NULL)
   , connections ()
   , controller (NULL)
   , session (NULL)
  {};

  struct BitTorrent_Configuration* configuration;
  BitTorrent_SessionConnections_t  connections;
  BitTorrent_IControl*             controller;
  BitTorrent_ISession_t*           session;
};

//////////////////////////////////////////
// peer
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Net_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct BitTorrent_Configuration,
                                  struct BitTorrent_ConnectionState,
                                  BitTorrent_RuntimeStatistic_t,
                                  BitTorrent_PeerStream_t,
                                  struct Net_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_ModuleHandlerConfiguration> BitTorrent_PeerTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Net_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_Configuration,
                                        struct BitTorrent_ConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_PeerStream_t,
                                        struct Net_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_ModuleHandlerConfiguration> BitTorrent_PeerAsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_PeerTCPHandler_t,
                                struct BitTorrent_Configuration,
                                struct BitTorrent_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketHandlerConfiguration,
                                BitTorrent_PeerStream_t,
                                struct Net_UserData> BitTorrent_PeerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_PeerAsynchTCPHandler_t,
                                      struct BitTorrent_Configuration,
                                      struct BitTorrent_ConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      struct Net_SocketHandlerConfiguration,
                                      BitTorrent_PeerStream_t,
                                      struct Net_UserData> BitTorrent_PeerAsynchTCPConnection_t;

// tracker
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Net_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct BitTorrent_Configuration,
                                  struct BitTorrent_ConnectionState,
                                  BitTorrent_RuntimeStatistic_t,
                                  BitTorrent_TrackerStream_t,
                                  struct Net_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_ModuleHandlerConfiguration> BitTorrent_TrackerTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Net_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_Configuration,
                                        struct BitTorrent_ConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_TrackerStream_t,
                                        struct Net_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_ModuleHandlerConfiguration> BitTorrent_TrackerAsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_PeerTCPHandler_t,
                                struct BitTorrent_Configuration,
                                struct BitTorrent_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketHandlerConfiguration,
                                BitTorrent_TrackerStream_t,
                                struct Net_UserData> BitTorrent_TrackerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_PeerAsynchTCPHandler_t,
                                      struct BitTorrent_Configuration,
                                      struct BitTorrent_ConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      struct Net_SocketHandlerConfiguration,
                                      BitTorrent_TrackerStream_t,
                                      struct Net_UserData> BitTorrent_TrackerAsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Net_SocketHandlerConfiguration> BitTorrent_IConnector_t;
//typedef Net_IConnector_T<ACE_INET_Addr,
//                         truct Net_ConnectorConfiguration> BitTorrent_IConnector_t;

// peer
typedef Net_Client_Connector_T<BitTorrent_PeerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_Configuration,
                               struct BitTorrent_ConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               struct Net_SocketHandlerConfiguration,
                               BitTorrent_PeerStream_t,
                               struct Net_UserData> BitTorrent_PeerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_PeerAsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_Configuration,
                                     struct BitTorrent_ConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     struct Net_SocketHandlerConfiguration,
                                     BitTorrent_PeerStream_t,
                                     struct Net_UserData> BitTorrent_PeerAsynchConnector_t;

// tracker
typedef Net_Client_Connector_T<BitTorrent_TrackerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_Configuration,
                               struct BitTorrent_ConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               struct Net_SocketHandlerConfiguration,
                               BitTorrent_TrackerStream_t,
                               struct Net_UserData> BitTorrent_TrackerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_TrackerAsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_Configuration,
                                     struct BitTorrent_ConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     struct Net_SocketHandlerConfiguration,
                                     BitTorrent_TrackerStream_t,
                                     struct Net_UserData> BitTorrent_TrackerAsynchConnector_t;

//////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct BitTorrent_Configuration,
                                 struct BitTorrent_ConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct Net_UserData> BitTorrent_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct BitTorrent_Configuration,
                                 struct BitTorrent_ConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct Net_UserData> BitTorrent_Connection_Manager_t;

typedef ACE_Singleton<BitTorrent_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> BITTORRENT_CONNECTIONMANAGER_SINGLETON;

//////////////////////////////////////////

typedef BitTorrent_Session_T<struct Net_SocketHandlerConfiguration,
                             struct BitTorrent_Configuration,
                             struct BitTorrent_ConnectionState,
                             BitTorrent_PeerStream_t,
                             BitTorrent_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
//                             BitTorrent_IConnection_t,
//                             BitTorrent_IConnection_t,
                             BitTorrent_PeerTCPConnection_t,
                             BitTorrent_TrackerTCPConnection_t,
                             BitTorrent_IConnection_Manager_t,
                             BitTorrent_PeerConnector_t,
                             BitTorrent_TrackerConnector_t,
                             struct BitTorrent_SessionState,
                             struct Net_UserData> BitTorrent_Session_t;
typedef BitTorrent_Session_T<struct Net_SocketHandlerConfiguration,
                             struct BitTorrent_Configuration,
                             struct BitTorrent_ConnectionState,
                             BitTorrent_PeerStream_t,
                             BitTorrent_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
//                             BitTorrent_IConnection_t,
//                             BitTorrent_IConnection_t,
                             BitTorrent_PeerAsynchTCPConnection_t,
                             BitTorrent_TrackerAsynchTCPConnection_t,
                             BitTorrent_IConnection_Manager_t,
                             BitTorrent_PeerAsynchConnector_t,
                             BitTorrent_TrackerAsynchConnector_t,
                             struct BitTorrent_SessionState,
                             struct Net_UserData> BitTorrent_AsynchSession_t;

#endif
