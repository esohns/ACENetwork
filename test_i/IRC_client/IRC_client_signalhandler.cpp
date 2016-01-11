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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"
#include "ace/Synch_Traits.h"

#include "common_tools.h"

#include "net_macros.h"

#include "IRC_client_curses.h"

IRC_Client_SignalHandler::IRC_Client_SignalHandler (bool useReactor_in,
                                                    bool useCursesLibrary_in)
 : inherited (this,          // event handler handle
              useReactor_in) // use reactor ?
 , configuration_ (NULL)
 , useCursesLibrary_ (useCursesLibrary_in)
 , useReactor_ (useReactor_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::IRC_Client_SignalHandler"));

}

IRC_Client_SignalHandler::~IRC_Client_SignalHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::~IRC_Client_SignalHandler"));

}

bool
IRC_Client_SignalHandler::initialize (const IRC_Client_SignalHandlerConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::initialize"));

  configuration_ =
      &const_cast<IRC_Client_SignalHandlerConfiguration&> (configuration_in);

  return true;
}

bool
IRC_Client_SignalHandler::handleSignal (int signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_SignalHandler::handleSignal"));

  int result = -1;

  bool abort = false;
  bool connect = false;
  bool shutdown = false;
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
//       // *TODO*
//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("shutting down...\n")));

      // shutdown...
      shutdown = true;

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
      connect = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR2:
#else
  case SIGABRT:
#endif
    {
      // abort connection...
      abort = true;

      break;
    }
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown signal: \"%S\", aborting\n"),
                  signal_in));
      return false;
    }
  } // end SWITCH

  // ...abort ?
  if (abort)
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abortLeastRecent ();

  // ...connect ?
  if (connect)
  {
    if (!configuration_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("not configured: cannot connect, continuing\n")));
      goto done_connect;
    } // end IF
    if (!configuration_->connector)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no connector: cannot connect, continuing\n")));
      goto done_connect;
    } // end IF

    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    try
    {
      handle = configuration_->connector->connect (configuration_->peerAddress);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Client_IConnector_t::connect(): \"%m\", continuing\n")));
    }
    if (handle == ACE_INVALID_HANDLE)
    {
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset(buffer, 0, sizeof (buffer));
      result = configuration_->peerAddress.addr_to_string (buffer,
                                                           sizeof (buffer));
      if (result == -1)
      {
        // *PORTABILITY*: tracing in a signal handler context is not portable
        // *TODO*
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
      } // end IF
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Client_IConnector_t::connect(\"%s\"): \"%m\", continuing\n"),
                  buffer));

      // release an existing connection, maybe that helps...
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abortLeastRecent ();
    } // end IF
  } // end IF
done_connect:

  // ...shutdown ?
  if (shutdown)
  {
    // step1: notify curses dispatch ?
    if (configuration_)
      if (configuration_->cursesState)
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (configuration_->cursesState->lock);

        configuration_->cursesState->finished = true;
      } // end IF

    // step2: stop event dispatch
    Common_Tools::finalizeEventDispatch (useReactor_,  // stop reactor ?
                                         !useReactor_, // stop proactor ?
                                         -1);          // group ID (--> don't block !)
  } // end IF

  return true;
}