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

#include "net_client_signalhandler.h"

#include "ace/Log_Msg.h"

//#include "common_timer_manager.h"
#include "common_tools.h"

#include "common_ui_gtk_manager.h"

#include "net_macros.h"

#include "test_u_connection_manager_common.h"

Net_Client_SignalHandler::Net_Client_SignalHandler ()
 : inherited (this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SignalHandler::Net_Client_SignalHandler"));

}

Net_Client_SignalHandler::~Net_Client_SignalHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SignalHandler::~Net_Client_SignalHandler"));

}

bool
Net_Client_SignalHandler::handleSignal (int signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SignalHandler::handleSignal"));

  int result = -1;
  Net_IInetConnectionManager_t* iconnection_manager_p =
      NET_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);

  bool abort = false;
  bool connect = false;
  bool shutdown = false;
  switch (signal_in)
  {
    case SIGINT:
// *PORTABILITY*: on Windows SIGQUIT is not defined
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGQUIT:
#endif
    {
//       // *PORTABILITY*: tracing in a signal handler context is not portable
//       // *TODO*
      //ACE_DEBUG((LM_DEBUG,
      //           ACE_TEXT("shutting down...\n")));

      shutdown = true;

      break;
    }
// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) and SIGTERM (15) instead...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR1:
#else
    case SIGBREAK:
#endif
    {
      // (try to) connect to server
      connect = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGUSR2:
#endif
    case SIGTERM:
    {
      // abort a connection
      abort = true;

      break;
    }
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", aborting\n"),
                  signal_in));
      return false;
    }
  } // end SWITCH

  // ------------------------------------

  // abort ?
  if (abort)
    iconnection_manager_p->abortLeastRecent ();

  // connect ?
  if (connect &&
      inherited::configuration_->connector)
  {
    ACE_ASSERT (inherited::configuration_->socketHandlerConfiguration);
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    try {
      inherited::configuration_->connector->initialize (*inherited::configuration_->socketHandlerConfiguration);
      handle =
        inherited::configuration_->connector->connect (inherited::configuration_->peerAddress);
    } catch (...) {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Client_IConnector::connect(): \"%m\", continuing\n")));
    }
    if (handle == ACE_INVALID_HANDLE)
    {
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result =
        inherited::configuration_->peerAddress.addr_to_string (buffer,
                                                               sizeof (buffer));
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Client_IConnector::connect(\"%s\"): \"%m\", continuing\n"),
                  buffer));
    } // end IF
  } // end IF

//check_shutdown:
  // ...shutdown ?
  if (shutdown)
  {
    // stop everything, i.e.
    // - leave event loop(s) handling signals, sockets, (maintenance) timers,
    //   exception handlers, ...
    // - activation timers (connection attempts, ...)
    // [- UI dispatch]

    // step1: stop GTK event processing
    // *NOTE*: triggering UI shutdown from a widget callback is more consistent,
    //         compared to doing it here
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, true);

    // step2: stop action timer (if any)
    if (inherited::configuration_->actionTimerId >= 0)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::configuration_->actionTimerId,
                                                                    &act_p);
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel action timer (ID: %d): \"%m\", continuing\n"),
                    inherited::configuration_->actionTimerId));
      inherited::configuration_->actionTimerId = -1;
    } // end IF

    // step3: cancel connection attempts (if any)
    if (inherited::configuration_->connector)
    {
      try {
        inherited::configuration_->connector->abort ();
      } catch (...) {
        // *PORTABILITY*: tracing in a signal handler context is not portable
        // *TODO*
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_Client_IConnector_t::abort(), continuing\n")));
      }
    } // end IF

    // step4: stop accepting connections, abort open connections
    iconnection_manager_p->stop (false, true);
    iconnection_manager_p->abort ();

    // step5: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (true,                                   // stop reactor ?
                                         !inherited::configuration_->useReactor, // stop proactor ?
                                         -1);                                    // group ID (--> don't block !)
  } // end IF

  return true;
}
