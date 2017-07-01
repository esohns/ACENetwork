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

#include <string>
#include <vector>

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_configuration.h"
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
#include "bittorrent_stream_common.h"

// forward declarations
template <typename AddressType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename ConfigurationType,
          typename SessionStateType>
class BitTorrent_ISession_T;
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
class BitTorrent_Session_T;

//////////////////////////////////////////

struct BitTorrent_PeerConnectionConfiguration;
struct BitTorrent_PeerConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct BitTorrent_PeerConnectionConfiguration,
                          struct BitTorrent_PeerConnectionState,
                          BitTorrent_RuntimeStatistic_t> BitTorrent_IPeerConnection_t;
struct BitTorrent_TrackerConnectionConfiguration;
struct BitTorrent_TrackerConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct BitTorrent_TrackerConnectionConfiguration,
                          struct BitTorrent_TrackerConnectionState,
                          BitTorrent_RuntimeStatistic_t> BitTorrent_ITrackerConnection_t;

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                struct BitTorrent_PeerConnectionConfiguration,
                                struct BitTorrent_PeerConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct Net_SocketHandlerConfiguration,
                                BitTorrent_PeerStream_t,
                                enum Stream_StateMachine_ControlState> BitTorrent_IPeerStreamConnection_t;

//////////////////////////////////////////

template <typename AddressType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename ConfigurationType,
          typename StateType>
class BitTorrent_ISession_T;
struct BitTorrent_SessionConfiguration;
struct BitTorrent_SessionState;
typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              struct BitTorrent_PeerConnectionConfiguration,
                              struct BitTorrent_TrackerConnectionConfiguration,
                              struct BitTorrent_PeerConnectionState,
                              BitTorrent_RuntimeStatistic_t,
                              struct Net_SocketConfiguration,
                              struct Net_SocketHandlerConfiguration,
                              struct Net_SocketHandlerConfiguration,
                              BitTorrent_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_SessionConfiguration,
                              struct BitTorrent_SessionState> BitTorrent_ISession_t;
struct BitTorrent_PeerConnectionState
 : Net_ConnectionState
{
  inline BitTorrent_PeerConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , connection (NULL)
   , handshake (NULL)
   , session (NULL)
  {};

  struct BitTorrent_Configuration* configuration;
  BitTorrent_IPeerConnection_t*    connection;
  struct BitTorrent_PeerHandshake* handshake;
  BitTorrent_ISession_t*           session;
};
struct BitTorrent_TrackerConnectionState
 : Net_ConnectionState
{
  inline BitTorrent_TrackerConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , connection (NULL)
   , session (NULL)
  {};

  struct BitTorrent_Configuration* configuration;
  BitTorrent_ITrackerConnection_t* connection;
  BitTorrent_ISession_t*           session;
};

struct BitTorrent_UserData;
//////////////////////////////////////////
// peer
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Net_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct BitTorrent_PeerConnectionConfiguration,
                                  struct BitTorrent_PeerConnectionState,
                                  BitTorrent_RuntimeStatistic_t,
                                  BitTorrent_PeerStream_t,
                                  struct BitTorrent_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_PeerModuleHandlerConfiguration> BitTorrent_PeerTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Net_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_PeerConnectionConfiguration,
                                        struct BitTorrent_PeerConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_PeerStream_t,
                                        struct BitTorrent_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_PeerModuleHandlerConfiguration> BitTorrent_PeerAsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_PeerTCPHandler_t,
                                struct BitTorrent_PeerConnectionConfiguration,
                                struct BitTorrent_PeerConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketHandlerConfiguration,
                                struct Net_ListenerConfiguration,
                                BitTorrent_PeerStream_t,
                                struct BitTorrent_UserData> BitTorrent_PeerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_PeerAsynchTCPHandler_t,
                                      struct BitTorrent_PeerConnectionConfiguration,
                                      struct BitTorrent_PeerConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      struct Net_SocketHandlerConfiguration,
                                      struct Net_ListenerConfiguration,
                                      BitTorrent_PeerStream_t,
                                      struct BitTorrent_UserData> BitTorrent_PeerAsynchTCPConnection_t;

// tracker
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Net_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct BitTorrent_TrackerConnectionConfiguration,
                                  struct BitTorrent_TrackerConnectionState,
                                  BitTorrent_RuntimeStatistic_t,
                                  BitTorrent_TrackerStream_t,
                                  struct BitTorrent_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct BitTorrent_TrackerModuleHandlerConfiguration> BitTorrent_TrackerTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Net_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct BitTorrent_TrackerConnectionConfiguration,
                                        struct BitTorrent_TrackerConnectionState,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_TrackerStream_t,
                                        struct BitTorrent_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct BitTorrent_TrackerModuleHandlerConfiguration> BitTorrent_TrackerAsynchTCPHandler_t;
