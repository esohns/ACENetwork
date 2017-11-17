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

#ifndef IRC_DEFINES_H
#define IRC_DEFINES_H

// protocol
// CRLF = "\r\n"
#define IRC_FRAME_BOUNDARY                      "\r\n"
#define IRC_FRAME_BOUNDARY_SIZE                 2

#define IRC_MAXIMUM_NOTICE_DELAY                3  // seconds
// *NOTE*: some servers perform DNS address resolution, which can take a while
#define IRC_MAXIMUM_WELCOME_DELAY               60 // seconds

// stream
// *NOTE*: according to RFC1459, IRC messages SHALL not exceed 512 bytes.
//         A buffer size of 512 bytes will allow "crunching" messages into a
//         single buffer. While this arguably "wastes" some memory, it allows
//         [easier/more robust] scanning / parsing
// *WARNING*: be aware that a single read from the connected socket may well
//            cover MORE than one complete message at a time, so this value is
//            just a (somewhat qualified) proposal
#define IRC_MAXIMUM_FRAME_SIZE                  512 // bytes
#define IRC_DEFAULT_MODULE_BISECTOR_NAME_STRING "IRCBisector"
#define IRC_DEFAULT_MODULE_MARSHAL_NAME_STRING  "IRCMarshal"
#define IRC_DEFAULT_MODULE_PARSER_NAME_STRING   "IRCParser"
#define IRC_DEFAULT_STREAM_NAME_STRING          "IRCStream"

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
#define IRC_DEFAULT_CRUNCH_MESSAGES             true

// output more debugging information
#define IRC_DEFAULT_LEX_TRACE                   false
#define IRC_DEFAULT_YACC_TRACE                  false

//#define IRC_STREAM_DEF_TRACE_ENABLED       false
#define IRC_STREAM_DEFAULT_AUTOPONG             true

// // default IRC configuration
// *NOTE*: bitset: "1100" [2]: +w; [3]: +i (see rfc2812 3.1.3/3.1.5 and
//         rfc1459 4.1.3)
// *NOTE*: hybrid-7.2.3 seems to have a bug: 0,4,8 --> +i
// *TODO*: --> ./etc/ircd.conf ?
#define IRC_DEFAULT_USERMODE                    0 // (!wallops && !invisible)
#define IRC_DEFAULT_SERVERNAME                  "*"

#define IRC_DEFAULT_SERVER_PORT                 6667

// use traditional/modern USER message syntax for connections ?
// *NOTE*: refer to RFC1459 Section 4.1.3 - RFC2812 Section 3.1.3
// true ? --> rfc1459 : --> rfc2812
#define IRC_PRT_USERMSG_TRADITIONAL             false
#define IRC_DEFAULT_NICKNAME                    "Wiz"
#define IRC_DEFAULT_CHANNEL                     "#foobar"
#define IRC_DEFAULT_AWAY_MESSAGE                "...be back soon..."
#define IRC_DEFAULT_LEAVE_REASON                "quitting..."
#define IRC_DEFAULT_KICK_REASON                 "come back later..."

// *NOTE*: these conform to RFC1459, but servers may allow use different sizes
// e.g. NICKLEN option, ...
#define IRC_PRT_MAXIMUM_NICKNAME_LENGTH         9
#define IRC_PRT_MAXIMUM_CHANNEL_LENGTH          200

// UI/input
#define IRC_INPUT_THREAD_NAME                   "input"
// *TODO*: make sure this is unique
#define IRC_INPUT_THREAD_GROUP_ID               COMMON_EVENT_DISPATCH_THREAD_GROUP_ID + 1

// UI/output
#define IRC_PRT_DEFAULT_ENCODING                IRC_CHARACTERENCODING_UTF8

// *TODO*: move these to irc_client_defines.h
// client
#define IRC_SESSION_LOG_FILENAME_PREFIX         "IRC_session"
#define IRC_SESSION_DEF_LOG                     false // log to file ? : stdout

#define IRC_CLIENT_DEFAULT_PRINT_PINGDOT        false
#define IRC_CLIENT_HANDLER_MODULE_NAME          "Handler"

#endif
