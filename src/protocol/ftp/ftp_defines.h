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

#ifndef FTP_DEFINES_H
#define FTP_DEFINES_H

// stream
#define FTP_DEFAULT_MODULE_MARSHAL_NAME_STRING           "FTPMarshal"
#define FTP_DEFAULT_MODULE_PARSER_NAME_STRING            "FTPParser"
#define FTP_DEFAULT_MODULE_PARSER_DATA_NAME_STRING       "FTPParserData"
#define FTP_DEFAULT_MODULE_STREAMER_NAME_STRING          "FTPStreamer"

#define FTP_DEFAULT_STREAM_NAME_STRING                   "FTPStream"

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
#define FTP_DEFAULT_CRUNCH_MESSAGES                      true

#define FTP_DEFAULT_STATISTIC_REPORTING_INTERVAL_S       0 // seconds {0: off}
#define FTP_DEFAULT_PRINT_PROGRESSDOT                    false

// protocol
#define FTP_DEFAULT_CLIENT_DATA_PORT                     10101 // TCP
#define FTP_DEFAULT_SERVER_CONTROL_PORT                  21 // TCP
#define FTP_DEFAULT_SERVER_DATA_PORT                     20 // TCP

#define FTP_DEFAULT_ANONYMOUS_USER                       "anonymous"
#define FTP_DEFAULT_ANONYMOUS_PASS                       "test@test.com"

#define FTP_TYPE_ASCII                                   "A"
#define FTP_TYPE_EBCDIC                                  "E"
#define FTP_TYPE_IMAGE                                   "I"
#define FTP_TYPE_LOCAL                                   "L"
#define FTP_TYPE_2_NON_PRINT                             "N"
#define FTP_TYPE_2_TELNET                                "T"
#define FTP_TYPE_2_CARRIAGE_CONTROL                      "C"

#endif
