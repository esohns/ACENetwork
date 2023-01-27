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

#ifndef IRC_CLIENT_SESSION_COMMON_H
#define IRC_CLIENT_SESSION_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"

#include "irc_common.h"
#include "irc_configuration.h"

#include "IRC_client_common.h"
#include "IRC_client_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (CURSES_SUPPORT)
#include "IRC_client_curses.h"
#endif // CURSES_SUPPORT
#endif // GUI_SUPPORT
#include "IRC_client_inputhandler.h"
#include "IRC_client_network.h"
#include "IRC_client_session.h"
//#include "IRC_client_stream.h"
#include "IRC_client_stream_common.h"

#if defined (GUI_SUPPORT)
#if defined (CURSES_SUPPORT)
typedef IRC_Client_Session_T<IRC_Client_TCPConnection_t,
                             struct IRC_Client_CursesState> IRC_Client_Session_t;
typedef IRC_Client_Session_T<IRC_Client_AsynchTCPConnection_t,
                             struct IRC_Client_CursesState> IRC_Client_AsynchSession_t;
#else
typedef IRC_Client_Session_T<IRC_Client_TCPConnection_t,
                             void> IRC_Client_Session_t;
typedef IRC_Client_Session_T<IRC_Client_AsynchTCPConnection_t,
                             void> IRC_Client_AsynchSession_t;
#endif // CURSES_SUPPORT
#else
typedef IRC_Client_Session_T<IRC_Client_TCPConnection_t,
                             void> IRC_Client_Session_t;
typedef IRC_Client_Session_T<IRC_Client_AsynchTCPConnection_t,
                             void> IRC_Client_AsynchSession_t;
#endif // GUI_SUPPORT

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               IRC_Client_Session_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               IRC_Client_ConnectionConfiguration,
                               struct IRC_SessionState,
                               IRC_Statistic_t,
                               Net_TCPSocketConfiguration_t,
                               IRC_Client_Stream_t,
                               struct Net_UserData> IRC_Client_SessionConnector_t;
typedef Net_Client_AsynchConnector_T<IRC_Client_AsynchSession_t,
                                     ACE_INET_Addr,
                                     IRC_Client_ConnectionConfiguration,
                                     struct IRC_SessionState,
                                     IRC_Statistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     IRC_Client_Stream_t,
                                     struct Net_UserData> IRC_Client_AsynchSessionConnector_t;

#endif
