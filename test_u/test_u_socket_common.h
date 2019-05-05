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

#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram.h>
//#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Dgram_Bcast.h>
#include <ace/SOCK_Dgram_Mcast.h>

#include "stream_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_asynch_netlinksockethandler.h"
#endif
#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_netlinksockethandler.h"
#endif
#include "net_tcpsockethandler.h"
#include "net_udpsockethandler.h"

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_stream_common.h"

// forward declarations
class Test_U_Stream;

typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_Statistic_t,
                                        Test_U_Stream,
                                        struct Net_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration> Test_U_AsynchTCPHandler_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        Net_SOCK_Netlink,
                                        Net_Netlink_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_Statistic_t,
                                        Test_U_Stream,
                                        struct Net_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct Test_U_SocketHandlerConfiguration> Test_U_AsynchNetlinkHandler_t;
#endif

typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        ACE_SOCK_DGRAM,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_Statistic_t,
                                        Test_U_Stream,
                                        struct Net_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct Test_U_SocketHandlerConfiguration> Test_U_AsynchUDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        ACE_SOCK_DGRAM_MCAST,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_Statistic_t,
                                        Test_U_Stream,
                                        struct Net_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct Test_U_SocketHandlerConfiguration> Test_U_AsynchIPMulticastHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                        ACE_SOCK_DGRAM_BCAST,
                                        ACE_INET_Addr,
                                        struct Test_U_ConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_Statistic_t,
                                        Test_U_Stream,
                                        struct Net_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_ModuleHandlerConfiguration,
                                        struct Test_U_SocketHandlerConfiguration> Test_U_AsynchIPBroadcastHandler_t;

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_U_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_Statistic_t,
                                  Test_U_Stream,
                                  struct Net_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration> Test_U_TCPHandler_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<struct Test_U_SocketHandlerConfiguration>,
                                  Net_Netlink_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_Statistic_t,
                                  Test_U_Stream,
                                  struct Net_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration,
                                  struct Test_U_SocketHandlerConfiguration> Test_U_NetlinkHandler_t;
#endif

//typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_SOCK_DGRAM,
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         struct Test_U_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_Statistic_t,
                                  Test_U_Stream,
                                  struct Net_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration,
                                  struct Test_U_SocketHandlerConfiguration> Test_U_UDPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_MCAST,
                                                         struct Test_U_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_Statistic_t,
                                  Test_U_Stream,
                                  struct Net_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration,
                                  struct Test_U_SocketHandlerConfiguration> Test_U_IPMulticastHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_BCAST,
                                                         struct Test_U_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_U_ConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_Statistic_t,
                                  Test_U_Stream,
                                  struct Net_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_U_ModuleHandlerConfiguration,
                                  struct Test_U_SocketHandlerConfiguration> Test_U_IPBroadcastHandler_t;

#endif