typedef Net_TCPConnectionBase_T<BitTorrent_TrackerTCPHandler_t,
                                struct BitTorrent_TrackerConnectionConfiguration,
                                struct BitTorrent_TrackerConnectionState,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketHandlerConfiguration,
                                struct Net_ListenerConfiguration,
                                BitTorrent_TrackerStream_t,
                                struct BitTorrent_UserData> BitTorrent_TrackerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_TrackerAsynchTCPHandler_t,
                                      struct BitTorrent_TrackerConnectionConfiguration,
                                      struct BitTorrent_TrackerConnectionState,
                                      BitTorrent_RuntimeStatistic_t,
                                      struct Net_SocketHandlerConfiguration,
                                      struct Net_ListenerConfiguration,
                                      BitTorrent_TrackerStream_t,
                                      struct BitTorrent_UserData> BitTorrent_TrackerAsynchTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct BitTorrent_PeerConnectionConfiguration> BitTorrent_IPeerConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         struct BitTorrent_TrackerConnectionConfiguration> BitTorrent_ITrackerConnector_t;

// peer
typedef Net_Client_Connector_T<BitTorrent_PeerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_PeerConnectionConfiguration,
                               struct BitTorrent_PeerConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               struct Net_SocketHandlerConfiguration,
                               BitTorrent_PeerStream_t,
                               struct BitTorrent_UserData> BitTorrent_PeerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_PeerAsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_PeerConnectionConfiguration,
                                     struct BitTorrent_PeerConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     struct Net_SocketHandlerConfiguration,
                                     BitTorrent_PeerStream_t,
                                     struct BitTorrent_UserData> BitTorrent_PeerAsynchConnector_t;

// tracker
typedef Net_Client_Connector_T<BitTorrent_TrackerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct BitTorrent_TrackerConnectionConfiguration,
                               struct BitTorrent_TrackerConnectionState,
                               BitTorrent_RuntimeStatistic_t,
                               struct Net_SocketHandlerConfiguration,
                               BitTorrent_TrackerStream_t,
                               struct BitTorrent_UserData> BitTorrent_TrackerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_TrackerAsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct BitTorrent_TrackerConnectionConfiguration,
                                     struct BitTorrent_TrackerConnectionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     struct Net_SocketHandlerConfiguration,
                                     BitTorrent_TrackerStream_t,
                                     struct BitTorrent_UserData> BitTorrent_TrackerAsynchConnector_t;

//////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct BitTorrent_PeerConnectionConfiguration,
                                 struct BitTorrent_PeerConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct BitTorrent_UserData> BitTorrent_IPeerConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct BitTorrent_PeerConnectionConfiguration,
                                 struct BitTorrent_PeerConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct BitTorrent_UserData> BitTorrent_PeerConnection_Manager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct BitTorrent_TrackerConnectionConfiguration,
                                 struct BitTorrent_TrackerConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct BitTorrent_UserData> BitTorrent_ITrackerConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct BitTorrent_TrackerConnectionConfiguration,
                                 struct BitTorrent_TrackerConnectionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 struct BitTorrent_UserData> BitTorrent_TrackerConnection_Manager_t;

typedef ACE_Singleton<BitTorrent_PeerConnection_Manager_t,
                      ACE_SYNCH_MUTEX> BITTORRENT_PEERCONNECTION_MANAGER_SINGLETON;
typedef ACE_Singleton<BitTorrent_TrackerConnection_Manager_t,
                      ACE_SYNCH_MUTEX> BITTORRENT_TRACKERCONNECTION_MANAGER_SINGLETON;

//////////////////////////////////////////

struct BitTorrent_PeerStreamConfiguration;
struct BitTorrent_PeerConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline BitTorrent_PeerConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , streamConfiguration (NULL)
   , userData (NULL)
  {
    PDUSize = BITTORRENT_BUFFER_SIZE;
  };

  struct BitTorrent_PeerStreamConfiguration* streamConfiguration;

  struct BitTorrent_UserData*                userData;
};
struct BitTorrent_TrackerStreamConfiguration;
struct BitTorrent_TrackerConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline BitTorrent_TrackerConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , streamConfiguration (NULL)
   , userData (NULL)
  {
    PDUSize = BITTORRENT_BUFFER_SIZE;
  };

  struct BitTorrent_TrackerStreamConfiguration* streamConfiguration;

  struct BitTorrent_UserData*                   userData;
};

typedef std::vector<ACE_INET_Addr> BitTorrent_PeerAddresses_t;
typedef BitTorrent_PeerAddresses_t::const_iterator BitTorrent_PeerAddressesIterator_t;
struct BitTorrent_SessionInitiationThreadData
{
  inline BitTorrent_SessionInitiationThreadData ()
   : addresses (NULL)
   , lock (NULL)
   , session (NULL)
  {};

  BitTorrent_PeerAddresses_t* addresses;
  ACE_SYNCH_MUTEX*            lock;
  Net_IInetSession_t*         session;
};

