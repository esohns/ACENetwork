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

#include "bittorrent_client_signalhandler.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "net_macros.h"

#if defined (GUI_SUPPORT)
#if defined (CURSES_SUPPORT)
#include "bittorrent_client_curses.h"
#endif // CURSES_SUPPORT
#endif // GUI_SUPPORT
#include "bittorrent_client_network.h"

BitTorrent_Client_SignalHandler::BitTorrent_Client_SignalHandler ()
 : inherited (this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_SignalHandler::BitTorrent_Client_SignalHandler"));

}

void
BitTorrent_Client_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_SignalHandler::handleSignal"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  bool abort = false;
  bool rerequest_peers_from_tracker = false;
  bool shutdown = false;
  switch (signal_in.signal)
  {
    case SIGINT:
    case SIGTERM:
// *PORTABILITY*: this isn't portable: on Windows SIGQUIT and SIGHUP are not defined...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGHUP:
    case SIGQUIT:
#endif // ACE_WIN32 || ACE_WIN64
    {
//       // *PORTABILITY*: tracing in a signal handler context is not portable
//       // *TODO*
//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("shutting down...\n")));

      // shutdown
      shutdown = true;

      break;
    }
// *PORTABILITY*: this isn't portable: on Windows SIGUSR1 and SIGUSR2 are not defined,
// so we handle SIGBREAK (21) and SIGABRT (22) instead...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case SIGBREAK:
#else
    case SIGUSR1:
#endif
    {
      // (try to) connect to tracjer and rerequst peers
      rerequest_peers_from_tracker = true;

      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case SIGABRT:
#else
    case SIGUSR2:
#endif
    {
      // abort one connection
      abort = true;

      break;
    }
    case SIGCLD:
    default:
    {
      if (signal_in.signal != SIGCLD)
      {
        // *PORTABILITY*: tracing in a signal handler context is not portable
        // *TODO*
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown signal: \"%S\", returning\n"),
                    signal_in.signal));
      } // end IF
      return;
    }
  } // end SWITCH

  BitTorrent_Client_IPeerConnection_Manager_t* connection_manager_p =
      BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  BitTorrent_Client_ITrackerConnection_Manager_t* connection_manager_2 =
      BITTORRENT_CLIENT_TRACKERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_2);

  // ...abort one ?
  if (abort)
  {
    connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);
  } // end IF

  if (rerequest_peers_from_tracker)
  { ACE_ASSERT (inherited::configuration_->controller);
    inherited::configuration_->controller->notifyTracker (ACE_TEXT_ALWAYS_CHAR (""),
                                                          BITTORRENT_EVENT_TRACKER_REREQUEST);
  } // end IF

  // ...shutdown ?
  if (shutdown)
  {
    connection_manager_p->stop (false, false);
    connection_manager_2->stop (false, false);
    connection_manager_p->abort (false);
    connection_manager_2->abort (false);

    // step1: stop all sessions
    ACE_ASSERT (inherited::configuration_->controller);
    inherited::configuration_->controller->stop (false, // wait for completion ?
                                                 true); // high priority ?

#if defined (GUI_SUPPORT)
#if defined (CURSES_USE)
    // step2: notify curses dispatch ?
    ACE_ASSERT (inherited::configuration_->cursesState);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::configuration_->cursesState->lock);
      inherited::configuration_->cursesState->finished = true;
    } // end IF
#endif // CURSES_USE
#endif // GUI_SUPPORT
  } // end IF
}
