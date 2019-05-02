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
#include "net_server_signalhandler.h"

#include <sstream>
#include <string>

#include "ace/Assert.h"
#include "ace/Log_Msg.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"

#include "common_icontrol.h"
#include "common_timer_manager_common.h"
#include "common_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "net_macros.h"

#include "test_u_connection_manager_common.h"

Server_SignalHandler::Server_SignalHandler (enum Common_SignalDispatchType dispatchMode_in,
                                            ACE_SYNCH_RECURSIVE_MUTEX* lock_in)
 : inherited (dispatchMode_in,
              lock_in,
              this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("Server_SignalHandler::Server_SignalHandler"));

}

void
Server_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Server_SignalHandler::handle"));

  int result = -1;
  Test_U_ITCPConnectionManager_t* iconnection_manager_p =
      TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_U_IUDPConnectionManager_t* iconnection_manager_2 =
    TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_2);

  bool shutdown = false;
  bool report = false;
  switch (signal_in.signal)
  {
// *PORTABILITY*: on Windows SIGHUP/SIGQUIT are not defined
// --> use SIGINT (2) and/or SIGTERM (15) instead...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGQUIT:
#endif
    case SIGINT:
    case SIGTERM:
    {
      //ACE_DEBUG((LM_DEBUG,
      //           ACE_TEXT("shutting down...\n")));

      // shutdown
      shutdown = true;

      break;
    }
// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR1:
#else
    case SIGBREAK:
#endif
    {
      // dump statistic
      report = true;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", returning\n"),
                  signal_in.signal));
      return;
    }
  } // end SWITCH

  // -------------------------------------

  // report ?
  if (report &&
      inherited::configuration_->statisticReportingHandler)
  {
    try {
      inherited::configuration_->statisticReportingHandler->report ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IStatistic::report(), returning\n")));
      return;
    }
  } // end IF

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

    // step2: invoke controller (if any)
    if (inherited::configuration_->TCPListener)
    {
      try {
        inherited::configuration_->TCPListener->stop ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IControl::stop(), returning\n")));
        return;
      }
    } // end IF

    // step3: stop timer
    if (inherited::configuration_->statisticReportingTimerId >= 0)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (inherited::configuration_->statisticReportingTimerId,
                                                              &act_p);
      if (result <= 0)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", returning\n"),
                    inherited::configuration_->statisticReportingTimerId));

        // clean up
        inherited::configuration_->statisticReportingTimerId = -1;

        return;
      } // end IF
      inherited::configuration_->statisticReportingTimerId = -1;
    } // end IF

    // step4: stop accepting connections, abort open connections
    iconnection_manager_p->stop (false, true);
    iconnection_manager_p->abort ();
    iconnection_manager_2->stop (false, true);
    iconnection_manager_2->abort ();

    // step5: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (inherited::configuration_->dispatchState->proactorGroupId,
                                         inherited::configuration_->dispatchState->reactorGroupId,
                                         false);                                                    // don't block
  } // end IF
}