template <typename SessionInterfaceType>
class BitTorrent_IControl_T;
typedef BitTorrent_IControl_T<BitTorrent_ISession_t> BitTorrent_IControl_t;
struct BitTorrent_SessionConfiguration
 : Net_SessionConfiguration
{
  inline BitTorrent_SessionConfiguration ()
   : Net_SessionConfiguration ()
   , connectionManager (NULL)
   , controller_ (NULL)
   , trackerConnectionManager (NULL)
   , metaInfo (NULL)
   , metaInfoFileName ()
   //, socketHandlerConfiguration (NULL)
   //, trackerSocketHandlerConfiguration (NULL)
   , peerConnectionConfiguration (NULL)
   , trackerConnectionConfiguration (NULL)
  {};

  BitTorrent_PeerConnection_Manager_t*              connectionManager;
  BitTorrent_IControl_t*                            controller_;
  BitTorrent_TrackerConnection_Manager_t*           trackerConnectionManager;
  Bencoding_Dictionary_t*                           metaInfo;
  std::string                                       metaInfoFileName;
  //struct Net_SocketHandlerConfiguration*  socketHandlerConfiguration;
  //struct Net_SocketHandlerConfiguration*  trackerSocketHandlerConfiguration;
  struct BitTorrent_PeerConnectionConfiguration*    peerConnectionConfiguration;
  struct BitTorrent_TrackerConnectionConfiguration* trackerConnectionConfiguration;
};

// *NOTE*: see also: https://wiki.theory.org/BitTorrentSpecification#Overview
struct BitTorrent_PeerStatus
{
  inline BitTorrent_PeerStatus ()
   : am_choking (true)
   , am_interested (false)
   , peer_choking (true)
   , peer_interested (false)
  {};

  bool am_choking;
  bool am_interested;
  bool peer_choking;
  bool peer_interested;
};
typedef std::map<Net_ConnectionId_t, struct BitTorrent_PeerStatus> BitTorrent_PeerStatus_t;
typedef BitTorrent_PeerStatus_t::iterator BitTorrent_PeerStatusIterator_t;
struct BitTorrent_SessionState
{
  inline BitTorrent_SessionState ()
   : connections ()
   , controller (NULL)
   , fileName ()
   , key ()
   , metaInfo (NULL)
   , peerId ()
   , peerStatus ()
   , trackerConnectionId (0)
   , trackerId ()
   , trackerRequestResponse (NULL)
   , trackerScrapeResponse (NULL)
  {};

  Net_Connections_t       connections;
  BitTorrent_IControl_t*  controller;
  std::string             fileName; // .torrent file
  std::string             key; // tracker-
  Bencoding_Dictionary_t* metaInfo;
  std::string             peerId;
  BitTorrent_PeerStatus_t peerStatus;
  Net_ConnectionId_t      trackerConnectionId;
  std::string             trackerId;
  Bencoding_Dictionary_t* trackerRequestResponse;
  Bencoding_Dictionary_t* trackerScrapeResponse;
};

typedef BitTorrent_Session_T<struct Net_SocketHandlerConfiguration,
                             struct Net_SocketHandlerConfiguration,
                             struct BitTorrent_PeerConnectionConfiguration,
                             struct BitTorrent_TrackerConnectionConfiguration,
                             struct BitTorrent_PeerConnectionState,
                             BitTorrent_PeerStream_t,
                             BitTorrent_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
                             BitTorrent_PeerHandler_Module,
                             BitTorrent_TrackerHandler_Module,
                             BitTorrent_PeerTCPConnection_t,
                             BitTorrent_TrackerTCPConnection_t,
                             BitTorrent_IPeerConnection_Manager_t,
                             BitTorrent_ITrackerConnection_Manager_t,
                             BitTorrent_PeerConnector_t,
                             BitTorrent_TrackerConnector_t,
                             struct BitTorrent_SessionConfiguration,
                             struct BitTorrent_SessionState,
                             struct BitTorrent_UserData,
                             struct BitTorrent_UserData,
                             BitTorrent_IControl_t,
                             struct Common_UI_State> BitTorrent_Session_t;
typedef BitTorrent_Session_T<struct Net_SocketHandlerConfiguration,
                             struct Net_SocketHandlerConfiguration,
                             struct BitTorrent_PeerConnectionConfiguration,
                             struct BitTorrent_TrackerConnectionConfiguration,
                             struct BitTorrent_PeerConnectionState,
                             BitTorrent_PeerStream_t,
                             BitTorrent_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
                             BitTorrent_PeerHandler_Module,
                             BitTorrent_TrackerHandler_Module,
                             BitTorrent_PeerAsynchTCPConnection_t,
                             BitTorrent_TrackerAsynchTCPConnection_t,
                             BitTorrent_IPeerConnection_Manager_t,
                             BitTorrent_ITrackerConnection_Manager_t,
                             BitTorrent_PeerAsynchConnector_t,
                             BitTorrent_TrackerAsynchConnector_t,
                             struct BitTorrent_SessionConfiguration,
                             struct BitTorrent_SessionState,
                             struct BitTorrent_UserData,
                             struct BitTorrent_UserData,
                             BitTorrent_IControl_t,
                             struct Common_UI_State> BitTorrent_AsynchSession_t;

//////////////////////////////////////////

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
class BitTorrent_Control_T;
typedef BitTorrent_Control_T<BitTorrent_AsynchSession_t,
                             BitTorrent_Session_t,
                             struct BitTorrent_SessionConfiguration,
                             BitTorrent_ISession_t,
                             struct BitTorrent_SessionState> BitTorrent_Control_t;

#endif
