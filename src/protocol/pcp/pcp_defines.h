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

#ifndef PCP_DEFINES_H
#define PCP_DEFINES_H

#include "ace/config-lite.h"

// stream
#define PCP_DEFAULT_MODULE_MARSHAL_NAME_STRING           "PCPMarshal"
#define PCP_DEFAULT_MODULE_PARSER_NAME_STRING            "PCPParser"
#define PCP_DEFAULT_MODULE_STREAMER_NAME_STRING          "PCPStreamer"
#define PCP_DEFAULT_STREAM_NAME_STRING                   "PCPStream"

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
#define PCP_DEFAULT_CRUNCH_MESSAGES                      true

#define PCP_DEFAULT_STATISTIC_REPORTING_INTERVAL_S       0 // seconds {0: off}
#define PCP_DEFAULT_PRINT_PROGRESSDOT                    false

// protocol
#define PCP_DEFAULT_CLIENT_PORT                          5350 // UDP
#define PCP_DEFAULT_SERVER_PORT                          5351 // UDP

#define PCP_MESSAGE_SIZE                                 1100

#define PCP_DEFAULT_MAP_LIFETIME_S                       3600 // seconds {0: delete mapping}
#define PCP_NONCE_BYTES                                  12

#endif
