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
// *PORTABILITY*: NIC device names are not portable
//                Win32: let the user choose the device identifier from a list
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH               true // SIO_LOOPBACK_FAST_PATH

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
#define NET_PROTOCOL_DEFAULT_UDP_BUFFER_SIZE                 65507

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
#define NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE               ACE_DEFAULT_MAX_SOCKET_BUFSIZ
#define NET_SOCKET_DEFAULT_TCP_NODELAY                       true  // SO_NODELAY
#define NET_SOCKET_DEFAULT_TCP_KEEPALIVE                     false // SO_KEEPALIVE
#define NET_SOCKET_DEFAULT_UDP_CONNECT                       false

// connection / handler
#define NET_CONNECTION_HANDLER_THREAD_NAME                   "connection dispatch"
#define NET_CONNECTION_HANDLER_THREAD_GROUP_ID               20
#define NET_CONNECTION_MAXIMUM_NUMBER_OF_OPEN                10

#define NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT        3 // seconds

// (asynchronous) connections
#define NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT                60 // second(s)
#define NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_INTERVAL       1  // second(s)

// event dispatch
//#define NET_EVENT_DISPATCH_THREAD_GROUP_ID                  22
//#define NET_EVENT_USE_THREAD_POOL                           false // multi-threaded event dispatch ? (reactor only)
#define NET_EVENT_DEFAULT_DISPATCH                           COMMON_EVENT_DISPATCH_PROACTOR

// stream
#define NET_STREAM_MAX_MESSAGES                              0  // 0 --> no limits
#define NET_STREAM_MESSAGE_DATA_BUFFER_SIZE                  16384 // bytes

#define NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL      0  // seconds [0: off]
#define NET_STREAM_DEFAULT_NAME                              "NetStream"

#define NET_STREAM_MODULE_SOCKETHANDLER_DEFAULT_NAME_STRING  "SocketHandler"

// statistic
#define NET_STATISTIC_DEFAULT_COLLECTION_INTERVAL            500 // ms [0: off]
#define NET_STATISTIC_DEFAULT_REPORTING_INTERVAL             0 // second(s) [0: off]

// platform
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define NET_EXE_NETWORKMANAGER_STRING                        "NetworkManager"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_SUPPORT)
#define NET_EXE_DHCLIENT_STRING                              "dhclient"
#define NET_EXE_DHCLIENT_LOCALHOST_IP_STRING                 "127.0.0.1"
#define NET_EXE_DHCLIENT_OMAPI_PORT                          7911

#define NET_EXE_DHCLIENT_OBJECT_INTERFACE_STRING             "interface"
#define NET_EXE_DHCLIENT_OBJECT_LEASE_STRING                 "lease"
#define NET_EXE_DHCLIENT_OBJECT_VALUE_HARDWAREADDRESS_STRING "hardware-address"
#define NET_EXE_DHCLIENT_OBJECT_VALUE_HARDWARETYPE_STRING    "hardware-type"
#define NET_EXE_DHCLIENT_OBJECT_VALUE_IPADDRESS_STRING       "ip-address"
#define NET_EXE_DHCLIENT_OBJECT_VALUE_NAME_STRING            "name"
#define NET_EXE_DHCLIENT_OBJECT_VALUE_STATE_STRING           "state"
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#endif
