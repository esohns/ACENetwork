/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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
#include "stdafx.h"

#include "net_transportlayer_base.h"

#include "net_macros.h"

Net_InetTransportLayer_Base::Net_InetTransportLayer_Base (Net_ClientServerRole_t role_in,
                                                          Net_TransportLayer_t transportLayer_in)
 : clientServerRole_ (role_in)
 , transportLayer_ (transportLayer_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::Net_InetTransportLayer_Base"));

}

Net_InetTransportLayer_Base::~Net_InetTransportLayer_Base ()
{
  NETWORK_TRACE(ACE_TEXT("Net_InetTransportLayer_Base::~Net_InetTransportLayer_Base"));

}

void
Net_InetTransportLayer_Base::initialize (Net_ClientServerRole_t role_in,
                                         const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::initialize"));

  clientServerRole_ = role_in;

  initialize (address_in);
}

void
Net_InetTransportLayer_Base::initialize (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::initialize"));

  ACE_UNUSED_ARG (address_in);
}

/////////////////////////////////////////

Net_NetlinkTransportLayer_Base::Net_NetlinkTransportLayer_Base ()
 : clientServerRole_ (ROLE_INVALID)
 , transportLayer_ (TRANSPORTLAYER_NETLINK)
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::Net_NetlinkTransportLayer_Base"));

}

Net_NetlinkTransportLayer_Base::~Net_NetlinkTransportLayer_Base ()
{
  NETWORK_TRACE(ACE_TEXT("Net_NetlinkTransportLayer_Base::~Net_NetlinkTransportLayer_Base"));

}

void
Net_NetlinkTransportLayer_Base::initialize (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::initialize"));

  ACE_UNUSED_ARG (address_in);
}
