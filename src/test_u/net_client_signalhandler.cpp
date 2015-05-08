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

#include "common_timer_manager.h"
#include "common_tools.h"

#include "net_connection_manager_common.h"
#include "net_macros.h"

Net_Client_SignalHandler::Net_Client_SignalHandler (bool useReactor_in)
 : inherited (this,          // event handler handle
              useReactor_in) // use reactor ?
 , configuration_ ()
 , useReactor_ (useReactor_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SignalHandler::Net_Client_SignalHandler"));

}

Net_Client_SignalHandler::~Net_Client_SignalHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SignalHandler::~Net_Client_SignalHandler"));

}

bool
Net_Client_SignalHandler::initialize (const Net_Client_SignalHandlerConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SignalHandler::initialize"));

  configuration_ = configuration_in;

  return true;
}

bool
Net_Client_SignalHandler::handleSignal (int signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SignalHandler::handleSignal"));

  int result = -1;

  bool stop_event_dispatching = false;
  bool connect = false;
  bool abort = false;
  switch (signal_in)
  {
    case SIGINT:
// *PORTABILITY*: on Windows SIGQUIT is not defined
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGQUIT:
#endif
    {
      //ACE_DEBUG((LM_DEBUG,
      //           ACE_TEXT("shutting down...\n")));

      // shutdown...
      stop_event_dispatching = true;

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
      // (try to) connect...
      connect = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGUSR2:
#endif
    case SIGTERM:
    {
      // (try to) abort a connection...
      abort = true;

      break;
    }
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", aborting\n"),
                  signal_in));
      return false;
    }
  } // end SWITCH

  // ------------------------------------

  // ...abort ?
  if (abort)
  {
    // release an existing connection...
    NET_CONNECTIONMANAGER_SINGLETON::instance ()->abortOldestConnection ();
  } // end IF

  // ...connect ?
  if (connect && configuration_.connector)
  {
    bool result_2 = false;
    try
    {
      result_2 = configuration_.connector->connect (configuration_.peerAddress);
    }
    catch (...)
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Client_IConnector::connect(): \"%m\", aborting\n")));
      return false;
    }
    if (!result_2)
    {
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset(buffer, 0, sizeof (buffer));
      result = configuration_.peerAddress.addr_to_string (buffer,
                                                          sizeof (buffer));
      // *PORTABILITY*: tracing in a signal handler context is not portable
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
      // *PORTABILITY*: tracing in a signal handler context is not portable
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Client_IConnector::connect(\"%s\"): \"%m\", aborting\n"),
                  buffer));
      return false;
    } // end IF
  } // end IF

  // ...shutdown ?
  if (stop_event_dispatching)
  {
    // stop everything, i.e.
    // - leave reactor event loop handling signals, sockets, (maintenance) timers...
    // --> (try to) terminate in a well-behaved manner

    // step1: stop all open connections

    // stop action timer (might spawn new connections otherwise)
    if (configuration_.actionTimerID >= 0)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (configuration_.actionTimerID,
                                                              &act_p);
      if (result <= 0)
      {
        // *PORTABILITY*: tracing in a signal handler context is not portable
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", aborting\n"),
                    configuration_.actionTimerID));

        // clean up
        configuration_.actionTimerID = -1;

        return false;
      } // end IF
      configuration_.actionTimerID = -1;
    } // end IF
    if (configuration_.connector)
    {
      try
      {
        configuration_.connector->abort ();
      }
      catch (...)
      {
        // *PORTABILITY*: tracing in a signal handler context is not portable
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_Client_IConnector_t::abort(), aborting\n")));
        return false;
      }
    } // end IF

    // step3: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (true,         // stop reactor ?
                                         !useReactor_, // stop proactor ?
                                         -1);          // group ID (--> don't block !)
  } // end IF

  return true;
}
