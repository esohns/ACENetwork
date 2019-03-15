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

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_configuration.h"
#include "common_timer_manager_common.h"

#include "stream_configuration.h"

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

#include "bittorrent_icontrol.h"
#include "bittorrent_isession.h"
#include "bittorrent_network.h"

//#include "bittorrent_client_stream_common.h"

//////////////////////////////////////////

struct BitTorrent_AllocatorConfiguration;
//struct BitTorrent_Client_PeerStreamConfiguration;
struct BitTorrent_Client_PeerStreamConfiguration
 : BitTorrent_PeerStreamConfiguration
{
  BitTorrent_Client_PeerStreamConfiguration ()
   : BitTorrent_PeerStreamConfiguration ()
//   , userData (NULL)
  {}

//  struct BitTorrent_Client_PeerUserData* userData;
};
struct BitTorrent_Client_PeerModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct BitTorrent_AllocatorConfiguration,
                               struct BitTorrent_Client_PeerStreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct BitTorrent_Client_PeerModuleHandlerConfiguration> BitTorrent_Client_PeerStreamConfiguration_t;
class BitTorrent_Client_PeerConnectionConfiguration;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_PeerConnectionConfiguration,
                                 struct BitTorrent_Client_PeerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerIConnection_Manager_t;
class BitTorrent_Client_PeerConnectionConfiguration
 : public Net_ConnectionConfiguration_T<struct BitTorrent_AllocatorConfiguration,
                                        struct BitTorrent_Client_PeerStreamConfiguration,
                                        NET_TRANSPORTLAYER_TCP>
{
 public:
  BitTorrent_Client_PeerConnectionConfiguration ()
   : Net_ConnectionConfiguration_T ()
   ///////////////////////////////////////
//   , connectionManager (NULL)
  {}

//  BitTorrent_Client_PeerIConnection_Manager_t*            connectionManager;
};
typedef std::map<std::string,
                 BitTorrent_Client_PeerConnectionConfiguration> BitTorrent_Client_PeerConnectionConfigurations_t;
typedef BitTorrent_Client_PeerConnectionConfigurations_t::iterator BitTorrent_Client_PeerConnectionConfigurationIterator_t;

struct BitTorrent_Client_PeerConnectionState;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_PeerConnectionConfiguration,
                                 struct BitTorrent_Client_PeerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerConnection_Manager_t;

//////////////////////////////////////////

struct BitTorrent_Client_TrackerStreamConfiguration;
struct BitTorrent_Client_TrackerModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct BitTorrent_AllocatorConfiguration,
                               struct BitTorrent_Client_TrackerStreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct BitTorrent_Client_TrackerModuleHandlerConfiguration> BitTorrent_Client_TrackerStreamConfiguration_t;
class BitTorrent_Client_TrackerConnectionConfiguration;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_TrackerConnectionConfiguration,
                                 struct BitTorrent_Client_TrackerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerIConnection_Manager_t;
class BitTorrent_Client_TrackerConnectionConfiguration
 : public Net_ConnectionConfiguration_T<struct BitTorrent_AllocatorConfiguration,
                                        struct BitTorrent_Client_TrackerStreamConfiguration,
                                        NET_TRANSPORTLAYER_TCP>
{
 public:
  BitTorrent_Client_TrackerConnectionConfiguration ()
   : Net_ConnectionConfiguration_T ()
   ///////////////////////////////////////
//   , connectionManager (NULL)
  {}

//  BitTorrent_Client_TrackerIConnection_Manager_t*            connectionManager;
};
typedef std::map<std::string,
                 BitTorrent_Client_TrackerConnectionConfiguration> BitTorrent_Client_TrackerConnectionConfigurations_t;
typedef BitTorrent_Client_TrackerConnectionConfigurations_t::iterator BitTorrent_Client_TrackerConnectionConfigurationIterator_t;

struct BitTorrent_Client_TrackerConnectionState;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_TrackerConnectionConfiguration,
                                 struct BitTorrent_Client_TrackerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerConnection_Manager_t;

typedef Net_IConnection_T<ACE_INET_Addr,
                          BitTorrent_Client_PeerConnectionConfiguration,
                          struct BitTorrent_Client_PeerConnectionState,
                          BitTorrent_Statistic_t> BitTorrent_Client_IPeerConnection_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          BitTorrent_Client_TrackerConnectionConfiguration,
                          struct BitTorrent_Client_TrackerConnectionState,
                          BitTorrent_Statistic_t> BitTorrent_Client_ITrackerConnection_t;

