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

#ifndef SSDP_DEFINES_H
#define SSDP_DEFINES_H

// protocol
#define SSDP_DEFAULT_SERVER_PORT                 1900
#define SSDP_ADDRESS_IPV4_MULTICAST              "239.255.255.250"

#define SSDP_DISCOVER_REQUEST_URI_STRING         "*"

// headers
#define SSDP_DISCOVER_S_HEADER_STRING            "S" // UUID header
#define SSDP_DISCOVER_MAN_HEADER_STRING          "MAN"
#define SSDP_DISCOVER_SERVICE_TYPE_HEADER_STRING "ST" // search target
#define SSDP_DISCOVER_MX_HEADER_STRING           "MX" // seconds to delay response

#define SSDP_DISCOVER_S_HEADER_PREFIX_STRING     "uuid::"
#define SSDP_DISCOVER_MAN_SSDP_DISCOVER_STRING   "ssdp:discover"
#define SSDP_DISCOVER_ST_SSDP_ALL_STRING         "ssdp:all"
#define SSDP_DISCOVER_ST_ROOT_DEVICE_STRING      "upnp:rootdevice"
#define SSDP_DISCOVER_MX_DEFAULT_DELAY_S         1 // 1-5

// stream
#define SSDP_DEFAULT_MODULE_STREAMER_NAME_STRING "SSDPStreamer"

#endif
