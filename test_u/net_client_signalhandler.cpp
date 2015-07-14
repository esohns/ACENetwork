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

#include "net_common.h"
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
Net_Client_SignalHandler::initialize (const Net_Client_SignalHandlerConfiguration& configuration_in)
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

  bool shutdown = false;
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
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", aborting\n"),
                  signal_in));
      return false;
    }
  } // end SWITCH

  // ------------------------------------

  // ...abort ?
  if (abort)
    NET_CONNECTIONMANAGER_SINGLETON::instance ()->abortOldestConnection ();

  // ...connect ?
  if (connect && configuration_.connector)
  {
    Net_SocketHandlerConfiguration* socket_handler_configuration_p = NULL;
    ACE_NEW_NORETURN (socket_handler_configuration_p,
                      Net_SocketHandlerConfiguration ());
    if (!socket_handler_configuration_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", continuing\n")));
      goto check_shutdown;
    } // end IF
    socket_handler_configuration_p->bufferSize =
      NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
    socket_handler_configuration_p->messageAllocator =
      configuration_.messageAllocator;
    socket_handler_configuration_p->socketConfiguration =
      configuration_.socketConfiguration;
    socket_handler_configuration_p->statisticCollectionInterval =
      configuration_.statisticCollectionInterval;

    Net_Client_ConnectorConfiguration connector_configuration;
    connector_configuration.connectionManager =
      NET_CONNECTIONMANAGER_SINGLETON::instance ();
    connector_configuration.socketHandlerConfiguration =
      socket_handler_configuration_p;
    //connector_configuration.statisticCollectionInterval =
    //  configuration_.statisticCollectionInterval;

    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    try
    {
      configuration_.connector->initialize (connector_configuration);
      handle = configuration_.connector->connect (configuration_.peerAddress);
    }
    catch (...)
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Client_IConnector::connect(): \"%m\", continuing\n")));
    }
    if (handle == ACE_INVALID_HANDLE)
    {
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset(buffer, 0, sizeof (buffer));
      result = configuration_.peerAddress.addr_to_string (buffer,
                                                          sizeof (buffer));
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Client_IConnector::connect(\"%s\"): \"%m\", continuing\n"),
                  buffer));
    } // end IF
  } // end IF

check_shutdown:
  // ...shutdown ?
  if (shutdown)
  {
    // step1: stop action timer (if any)
    if (configuration_.actionTimerId >= 0)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (configuration_.actionTimerId,
                                                                    &act_p);
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel action timer (ID: %d): \"%m\", continuing\n"),
                    configuration_.actionTimerId));
      configuration_.actionTimerId = -1;
    } // end IF

    // step2: cancel connection attempts (if any)
    if (configuration_.connector)
    {
      try
      {
        configuration_.connector->abort ();
      }
      catch (...)
      {
        // *PORTABILITY*: tracing in a signal handler context is not portable
        // *TODO*
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_Client_IConnector_t::abort(), continuing\n")));
      }
    } // end IF

    // step3: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (true,         // stop reactor ?
                                         !useReactor_, // stop proactor ?
                                         -1);          // group ID (--> don't block !)
  } // end IF

  return true;
}
