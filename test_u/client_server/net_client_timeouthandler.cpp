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
#include "net_client_timeouthandler.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "net_client_defines.h"
#include "net_client_server_defines.h"

#include "test_u_common_tools.h"

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_message.h"
#include "test_u_sessionmessage.h"

Client_TimeoutHandler::Client_TimeoutHandler (enum ActionModeType mode_in,
                                              unsigned int maximumNumberOfConnections_in,
                                              const struct Test_U_ProtocolConfiguration& protocolConfiguration_in,
                                              const Test_U_TCPConnectionConfiguration& TCPConnectionConfiguration_in,
                                              const Test_U_UDPConnectionConfiguration& UDPConnectionConfiguration_in,
                                              enum Common_EventDispatchType eventDispatch_in)
 : inherited (this,  // dispatch interface
              false) // invoke only once ?
 , alternatingModeState_ (ALTERNATING_STATE_CONNECT)
 , connectionConfiguration_ (&const_cast<Test_U_TCPConnectionConfiguration&> (TCPConnectionConfiguration_in))
 , UDPConnectionConfiguration_ (&const_cast<Test_U_UDPConnectionConfiguration&> (UDPConnectionConfiguration_in))
 , eventDispatch_ (eventDispatch_in)
 , lock_ ()
 , maximumNumberOfConnections_ (maximumNumberOfConnections_in)
 , mode_ (mode_in)
 , protocolConfiguration_ (&const_cast<struct Test_U_ProtocolConfiguration&> (protocolConfiguration_in))
 , AsynchTCPConnector_ (true)
 , TCPConnector_ (true)
 , AsynchUDPConnector_ (true)
 , UDPConnector_ (true)
#if defined (SSL_SUPPORT)
 , SSLConnector_ (true)
#endif // SSL_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Client_TimeoutHandler::Client_TimeoutHandler"));

  AsynchTCPConnector_.initialize (*connectionConfiguration_);
  TCPConnector_.initialize (*connectionConfiguration_);
  AsynchUDPConnector_.initialize (*UDPConnectionConfiguration_);
  UDPConnector_.initialize (*UDPConnectionConfiguration_);
#if defined (SSL_SUPPORT)
  SSLConnector_.initialize (*connectionConfiguration_);
#endif // SSL_SUPPORT
}

void
Client_TimeoutHandler::handle (const void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Client_TimeoutHandler::handle"));

  ACE_UNUSED_ARG (arg_in);

//  int result = -1;
//  int index_i = 0;
  Test_U_TCPConnectionManager_t::CONNECTION_T* connection_p = NULL;
  Test_U_TCPConnectionManager_t::CONNECTION_T* connection_2 = NULL;
//  Test_U_UDPConnectionManager_t::CONNECTION_T* connection_2_p = NULL;
//  Test_U_UDPConnectionManager_t::CONNECTION_T* connection_2_2 = NULL;
  bool do_abort = false;
  bool do_abort_oldest = false;
  bool do_abort_youngest = false;
  bool do_connect = false;
  bool do_ping = false;
  typename Test_U_TCPConnectionManager_t::INTERFACE_T* connection_manager_p =
    TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
//  typename Test_U_UDPConnectionManager_t::INTERFACE_T* connection_manager_2 =
//    TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();

  // sanity check(s)
  ACE_ASSERT (connection_manager_p);

  unsigned int number_of_connections_i = 0;

//  connection_manager_p->lock ();
  number_of_connections_i = connection_manager_p->count ();
//  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    switch (mode_)
    {
      case ACTION_NORMAL:
      {
        if (!number_of_connections_i)
          goto continue_;

        connection_2 = Test_U_Common_Tools::getRandomConnection (connection_manager_p);
        if (!connection_2)
        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to retrieve connection #%d/%d, returning\n"),
//                      index_i, number_of_connections_i));
          goto continue_;
        } // end IF

        do_ping = true;

        break;
      }
      case ACTION_ALTERNATING:
      {
        switch (alternatingModeState_)
        {
          case ALTERNATING_STATE_CONNECT:
          {
            // sanity check: max num connections already reached ?
            // --> abort the oldest one first
            if (maximumNumberOfConnections_ &&
                (number_of_connections_i >= maximumNumberOfConnections_))
              do_abort_oldest = true;

            do_connect = true;

            break;
          }
          case ALTERNATING_STATE_ABORT:
          {
            // sanity check
            if (number_of_connections_i == 0)
              break; // nothing to do...

            connection_p = Test_U_Common_Tools::getRandomConnection (connection_manager_p);
            if (!connection_p)
            {
//              ACE_DEBUG ((LM_ERROR,
//                          ACE_TEXT ("failed to retrieve connection #%d/%d, returning\n"),
//                          index_i, number_of_connections_i));
              goto continue_;
            } // end IF

            do_abort = true;

            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("unknown/invalid alternating state (was: %d), returning\n"),
                        alternatingModeState_));
            goto continue_;
          }
        } // end SWITCH

        // cycle mode
        alternatingModeState_ =
          static_cast<enum Client_TimeoutHandler::AlternatingModeStateType> (alternatingModeState_ + 1);
        if (alternatingModeState_ == ALTERNATING_STATE_MAX)
          alternatingModeState_ = ALTERNATING_STATE_CONNECT;

        break;
      }
      case ACTION_STRESS:
      {
        // allow some probability_f for closing connections in between
        float probability_f =
            static_cast<float> (Test_U_Common_Tools::randomGenerator_ ()) / 100.0F;
        if ((number_of_connections_i > 0) &&
            (probability_f <= NET_CLIENT_DEFAULT_ABORT_PROBABILITY))
          do_abort_youngest = true;

        // allow some probability_f for opening connections in between
        probability_f = static_cast<float> (Test_U_Common_Tools::randomGenerator_ ()) / 100.0F;
        if (probability_f <= NET_CLIENT_DEFAULT_CONNECT_PROBABILITY)
          do_connect = true;

        // ping the server

        // sanity check
        if (!number_of_connections_i ||
            ((number_of_connections_i == 1) && do_abort_youngest))
          break;

        connection_2 = Test_U_Common_Tools::getRandomConnection (connection_manager_p);
        if (!connection_2)
        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to retrieve connection #%d/%d, returning\n"),
//                      index_i, number_of_connections_i));
          goto continue_;
        } // end IF

        do_ping = true;

        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unknown/invalid mode (was: %d), returning\n"),
                    mode_));
        goto continue_;
      }
    } // end SWITCH
