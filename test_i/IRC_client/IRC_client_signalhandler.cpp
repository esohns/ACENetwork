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
#include "IRC_client_signalhandler.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
//#include "ace/Proactor.h"
//#include "ace/Reactor.h"
#include "ace/Synch_Traits.h"

#include "common_tools.h"

#include "net_macros.h"

#include "IRC_client_curses.h"
#include "IRC_client_network.h"

#include "IRC_client_gui_common.h"

IRC_Client_SignalHandler::IRC_Client_SignalHandler (bool useReactor_in,
                                                    bool useCursesLibrary_in)
 : inherited (this) // event handler handle
 , useCursesLibrary_ (useCursesLibrary_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::IRC_Client_SignalHandler"));

}

IRC_Client_SignalHandler::~IRC_Client_SignalHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::~IRC_Client_SignalHandler"));

}

void
IRC_Client_SignalHandler::handle (int signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::handleSignal"));

//  int result = -1;
  IRC_Client_IConnection_Manager_t* connection_manager_p =
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();

  bool abort = false;
  bool connect = false;
  bool shutdown = false;
  switch (signal_in)
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
      // (try to) connect
      connect = true;

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
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown signal: \"%S\", returning\n"),
                  signal_in));
      return;
    }
  } // end SWITCH

  // ...abort ?
  if (abort)
    connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);

  // ...connect ?
  if (connect)
  {
    if (!inherited::configuration_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("not configured: cannot connect, continuing\n")));
      goto done_connect;
    } // end IF
    if (!inherited::configuration_->connector)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no connector: cannot connect, continuing\n")));
      goto done_connect;
    } // end IF

    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    try {
      handle =
        inherited::configuration_->connector->connect (inherited::configuration_->peerAddress);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Client_IConnector_t::connect(): \"%m\", continuing\n")));
    }
    if (handle == ACE_INVALID_HANDLE)
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Client_IConnector_t::connect(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::configuration_->peerAddress).c_str ())));

      // release an existing connection (that may resolve the issue)
      connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);
    } // end IF
  } // end IF
done_connect:

  // ...shutdown ?
  if (shutdown)
  {
    if (inherited::configuration_)
    {
      // step1: stop GTK event dispatch ?
      if (inherited::configuration_->hasUI)
        IRC_CLIENT_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false,  // wait ?
                                                                false); // N/A
      else
      {
        if (inherited::configuration_->cursesState)
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::configuration_->cursesState->lock);
          inherited::configuration_->cursesState->finished = true;
        } // end IF
      } // end IF
    } // end IF

    // step2: stop event dispatch
    Common_Tools::finalizeEventDispatch (inherited::configuration_->useReactor,  // stop reactor ?
                                         !inherited::configuration_->useReactor, // stop proactor ?
                                         -1);                                    // group ID (--> don't block !)
  } // end IF
}
