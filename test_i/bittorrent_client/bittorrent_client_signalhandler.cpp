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

#include "http_scanner.h"

#include "bittorrent_client_signalhandler.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
//#include "ace/Proactor.h"
//#include "ace/Reactor.h"

#include "common_tools.h"

#include "net_macros.h"

#include "bittorrent_client_curses.h"
#include "bittorrent_client_network.h"

BitTorrent_Client_SignalHandler::BitTorrent_Client_SignalHandler (enum Common_SignalDispatchType dispatchMode_in,
                                                                  ACE_SYNCH_RECURSIVE_MUTEX* lock_in,
                                                                  bool useCursesLibrary_in)
 : inherited (dispatchMode_in,
              lock_in,
              this) // event handler handle
 , useCursesLibrary_ (useCursesLibrary_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_SignalHandler::BitTorrent_Client_SignalHandler"));

}

void
BitTorrent_Client_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_SignalHandler::handleSignal"));

//  int result = -1;

  bool abort = false;
//  bool connect = false;
  bool shutdown = false;
  switch (signal_in.signal)
  {
    case SIGINT:
    case SIGTERM:
// *PORTABILITY*: this isn't portable: on Windows SIGQUIT and SIGHUP are not defined...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGQUIT:
#endif
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
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR1:
#else
  case SIGBREAK:
#endif
    {
//      // (try to) connect
//      connect = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR2:
#else
  case SIGABRT:
#endif
    {
      // abort connection
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

  // ...abort ?
  if (abort)
  {
    BitTorrent_Client_IPeerConnection_Manager_t* connection_manager_p =
        BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (connection_manager_p);

    connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);
  } // end IF

  // ...shutdown ?
  if (shutdown)
  {
    // step1: notify curses dispatch ?
    if (inherited::configuration_)
      if (inherited::configuration_->cursesState)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::configuration_->cursesState->lock);
        inherited::configuration_->cursesState->finished = true;
      } // end IF

    // step2: stop event dispatch
    Common_Tools::finalizeEventDispatch (inherited::configuration_->dispatchState->proactorGroupId,
                                         inherited::configuration_->dispatchState->reactorGroupId,
                                         false);                                                    // don't block
  } // end IF
}
