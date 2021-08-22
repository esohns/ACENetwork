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

//#include "ace/Synch.h"
#include "test_u_HTTP_decoder_signalhandler.h"

#include "ace/Log_Msg.h"

//#include "common_timer_manager.h"
#include "common_tools.h"

#include "net_macros.h"

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_session_message.h"

Test_U_Protocol_SignalHandler::Test_U_Protocol_SignalHandler (enum Common_SignalDispatchType dispatchMode_in,
                                                              ACE_SYNCH_RECURSIVE_MUTEX* lock_in)
 : inherited (dispatchMode_in,
              lock_in,
              this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Protocol_SignalHandler::Test_U_Protocol_SignalHandler"));

}

void
Test_U_Protocol_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Protocol_SignalHandler::handle"));

//  int result = -1;

  bool statistic = false;
  bool shutdown = false;
  switch (signal_in.signal)
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
      // print statistic
      statistic = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGUSR2:
#endif
    case SIGTERM:
    {
      // print statistic
      statistic = true;

      break;
    }
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", returning\n"),
                  signal_in));
      return;
    }
  } // end SWITCH

  // ------------------------------------

  // print statistic ?
  if (statistic)
  {
    try {
      //handle = configuration_.connector->connect (configuration_.peerAddress);
    } catch (...) {
      //// *PORTABILITY*: tracing in a signal handler context is not portable
      //// *TODO*
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("caught exception in Net_Client_IConnector::connect(): \"%m\", continuing\n")));
    }
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

    //// step1: stop action timer (if any)
    //if (configuration_.actionTimerId >= 0)
    //{
    //  const void* act_p = NULL;
    //  result =
    //      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (configuration_.actionTimerId,
    //                                                                &act_p);
    //  // *PORTABILITY*: tracing in a signal handler context is not portable
    //  // *TODO*
    //  if (result <= 0)
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to cancel action timer (ID: %d): \"%m\", continuing\n"),
    //                configuration_.actionTimerId));
    //  configuration_.actionTimerId = -1;
    //} // end IF

    // step2: stop/abort(/wait) for connections
    Test_U_IConnectionManager_t* iconnection_manager_p =
        TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
    ACE_ASSERT (iconnection_manager_p);
    iconnection_manager_p->stop (false, // wait ?
                                 true); // high priority ?
    iconnection_manager_p->abort ();

    // step5: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (inherited::configuration_->dispatchState->proactorGroupId,
                                         inherited::configuration_->dispatchState->reactorGroupId,
                                         false);                                                    // don't block

    // *IMPORTANT NOTE*: there is no real reason to wait here
  } // end IF
}
