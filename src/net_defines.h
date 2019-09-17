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

#include "ace/config-lite.h"

// interface
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#define NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH               true // SIO_LOOPBACK_FAST_PATH
#else
#define NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH               false // SIO_LOOPBACK_FAST_PATH
#endif // _WIN32_WINNT_WIN8

// *PORTABILITY*: NIC device names are not portable
//                Win32: let the user choose the device identifier from a list
#define NET_INTERFACE_DEFAULT_ETHERNET                       ""
#define NET_INTERFACE_DEFAULT_LOOPBACK                       ""
#define NET_INTERFACE_DEFAULT_PPP                            ""
#define NET_INTERFACE_DEFAULT_WLAN                           ""
#else
#define NET_INTERFACE_DEFAULT_ETHERNET                       "eth0"
#define NET_INTERFACE_DEFAULT_LOOPBACK                       "lo"
#define NET_INTERFACE_DEFAULT_PPP                            "ppp0"
//#define NET_INTERFACE_DEFAULT_WLAN                          "wlan0" // Debian-style
#define NET_INTERFACE_DEFAULT_WLAN                           "wlp3s0" // openSUSE/Ubuntu-style
#endif // ACE_WIN32 || ACE_WIN64
#define NET_INTERFACE_DEFAULT_USE_LOOPBACK                   false

// addresses
// *TODO*: use platform macros wherever possible
#define NET_ADDRESS_LINK_ETHERNET_ADDRESS_STRING_SIZE        (ETH_ALEN * 2) + (ETH_ALEN - 1) + 1 // "ab:cd:ef:gh:ij:kl\0"

#if defined (ACE_LINUX)
// *NOTE*: binding to these ports requires the CAP_NET_BIND_SERVICE capability
#define NET_ADDRESS_MAXIMUM_PRIVILEGED_PORT                  1023
#endif // ACE_LINUX
//#define NET_ADDRESS_IP_BROADCAST                        "255.255.255.255"
#define NET_ADDRESS_NSLOOKUP_RESULT_ADDRESS_KEY_STRING       "Address"

#define NET_ADDRESS_DEFAULT_IP_MULTICAST                     "224.0.0.1"
#define NET_ADDRESS_DEFAULT_PORT                             10001

// protocols
// --- Netlink ---

// *IMPORTANT NOTE*: must match with the kernel module implementation !
#define NET_PROTOCOL_DEFAULT_NETLINK                         NETLINK_GENERIC
#define NET_PROTOCOL_DEFAULT_NETLINK_GROUP                   1

// --- UDP ---

// *NOTE*: see also: SO_MAX_MSG_SIZE
#define NET_PROTOCOL_DEFAULT_UDP_BUFFER_SIZE                 65507 // bytes

// protocol parsers
//#define YY_END_OF_BUFFER_CHAR                               0 // "\0\0"
//#define NET_PROTOCOL_PARSER_FLEX_BUFFER_BOUNDARY_SIZE       2 // byte(s)
// *IMPORTANT NOTE*: scans buffers in-place (avoids a copy,
//         see: http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html)
//         --> in order to use yy_scan_buffer(), the buffer needs to have been
//             prepared for usage by flex: buffers need two trailing '\0's
//             BEYOND their datas' tail byte (i.e. at positions length() + 1 and
//             length() + 2)
//#define NET_PROTOCOL_PARSER_FLEX_USE_YY_SCAN_BUFFER         true

// output more debugging information ?
//#define NET_PROTOCOL_PARSER_DEFAULT_LEX_TRACE               false
//#define NET_PROTOCOL_PARSER_DEFAULT_YACC_TRACE              false

// socket
#if defined (ACE_LINUX)
#define NET_SOCKET_DEFAULT_ERRORQUEUE                        true  // IP_RECVERR
#endif // ACE_LINUX
#define NET_SOCKET_DEFAULT_LINGER                            true  // SO_LINGER
#define NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE               ACE_DEFAULT_MAX_SOCKET_BUFSIZ // bytes
#define NET_SOCKET_DEFAULT_TCP_NODELAY                       true  // SO_NODELAY
#define NET_SOCKET_DEFAULT_TCP_KEEPALIVE                     false // SO_KEEPALIVE
#define NET_SOCKET_DEFAULT_UDP_CONNECT                       false

// connection / handler
#define NET_CONNECTION_HANDLER_THREAD_NAME                   "connection dispatch"
#define NET_CONNECTION_HANDLER_THREAD_GROUP_ID               20
#define NET_CONNECTION_MAXIMUM_NUMBER_OF_OPEN                10

#define NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT_S      5 // seconds

// (asynchronous) connections
#define NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_S              10 // second(s)
#define NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_INTERVAL_S     1  // second(s)

// event dispatch
//#define NET_EVENT_DISPATCH_THREAD_GROUP_ID                  22
//#define NET_EVENT_USE_THREAD_POOL                           false // multi-threaded event dispatch ? (reactor only)
#define NET_EVENT_DEFAULT_DISPATCH                           COMMON_EVENT_DISPATCH_PROACTOR

// stream
#define NET_STREAM_MAX_MESSAGES                              0  // 0 --> no limits
#define NET_STREAM_MESSAGE_DATA_BUFFER_SIZE                  16384 // bytes

#define NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S    0  // seconds [0: off]
#define NET_STREAM_DEFAULT_NAME                              "NetStream"

#define NET_STREAM_MODULE_SOCKETHANDLER_DEFAULT_NAME_STRING  "SocketHandler"

// statistic
// *NOTE*: lower values --> more overhead, better estimations
//         higher values --> less overhead, worse estimations
#define NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS              500 // ms
#define NET_STATISTIC_DEFAULT_COLLECTION_INTERVAL_MS         0 // ms [0: off]
#define NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S           0 // second(s) [0: off]

// platform
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define NET_EXE_NETWORKMANAGER_STRING                        "NetworkManager"
#endif // ACE_WIN32 || ACE_WIN64

// modules
#define NET_MODULE_CLIENTSERVER_DIRECTORY_STRING             "client_server"
#define NET_MODULE_PROTOCOL_DIRECTORY_STRING                 "protocol"
#define NET_MODULE_WLAN_DIRECTORY_STRING                     "wlan"

#endif
