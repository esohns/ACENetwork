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

#include "file_server_signalhandler.h"

#include <sstream>
#include <string>

#include "ace/Assert.h"
#include "ace/Log_Msg.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"

#include "common_icontrol.h"

#include "common_event_tools.h"

#include "common_timer_manager_common.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager.h"
#endif // GTK_SUPPORT

#include "net_macros.h"

#include "test_u_connection_manager_common.h"

FileServer_SignalHandler::FileServer_SignalHandler ()
: inherited (this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("FileServer_SignalHandler::FileServer_SignalHandler"));

}

void
FileServer_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("FileServer_SignalHandler::handle"));

  int result = -1;
  FileServer_TCPIConnectionManager_t* iconnection_manager_p =
      FILESERVER_TCPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);

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
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->dispatchState);

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

    // step1: invoke controller (if any)
    if (inherited::configuration_->listener)
    {
      try {
        inherited::configuration_->listener->stop ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_ITask::stop(), returning\n")));
        return;
      }
    } // end IF

    // step2: stop timer
    if (inherited::configuration_->statisticReportingTimerId >= 0)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (inherited::configuration_->statisticReportingTimerId,
                                                              &act_p);
      if (result <= 0)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to cancel timer (id: %d): \"%m\", returning\n"),
                    inherited::configuration_->statisticReportingTimerId));

        // clean up
        inherited::configuration_->statisticReportingTimerId = -1;

        return;
      } // end IF
      inherited::configuration_->statisticReportingTimerId = -1;
    } // end IF

    // step3: stop accepting connections, abort open connections
    iconnection_manager_p->stop (false, true);
    iconnection_manager_p->abort ();

    // step5: stop reactor (&& proactor, if applicable)
    Common_Event_Tools::finalizeEventDispatch (*inherited::configuration_->dispatchState,
                                               false);                                    // don't block
 } // end IF
}
