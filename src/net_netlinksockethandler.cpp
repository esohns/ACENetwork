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

#include <ace/Synch.h>
#include "net_netlinksockethandler.h"

#if defined (ACE_HAS_NETLINK)
#include <cmath>

#include <ace/OS.h>

int
Net_Netlink_Addr::addr_to_string (ACE_TCHAR buffer_in[],
                                  size_t size_in,
                                  int IPAddressFormat_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Netlink_Addr::addr_to_string"));

  ACE_UNUSED_ARG (IPAddressFormat_in);

  ACE_OS::memset (buffer_in, 0, size_in);

  // sanity check(s)
  int pid = inherited::get_pid ();
  int gid = inherited::get_gid ();
  size_t total_length =
      (static_cast<size_t> (::floorf (::log10f (static_cast<float> (pid)))) + 1) +
      1 + // sizeof (':')
      (static_cast<size_t> (::floorf (::log10f (static_cast<float> (gid))) + 1)) +
      1; // sizeof ('\0'), terminating NUL
  if (size_in < total_length)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("buffer too small (was: %u, need: %u), aborting\n")));
    return -1;
  } // end IF

  ACE_TCHAR const *format = ACE_TEXT ("%u:%u");
  int result = ACE_OS::sprintf (buffer_in, format,
                                pid, gid);
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", aborting\n")));

  return (static_cast<unsigned int> (result) == (total_length -1) ? 0 : -1);
}

Net_Netlink_Addr&
Net_Netlink_Addr::operator= (const ACE_Addr& rhs)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Netlink_Addr::operator="));

  *this = rhs;

  return *this;
}
#endif
