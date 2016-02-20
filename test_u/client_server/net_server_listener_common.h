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

#ifndef NET_SERVER_LISTENER_COMMON_H
#define NET_SERVER_LISTENER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"

#include "net_server_asynchlistener.h"
#include "net_server_common.h"
#include "net_server_listener.h"

#include "test_u_stream_common.h"
#include "test_u_tcpconnection.h"

// foward declarations
class Net_Stream;

typedef Net_Server_AsynchListener_T<Net_AsynchTCPConnection,
                                    /////
                                    ACE_INET_Addr,
                                    Net_Server_ListenerConfiguration,
                                    Net_ConnectionState,
                                    Net_Stream,
                                    /////
                                    Net_SocketHandlerConfiguration,
                                    /////
                                    Net_UserData> Net_Server_AsynchListener_t;
typedef Net_Server_Listener_T<Net_TCPConnection,
                              ACE_SOCK_ACCEPTOR,
                              ///////////
                              ACE_INET_Addr,
                              Net_Server_ListenerConfiguration,
                              Net_ConnectionState,
                              Net_Stream,
                              ///////////
                              Net_SocketHandlerConfiguration,
                              ///////////
                              Net_UserData> Net_Server_Listener_t;

typedef ACE_Singleton<Net_Server_AsynchListener_t,
                      ACE_Recursive_Thread_Mutex> NET_SERVER_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Net_Server_Listener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> NET_SERVER_LISTENER_SINGLETON;

#endif
