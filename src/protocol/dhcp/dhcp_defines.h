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

#ifndef DHCP_DEFINES_H
#define DHCP_DEFINES_H

// stream
#define DHCP_DEFAULT_MODULE_DISCOVER_NAME_STRING          "DHCPDiscover"
#define DHCP_DEFAULT_MODULE_MARSHAL_NAME_STRING           "DHCPMarshal"
#define DHCP_DEFAULT_MODULE_PARSER_NAME_STRING            "DHCPParser"
#define DHCP_DEFAULT_MODULE_STREAMER_NAME_STRING          "DHCPStreamer"
#define DHCP_DEFAULT_STREAM_NAME_STRING                   "DHCPStream"
#define DHCP_BUFFER_SIZE                                  1024

// "crunch" messages for easier parsing ?
// *NOTE*: this comes at the cost of alloc/free, memcopy and locking per
//         (fragmented) message, i.e. should probably be avoided ...
//         OTOH, setting up the buffer correctly allows using the
//         yy_scan_buffer() (instead of yy_scan_bytes()) method, avoiding a copy
//         of the data at that stage --> adding the easier/more robust parsing,
//         this MAY be a viable tradeoff...
// *NOTE*: the current implementation uses both approaches in different phases:
//         - yy_scan_bytes (extra copy) for bisecting the frames
//         - yy_scan_buffer (crunching) during parsing/analysis
// *TODO*: write a (robust) flex-scanner/bison parser that can handle
//         switching of buffers/"backing-up" reliably and stress-test the
//         application to see which option proves to be more efficient...
#define DHCP_DEFAULT_CRUNCH_MESSAGES                      true

#define DHCP_DEFAULT_STATISTIC_REPORTING_INTERVAL         0 // seconds {0: off}
#define DHCP_DEFAULT_PRINT_PROGRESSDOT                    false

// protocol
#define DHCP_DEFAULT_CLIENT_PORT                          68 // UDP
#define DHCP_DEFAULT_SERVER_PORT                          67 // UDP (bcast)

// *TODO*: this is the minimum, DHCP supports negotiation of larger frames
#define DHCP_MESSAGE_SIZE                                 576

// flags
#define DHCP_DEFAULT_FLAGS_BROADCAST                      false
#define DHCP_FLAGS_BROADCAST                              0x8000 // 2 bytes

#define DHCP_CHADDR_SIZE                                  16
#define DHCP_SNAME_SIZE                                   64
#define DHCP_FILE_SIZE                                    128

// *NOTE*: already in network (== big-endian) byte order
//         --> swap on little-endian systems
#define DHCP_MAGIC_COOKIE                                 0x63825363 // 99.130.83.99
#define DHCP_OPTIONS_SIZE                                 312 // min. size

#define DHCP_OPTION_TAG_PAD                               0
#define DHCP_OPTION_TAG_END                               255

// *NOTE*: see RFC-2131 (Table 1), and "ARP" section of RFC-1700
// *TODO*: should correspond with the target hardware
#define DHCP_FRAME_HTYPE                                  1 // --> (10Mb) Ethernet
// *TODO*: should correspond with the target hardware
#define DHCP_FRAME_HLEN                                   6 // --> (10Mb) Ethernet

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DHCLIENT_SUPPORT)
#define DHCP_DHCLIENT_CONNECTION_RETRIES                  3
#define DHCP_DHCLIENT_INTERFACE_STATE_UP_STRING           "up"
#define DHCP_DHCLIENT_LEASES_FILE                         "/var/lib/dhcp/dhclient.leases"
#define DHCP_DHCLIENT_LOCALHOST_IP_STRING                 "127.0.0.1"
#define DHCP_DHCLIENT_OMAPI_PORT                          7911
#define DHCP_DHCLIENT_STRING                              "dhclient"
#define DHCP_DHCLIENT_SWITCH_RUN_IN_FOREGROUND_STRING     "d"

#define DHCP_DHCLIENT_OBJECT_INTERFACE_STRING             "interface"
#define DHCP_DHCLIENT_OBJECT_LEASE_STRING                 "lease"
#define DHCP_DHCLIENT_OBJECT_VALUE_HARDWAREADDRESS_STRING "hardware-address"
#define DHCP_DHCLIENT_OBJECT_VALUE_HARDWARETYPE_STRING    "hardware-type"
#define DHCP_DHCLIENT_OBJECT_VALUE_IPADDRESS_STRING       "ip-address"
#define DHCP_DHCLIENT_OBJECT_VALUE_NAME_STRING            "name"
#define DHCP_DHCLIENT_OBJECT_VALUE_STATE_STRING           "state"
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#endif
