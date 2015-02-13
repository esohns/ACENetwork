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

#include "net_transportlayer_ip_cast.h"

#include <sstream>
#include <string>

#include "net_defines.h"
#include "net_macros.h"

Net_TransportLayer_IP_Broadcast::Net_TransportLayer_IP_Broadcast (Net_ClientServerRole_t clientServerRole_in)
 : inherited (clientServerRole_in,
              TRANSPORTLAYER_IP_BROADCAST)
// , inherited2 ()
// , myAddress ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_IP_Broadcast::Net_TransportLayer_IP_Broadcast"));

  std::string address = ACE_TEXT_ALWAYS_CHAR (NET_DEFAULT_IP_BROADCAST_ADDRESS);
  address += ':';
  std::ostringstream converter;
  converter << NET_DEFAULT_PORT;
  address += converter.str();
  if (address_.set (ACE_TEXT(address.c_str ()), AF_INET) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  if (inherited2::open (address_,
                        PF_INET,
                        0,
                        0,
                        NULL) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::open(): \"%m\", continuing\n")));
}

Net_TransportLayer_IP_Broadcast::~Net_TransportLayer_IP_Broadcast ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_IP_Broadcast::~Net_TransportLayer_IP_Broadcast"));

  if (inherited2::close () == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::close(): \"%m\", continuing\n")));
}

void
Net_TransportLayer_IP_Broadcast::init (unsigned short port_in,
                                       bool useLoopback_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_IP_Broadcast::init"));

  port_ = port_in;
  useLoopback_ = useLoopback_in;

  if (address_.get_port_number () != port_)
  {
    if (inherited2::close () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::close(): \"%m\", continuing\n")));
    address_.set_port_number (port_, 1);
  } // end IF
  if (inherited2::open (address_,
                        PF_INET,
                        0,
                        0,
                        NULL) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::open(): \"%m\", continuing\n")));
}

/////////////////////////////////////////

Net_TransportLayer_IP_Multicast::Net_TransportLayer_IP_Multicast (Net_ClientServerRole_t clientServerRole_in)
 : inherited (clientServerRole_in,
              TRANSPORTLAYER_IP_MULTICAST)
 , inherited2 (ACE_SOCK_Dgram_Mcast::DEFOPTS)
 , joined_ (false)
// , address_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_IP_Multicast::Net_TransportLayer_IP_Multicast"));

  std::string address = ACE_TEXT_ALWAYS_CHAR (NET_DEFAULT_IP_MULTICAST_ADDRESS);
  address += ':';
  std::ostringstream converter;
  converter << NET_DEFAULT_PORT;
  address += converter.str ();
  if (address_.set (ACE_TEXT (address.c_str ()), AF_INET) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  if (inherited2::open (address_,
                        NULL,
                        1) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::open(): \"%m\", continuing\n")));
}

Net_TransportLayer_IP_Multicast::~Net_TransportLayer_IP_Multicast ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_IP_Multicast::~Net_TransportLayer_IP_Multicast"));

  if (joined_)
    if (inherited2::leave (address_,
                           NULL) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::leave(): \"%m\", continuing\n")));
  if (inherited2::close () == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::close(): \"%m\", continuing\n")));
}

void
Net_TransportLayer_IP_Multicast::init (unsigned short port_in,
                                       bool useLoopback_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_IP_Multicast::init"));

  port_ = port_in;
  useLoopback_ = useLoopback_in;

  if (address_.get_port_number () != port_)
  {
    if (joined_)
    {
      if (leave (address_,
                 NULL) == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::leave(): \"%m\", continuing\n")));
      joined_ = false;
    } // end IF
    if (inherited2::close () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::close(): \"%m\", continuing\n")));
    address_.set_port_number (port_, 1);
  } // end IF
  if (inherited2::open (address_,
                        NULL,
                        1) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::open(): \"%m\", continuing\n")));
  if (inherited2::join (address_,
                        1,
                        NULL) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::join(): \"%m\", continuing\n")));
  else
    joined_ = true;
}
