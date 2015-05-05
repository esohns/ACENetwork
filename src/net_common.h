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

#ifndef NET_COMMON_H
#define NET_COMMON_H

#include "stream_messageallocatorheap_base.h"

#include "net_message.h"
#include "net_sessionmessage.h"

enum Net_ClientServerRole_t
{
  ROLE_INVALID = -1,
  ROLE_CLIENT = 0,
  ROLE_SERVER,
  ///////////////////////////////////////
  ROLE_MAX
};

enum Net_TransportLayer_t
{
  TRANSPORTLAYER_INVALID = -1,
  TRANSPORTLAYER_IP_BROADCAST = 0,
  TRANSPORTLAYER_IP_MULTICAST,
  TRANSPORTLAYER_NETLINK,
  TRANSPORTLAYER_TCP,
  TRANSPORTLAYER_UDP,
  ///////////////////////////////////////
  TRANSPORTLAYER_MAX
};

typedef Stream_MessageAllocatorHeapBase_T<Net_Message,
                                          Net_SessionMessage> Net_StreamMessageAllocator_t;

#endif
