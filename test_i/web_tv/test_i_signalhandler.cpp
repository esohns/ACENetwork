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

#include "test_i_signalhandler.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "net_macros.h"

#include "test_i_connection_manager_common.h"

Test_I_SignalHandler::Test_I_SignalHandler ()
 : inherited (this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_SignalHandler::Test_I_SignalHandler"));

}

void
Test_I_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_SignalHandler::handle"));

  bool statistic = false;
  bool shutdown = false;
  switch (signal_in.signal)
  {
    case SIGINT:
// *PORTABILITY*: on Windows SIGQUIT is not defined
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGQUIT:
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  case SIGBREAK:
#else
  case SIGUSR1:
#endif // ACE_WIN32 || ACE_WIN64
    {
      // print statistic
      statistic = true;

      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGHUP:
    case SIGUSR2:
#endif // ACE_WIN32 || ACE_WIN64
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
                  ACE_TEXT ("received invalid/unknown signal (was: %d --> \"%S\"), returning\n"),
                  signal_in.signal, signal_in.signal));

      ACE_OS::last_error (EINVAL);

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
    Test_I_IConnectionManager_t* iconnection_manager_p =
        TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
    ACE_ASSERT (iconnection_manager_p);
    iconnection_manager_p->stop (false,  // wait ?
                                 false); // high priority ?
    iconnection_manager_p->abort (false); // wait ?
    Test_I_IConnectionManager_3_t* iconnection_manager_2 =
      TEST_I_CONNECTIONMANAGER_SINGLETON_3::instance ();
    ACE_ASSERT (iconnection_manager_2);
    iconnection_manager_2->stop (false,  // wait ?
                                 false); // high priority ?
    iconnection_manager_2->abort (false); // wait ?

    // step5: stop reactor (&& proactor, if applicable)
    //Common_Tools::finalizeEventDispatch (inherited::configuration_->dispatchState->proactorGroupId,
    //                                     inherited::configuration_->dispatchState->reactorGroupId,
    //                                     false);                                                    // don't block

    // *IMPORTANT NOTE*: there is no real reason to wait here
  } // end IF
}
