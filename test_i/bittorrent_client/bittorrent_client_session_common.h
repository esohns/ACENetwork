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

#include "stream_common.h"

#include "bittorrent_session.h"

#include "bittorrent_client_common.h"
#include "bittorrent_client_configuration.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream.h"
#include "bittorrent_client_stream_common.h"

typedef BitTorrent_Session_T<struct BitTorrent_Client_Configuration,
                             struct BitTorrent_Client_ConnectionState,
                             BitTorrent_Client_PeerStream,
                             BitTorrent_Client_TrackerStream,
                             enum Stream_StateMachine_ControlState,
                             BitTorrent_Client_PeerTCPConnection_t,
                             BitTorrent_Client_TrackerTCPConnection_t,
                             BitTorrent_Client_IConnection_Manager_t,
                             struct BitTorrent_Client_SessionState> BitTorrent_Client_Session_t;
typedef BitTorrent_Session_T<struct BitTorrent_Client_Configuration,
                             struct BitTorrent_Client_ConnectionState,
                             BitTorrent_Client_PeerStream,
                             BitTorrent_Client_TrackerStream,
                             enum Stream_StateMachine_ControlState,
                             BitTorrent_Client_AsynchPeerTCPConnection_t,
                             BitTorrent_Client_AsynchTrackerTCPConnection_t,
                             BitTorrent_Client_IConnection_Manager_t,
                             struct BitTorrent_Client_SessionState> BitTorrent_Client_AsynchSession_t;

#endif
