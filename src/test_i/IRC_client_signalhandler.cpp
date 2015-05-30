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

#include "ace/Log_Msg.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"

#include "net_macros.h"

#include "IRC_client_common.h"
#include "IRC_client_network.h"

IRC_Client_SignalHandler::IRC_Client_SignalHandler (bool useReactor_in)
 : inherited (this,          // event handler handle
              useReactor_in) // use reactor ?
 , configuration_ (NULL)
 , useReactor_ (useReactor_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::IRC_Client_SignalHandler"));

}

IRC_Client_SignalHandler::~IRC_Client_SignalHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::~IRC_Client_SignalHandler"));

}

bool
IRC_Client_SignalHandler::initialize (const IRC_Client_SignalHandlerConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::initialize"));

  configuration_ =
      &const_cast<IRC_Client_SignalHandlerConfiguration_t&> (configuration_in);

  return true;
}

bool
IRC_Client_SignalHandler::handleSignal (int signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::handleSignal"));

  int result = -1;
  bool stop_event_dispatching = false;
  bool connect_to_server = false;
  bool abort_oldest = false;
  switch (signal_in)
  {
    case SIGINT:
    case SIGTERM:
// *PORTABILITY*: this isn't portable: on Windows SIGQUIT and SIGHUP are not defined...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGQUIT:
#endif
    {
//       // *PORTABILITY*: tracing in a signal handler context is not portable
//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("shutting down...\n")));

      // shutdown...
      stop_event_dispatching = true;

      break;
    }
// *PORTABILITY*: this isn't portable: on Windows SIGUSR1 and SIGUSR2 are not defined,
// so we handle SIGBREAK (21) and SIGABRT (22) instead...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR1:
#else
  case SIGBREAK:
#endif
    {
      // (try to) connect...
      connect_to_server = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR2:
#else
  case SIGABRT:
#endif
    {
      // (try to) abort oldest connection...
      abort_oldest = true;

      break;
    }
    default:
    {
      //// *PORTABILITY*: tracing in a signal handler context is not portable
      //ACE_DEBUG((LM_ERROR,
      //           ACE_TEXT("received unknown signal: \"%S\", continuing\n"),
      //           signal_in));

      break;
    }
  } // end SWITCH

  // ...abort ?
  if (abort_oldest)
  {
    // release an existing connection...
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abortOldestConnection ();
  } // end IF

  // ...connect ?
  if (connect_to_server && configuration_->connector)
  {
    bool result_2 = false;
    try
    {
      result_2 = configuration_->connector->connect (configuration_->peerAddress);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Client_IConnector_t::connect(): \"%m\", continuing\n")));
    }
    if (!result_2)
    {
      // debug info
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset(buffer, 0, sizeof (buffer));
      result = configuration_->peerAddress.addr_to_string (buffer,
                                                           sizeof (buffer));
      if (result == -1)
      {
        // *PORTABILITY*: tracing in a signal handler context is not portable
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
      } // end IF
      // *PORTABILITY*: tracing in a signal handler context is not portable
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Client_IConnector_t::connect(\"%s\"): \"%m\", continuing\n"),
                  buffer));

      // release an existing connection, maybe that helps...
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abortOldestConnection ();
    } // end IF
  } // end IF

  // ...shutdown ?
  if (stop_event_dispatching)
  {
    // stop everything, i.e.
    // - leave reactor event loop handling signals, sockets, (maintenance) timers...
    // - break out of any (blocking) calls
    // --> (try to) terminate in a well-behaved manner

    // stop reactor
    ACE_Reactor* reactor_p = inherited::reactor ();
    ACE_ASSERT (reactor_p);
    ACE_Proactor* proactor_p = ACE_Proactor::instance ();
    ACE_ASSERT (proactor_p);
    if ((reactor_p->end_event_loop () == -1) ||
        (proactor_p->end_event_loop () == -1))
    {
      //// *PORTABILITY*: tracing in a signal handler context is not portable
      //ACE_DEBUG((LM_ERROR,
      //           ACE_TEXT("failed to terminate event handling: \"%m\", continuing\n")));
    } // end IF

    // de-register from the reactor...
    return false;
  } // end IF

  return true;
}
