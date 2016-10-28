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
#include <ace/SOCK_Connector.h>

#include "bittorrent_common.h"
#include "bittorrent_session.h"

#include "bittorrent_client_common.h"
#include "bittorrent_client_configuration.h"
#include "bittorrent_client_curses.h" // *TODO*: remove this ASAP
#include "bittorrent_client_inputhandler.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream.h"
#include "bittorrent_client_stream_common.h"

typedef Net_Client_Connector_T<BitTorrent_Client_TCPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               BitTorrent_Client_Configuration,
                               BitTorrent_Client_SessionState,
                               BitTorrent_RuntimeStatistic_t,
                               BitTorrent_Client_Stream,
                               BitTorrent_Client_SocketHandlerConfiguration,
                               BitTorrent_Client_UserData> BitTorrent_Client_Connector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_Client_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     BitTorrent_Client_Configuration,
                                     BitTorrent_Client_SessionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     BitTorrent_Client_Stream,
                                     BitTorrent_Client_SocketHandlerConfiguration,
                                     BitTorrent_Client_UserData> BitTorrent_Client_AsynchConnector_t;

typedef BitTorrent_Session_T<BitTorrent_Client_TCPConnection_t,
                             BitTorrent_Client_SessionData,
//                      BitTorrent_IControl,
                             BitTorrent_Client_ISessionNotify_t,
                             BitTorrent_Client_Configuration,
                             BitTorrent_Message,
                             BitTorrent_Client_SessionMessage,
                             BitTorrent_Client_SocketHandlerConfiguration,
                             BitTorrent_Client_ModuleHandlerConfiguration,
                             BitTorrent_Client_CursesState,
                             BitTorrent_Client_IConnection_Manager_t,
                             BitTorrent_Client_InputHandler,
                             BitTorrent_Client_InputHandlerConfiguration,
                             BitTorrent_Client_IOStream_t> BitTorrent_Client_Session_t;
typedef BitTorrent_Session_T<BitTorrent_Client_AsynchTCPConnection_t,
                             BitTorrent_Client_SessionData,
//                      BitTorrent_IControl,
                             BitTorrent_Client_ISessionNotify_t,
                             BitTorrent_Client_Configuration,
                             BitTorrent_Message,
                             BitTorrent_Client_SessionMessage,
                             BitTorrent_Client_SocketHandlerConfiguration,
                             BitTorrent_Client_ModuleHandlerConfiguration,
                             BitTorrent_Client_CursesState,
                             BitTorrent_Client_IConnection_Manager_t,
                             BitTorrent_Client_InputHandler,
                             BitTorrent_Client_InputHandlerConfiguration,
                             BitTorrent_Client_IOStream_t> BitTorrent_Client_AsynchSession_t;

typedef Net_Client_Connector_T<BitTorrent_Client_Session_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               BitTorrent_Client_Configuration,
                               BitTorrent_Client_SessionState,
                               BitTorrent_RuntimeStatistic_t,
                               BitTorrent_Client_Stream,
                               BitTorrent_Client_SocketHandlerConfiguration,
                               BitTorrent_Client_UserData> BitTorrent_Client_SessionConnector_t;
typedef Net_Client_AsynchConnector_T<BitTorrent_Client_AsynchSession_t,
                                     ACE_INET_Addr,
                                     BitTorrent_Client_Configuration,
                                     BitTorrent_Client_SessionState,
                                     BitTorrent_RuntimeStatistic_t,
                                     BitTorrent_Client_Stream,
                                     BitTorrent_Client_SocketHandlerConfiguration,
                                     BitTorrent_Client_UserData> BitTorrent_Client_AsynchSessionConnector_t;

#endif
