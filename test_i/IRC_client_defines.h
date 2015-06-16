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

#ifndef IRC_CLIENT_DEFINES_H
#define IRC_CLIENT_DEFINES_H

// protocol
// "\0\0"
#define YY_END_OF_BUFFER_CHAR                    0
#define IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE     2
// CRLF = "\r\n"
#define IRC_CLIENT_IRC_FRAME_BOUNDARY            "\r\n"
#define IRC_CLIENT_IRC_FRAME_BOUNDARY_SIZE       2
#define IRC_CLIENT_IRC_FRAME_MAXSIZE             512

#define IRC_CLIENT_IRC_MAX_WELCOME_DELAY         3 // seconds

// stream
// *NOTE*: according to RFC1459, IRC messages SHALL not exceed 512 bytes.
// - a size of 512 bytes will allow "crunching" messages into a single buffer
//   --> while this arguably "wastes" some memory, it allows [easier/more robust]
//       scanning / parsing...
// - provide an extra 2 '\0' "resilience" bytes needed for scanning with "flex"
// *WARNING*: be aware that a single read from the connected socket may well
// cover MORE than one complete message at a time, so this value is just a
// (somewhat qualified) suggestion...
#define IRC_CLIENT_BUFFER_SIZE                   (IRC_CLIENT_IRC_FRAME_MAXSIZE + IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE)

// "crunch" messages for easier parsing ?
// *NOTE*: comes at the cost of alloc/free, memcopy and locking per fragmented
// message --> should probably be avoided ...
// OTOH, setting up the buffer correctly allows using the yy_scan_buffer()
// (instead of yy_scan_bytes()) call, avoiding a copy of the data at that stage
// --> adding the easier/more robust parsing, this MAY be a viable tradeoff...
// *NOTE*: the current implementation uses both approaches in different phases:
// - yy_scan_bytes (extra copy) for bisecting the frames
// - yy_scan_buffer (crunching) during parsing/analysis
// *TODO*: write a (robust) flex-scanner/bison parser that can handle
// switching of buffers/"backing-up" reliably and stress-test the application
// to see which option proves to be better...
#define IRC_CLIENT_DEF_CRUNCH_MESSAGES           true

// output more debugging information
#define IRC_CLIENT_DEF_LEX_TRACE                 false
#define IRC_CLIENT_DEF_YACC_TRACE                false

//#define IRC_CLIENT_STREAM_DEF_TRACE_ENABLED       false
#define IRC_CLIENT_STREAM_DEF_AUTOPONG           true

// // default IRC configuration
// *NOTE*: bitset: "1100" [2]: +w; [3]: +i (see rfc2812 3.1.3/3.1.5 and rfc1459 4.1.3)
#define IRC_CLIENT_DEF_IRC_USERMODE              0 // (!wallops && !invisible)
// #define RPG_NET_PROTOCOL_DEF_IRC_HOSTNAME              "localhost"
// #define RPG_NET_PROTOCOL_DEF_IRC_SERVERNAME            "localhost"
#define IRC_CLIENT_DEF_IRC_SERVERNAME            "*"

#define IRC_CLIENT_CNF_DEF_INI_FILE              "IRC_client.ini"
// .ini section headers
#define IRC_CLIENT_CNF_LOGIN_SECTION_HEADER      "login"
#define IRC_CLIENT_CNF_CONNECTION_SECTION_HEADER "connections"

// phonebook section headers
#define IRC_CLIENT_CNF_TIMESTAMP_SECTION_HEADER  "timestamp"
#define IRC_CLIENT_CNF_NETWORKS_SECTION_HEADER   "networks"
#define IRC_CLIENT_CNF_SERVERS_SECTION_HEADER    "servers"

#define IRC_CLIENT_DEF_CLIENT_USES_REACTOR       false
#define IRC_CLIENT_DEF_NUM_TP_THREADS            3

#define IRC_CLIENT_DEF_STATSINTERVAL             0 // seconds: 0 --> OFF

#define IRC_CLIENT_DEF_SERVER_HOSTNAME           ACE_LOCALHOST
#define IRC_CLIENT_DEF_SERVER_PORT               6667

// use traditional/modern USER message syntax for connections ?
// *NOTE*: refer to RFC1459 Section 4.1.3 - RFC2812 Section 3.1.3
// true ? --> rfc1459 : --> rfc2812
#define IRC_CLIENT_CNF_IRC_USERMSG_TRADITIONAL   false
// *NOTE*: hybrid-7.2.3 seems to have a bug: 0,4,8 --> +i
// *TODO*: --> ./etc/ircd.conf ?
#define IRC_CLIENT_DEF_IRC_USERMODE              0
#define IRC_CLIENT_DEF_IRC_NICK                  "Wiz"
#define IRC_CLIENT_DEF_IRC_CHANNEL               "#foobar"
#define IRC_CLIENT_DEF_IRC_AWAY_MESSAGE          "...be back soon..."
#define IRC_CLIENT_DEF_IRC_LEAVE_REASON          "quitting..."
#define IRC_CLIENT_DEF_IRC_KICK_REASON           "come back later..."

// *NOTE*: these conform to RFC1459, but servers may allow use different sizes
// e.g. NICKLEN option, ...
#define IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH       9
#define IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH    200

// UI/output
#define IRC_CLIENT_DEF_PRINT_PINGDOT             false

// client
#define IRC_CLIENT_DEF_CONNECTION_TIMEOUT        2 // second(s)

#endif
