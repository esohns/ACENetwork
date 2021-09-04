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

#include "bittorrent_session.h"

ACE_THR_FUNC_RETURN
net_bittorrent_session_setup_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_bittorrent_session_setup_function"));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = -1;
#else
  result = arg_in;
#endif

  struct BitTorrent_SessionInitiationThreadData* data_p =
    static_cast<struct BitTorrent_SessionInitiationThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->addresses);
  ACE_ASSERT (data_p->lock);
  ACE_ASSERT (data_p->session);

  ACE_INET_Addr peer_address;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *data_p->lock, result);
    peer_address = data_p->addresses->back ();
    data_p->addresses->pop_back ();
  } // end lock scope

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connecting to peer \"%s\"...\n"),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));

  data_p->session->connect (peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

  return result;
}
