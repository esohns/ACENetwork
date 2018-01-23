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

#ifndef TEST_U_SOCKET_COMMON_H
#define TEST_U_SOCKET_COMMON_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Dgram.h"
//#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Dgram_Bcast.h"
#include "ace/SOCK_Dgram_Mcast.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_asynch_netlinksockethandler.h"
#endif
#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_netlinksockethandler.h"
#endif
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"
#include "net_udpconnection_base.h"
#include "net_udpsockethandler.h"

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_stream_common.h"

// forward declarations
class Test_U_Stream;

typedef Net_AsynchTCPSocketHandler_T<struct ClientServer_SocketHandlerConfiguration> ClientServer_AsynchTCPSocketHandler_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_AsynchNetlinkSocketHandler_T<struct ClientServer_SocketHandlerConfiguration> ClientServer_AsynchNetlinkSocketHandler_t;
#endif

typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram,
                                     struct ClientServer_SocketHandlerConfiguration> ClientServer_AsynchUDPSocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Mcast,
                                     struct ClientServer_SocketHandlerConfiguration> ClientServer_AsynchIPMulticastSocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Bcast,
                                     struct ClientServer_SocketHandlerConfiguration> ClientServer_AsynchIPBroadcastSocketHandler_t;

typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
                               ACE_SOCK_STREAM,
                               struct ClientServer_SocketHandlerConfiguration> ClientServer_TCPSocketHandler_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_NetlinkSocketHandler_T<struct ClientServer_SocketHandlerConfiguration> ClientServer_NetlinkSocketHandler_t;
#endif
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram,
                               struct ClientServer_SocketHandlerConfiguration> ClientServer_UDPSocketHandler_t;
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram_Mcast,
                               struct ClientServer_SocketHandlerConfiguration> ClientServer_IPMulticastSocketHandler_t;
typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
                               Net_SOCK_Dgram_Bcast,
                               struct ClientServer_SocketHandlerConfiguration> ClientServer_IPBroadcastSocketHandler_t;

#endif
