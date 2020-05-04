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
#include "net_client_signalhandler.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_timer_manager_common.h"
#include "common_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
//#include "common_ui_gtk_manager.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "net_common_tools.h"
#include "net_macros.h"

#include "test_u_connection_manager_common.h"
#include "test_u_sessionmessage.h"

Client_SignalHandler::Client_SignalHandler (enum Common_EventDispatchType eventDispatchMode_in,
                                            enum Common_SignalDispatchType signalDispatchMode_in,
                                            ACE_SYNCH_RECURSIVE_MUTEX* lock_in)
 : inherited (signalDispatchMode_in,
              lock_in,
              this) // event handler handle
 , address_ ()
 , eventDispatch_ (eventDispatchMode_in)
 , timerId_ (-1)
 , AsynchTCPConnector_ (true)
 , TCPConnector_ (true)
 , AsynchUDPConnector_ (true)
 , UDPConnector_ (true)
#if defined (SSL_SUPPORT)
 , SSLConnector_ (true)
#endif // SSL_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Client_SignalHandler::Client_SignalHandler"));

}

bool
Client_SignalHandler::initialize (const struct Client_SignalHandlerConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Client_SignalHandler::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.dispatchState);
  ACE_ASSERT (configuration_in.dispatchState->configuration);
  ACE_ASSERT (configuration_in.TCPConnectionConfiguration);
  ACE_ASSERT (configuration_in.UDPConnectionConfiguration);
  ACE_ASSERT (configuration_in.protocolConfiguration);

  // *TODO*: remove type inference
  address_ = configuration_in.address;
  timerId_ = configuration_in.actionTimerId;

  AsynchTCPConnector_.initialize (*configuration_in.TCPConnectionConfiguration);
  TCPConnector_.initialize (*configuration_in.TCPConnectionConfiguration);
  AsynchUDPConnector_.initialize (*configuration_in.UDPConnectionConfiguration);
  UDPConnector_.initialize (*configuration_in.UDPConnectionConfiguration);
#if defined (SSL_SUPPORT)
  SSLConnector_.initialize (*configuration_in.TCPConnectionConfiguration);
#endif // SSL_SUPPORT

  return inherited::initialize (configuration_in);
}

