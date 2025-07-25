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

#include "IRC_client_signalhandler.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "common_event_tools.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#include "net_macros.h"

#if defined (CURSES_SUPPORT)
#include "IRC_client_curses.h"
#endif // CURSES_SUPPORT
#include "IRC_client_network.h"

#include "IRC_client_gui_common.h"

IRC_Client_SignalHandler::IRC_Client_SignalHandler ()
 : inherited (this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::IRC_Client_SignalHandler"));

}

void
IRC_Client_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::handleSignal"));

  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->dispatchState);

  IRC_Client_Connection_Manager_t* connection_manager_p =
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();

  bool abort = false;
  bool connect = false;
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
#endif // ACE_WIN32 || ACE_WIN64
    {
      // (try to) connect
      connect = true;

      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case SIGABRT:
#else
    case SIGUSR2:
#endif // ACE_WIN32 || ACE_WIN64
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
                  signal_in.signal));
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
#if defined (CURSES_USE)
    COMMON_UI_CURSES_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                           true); // high priority ?
#elif defined (GTK_USE)
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                        true); // high priority ?
#endif // CURSES_USE || GTK_USE

    // step2: stop event dispatch
    Common_Event_Tools::finalizeEventDispatch (*inherited::configuration_->dispatchState,
                                               false,                                    // don't block
                                               false);                                   // don't delete singletons
  } // end IF
}
