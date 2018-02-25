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

#if defined (ACE_HAS_NETLINK)
#include <cmath>

#include "ace/OS.h"

#include "common_math_defines.h"

int
Net_Netlink_Addr::addr_to_string (ACE_TCHAR buffer_out[],
                                  size_t size_in,
                                  int IPAddressFormat_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Netlink_Addr::addr_to_string"));

  ACE_UNUSED_ARG (IPAddressFormat_in);

  // initialize return value(s)
  ACE_OS::memset (buffer_out, 0, size_in);

  // sanity check(s)
  int pid = inherited::get_pid ();
  int gid = inherited::get_gid ();
  size_t total_length =
      static_cast<size_t> (COMMON_MATH_NUMDIGITS_INT (pid)) +
      1 + // sizeof (':')
      static_cast<size_t> (COMMON_MATH_NUMDIGITS_INT (gid)) +
      1; // sizeof ('\0'), terminating NUL
  if (size_in < total_length)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("buffer too small (was: %u, need: %u), aborting\n"),
                size_in, total_length));
    return -1;
  } // end IF

  int result = ACE_OS::sprintf (buffer_out,
                                ACE_TEXT ("%u:%u"),
                                static_cast<unsigned int> (pid),
                                static_cast<unsigned int> (gid));
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", aborting\n")));

  return (static_cast<size_t> (result) == (total_length - 1) ? 0 : -1);
}
#endif // ACE_HAS_NETLINK
