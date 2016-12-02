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

Net_InetTransportLayer_Base::Net_InetTransportLayer_Base (Net_TransportLayerType transportLayer_in)
 : dispatch_ (COMMON_DISPATCH_INVALID)
 , role_ (NET_ROLE_INVALID)
 , transportLayer_ (transportLayer_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::Net_InetTransportLayer_Base"));

}

Net_InetTransportLayer_Base::~Net_InetTransportLayer_Base ()
{
  NETWORK_TRACE(ACE_TEXT("Net_InetTransportLayer_Base::~Net_InetTransportLayer_Base"));

}

enum Common_DispatchType
Net_InetTransportLayer_Base::dispatch ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::dispatch"));

  return dispatch_;
}

enum Net_ClientServerRole
Net_InetTransportLayer_Base::role ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::role"));

  return role_;
}

enum Net_TransportLayerType
Net_InetTransportLayer_Base::transportLayer ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::transportLayer"));

  return transportLayer_;
}

bool
Net_InetTransportLayer_Base::initialize (enum Common_DispatchType dispatch_in,
                                         enum Net_ClientServerRole role_in,
                                         const Net_SocketConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::initialize"));

  dispatch_ = dispatch_in;
  role_ = role_in;
  ACE_UNUSED_ARG (configuration_in);

  return true;
}

void
Net_InetTransportLayer_Base::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayer_Base::finalize"));

  ACE_ASSERT (false);
}

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
Net_NetlinkTransportLayer_Base::Net_NetlinkTransportLayer_Base ()
 : dispatch_(COMMON_DISPATCH_INVALID)
 , role_(NET_ROLE_INVALID)
 , transportLayer_ (NET_TRANSPORTLAYER_NETLINK)
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::Net_NetlinkTransportLayer_Base"));

}

Net_NetlinkTransportLayer_Base::~Net_NetlinkTransportLayer_Base ()
{
  NETWORK_TRACE(ACE_TEXT("Net_NetlinkTransportLayer_Base::~Net_NetlinkTransportLayer_Base"));

}

enum Common_DispatchType
Net_NetlinkTransportLayer_Base::dispatch ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::dispatch"));

  return dispatch_;
}

enum Net_ClientServerRole
Net_NetlinkTransportLayer_Base::role ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::role"));

  return role_;
}

enum Net_TransportLayerType
Net_NetlinkTransportLayer_Base::transportLayer ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::transportLayer"));

  return transportLayer_;
}

bool
Net_NetlinkTransportLayer_Base::initialize (enum Common_DispatchType dispatch_in,
                                            enum Net_ClientServerRole role_in,
                                            const Net_SocketConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::initialize"));

  dispatch_ = dispatch_in;
  role_ = role_in;

  ACE_UNUSED_ARG (configuration_in);

  return true;
}

void
Net_NetlinkTransportLayer_Base::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkTransportLayer_Base::finalize"));

  ACE_ASSERT (false);
}
#endif
