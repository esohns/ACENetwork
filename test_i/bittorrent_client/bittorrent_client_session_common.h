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

#include "stream_statemachine_common.h"

#include "bittorrent_control.h"
#include "bittorrent_network.h"
#include "bittorrent_session.h"

#include "bittorrent_client_gui_common.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

struct BitTorrent_Client_SessionState
 : BitTorrent_SessionState
{
  BitTorrent_Client_SessionState ()
   : BitTorrent_SessionState ()
   , peerStreamHandler (NULL)
   ///////////////////////////////////////
   , trackerStreamHandler (NULL)
  {}

  BitTorrent_Client_PeerStreamHandler_t*    peerStreamHandler;

  BitTorrent_Client_TrackerStreamHandler_t* trackerStreamHandler;
};

typedef BitTorrent_Session_T<BitTorrent_Client_PeerConnectionConfiguration,
                             BitTorrent_Client_TrackerConnectionConfiguration,
                             struct BitTorrent_Client_PeerConnectionState,
                             BitTorrent_Client_PeerStream_t,
                             BitTorrent_Client_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
                             BitTorrent_Client_PeerStreamHandler_t,
                             BitTorrent_Client_TrackerStreamHandler_t,
                             BitTorrent_Client_PeerTCPConnection_t,
                             BitTorrent_Client_TrackerTCPConnection_t,
                             BitTorrent_Client_PeerConnection_Manager_t,
                             BitTorrent_Client_TrackerConnection_Manager_t,
                             BitTorrent_Client_PeerConnector_t,
                             BitTorrent_Client_TrackerConnector_t,
                             struct BitTorrent_Client_SessionConfiguration,
                             struct BitTorrent_Client_SessionState,
                             struct Stream_UserData,
                             struct Stream_UserData,
                             struct Net_UserData,
                             struct Net_UserData,
                             BitTorrent_Client_IControl_t,
                             struct BitTorrent_Client_UI_CBData> BitTorrent_Client_Session_t;
typedef BitTorrent_Session_T<BitTorrent_Client_PeerConnectionConfiguration,
                             BitTorrent_Client_TrackerConnectionConfiguration,
                             struct BitTorrent_Client_PeerConnectionState,
                             BitTorrent_Client_PeerStream_t,
                             BitTorrent_Client_TrackerStream_t,
                             enum Stream_StateMachine_ControlState,
                             BitTorrent_Client_PeerStreamHandler_t,
                             BitTorrent_Client_TrackerStreamHandler_t,
                             BitTorrent_Client_AsynchPeerTCPConnection_t,
                             BitTorrent_Client_AsynchTrackerTCPConnection_t,
                             BitTorrent_Client_PeerConnection_Manager_t,
                             BitTorrent_Client_TrackerConnection_Manager_t,
                             BitTorrent_Client_AsynchPeerConnector_t,
                             BitTorrent_Client_AsynchTrackerConnector_t,
                             struct BitTorrent_Client_SessionConfiguration,
                             struct BitTorrent_Client_SessionState,
                             struct Stream_UserData,
                             struct Stream_UserData,
                             struct Net_UserData,
                             struct Net_UserData,
                             BitTorrent_Client_IControl_t,
                             struct BitTorrent_Client_UI_CBData> BitTorrent_Client_AsynchSession_t;

//////////////////////////////////////////

typedef BitTorrent_Control_T<BitTorrent_Client_AsynchSession_t,
                             BitTorrent_Client_Session_t,
                             struct BitTorrent_Client_SessionConfiguration,
                             BitTorrent_Client_ISession_t,
                             struct BitTorrent_Client_SessionState> BitTorrent_Client_Control_t;

#endif