void
Client_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  NETWORK_TRACE (ACE_TEXT ("Client_SignalHandler::handle"));

  int result = -1;
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  typename Test_U_TCPConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
  typename Test_U_UDPConnectionManager_t::INTERFACE_T* iconnection_manager_2 =
      TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  ACE_ASSERT (iconnection_manager_2);

  bool abort = false;
  bool connect = false;
  bool shutdown = false;
  switch (signal_in.signal)
  {
    case SIGINT:
// *PORTABILITY*: on Windows SIGQUIT is not defined
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGQUIT:
#endif
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      //ACE_DEBUG((LM_DEBUG,
      //           ACE_TEXT("shutting down\n")));

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
    { // (try to) connect to server
      connect = true;
      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGUSR2:
#endif
    case SIGTERM:
    { // abort a connection
      abort = true;
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

  // -------------------------------------
  ACE_ASSERT (inherited::configuration_->protocolConfiguration);

  // abort ?
  if (abort)
  {
    switch (inherited::configuration_->protocolConfiguration->transportLayer)
    {
      case NET_TRANSPORTLAYER_TCP:
      case NET_TRANSPORTLAYER_SSL:
        iconnection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);
        break;
      case NET_TRANSPORTLAYER_UDP:
        iconnection_manager_2->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    inherited::configuration_->protocolConfiguration->transportLayer));
        return;
      }
    } // end SWITCH
  } // end IF

  Test_U_ITCPConnector_t* ssl_tcp_connector_p = NULL;
  Test_U_IUDPConnector_t* udp_connector_p = NULL;
  switch (inherited::configuration_->protocolConfiguration->transportLayer)
  {
    case NET_TRANSPORTLAYER_TCP:
    {
      switch (eventDispatch_)
      {
        case COMMON_EVENT_DISPATCH_PROACTOR:
        {
          ssl_tcp_connector_p = &AsynchTCPConnector_;
          break;
        }
        case COMMON_EVENT_DISPATCH_REACTOR:
        {
          ssl_tcp_connector_p = &TCPConnector_;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event dispatch type (was: %d), returning\n"),
                      eventDispatch_));
          return;
        }
      } // end SWITCH
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      switch (eventDispatch_)
      {
        case COMMON_EVENT_DISPATCH_PROACTOR:
        {
          udp_connector_p = &AsynchUDPConnector_;
          break;
        }
        case COMMON_EVENT_DISPATCH_REACTOR:
        {
          udp_connector_p = &UDPConnector_;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event dispatch type (was: %d), returning\n"),
                      eventDispatch_));
          return;
        }
      } // end SWITCH
      break;
    }
    case NET_TRANSPORTLAYER_SSL:
    {
#if defined (SSL_SUPPORT)
      ssl_tcp_connector_p = &SSLConnector_;
#endif // SSL_SUPPORT
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                  inherited::configuration_->protocolConfiguration->transportLayer));
      return;
    }
  } // end SWITCH

  // connect ?
  if (connect)
  {
    ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
    switch (inherited::configuration_->protocolConfiguration->transportLayer)
    {
      case NET_TRANSPORTLAYER_TCP:
      case NET_TRANSPORTLAYER_SSL:
      { ACE_ASSERT (ssl_tcp_connector_p);
        try {
          handle_h = ssl_tcp_connector_p->connect (address_);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_IConnector_t::connect(%s), returning\n"),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
          return;
        }
        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      { ACE_ASSERT (udp_connector_p);
        try {
          handle_h = udp_connector_p->connect (address_);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_IConnector_t::connect(%s), returning\n"),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
          return;
        }
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    inherited::configuration_->protocolConfiguration->transportLayer));
        return;
      }
    } // end SWITCH
    //if (handle_h == ACE_INVALID_HANDLE)
    //{
    //  // *PORTABILITY*: tracing in a signal handler context is not portable
    //  // *TODO*
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Net_IConnector::connect(%s): \"%m\", continuing\n"),
    //              ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
    //} // end IF
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

    // step2: stop action timer (if any)
    if (timerId_ >= 0)
    {
      const void* act_p = NULL;
      result = timer_manager_p->cancel_timer (timerId_,
                                              &act_p);
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      if (unlikely (result <= 0))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_ITimer_T::cancel_timer(%d): \"%m\", continuing\n"),
                    timerId_));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("cancelled action timer (id was: %d)\n"),
                    timerId_));
      timerId_ = -1;
    } // end IF
    timer_manager_p->stop (false, // wait for completion ?
                           true); // N/A

    // step3: cancel connection attempts (if any)
    if (eventDispatch_ == COMMON_EVENT_DISPATCH_PROACTOR)
    {
      switch (inherited::configuration_->protocolConfiguration->transportLayer)
      {
        case NET_TRANSPORTLAYER_TCP:
        case NET_TRANSPORTLAYER_SSL:
        { ACE_ASSERT (ssl_tcp_connector_p);
          Test_U_ITCPAsynchConnector_t* iasynch_connector_p =
              dynamic_cast<Test_U_ITCPAsynchConnector_t*> (ssl_tcp_connector_p);
          // *NOTE*: SSL currently supported 'synchronous-only'
//          ACE_ASSERT (iasynch_connector_p);
          if (iasynch_connector_p)
          {
            try {
              iasynch_connector_p->abort ();
            } catch (...) {
              // *PORTABILITY*: tracing in a signal handler context is not portable
              // *TODO*
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in Net_IAsynchConnector_T::abort(), continuing\n")));
            }
          } // end IF
          break;
        }
        case NET_TRANSPORTLAYER_UDP:
        { ACE_ASSERT (udp_connector_p);
          Test_U_IUDPAsynchConnector_t* iasynch_connector_p =
              dynamic_cast<Test_U_IUDPAsynchConnector_t*> (udp_connector_p);
          ACE_ASSERT (iasynch_connector_p);
          try {
            iasynch_connector_p->abort ();
          } catch (...) {
            // *PORTABILITY*: tracing in a signal handler context is not portable
            // *TODO*
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Net_IAsynchConnector_T::abort(), continuing\n")));
          }
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                      inherited::configuration_->protocolConfiguration->transportLayer));
          return;
        }
      } // end SWITCH
    } // end IF

    // step4: stop accepting connections, abort open connections
    switch (inherited::configuration_->protocolConfiguration->transportLayer)
    {
      case NET_TRANSPORTLAYER_TCP:
      case NET_TRANSPORTLAYER_SSL:
      {
        iconnection_manager_p->stop (false, // wait for completion ?
                                     true); // N/A
        iconnection_manager_p->abort ();
        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      {
        iconnection_manager_2->stop (false, // wait for completion ?
                                     true); // N/A
        iconnection_manager_2->abort ();
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    inherited::configuration_->protocolConfiguration->transportLayer));
        return;
      }
    } // end SWITCH

    // step5: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (inherited::configuration_->dispatchState->proactorGroupId,
                                         inherited::configuration_->dispatchState->reactorGroupId,
                                         false);                                                    // don't block

    // step6: stop UI ?
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
#endif // GTK_USE
#endif // GUI_SUPPORT
  } // end IF
}
