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

#ifndef NET_DEFINES_H
#define NET_DEFINES_H

//#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//#include "linux/netlink.h"
//#endif

//#include "ace/Default_Constants.h"

//#include "stream_defines.h"

// interface
// *PORTABILITY*: interface names are not portable, so let the user choose the
//                interface from a list on Windows (see select_interface())...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_INTERFACE_DEFAULT                    ""
#else
#define NET_INTERFACE_DEFAULT                    "eth0"
#define NET_INTERFACE_LOOPBACK                   "lo"
#endif
#define NET_INTERFACE_DEFAULT_USE_LOOPBACK       false
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH   true // SIO_LOOPBACK_FAST_PATH
#endif

// addresses
#define NET_ADDRESS_DEFAULT_PORT                 10001
#define NET_ADDRESS_DEFAULT_IP_MULTICAST         "224.0.0.1"
#define NET_ADDRESS_DEFAULT_IP_BROADCAST         "255.255.255.255"

// protocol
// *IMPORTANT NOTE*: must match with the kernel module implementation !
#define NET_PROTOCOL_DEFAULT_NETLINK             NETLINK_GENERIC
#define NET_PROTOCOL_DEFAULT_NETLINK_GROUP       1

// socket
#define NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE   ACE_DEFAULT_MAX_SOCKET_BUFSIZ
#define NET_SOCKET_DEFAULT_LINGER                true  // SO_LINGER
#define NET_SOCKET_DEFAULT_TCP_NODELAY           true  // SO_NODELAY
#define NET_SOCKET_DEFAULT_TCP_KEEPALIVE         false // SO_KEEPALIVE

// connection / handler
#define NET_CONNECTION_MAXIMUM_NUMBER_OF_OPEN    10
#define NET_CONNECTION_HANDLER_THREAD_NAME       "connection dispatch"
#define NET_CONNECTION_HANDLER_THREAD_GROUP_ID   2

// event dispatch
// default event dispatcher (default: use asynch I/O (proactor))
#define NET_EVENT_USE_REACTOR                    false
#define NET_EVENT_USE_THREAD_POOL                false // multi-threaded event dispatch ?
#define NET_EVENT_TASK_GROUP_ID                  100

// stream
#define NET_STREAM_MAX_MESSAGES                  0  // 0 --> no limits
#define NET_STREAM_MESSAGE_DATA_BUFFER_SIZE      STREAM_MESSAGE_DATA_BUFFER_SIZE
#define NET_STREAM_DEFAULT_STATISTIC_REPORTING   0  // seconds [0: off]
#define NET_STREAM_DEFAULT_NAME                  "NetStream"

#endif
