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

#ifndef NET_NETLINK_ADDRESS_H
#define NET_NETLINK_ADDRESS_H

#include "ace/Assert.h"
#include "ace/Netlink_Addr.h"

// forward declarations
struct sockaddr_nl;

class Net_Netlink_Addr
 : public ACE_Netlink_Addr
{
  typedef ACE_Netlink_Addr inherited;

 public:
  Net_Netlink_Addr ()
   : inherited ()
  {}
  Net_Netlink_Addr (const sockaddr_nl* address_in,
                    int length_in)
   : inherited (address_in,
                length_in)
  {}
  inline virtual ~Net_Netlink_Addr () {}

  inline Net_Netlink_Addr& operator= (const ACE_Addr& rhs) { *this = rhs; return *this; }

  virtual int addr_to_string (ACE_TCHAR[],    // buffer
                              size_t,         // size
                              int = 1) const; // ipaddr_format
  inline bool is_any (void) const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }

  inline void reset (void) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

#endif
