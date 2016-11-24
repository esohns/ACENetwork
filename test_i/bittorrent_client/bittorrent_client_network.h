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

#include "bittorrent_network.h"
#include "bittorrent_stream_common.h"

#include "bittorrent_client_common.h"
//#include "bittorrent_client_stream.h"
#include "bittorrent_client_stream_common.h"

// forward declarations
struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_UserData;
template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename ConfigurationType,
          typename StateType>
class BitTorrent_ISession_T;

//////////////////////////////////////////

struct BitTorrent_Client_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct BitTorrent_Client_Configuration,
                          struct BitTorrent_Client_ConnectionState,
                          BitTorrent_RuntimeStatistic_t> BitTorrent_Client_IConnection_t;

struct BitTorrent_Client_SocketHandlerConfiguration;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct BitTorrent_Client_Configuration,
                                struct BitTorrent_Client_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct BitTorrent_Client_SocketHandlerConfiguration,
                                BitTorrent_Client_PeerStream_t,
                                enum Stream_StateMachine_ControlState> BitTorrent_Client_IPeerStreamConnection_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct BitTorrent_Client_Configuration,
                                struct BitTorrent_Client_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct BitTorrent_Client_SocketHandlerConfiguration,
                                BitTorrent_Client_TrackerStream_t,
                                enum Stream_StateMachine_ControlState> BitTorrent_Client_ITrackerStreamConnection_t;

//typedef Net_ISession_T<ACE_INET_Addr,
//                       struct Net_SocketConfiguration,
//                       struct BitTorrent_Client_Configuration,
//                       struct BitTorrent_Client_ConnectionState,
//                       BitTorrent_Client_RuntimeStatistic_t,
//                       BitTorrent_Client_PeerStream_t> BitTorrent_Client_ISession_t;

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

struct BitTorrent_Client_SessionConfiguration;
struct BitTorrent_Client_SessionState;
typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              struct BitTorrent_Client_Configuration,
                              struct BitTorrent_Client_ConnectionState,
                              BitTorrent_RuntimeStatistic_t,
                              struct Net_SocketConfiguration,
                              struct BitTorrent_Client_SocketHandlerConfiguration,
                              BitTorrent_Client_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_Client_SessionConfiguration,
                              struct BitTorrent_Client_SessionState> BitTorrent_Client_ISession_t;
