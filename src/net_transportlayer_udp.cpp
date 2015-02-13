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

#include "net_transportlayer_udp.h"

#include <sstream>
#include <string>

#include "net_defines.h"
#include "net_macros.h"

Net_TransportLayer_UDP::Net_TransportLayer_UDP ()
 : inherited (ROLE_INVALID,
              TRANSPORTLAYER_UDP)
 //, inherited2 ()
//   address_ (),
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_UDP::Net_TransportLayer_UDP"));

  //std::string address = ACE_TEXT_ALWAYS_CHAR (NET_DEFAULT_IP_BROADCAST_ADDRESS);
  //address += ':';
  //std::ostringstream converter;
  //converter << NET_DEFAULT_PORT;
  //address += converter.str ();
  //if (address_.set (ACE_TEXT (address.c_str ()), AF_INET) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  //if (inherited2::open (address_,
  //                      ACE_PROTOCOL_FAMILY_INET,
  //                      0,
  //                      0) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_SOCK_Dgram::open(): \"%m\", continuing\n")));
}

Net_TransportLayer_UDP::~Net_TransportLayer_UDP ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_TransportLayer_UDP::~Net_TransportLayer_UDP"));

  //if (inherited2::close () == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_SOCK_Dgram::close(): \"%m\", continuing\n")));
}
