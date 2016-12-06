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

#ifndef BITTORRENT_CLIENT_SESSION_COMMON_H
#define BITTORRENT_CLIENT_SESSION_COMMON_H

#include <ace/INET_Addr.h>

#include "stream_common.h"

#include "net_configuration.h"

#include "bittorrent_isession.h"
#include "bittorrent_network.h"
#include "bittorrent_session.h"

#include "bittorrent_client_configuration.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

#include "bittorrent_client_gui_common.h"

// forward declarations
struct BitTorrent_Client_PeerConnectionConfiguration;
struct BitTorrent_Client_TrackerConnectionConfiguration;
struct BitTorrent_Client_PeerConnectionState;
struct BitTorrent_Client_PeerSocketHandlerConfiguration;
struct BitTorrent_Client_TrackerSocketHandlerConfiguration;

struct BitTorrent_Client_SessionConfiguration;
struct BitTorrent_Client_SessionState;
//typedef BitTorrent_ISession_T<ACE_INET_Addr,
//                              struct BitTorrent_Client_PeerConnectionConfiguration,
//                              struct BitTorrent_Client_TrackerConnectionConfiguration,
//                              struct BitTorrent_Client_PeerConnectionState,
//                              BitTorrent_RuntimeStatistic_t,
//                              struct Net_SocketConfiguration,
//                              struct BitTorrent_Client_PeerSocketHandlerConfiguration,
//                              struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
//                              BitTorrent_Client_PeerStream_t,
//                              enum Stream_StateMachine_ControlState,
//                              struct BitTorrent_Client_SessionConfiguration,
//                              struct BitTorrent_Client_SessionState> BitTorrent_Client_ISession_t;

template <typename SessionInterfaceType>
class BitTorrent_IControl_T;
typedef BitTorrent_IControl_T<BitTorrent_Client_ISession_t> BitTorrent_Client_IControl_t;
struct BitTorrent_Client_SessionConfiguration
 : BitTorrent_SessionConfiguration
{
  inline BitTorrent_Client_SessionConfiguration ()
   : BitTorrent_SessionConfiguration ()
   , connectionManager (NULL)
   , controller (NULL)
   , trackerConnectionManager (NULL)
   , socketHandlerConfiguration (NULL)
   , trackerSocketHandlerConfiguration (NULL)
  {};

  BitTorrent_Client_PeerConnection_Manager_t*                 connectionManager;
  BitTorrent_Client_IControl_t*                               controller;
  BitTorrent_Client_TrackerConnection_Manager_t*              trackerConnectionManager;
  struct BitTorrent_Client_PeerSocketHandlerConfiguration*    socketHandlerConfiguration;
  struct BitTorrent_Client_TrackerSocketHandlerConfiguration* trackerSocketHandlerConfiguration;
};

//typedef std::map<std::string, BitTorrent_Client_Connections_t> BitTorrent_Client_SessionConnections_t;
//typedef BitTorrent_Client_SessionConnections_t::iterator BitTorrent_Client_SessionConnectionsIterator_t;
struct BitTorrent_Client_SessionState
 : BitTorrent_SessionState
{
  inline BitTorrent_Client_SessionState ()
   : controller (NULL)
   , session (NULL)
  {};

  BitTorrent_Client_IControl_t* controller;
  BitTorrent_Client_ISession_t* session;
};

typedef BitTorrent_Session_T<struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                             struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                             struct BitTorrent_Client_PeerConnectionConfiguration,
                             struct BitTorrent_Client_TrackerConnectionConfiguration,
                             struct BitTorrent_Client_PeerConnectionState,
                             BitTorrent_Client_PeerStream_t,
                             BitTorrent_Client_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
                             struct BitTorrent_Client_PeerModuleHandlerConfiguration,
                             struct BitTorrent_Client_TrackerModuleHandlerConfiguration,
                             BitTorrent_Client_PeerTCPConnection_t,
                             BitTorrent_Client_TrackerTCPConnection_t,
                             BitTorrent_Client_IPeerConnection_Manager_t,
                             BitTorrent_Client_ITrackerConnection_Manager_t,
                             BitTorrent_Client_PeerConnector_t,
                             BitTorrent_Client_TrackerConnector_t,
                             struct BitTorrent_Client_SessionConfiguration,
                             struct BitTorrent_Client_SessionState,
                             struct BitTorrent_Client_PeerUserData,
                             struct BitTorrent_Client_TrackerUserData,
                             BitTorrent_Client_IControl_t,
                             struct BitTorrent_Client_GTK_CBData> BitTorrent_Client_Session_t;
typedef BitTorrent_Session_T<struct BitTorrent_Client_PeerSocketHandlerConfiguration,
                             struct BitTorrent_Client_TrackerSocketHandlerConfiguration,
                             struct BitTorrent_Client_PeerConnectionConfiguration,
                             struct BitTorrent_Client_TrackerConnectionConfiguration,
                             struct BitTorrent_Client_PeerConnectionState,
                             BitTorrent_Client_PeerStream_t,
                             BitTorrent_Client_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
                             struct BitTorrent_Client_PeerModuleHandlerConfiguration,
                             struct BitTorrent_Client_TrackerModuleHandlerConfiguration,
                             BitTorrent_Client_AsynchPeerTCPConnection_t,
                             BitTorrent_Client_AsynchTrackerTCPConnection_t,
                             BitTorrent_Client_IPeerConnection_Manager_t,
                             BitTorrent_Client_ITrackerConnection_Manager_t,
                             BitTorrent_Client_AsynchPeerConnector_t,
                             BitTorrent_Client_AsynchTrackerConnector_t,
                             struct BitTorrent_Client_SessionConfiguration,
                             struct BitTorrent_Client_SessionState,
                             struct BitTorrent_Client_PeerUserData,
                             struct BitTorrent_Client_TrackerUserData,
                             BitTorrent_Client_IControl_t,
                             struct BitTorrent_Client_GTK_CBData> BitTorrent_Client_AsynchSession_t;

//////////////////////////////////////////

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
class BitTorrent_Control_T;
typedef BitTorrent_Control_T<BitTorrent_Client_AsynchSession_t,
                             BitTorrent_Client_Session_t,
                             struct BitTorrent_Client_SessionConfiguration,
                             BitTorrent_Client_ISession_t,
                             struct BitTorrent_Client_SessionState> BitTorrent_Client_Control_t;

#endif