struct BitTorrent_Client_ConnectionState
 : Net_ConnectionState
{
  inline BitTorrent_Client_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , connection (NULL)
   , session (NULL)
   , userData (NULL)
  {};

  struct BitTorrent_Client_Configuration* configuration;
  BitTorrent_Client_IConnection_t*        connection;
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
                                  BitTorrent_Client_PeerStream_t,
                                  struct BitTorrent_Client_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_Client_ModuleHandlerConfiguration> BitTorrent_Client_PeerTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct BitTorrent_Client_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_Client_Configuration,
                                        struct BitTorrent_Client_ConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_Client_PeerStream_t,
                                        struct BitTorrent_Client_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_Client_ModuleHandlerConfiguration> BitTorrent_Client_AsynchPeerTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_Client_PeerTCPHandler_t,
                                struct BitTorrent_Client_Configuration,
                                struct BitTorrent_Client_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct BitTorrent_Client_SocketHandlerConfiguration,
                                BitTorrent_Client_PeerStream_t,
                                struct BitTorrent_Client_UserData> BitTorrent_Client_PeerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_Client_AsynchPeerTCPHandler_t,
                                      struct BitTorrent_Client_Configuration,
                                      struct BitTorrent_Client_ConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      struct BitTorrent_Client_SocketHandlerConfiguration,
                                      BitTorrent_Client_PeerStream_t,
                                      struct BitTorrent_Client_UserData> BitTorrent_Client_AsynchPeerTCPConnection_t;

//----------------------------------------

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct BitTorrent_Client_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct BitTorrent_Client_Configuration,
                                  struct BitTorrent_Client_ConnectionState,
                                  BitTorrent_RuntimeStatistic_t,
                                  BitTorrent_Client_TrackerStream_t,
                                  struct BitTorrent_Client_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_Client_ModuleHandlerConfiguration> BitTorrent_Client_TrackerTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct BitTorrent_Client_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_Client_Configuration,
                                        struct BitTorrent_Client_ConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_Client_TrackerStream_t,
                                        struct BitTorrent_Client_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_Client_ModuleHandlerConfiguration> BitTorrent_Client_AsynchTrackerTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_Client_TrackerTCPHandler_t,
                                struct BitTorrent_Client_Configuration,
                                struct BitTorrent_Client_ConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct BitTorrent_Client_SocketHandlerConfiguration,
                                BitTorrent_Client_TrackerStream_t,
                                struct BitTorrent_Client_UserData> BitTorrent_Client_TrackerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_Client_AsynchTrackerTCPHandler_t,
                                      struct BitTorrent_Client_Configuration,
                                      struct BitTorrent_Client_ConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      struct BitTorrent_Client_SocketHandlerConfiguration,
                                      BitTorrent_Client_TrackerStream_t,
                                      struct BitTorrent_Client_UserData> BitTorrent_Client_AsynchTrackerTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct BitTorrent_Client_SocketHandlerConfiguration> BitTorrent_Client_IConnector_t;
//typedef Net_IConnector_T<ACE_INET_Addr,
//                         BitTorrent_Client_ConnectorConfiguration> BitTorrent_Client_IConnector_t;

// peer
typedef Net_Client_Connector_T<BitTorrent_Client_PeerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_Client_Configuration,
                               struct BitTorrent_Client_ConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               struct BitTorrent_Client_SocketHandlerConfiguration,
                               BitTorrent_Client_PeerStream_t,
                               struct BitTorrent_Client_UserData> BitTorrent_Client_PeerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_Client_AsynchPeerTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_Client_Configuration,
                                     struct BitTorrent_Client_ConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     struct BitTorrent_Client_SocketHandlerConfiguration,
                                     BitTorrent_Client_PeerStream_t,
                                     struct BitTorrent_Client_UserData> BitTorrent_Client_AsynchPeerConnector_t;

// tracker
typedef Net_Client_Connector_T<BitTorrent_Client_TrackerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_Client_Configuration,
                               struct BitTorrent_Client_ConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               struct BitTorrent_Client_SocketHandlerConfiguration,
                               BitTorrent_Client_TrackerStream_t,
                               struct BitTorrent_Client_UserData> BitTorrent_Client_TrackerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_Client_AsynchTrackerTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_Client_Configuration,
                                     struct BitTorrent_Client_ConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     struct BitTorrent_Client_SocketHandlerConfiguration,
                                     BitTorrent_Client_TrackerStream_t,
                                     struct BitTorrent_Client_UserData> BitTorrent_Client_AsynchTrackerConnector_t;

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
                      ACE_SYNCH_MUTEX> BITTORRENT_CLIENT_CONNECTIONMANAGER_SINGLETON;

//////////////////////////////////////////

//typedef std::map<std::string, BitTorrent_Client_Connections_t> BitTorrent_Client_SessionConnections_t;
//typedef BitTorrent_Client_SessionConnections_t::iterator BitTorrent_Client_SessionConnectionsIterator_t;
template <typename SessionInterfaceType>
class BitTorrent_IControl_T;
typedef BitTorrent_IControl_T<BitTorrent_Client_ISession_t> BitTorrent_Client_IControl_t;

struct BitTorrent_Client_SessionConfiguration
 : BitTorrent_SessionConfiguration
{
  inline BitTorrent_Client_SessionConfiguration ()
   : BitTorrent_SessionConfiguration ()
   , connectionManager (NULL)
   , socketHandlerConfiguration (NULL)
   , trackerSocketHandlerConfiguration (NULL)
  {};

  BitTorrent_Client_Connection_Manager_t*              connectionManager;
  struct BitTorrent_Client_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct BitTorrent_Client_SocketHandlerConfiguration* trackerSocketHandlerConfiguration;
};

struct BitTorrent_Client_SessionState
 : BitTorrent_SessionState
{
  inline BitTorrent_Client_SessionState ()
   : controller (NULL)
   , session (NULL)
//   , userData (NULL)
  {};

//  struct BitTorrent_Client_Configuration* configuration;
  BitTorrent_Client_IControl_t*      controller;
  BitTorrent_Client_ISession_t*      session;
//  struct BitTorrent_Client_UserData* userData;
};

#endif
