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

#ifndef NET_SOCKET_COMMON_H
#define NET_SOCKET_COMMON_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/Netlink_Addr.h"
#endif
#include "ace/SOCK_Dgram.h"
//#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Dgram_Bcast.h"
#include "ace/SOCK_Dgram_Mcast.h"

#include "stream_common.h"

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "net_asynch_netlinksockethandler.h"
#endif
#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_configuration.h"
#include "net_stream.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_common.h"
#include "net_stream_tcpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "net_netlinksockethandler.h"
#endif
#include "net_tcpsockethandler.h"
#include "net_udpsockethandler.h"

typedef Net_StreamAsynchTCPSocketBase_T<ACE_INET_Addr,
                                        Net_SocketConfiguration,
                                        Net_Configuration,
                                        Stream_ModuleConfiguration,
                                        Net_StreamUserData,
                                        Net_ConnectionState,
                                        Stream_Statistic,
                                        Net_Stream,
                                        Net_AsynchTCPSocketHandler_T<Net_SocketHandlerConfiguration> > Net_AsynchTCPHandler_t;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
typedef Net_StreamAsynchUDPSocketBase_T<ACE_Netlink_Addr,
                                        Net_SocketConfiguration,
                                        Net_Configuration,
                                        Stream_ModuleConfiguration,
                                        Net_StreamUserData,
                                        Net_ConnectionState,
                                        Stream_Statistic,
                                        Net_Stream,
                                        ACE_SOCK_NETLINK,
                                        Net_AsynchNetlinkSocketHandler_T<Net_SocketHandlerConfiguration> > Net_AsynchNetlinkHandler_t;
#endif
typedef Net_StreamAsynchUDPSocketBase_T<ACE_INET_Addr,
                                        Net_SocketConfiguration,
                                        Net_Configuration,
                                        Stream_ModuleConfiguration,
                                        Net_StreamUserData,
                                        Net_ConnectionState,
                                        Stream_Statistic,
                                        Net_Stream,
                                        ACE_SOCK_DGRAM,
                                        Net_AsynchUDPSocketHandler_T<Net_SocketHandlerConfiguration> > Net_AsynchUDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<ACE_INET_Addr,
                                        Net_SocketConfiguration,
                                        Net_Configuration,
                                        Stream_ModuleConfiguration,
                                        Net_StreamUserData,
                                        Net_ConnectionState,
                                        Stream_Statistic,
                                        Net_Stream,
                                        ACE_SOCK_DGRAM_MCAST,
                                        Net_AsynchUDPSocketHandler_T<Net_SocketHandlerConfiguration> > Net_AsynchIPMulticastHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<ACE_INET_Addr,
                                        Net_SocketConfiguration,
                                        Net_Configuration,
                                        Stream_ModuleConfiguration,
                                        Net_StreamUserData,
                                        Net_ConnectionState,
                                        Stream_Statistic,
                                        Net_Stream,
                                        ACE_SOCK_DGRAM_BCAST,
                                        Net_AsynchUDPSocketHandler_T<Net_SocketHandlerConfiguration> > Net_AsynchIPBroadcastHandler_t;

typedef Net_StreamTCPSocketBase_T<ACE_INET_Addr,
                                  Net_SocketConfiguration,
                                  Net_Configuration,
                                  Stream_ModuleConfiguration,
                                  Net_StreamUserData,
                                  Net_ConnectionState,
                                  Stream_Statistic,
                                  Net_Stream,
                                  Net_TCPSocketHandler_T<Net_SocketHandlerConfiguration> > Net_TCPHandler_t;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
typedef Net_StreamUDPSocketBase_T<ACE_Netlink_Addr,
                                  Net_SocketConfiguration,
                                  Net_Configuration,
                                  Stream_ModuleConfiguration,
                                  Net_StreamUserData,
                                  Net_ConnectionState,
                                  Stream_Statistic,
                                  Net_Stream,
                                  Net_NetlinkSocketHandler_T<Net_SocketHandlerConfiguration> > Net_NetlinkHandler_t;
#endif
typedef Net_StreamUDPSocketBase_T<ACE_INET_Addr,
                                  Net_SocketConfiguration,
                                  Net_Configuration,
                                  Stream_ModuleConfiguration,
                                  Net_StreamUserData,
                                  Net_ConnectionState,
                                  Stream_Statistic,
                                  Net_Stream,
                                  Net_UDPSocketHandler_T<ACE_SOCK_DGRAM,
                                                         Net_SocketHandlerConfiguration> > Net_UDPHandler_t;
typedef Net_StreamUDPSocketBase_T<ACE_INET_Addr,
                                  Net_SocketConfiguration,
                                  Net_Configuration,
                                  Stream_ModuleConfiguration,
                                  Net_StreamUserData,
                                  Net_ConnectionState,
                                  Stream_Statistic,
                                  Net_Stream,
                                  Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_MCAST,
                                                         Net_SocketHandlerConfiguration> > Net_IPMulticastHandler_t;
typedef Net_StreamUDPSocketBase_T<ACE_INET_Addr,
                                  Net_SocketConfiguration,
                                  Net_Configuration,
                                  Stream_ModuleConfiguration,
                                  Net_StreamUserData,
                                  Net_ConnectionState,
                                  Stream_Statistic,
                                  Net_Stream,
                                  Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_BCAST,
                                                         Net_SocketHandlerConfiguration> > Net_IPBroadcastHandler_t;

#endif