struct BitTorrent_Client_PeerSessionData;
template <typename DataType>
class Stream_SessionData_T;
typedef Stream_SessionData_T<struct BitTorrent_Client_PeerSessionData> BitTorrent_Client_PeerSessionData_t;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct BitTorrent_AllocatorConfiguration> BitTorrent_Client_ControlMessage_t;
template <typename SessionDataType,
          typename UserDataType>
class BitTorrent_Message_T;
typedef BitTorrent_Message_T<BitTorrent_Client_PeerSessionData_t,
                             struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerMessage_t;
template <typename SessionDataType,
          typename UserDataType>
class BitTorrent_SessionMessage_T;
typedef BitTorrent_SessionMessage_T<struct BitTorrent_Client_PeerSessionData,
                                    struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerSessionMessage_t;
template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType,
          typename SessionStateType,
          typename ConnectionManagerType,
          typename UserDataType>
class BitTorrent_PeerStream_T;
typedef BitTorrent_PeerStream_T<struct BitTorrent_Client_PeerStreamState,
                                struct BitTorrent_Client_PeerStreamConfiguration,
                                BitTorrent_Statistic_t,
                                Common_Timer_Manager_t,
                                struct BitTorrent_Client_PeerModuleHandlerConfiguration,
                                struct BitTorrent_Client_PeerSessionData,
                                BitTorrent_Client_PeerSessionData_t,
                                BitTorrent_Client_ControlMessage_t,
                                BitTorrent_Client_PeerMessage_t,
                                BitTorrent_Client_PeerSessionMessage_t,
                                BitTorrent_Client_PeerConnectionConfiguration,
                                struct BitTorrent_Client_PeerConnectionState,
                                struct BitTorrent_Client_SocketHandlerConfiguration,
                                struct BitTorrent_Client_SessionState,
                                BitTorrent_Client_PeerConnection_Manager_t,
                                struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerStream_t;
typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              BitTorrent_Client_PeerConnectionConfiguration,
                              BitTorrent_Client_TrackerConnectionConfiguration,
                              struct BitTorrent_Client_PeerConnectionState,
                              BitTorrent_Statistic_t,
                              struct Net_SocketConfiguration,
                              struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                              struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                              BitTorrent_Client_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_Client_SessionConfiguration,
                              struct BitTorrent_Client_SessionState> BitTorrent_Client_ISession_t;
struct BitTorrent_Client_PeerConnectionState
 : BitTorrent_PeerConnectionState
{
  BitTorrent_Client_PeerConnectionState ()
   : BitTorrent_PeerConnectionState ()
   , configuration (NULL)
   , connection (NULL)
   , session (NULL)
   , userData (NULL)
  {}

  struct BitTorrent_Client_Configuration* configuration;
  BitTorrent_Client_IPeerConnection_t*    connection;
  BitTorrent_Client_ISession_t*           session;

  struct BitTorrent_Client_PeerUserData*  userData;
};
struct BitTorrent_Client_TrackerConnectionState
 : BitTorrent_TrackerConnectionState
{
  BitTorrent_Client_TrackerConnectionState ()
   : BitTorrent_TrackerConnectionState ()
   , configuration (NULL)
   , connection (NULL)
   , session (NULL)
   , userData (NULL)
  {}

  struct BitTorrent_Client_Configuration*   configuration;
  BitTorrent_Client_ITrackerConnection_t*   connection;
  BitTorrent_Client_ISession_t*             session;

  struct BitTorrent_Client_TrackerUserData* userData;
};

//////////////////////////////////////////

typedef BitTorrent_IControl_T<BitTorrent_Client_ISession_t> BitTorrent_Client_IControl_t;

//////////////////////////////////////////

struct BitTorrent_Client_SessionConfiguration
 : BitTorrent_SessionConfiguration
{
  BitTorrent_Client_SessionConfiguration ()
   : BitTorrent_SessionConfiguration ()
   , connectionConfiguration (NULL)
   , connectionManager (NULL)
   , controller (NULL)
   , trackerConnectionConfiguration (NULL)
   , trackerConnectionManager (NULL)
  {}

  BitTorrent_Client_PeerConnectionConfiguration*    connectionConfiguration;
  BitTorrent_Client_PeerConnection_Manager_t*         connectionManager;
  BitTorrent_Client_IControl_t*                       controller;
  BitTorrent_Client_TrackerConnectionConfiguration* trackerConnectionConfiguration;
  BitTorrent_Client_TrackerConnection_Manager_t*      trackerConnectionManager;
};