//  } // end lock scope

continue_:
//  connection_manager_p->unlock ();

  // -------------------------------------

  if (do_abort)
  { ACE_ASSERT (connection_p);
    try {
      connection_p->close ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::close(), returning\n")));
      connection_p->decrease (); connection_p = NULL;
      if (connection_2)
      {
        connection_2->decrease (); connection_2 = NULL;
      } // end IF
      return;
    }

    connection_p->decrease (); connection_p = NULL;
  } // end IF

  if (do_abort_oldest)
    connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);

  if (do_abort_youngest)
    connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_MOST);

  if (do_connect)
  {
    ACE_INET_Addr peer_address;
    Test_U_ITCPConnector_t* ssl_tcp_connector_p = NULL;
    Test_U_IUDPConnector_t* udp_connector_p = NULL;
    switch (protocolConfiguration_->transportLayer)
    {
      case NET_TRANSPORTLAYER_TCP:
      {
        peer_address = connectionConfiguration_->socketConfiguration.address;
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
        peer_address = UDPConnectionConfiguration_->socketConfiguration.peerAddress;
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
        peer_address = connectionConfiguration_->socketConfiguration.address;
#if defined (SSL_SUPPORT)
        ssl_tcp_connector_p = &SSLConnector_;
#endif // SSL_SUPPORT
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    protocolConfiguration_->transportLayer));
        if (connection_2)
        {
          connection_2->decrease (); connection_2 = NULL;
        } // end IF
        return;
      }
    } // end SWITCH

    ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
    switch (protocolConfiguration_->transportLayer)
    {
      case NET_TRANSPORTLAYER_TCP:
      case NET_TRANSPORTLAYER_SSL:
      { ACE_ASSERT (ssl_tcp_connector_p);
        try {
          handle_h = ssl_tcp_connector_p->connect (peer_address);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_IConnector_t::connect(%s), returning\n"),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
          if (connection_2)
          {
            connection_2->decrease (); connection_2 = NULL;
          } // end IF
          return;
        }
        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      { ACE_ASSERT (udp_connector_p);
        try {
          handle_h = udp_connector_p->connect (peer_address);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_IConnector_t::connect(%s), returning\n"),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
          if (connection_2)
          {
            connection_2->decrease (); connection_2 = NULL;
          } // end IF
          return;
        }
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    protocolConfiguration_->transportLayer));
        if (connection_2)
        {
          connection_2->decrease (); connection_2 = NULL;
        } // end IF
        return;
      }
    } // end SWITCH
    if (handle_h == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnector::connect(%s), returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
      if (connection_2)
      {
        connection_2->decrease (); connection_2 = NULL;
      } // end IF
      return;
    } // end IF
  } // end IF

  if (do_ping)
  { ACE_ASSERT (connection_2);
    Net_IPing* iping_p = dynamic_cast<Net_IPing*> (connection_2);
    if (!iping_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Net_IPing>(0x%@), returning\n"),
                  connection_2));
      connection_2->decrease (); connection_2 = NULL;
      return;
    } // end IF

    try {
      iping_p->ping ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IPing::ping(), returning\n")));
      connection_2->decrease (); connection_2 = NULL;
      return;
    }

    connection_2->decrease (); connection_2 = NULL;
  } // end IF
}
