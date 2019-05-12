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

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
#include "net_asynch_netlinksockethandler.h"
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_connection_configuration.h"
#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
#include "net_netlinksockethandler.h"
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"
#include "net_udpconnection_base.h"
#include "net_udpsockethandler.h"

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_stream_common.h"

// forward declarations
class Test_U_Stream;

//typedef Net_AsynchTCPSocketHandler_T<Net_TCPSocketConfiguration_t> Test_U_AsynchTCPSocketHandler_t;
//
//#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
//typedef Net_AsynchNetlinkSocketHandler_T<Net_NetlinkSocketConfiguration_t> Test_U_AsynchNetlinkSocketHandler_t;
//#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
//
//typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram,
//                                     Net_UDPSocketConfiguration_t> Test_U_AsynchUDPSocketHandler_t;
//typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Mcast,
//                                     Net_UDPSocketConfiguration_t> Test_U_AsynchIPMulticastSocketHandler_t;
//typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram_Bcast,
//                                     Net_UDPSocketConfiguration_t> Test_U_AsynchIPBroadcastSocketHandler_t;
//
////////////////////////////////////////////
//
//typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
//                               ACE_SOCK_STREAM,
//                               Net_TCPSocketConfiguration_t> Test_U_TCPSocketHandler_t;
//
//#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
//typedef Net_NetlinkSocketHandler_T<Net_NetlinkSocketConfiguration_t> Test_U_NetlinkSocketHandler_t;
//#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
//
//typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
//                               Net_SOCK_Dgram,
//                               Net_UDPSocketConfiguration_t> Test_U_UDPSocketHandler_t;
//typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
//                               Net_SOCK_Dgram_Mcast,
//                               Net_UDPSocketConfiguration_t> Test_U_IPMulticastSocketHandler_t;
//typedef Net_UDPSocketHandler_T<ACE_MT_SYNCH,
//                               Net_SOCK_Dgram_Bcast,
//                               Net_UDPSocketConfiguration_t> Test_U_IPBroadcastSocketHandler_t;

#endif
