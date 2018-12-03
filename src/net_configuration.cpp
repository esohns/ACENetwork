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
#include "stdafx.h"

#include "ace/Synch.h"
#include "net_configuration.h"

#if defined (NETLINK_SUPPORT)
#include "netlink/addr.h"

#include "ace/OS.h"
#endif // NETLINK_SUPPORT

#if defined (NETLINK_SUPPORT)
int
Net_Netlink_Addr::addr_to_string (ACE_TCHAR buffer_out[],
                                  size_t size_in,
                                  int IPAddressFormat_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Netlink_Addr::addr_to_string"));

  ACE_UNUSED_ARG (IPAddressFormat_in);

  // initialize return value(s)
  ACE_OS::memset (buffer_out, 0, size_in);

  int address_size_i = inherited::get_addr_size ();

  struct nl_addr* nl_addr_p =
      nl_addr_alloc (static_cast<size_t> (address_size_i));
  if (unlikely (!nl_addr_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nl_addr_alloc(%d): \"%m\", aborting\n"),
                address_size_i));
    return -1;
  } // end IF
  int result =
      nl_addr_set_binary_addr (nl_addr_p,
                               inherited::get_addr (),
                               static_cast<size_t> (address_size_i));
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_addr_set_binary_addr(): \"%s\", aborting\n"),
                ACE_TEXT (nl_geterror (result))));
    nl_addr_put (nl_addr_p);
    return -1;
  } // end IF

  if (unlikely (!nl_addr2str (nl_addr_p,
                              buffer_out,
                              size_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_addr2str(): \"%s\", aborting\n"),
                ACE_TEXT (nl_geterror (result))));
    nl_addr_put (nl_addr_p);
    return -1;
  } // end IF

  nl_addr_put (nl_addr_p);

  return 0;
}
#endif // NETLINK_SUPPORT
