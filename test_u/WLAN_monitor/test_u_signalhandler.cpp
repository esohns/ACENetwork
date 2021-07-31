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
#include "test_u_signalhandler.h"

#include <sstream>
#include <string>

#include "ace/Assert.h"
#include "ace/Log_Msg.h"

#include "common_icontrol.h"
#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "net_macros.h"

//#include "net_wlan_configuration.h"

Test_U_SignalHandler::Test_U_SignalHandler (enum Common_SignalDispatchType dispatchMode_in,
                                            ACE_SYNCH_RECURSIVE_MUTEX* lock_in)
 : inherited (dispatchMode_in,
              lock_in,
              this) // event handler handle
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_SignalHandler::Test_U_SignalHandler"));

}

void
Test_U_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_SignalHandler::handle"));

  //  sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->monitor);

  int result = -1;
  bool shutdown = false;
  bool report = false;
  switch (signal_in.signal)
  {
// *PORTABILITY*: on Windows SIGHUP/SIGQUIT are not defined
// --> use SIGINT (2) and/or SIGTERM (15) instead...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGHUP:
    case SIGQUIT:
#endif // ACE_WIN32 || defined ACE_WIN64
    case SIGINT:
    case SIGTERM:
    {
      shutdown = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//#if defined (_DEBUG)
//      // *NOTE*: gdb raises SIGINT with code SI_USER on 'interrupt'
//      //         --> do not shutdown in this case
//      shutdown = !(Common_Error_Tools::inDebugSession () &&
//                   (signal_in.signal == SIGINT)    &&
//                   (signal_in.siginfo.si_code == SI_USER));
//#endif // _DEBUG
#endif // ACE_WIN32 || defined ACE_WIN64
      break;
    }
// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case SIGBREAK:
#else
    case SIGUSR1:
#endif // ACE_WIN32 || defined ACE_WIN64
    {
      // dump statistic
      report = true;
      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGCHLD:
      break;
#endif // ACE_WIN32 || defined ACE_WIN64
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
    // - activation timers (connection attempts, ...)
    //   exception handlers, ...
    // - leave event loop(s) handling signals, sockets, (maintenance) timers,
    // [- UI dispatch]

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
                    ACE_TEXT ("failed to cancel timer (id: %d): \"%m\", returning\n"),
                    inherited::configuration_->statisticReportingTimerId));

        inherited::configuration_->statisticReportingTimerId = -1;
        return;
      } // end IF
      inherited::configuration_->statisticReportingTimerId = -1;
    } // end IF

    // step4: stop monitor
    inherited::configuration_->monitor->stop (false, // wait ?
                                              true);

//    // step1: stop GTK event processing ?
//    // *NOTE*: triggering UI shutdown from a widget callback is more consistent,
//    //         compared to doing it here
//    if (CBData_)
//      WLANMONITOR_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false,
//                                                               true);

//    // step5: stop reactor (&& proactor, if applicable)
//    Common_Tools::finalizeEventDispatch (inherited::configuration_->useReactor,  // stop reactor ?
//                                         !inherited::configuration_->useReactor, // stop proactor ?
//                                         -1);                                    // group id (--> don't block)
  } // end IF
}
