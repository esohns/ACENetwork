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

#include "ace/Log_Msg.h"

#include "common_event_tools.h"

#include "common_timer_manager_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_macros.h"

#include "test_i_connection_manager_common.h"

template <typename ConfigurationType,
          typename TCPConnectionManagerType,
          typename UDPConnectionManagerType>
Test_I_AVStream_Server_SignalHandler_T<ConfigurationType,
                                       TCPConnectionManagerType,
                                       UDPConnectionManagerType>::Test_I_AVStream_Server_SignalHandler_T ()
 : inherited (this) // event handler handle
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_SignalHandler_T::Test_I_AVStream_Server_SignalHandler_T"));

}

template <typename ConfigurationType,
          typename TCPConnectionManagerType,
          typename UDPConnectionManagerType>
void
Test_I_AVStream_Server_SignalHandler_T<ConfigurationType,
                                       TCPConnectionManagerType,
                                       UDPConnectionManagerType>::handle (const struct Common_Signal& signal_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Server_SignalHandler_T::handle"));

  int result = -1;
  bool close_all = false;
  bool shutdown = false;
  bool statistic = false;
  switch (signal_in.signal)
  {
    // *PORTABILITY*: on Windows (TM) SIGQUIT is not defined
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case SIGINT:
#else
    case SIGINT:
      break;
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
// --> use SIGBREAK (21) and SIGTERM (15) instead
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
    // *PORTABILITY*: on Windows (TM) SIGHUP and SIGUSRx are not defined
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGHUP:
    case SIGUSR2:
#endif // ACE_WIN32 || ACE_WIN64
    case SIGTERM:
    {
      // close
      close_all = true;
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
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionManager);
  ACE_ASSERT (inherited::configuration_->dispatchState);

  // print statistic ?
  if (statistic &&
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

  //Test_I_Target_IInetConnectionManager_t* connection_manager_p =
  //  TEST_I_TARGET_CONNECTIONMANAGER_SINGLETON::instance ();
  //ACE_ASSERT (connection_manager_p);
  if (close_all)
    //connection_manager_p->abort (false);
    inherited::configuration_->connectionManager->abort ();

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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                          true); // high priority ?
#endif // GTK_USE
#endif // GUI_SUPPORT

    // step2: invoke controller (if any)
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

    // step3: stop timer
    if (inherited::configuration_->statisticReportingTimerId >= 0)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (inherited::configuration_->statisticReportingTimerId,
                                                              &act_p);
      if (result <= 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (id: %d): \"%m\", returning\n"),
                    inherited::configuration_->statisticReportingTimerId));
        inherited::configuration_->statisticReportingTimerId = -1;
        return;
      } // end IF
      inherited::configuration_->statisticReportingTimerId = -1;
    } // end IF

    // step4: stop/abort(/wait) for connection(s)
    //connection_manager_p->stop ();
    //connection_manager_p->abort ();
//    connection_manager_p->wait ();
    inherited::configuration_->connectionManager->stop (false, true);
    inherited::configuration_->connectionManager->abort ();
    inherited::configuration_->connectionManager->wait ();

    // step5: stop reactor (&& proactor, if applicable)
    Common_Event_Tools::finalizeEventDispatch (*inherited::configuration_->dispatchState,
                                               false);                                    // don't block
  } // end IF
}
