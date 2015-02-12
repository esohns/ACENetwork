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

#include "ace/Default_Constants.h"

// *** network-related ***
// *PORTABILITY*: interface names are not portable, so we let the
// user choose the interface from a list on Windows (see select_Interface())...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_DEFAULT_NETWORK_INTERFACE             ""
#else
#define NET_DEFAULT_NETWORK_INTERFACE             "eth0"
#endif

#define NET_DEFAULT_PORT                          32767
#define NET_DEFAULT_IP_MULTICAST_ADDRESS          "224.0.0.1"
#define NET_DEFAULT_IP_BROADCAST_ADDRESS          "255.255.255.255"

// default event dispatcher (default: use asynch I/O (proactor))
#define NET_USES_REACTOR                          false
#define NET_CONNECTION_HANDLER_THREAD_NAME        "connection dispatch"
#define NET_CONNECTION_HANDLER_THREAD_GROUP_ID    2

#define NET_DEFAULT_SOCKET_RECEIVE_BUFFER_SIZE    ACE_DEFAULT_MAX_SOCKET_BUFSIZ
#define NET_DEFAULT_TCP_NODELAY                   true
#define NET_DEFAULT_TCP_KEEPALIVE                 false
#define NET_DEFAULT_TCP_LINGER                    10 // seconds {0 --> off}

#define NET_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS    10

// *** pro/reactor-related ***
#define NET_TASK_GROUP_ID                         100

#define NET_STATISTICS_COLLECTION_INTERVAL        60 // seconds [0 --> OFF]
#define NET_DEFAULT_STATISTICS_REPORTING_INTERVAL 0  // seconds [0 --> OFF]

#endif
