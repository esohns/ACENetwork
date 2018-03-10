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
#define BITTORRENT_BUFFER_SIZE                            16384
#define BITTORRENT_DEFAULT_MODULE_MARSHAL_NAME_STRING     "BitTorrentPeerMarshal"
#define BITTORRENT_DEFAULT_MODULE_HANDLER_NAME_STRING     "BitTorrentPeerHandler"
#define BITTORRENT_DEFAULT_STREAM_NAME_STRING             "BitTorrentPeerStream"

// (strip protocol data and) 'crunch' parsed messages for easier downstream
// processing ?
// *NOTE*: this comes at the cost of malloc/free and memcpy per
//         (fragmented) message, i.e. should probably be avoided.
#define BITTORRENT_DEFAULT_CRUNCH_MESSAGES                false
// *IMPORTANT NOTE*: scans buffers in-place (avoids a copy,
//         see: http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html)
//         --> in order to use yy_scan_buffer(), the buffer needs to have been
//             prepared for usage by flex: buffers need two trailing '\0's
//             BEYOND their datas' tail byte (i.e. at positions length() + 1 and
//             length() + 2)
//#define BITTORRENT_DEFAULT_USE_YY_SCAN_BUFFER             true

#define BITTORRENT_DEFAULT_STATISTIC_REPORTING_INTERVAL   0 // seconds: 0 --> OFF

// connection
#define BITTORRENT_DEFAULT_PORT                           6881

// protocol
#define BITTORRENT_PRT_PEER_ID_LENGTH                     20 // bytes
#define BITTORRENT_PRT_INFO_HASH_SIZE                     20 // bytes

#define BITTORRENT_METAINFO_ANNOUNCE_KEY                  "announce"
#define BITTORRENT_METAINFO_INFO_KEY                      "info"
#define BITTORRENT_METAINFO_INFO_LENGTH_KEY               "length"
#define BITTORRENT_METAINFO_INFO_FILES_KEY                "files"

#define BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER       "info_hash"
#define BITTORRENT_TRACKER_REQUEST_PEER_ID_HEADER         "peer_id"
#define BITTORRENT_TRACKER_REQUEST_PEER_ID_CLIENT_ID      "LN"
#define BITTORRENT_TRACKER_REQUEST_PORT_HEADER            "port"
#define BITTORRENT_TRACKER_REQUEST_UPLOADED_HEADER        "uploaded"
#define BITTORRENT_TRACKER_REQUEST_DOWNLOADED_HEADER      "downloaded"
#define BITTORRENT_TRACKER_REQUEST_LEFT_HEADER            "left"
#define BITTORRENT_TRACKER_REQUEST_COMPACT_HEADER         "compact"
#define BITTORRENT_TRACKER_REQUEST_NO_PEER_ID_HEADER      "no_peer_id"
#define BITTORRENT_TRACKER_REQUEST_EVENT_HEADER           "event"
#define BITTORRENT_TRACKER_REQUEST_EVENT_STARTED_STRING   "started"
#define BITTORRENT_TRACKER_REQUEST_EVENT_STOPPED_STRING   "stopped"
#define BITTORRENT_TRACKER_REQUEST_EVENT_COMPLETED_STRING "completed"
#define BITTORRENT_TRACKER_REQUEST_IP_HEADER              "ip"
#define BITTORRENT_TRACKER_REQUEST_NUMWANT_HEADER         "numwant"
#define BITTORRENT_TRACKER_REQUEST_KEY_HEADER             "key"
#define BITTORRENT_TRACKER_REQUEST_TRACKERID_HEADER       "trackerid"

#define BITTORRENT_DEFAULT_TRACKER_REQUEST_NUMWANT_PEERS  50

// response
#define BITTORRENT_TRACKER_RESPONSE_INTERVAL_HEADER       "interval"
#define BITTORRENT_TRACKER_RESPONSE_PEERS_HEADER          "peers"
// scrape
#define BITTORRENT_TRACKER_SCRAPE_URI_PREFIX              "scrape"
#define BITTORRENT_TRACKER_SCRAPE_RESPONSE_FILES_HEADER   "files"

#define BITTORRENT_PEER_HANDSHAKE_PSTR_STRING             "BitTorrent protocol"
#define BITTORRENT_PEER_HANDSHAKE_RESERVED_SIZE           8 // bytes


// session
#define BITTORRENT_SESSION_HANDLER_THREAD_NAME            "bittorrent_session"
#define BITTORRENT_SESSION_HANDLER_THREAD_GROUP_ID        NET_CONNECTION_HANDLER_THREAD_GROUP_ID + 1 // *TODO*
#define BITTORRENT_SESSION_LOG_FILENAME_PREFIX            "bittorrent_session"
#define BITTORRENT_DEFAULT_SESSION_LOG                    false // log to file ? : stdout

// client
#define BITTORRENT_DEFAULT_HANDLER_MODULE_NAME            "BitTorrentHandler"

#endif
