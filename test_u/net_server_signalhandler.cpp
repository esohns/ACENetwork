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

#include "net_server_signalhandler.h"

#include <sstream>
#include <string>

#include "ace/Assert.h"
#include "ace/Log_Msg.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"

#include "common_icontrol.h"
#include "common_timer_manager.h"
#include "common_tools.h"

#include "net_connection_manager_common.h"
#include "net_macros.h"

Net_Server_SignalHandler::Net_Server_SignalHandler (bool useReactor_in)
 : inherited (this,          // event handler handle
              useReactor_in) // use reactor ?
 , configuration_ ()
 , useReactor_ (useReactor_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SignalHandler::Net_Server_SignalHandler"));

//  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
}

Net_Server_SignalHandler::~Net_Server_SignalHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SignalHandler::~Net_Server_SignalHandler"));

}

bool
Net_Server_SignalHandler::initialize (const Net_Server_SignalHandlerConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SignalHandler::initialize"));

  configuration_ = configuration_in;

  return true;
}

bool
Net_Server_SignalHandler::handleSignal (int signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SignalHandler::handleSignal"));

  int result = -1;

  bool stop_event_dispatching = false;
  bool report = false;
  switch (signal_in)
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

      // shutdown...
      stop_event_dispatching = true;

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
      // dump statistics
      report = true;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", aborting\n"),
                  signal_in));
      return false;
    }
  } // end SWITCH

  // ------------------------------------

  // report ?
  if (report && configuration_.statisticReportingHandler)
  {
    try
    {
      configuration_.statisticReportingHandler->report ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
      return false;
    }
  } // end IF

  // ...shutdown ?
  if (stop_event_dispatching)
  {
    // stop everything, i.e.
    // - leave reactor event loop handling signals, sockets, (maintenance) timers...
    // --> (try to) terminate in a well-behaved manner

    // step1: invoke controller (if any)
    if (configuration_.listener)
    {
      try
      {
        configuration_.listener->stop ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IControl::stop(), aborting\n")));
        return false;
      }
    } // end IF

    // step2: stop timer
    if (configuration_.statisticReportingTimerID >= 0)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (configuration_.statisticReportingTimerID,
                                                              &act_p);
      if (result <= 0)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", aborting\n"),
                    configuration_.statisticReportingTimerID));

        // clean up
        configuration_.statisticReportingTimerID = -1;

        return false;
      } // end IF
      configuration_.statisticReportingTimerID = -1;
    } // end IF

    // step3: stop/abort/wait for connections
    NET_CONNECTIONMANAGER_SINGLETON::instance ()->stop ();
    NET_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
    // *IMPORTANT NOTE*: as long as connections are inactive (i.e. events are
    // dispatched by reactor thread(s), there is no real reason to wait here)
    //RPG_NET_CONNECTIONMANAGER_SINGLETON::instance()->waitConnections();

    // step4: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (true,         // stop reactor ?
                                         !useReactor_, // stop proactor ?
                                         -1);          // group ID (--> don't block !)
  } // end IF

  return true;
}