//////////////////////////////////////////

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                BitTorrent_Client_PeerConnectionConfiguration,
                                struct BitTorrent_Client_PeerConnectionState,
                                BitTorrent_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                                BitTorrent_Client_PeerStream_t,
                                enum Stream_StateMachine_ControlState> BitTorrent_Client_IPeerStreamConnection_t;

struct BitTorrent_Client_TrackerSessionData;
typedef Stream_SessionData_T<struct BitTorrent_Client_TrackerSessionData> BitTorrent_Client_TrackerSessionData_t;
typedef BitTorrent_TrackerMessage_T<BitTorrent_Client_TrackerSessionData_t,
                                    struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerMessage_t;
typedef BitTorrent_SessionMessage_T<struct BitTorrent_Client_TrackerSessionData,
                                    struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerSessionMessage_t;
template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename PeerStreamType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType, // socket-
          typename SessionStateType,
          typename ConnectionManagerType,
          typename UserDataType>
class BitTorrent_TrackerStream_T;
typedef BitTorrent_TrackerStream_T<struct BitTorrent_Client_TrackerStreamState,
                                   struct BitTorrent_Client_TrackerStreamConfiguration,
                                   BitTorrent_Statistic_t,
                                   Common_Timer_Manager_t,
                                   struct BitTorrent_Client_TrackerModuleHandlerConfiguration,
                                   struct BitTorrent_Client_TrackerSessionData,
                                   BitTorrent_Client_TrackerSessionData_t,
                                   BitTorrent_Client_ControlMessage_t,
                                   BitTorrent_Client_TrackerMessage_t,
                                   BitTorrent_Client_TrackerSessionMessage_t,
                                   BitTorrent_Client_PeerStream_t,
                                   BitTorrent_Client_TrackerConnectionConfiguration,
                                   struct BitTorrent_Client_TrackerConnectionState,
                                   struct BitTorrent_Client_SocketHandlerConfiguration,
                                   struct BitTorrent_Client_SessionState,
                                   BitTorrent_Client_TrackerConnection_Manager_t,
                                   struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerStream_t;
typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                BitTorrent_Client_TrackerConnectionConfiguration,
                                struct BitTorrent_Client_TrackerConnectionState,
                                BitTorrent_Statistic_t,
                                struct Net_SocketConfiguration,
                                struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                                BitTorrent_Client_TrackerStream_t,
                                enum Stream_StateMachine_ControlState> BitTorrent_Client_ITrackerStreamConnection_t;

//typedef Net_ISession_T<ACE_INET_Addr,
//                       struct Net_SocketConfiguration,
//                       struct BitTorrent_Client_Configuration,
//                       struct BitTorrent_Client_ConnectionState,
//                       BitTorrent_Client_RuntimeStatistic_t,
//                       BitTorrent_Client_PeerStream_t> BitTorrent_Client_ISession_t;

//////////////////////////////////////////

typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
                               ACE_SOCK_STREAM,
                               struct BitTorrent_Client_PeerSocketHandlerConfiguration> BitTorrent_Client_PeerTCPSocketHandler_t;
typedef Net_AsynchTCPSocketHandler_T<struct BitTorrent_Client_PeerSocketHandlerConfiguration> BitTorrent_Client_AsynchPeerTCPSocketHandler_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                BitTorrent_Client_PeerTCPSocketHandler_t,
                                BitTorrent_Client_PeerConnectionConfiguration,
                                struct BitTorrent_Client_PeerConnectionState,
                                BitTorrent_Statistic_t,
                                struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                                struct Net_ListenerConfiguration,
                                BitTorrent_Client_PeerStream_t,
                                Common_Timer_Manager_t,
                                struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_Client_AsynchPeerTCPSocketHandler_t,
                                      BitTorrent_Client_PeerConnectionConfiguration,
                                      struct BitTorrent_Client_PeerConnectionState,
                                      BitTorrent_Statistic_t,
                                      struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                                      struct Net_ListenerConfiguration,
                                      BitTorrent_Client_PeerStream_t,
                                      Common_Timer_Manager_t,
                                      struct BitTorrent_Client_PeerUserData> BitTorrent_Client_AsynchPeerTCPConnection_t;

