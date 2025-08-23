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

#include "net_server_listener_base.h"

Net_Server_Listener_Base::Net_Server_Listener_Base ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_Base::Net_Server_Listener_Base"));

}

void
Net_Server_Listener_Base::subscribe (INOTIFY_T* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_Base::subscribe"));

  // sanity check(s)
  ACE_ASSERT (interfaceHandle_in);

  { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, lock_);
    subscribers_.push_back (interfaceHandle_in);
  } // end lock scope
}

void
Net_Server_Listener_Base::unsubscribe (INOTIFY_T* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_Base::unsubscribe"));

  { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, lock_);
    SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
    for (;
         iterator != subscribers_.end ();
         iterator++)
      if ((*iterator) == interfaceHandle_in)
        break;

    if (iterator != subscribers_.end ())
      subscribers_.erase (iterator);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %@), continuing\n"),
                  interfaceHandle_in));
  } // end lock scope
}
