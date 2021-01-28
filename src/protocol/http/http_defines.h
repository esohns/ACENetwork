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

// stream
#define HTTP_DEFAULT_MODULE_MARSHAL_NAME_STRING   "HTTPMarshal"
#define HTTP_DEFAULT_MODULE_PARSER_NAME_STRING    "HTTPParser"
#define HTTP_DEFAULT_STREAM_NAME_STRING           "HTTPStream"

#define HTTP_DEFAULT_CLOSE_AFTER_RECEPTION        false

// (strip protocol data and) 'crunch' parsed messages for easier downstream
// processing ?
// *NOTE*: this comes at the cost of malloc/free and memcpy per
//         (fragmented) message, i.e. should probably be avoided.
#define HTTP_DEFAULT_CRUNCH_MESSAGES              false
// *IMPORTANT NOTE*: scans buffers in-place (avoids a copy,
//         see: http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html)
//         --> in order to use yy_scan_buffer(), the buffer needs to have been
//             prepared for usage by flex: buffers need two trailing '\0's
//             BEYOND their datas' tail byte (i.e. at positions length() + 1 and
//             length() + 2)
//#define HTTP_DEFAULT_USE_YY_SCAN_BUFFER           true

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

// parser
#define HTTP_PRT_LEXER_DFA_TABLES_FILENAME        "http_scanner.tab"
#define HTTP_PRT_REGEX_CHUNK_LINE                 "^([[:xdigit:]]+)(?:(;.+)*)(?:\\r\\n)$"

//////////////////////////////////////////

#define HTTPS_DEFAULT_SERVER_PORT                 443

#endif
