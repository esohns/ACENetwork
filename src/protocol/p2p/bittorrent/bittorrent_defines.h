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

#ifndef BITTORRENT_DEFINES_H
#define BITTORRENT_DEFINES_H

// stream
// *TODO*: this makes no sense, remove ASAP
#define BITTORRENT_BUFFER_SIZE                          16384

// (strip protocol data and) 'crunch' parsed messages for easier downstream
// processing ?
// *NOTE*: this comes at the cost of malloc/free and memcpy per
//         (fragmented) message, i.e. should probably be avoided.
#define BITTORRENT_DEFAULT_CRUNCH_MESSAGES              false
// *IMPORTANT NOTE*: scans buffers in-place (avoids a copy,
//         see: http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html)
//         --> in order to use yy_scan_buffer(), the buffer needs to have been
//             prepared for usage by flex: buffers need two trailing '\0's
//             BEYOND their datas' tail byte (i.e. at positions length() + 1 and
//             length() + 2)
#define BITTORRENT_DEFAULT_USE_YY_SCAN_BUFFER           true

#define BITTORRENT_DEFAULT_STATISTIC_REPORTING_INTERVAL 0 // seconds: 0 --> OFF

// connection
#define BITTORRENT_DEFAULT_SERVER_HOSTNAME              ACE_LOCALHOST
#define BITTORRENT_DEFAULT_SERVER_PORT                  80

// protocol


// session
#define BITTORRENT_SESSION_LOG_FILENAME_PREFIX          "bittorrent_session"
#define BITTORRENT_DEFAULT_SESSION_LOG                  false // log to file ? : stdout

// client
#define BITTORRENT_DEFAULT_HANDLER_MODULE_NAME          "Handler"

#endif
