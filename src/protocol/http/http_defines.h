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

#ifndef HTTP_DEFINES_H
#define HTTP_DEFINES_H

//#include "ace/Default_Constants.h"

// stream
#define HTTP_BUFFER_SIZE                          16384

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
#define HTTP_DEFAULT_CRUNCH_MESSAGES              true
// *IMPORTANT NOTE*: scans buffers in-place (avoids a copy,
//         see: http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html)
//         --> in order to use yy_scan_buffer(), the buffer needs to have been
//             prepared for usage by flex: buffers need two trailing '\0's
//             BEYOND their datas' tail byte (i.e. at positions length() + 1 and
//             length() + 2)
#define HTTP_DEFAULT_USE_YY_SCAN_BUFFER           true

#define HTTP_DEFAULT_STATISTIC_REPORTING_INTERVAL 0 // seconds: 0 --> OFF

#define HTTP_DEFAULT_SERVER_HOSTNAME              ACE_LOCALHOST
#define HTTP_DEFAULT_SERVER_PORT                  80

// protocol
#define HTTP_DEFAULT_PRINT_PROGRESSDOT            false
#define HTTP_COMMAND_STRING_RESPONSE              "HTTP_RESPONSE"
#define HTTP_PRT_VERSION_STRING_PREFIX            "HTTP/"
// headers
#define HTTP_PRT_HEADER_ACCEPT_STRING             "Accept"
#define HTTP_PRT_HEADER_ACCEPT_ENCODING_STRING    "Accept-Encoding"
#define HTTP_PRT_HEADER_ACCEPT_LANGUAGE_STRING    "Accept-Language"
#define HTTP_PRT_HEADER_AGENT_STRING              "User-Agent"
#define HTTP_PRT_HEADER_CACHE_CONTROL_STRING      "Cache-Control"
#define HTTP_PRT_HEADER_CONNECTION_STRING         "Connection"
#define HTTP_PRT_HEADER_CONTENT_ENCODING_STRING   "Content-Encoding"
#define HTTP_PRT_HEADER_CONTENT_LENGTH_STRING     "Content-Length"
#define HTTP_PRT_HEADER_CONTENT_TYPE_STRING       "Content-Type"
#define HTTP_PRT_HEADER_COOKIE_STRING             "Cookie"
#define HTTP_PRT_HEADER_HOST_STRING               "Host"
#define HTTP_PRT_HEADER_LOCATION_STRING           "Location"
#define HTTP_PRT_HEADER_REFERER_STRING            "Referer"
#define HTTP_PRT_HEADER_TRACKING_STRING           "DNT"
#define HTTP_PRT_HEADER_TRANSFER_ENCODING_STRING  "Transfer-Encoding"

#define HTTP_PRT_TRANSFER_ENCODING_CHUNKED_STRING "chunked"

#define HTTP_PRT_MIMETYPE_WWWURLENCODING_STRING   "application/x-www-form-urlencoded"

// regular expressions
#define HTTP_PRT_REGEX_CHUNK_LINE                 "^([[:xdigit:]]+)(?:(;.+)*)(?:\\r\\n)$"

#endif
