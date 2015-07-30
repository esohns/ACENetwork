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
//#include "ace/Singleton.h"
//#include "ace/Synch.h"
//
#include "stream_common.h"
//
//#include "net_asynch_tcpsockethandler.h"
#include "net_configuration.h"
//#include "net_connection_manager.h"
//#include "net_iconnectionmanager.h"
//#include "net_stream_asynch_tcpsocket_base.h"
//#include "net_stream_tcpsocket_base.h"
//#include "net_tcpsockethandler.h"
//#include "net_tcpconnection_base.h"
//
//#include "net_client_asynchconnector.h"
//#include "net_client_connector.h"
//
#include "IRC_client_configuration.h"
#include "IRC_client_network.h"
#include "IRC_client_stream.h"
#include "IRC_client_stream_common.h"
#include "IRC_client_IRCsession.h"

typedef Net_Client_Connector_T<IRC_Client_TCPConnection_t,
                               //////////
                               ACE_INET_Addr,
                               IRC_Client_Configuration,
                               IRC_Client_ConnectionState,
                               IRC_Client_Stream,
                               //////////
                               IRC_Client_SocketHandlerConfiguration,
                               //////////
                               IRC_Client_StreamUserData> IRC_Client_Connector_t;
typedef Net_Client_AsynchConnector_T<IRC_Client_AsynchTCPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     IRC_Client_Configuration,
                                     IRC_Client_ConnectionState,
                                     IRC_Client_Stream,
                                     ////
                                     IRC_Client_SocketHandlerConfiguration,
                                     ////
                                     IRC_Client_StreamUserData> IRC_Client_AsynchConnector_t;

typedef IRC_Client_IRCSession_T<IRC_Client_TCPConnection_t> IRC_Client_IRCSession_t;
typedef IRC_Client_IRCSession_T<IRC_Client_AsynchTCPConnection_t> IRC_Client_AsynchIRCSession_t;

typedef Net_Client_Connector_T<IRC_Client_IRCSession_t,
                               //////////
                               ACE_INET_Addr,
                               IRC_Client_Configuration,
                               IRC_Client_ConnectionState,
                               IRC_Client_Stream,
                               //////////
                               IRC_Client_SocketHandlerConfiguration,
                               //////////
                               IRC_Client_StreamUserData> IRC_Client_SessionConnector_t;
typedef Net_Client_AsynchConnector_T<IRC_Client_AsynchIRCSession_t,
                                     ////
                                     ACE_INET_Addr,
                                     IRC_Client_Configuration,
                                     IRC_Client_ConnectionState,
                                     IRC_Client_Stream,
                                     ////
                                     IRC_Client_SocketHandlerConfiguration,
                                     ////
                                     IRC_Client_StreamUserData> IRC_Client_AsynchSessionConnector_t;

#endif