//----------------------------------------

typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
                               ACE_SOCK_STREAM,
                               struct BitTorrent_Client_TrackerSocketHandlerConfiguration> BitTorrent_Client_TrackerTCPSocketHandler_t;
typedef Net_AsynchTCPSocketHandler_T<struct BitTorrent_Client_TrackerSocketHandlerConfiguration> BitTorrent_Client_AsynchTrackerTCPSocketHandler_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                BitTorrent_Client_TrackerTCPSocketHandler_t,
                                BitTorrent_Client_TrackerConnectionConfiguration,
                                struct BitTorrent_Client_TrackerConnectionState,
                                BitTorrent_Statistic_t,
                                struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                                struct Net_ListenerConfiguration,
                                BitTorrent_Client_TrackerStream_t,
                                Common_Timer_Manager_t,
                                struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<BitTorrent_Client_AsynchTrackerTCPSocketHandler_t,
                                      BitTorrent_Client_TrackerConnectionConfiguration,
                                      struct BitTorrent_Client_TrackerConnectionState,
                                      BitTorrent_Statistic_t,
                                      struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                                      struct Net_ListenerConfiguration,
                                      BitTorrent_Client_TrackerStream_t,
                                      Common_Timer_Manager_t,
                                      struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_AsynchTrackerTCPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         BitTorrent_Client_PeerConnectionConfiguration> BitTorrent_Client_IPeerConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         BitTorrent_Client_TrackerConnectionConfiguration> BitTorrent_Client_ITrackerConnector_t;

// peer
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               BitTorrent_Client_PeerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               BitTorrent_Client_PeerConnectionConfiguration,
                               struct BitTorrent_Client_PeerConnectionState,
                               BitTorrent_Statistic_t,
                               struct Net_TCPSocketConfiguration,
                               struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                               BitTorrent_Client_PeerStream_t,
                               struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_Client_AsynchPeerTCPConnection_t,
                                     ACE_INET_Addr,
                                     BitTorrent_Client_PeerConnectionConfiguration,
                                     struct BitTorrent_Client_PeerConnectionState,
                                     BitTorrent_Statistic_t,
                                     struct Net_TCPSocketConfiguration,
                                     struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                                     BitTorrent_Client_PeerStream_t,
                                     struct BitTorrent_Client_PeerUserData> BitTorrent_Client_AsynchPeerConnector_t;

// tracker
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               BitTorrent_Client_TrackerTCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               BitTorrent_Client_TrackerConnectionConfiguration,
                               struct BitTorrent_Client_TrackerConnectionState,
                               BitTorrent_Statistic_t,
                               struct Net_TCPSocketConfiguration,
                               struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                               BitTorrent_Client_TrackerStream_t,
                               struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_Client_AsynchTrackerTCPConnection_t,
                                     ACE_INET_Addr,
                                     BitTorrent_Client_TrackerConnectionConfiguration,
                                     struct BitTorrent_Client_TrackerConnectionState,
                                     BitTorrent_Statistic_t,
                                     struct Net_TCPSocketConfiguration,
                                     struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                                     BitTorrent_Client_TrackerStream_t,
                                     struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_AsynchTrackerConnector_t;

//////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_PeerConnectionConfiguration,
                                 struct BitTorrent_Client_PeerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_PeerUserData> BitTorrent_Client_IPeerConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_PeerConnectionConfiguration,
                                 struct BitTorrent_Client_PeerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_PeerUserData> BitTorrent_Client_PeerConnection_Manager_t;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_TrackerConnectionConfiguration,
                                 struct BitTorrent_Client_TrackerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_ITrackerConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_Client_TrackerConnectionConfiguration,
                                 struct BitTorrent_Client_TrackerConnectionState,
                                 BitTorrent_Statistic_t,
                                 struct BitTorrent_Client_TrackerUserData> BitTorrent_Client_TrackerConnection_Manager_t;

typedef ACE_Singleton<BitTorrent_Client_PeerConnection_Manager_t,
                      ACE_SYNCH_MUTEX> BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON;
typedef ACE_Singleton<BitTorrent_Client_TrackerConnection_Manager_t,
                      ACE_SYNCH_MUTEX> BITTORRENT_CLIENT_TRACKERCONNECTION_MANAGER_SINGLETON;

#endif
