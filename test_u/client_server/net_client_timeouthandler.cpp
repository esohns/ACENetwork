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

#include "ace/Synch.h"
#include "net_client_timeouthandler.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "net_client_defines.h"

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_defines.h"
#include "test_u_message.h"
#include "test_u_sessionmessage.h"

Client_TimeoutHandler::Client_TimeoutHandler (enum ActionModeType mode_in,
                                              unsigned int maximumNumberOfConnections_in,
                                              Client_IConnector_t* connector_in)
 : inherited (this,  // dispatch interface
              false) // invoke only once ?
 , alternatingModeState_ (ALTERNATING_STATE_CONNECT)
 , connector_ (connector_in)
 , lock_ ()
 , maximumNumberOfConnections_ (maximumNumberOfConnections_in)
 , mode_ (mode_in)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , randomStateInitializationBuffer_ ()
 , randomState_ ()
#endif // ACE_WIN32 || ACE_WIN64
 , randomDistribution_ (1, 100)
 , randomEngine_ ()
 , randomGenerator_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Client_TimeoutHandler::Client_TimeoutHandler"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_OS::memset (randomStateInitializationBuffer_,
                  0,
                  sizeof (randomStateInitializationBuffer_));
  int result =
    ::initstate_r (Common_Tools::randomSeed_,
                   randomStateInitializationBuffer_, sizeof (randomStateInitializationBuffer_),
                   &randomState_);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::initstate_r(): \"%m\", continuing\n")));
  result = ::srandom_r (Common_Tools::randomSeed_, &randomState_);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::srandom_r(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
  randomGenerator_ = std::bind (randomDistribution_, randomEngine_);
}

void
Client_TimeoutHandler::mode (enum ActionModeType mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("Client_TimeoutHandler::mode"));

  ACE_ASSERT (mode_in < ACTION_MAX);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    mode_ = mode_in;
  } // end lock scope
}

enum Client_TimeoutHandler::ActionModeType
Client_TimeoutHandler::mode () const
{
  NETWORK_TRACE (ACE_TEXT ("Client_TimeoutHandler::mode"));

  // initialize return value(s)
  enum ActionModeType result = ACTION_INVALID;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, ACTION_INVALID);
    result = mode_;
  } // end lock scope

  return result;
}

void
Client_TimeoutHandler::handle (const void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Client_TimeoutHandler::handle"));

  ACE_UNUSED_ARG (arg_in);

  int result = -1;
  int index_i = 0;
  ClientServer_InetConnectionManager_t::CONNECTION_T* connection_p = NULL;
  ClientServer_InetConnectionManager_t::CONNECTION_T* connection_2 = NULL;
  bool do_abort = false;
  bool do_abort_oldest = false;
  bool do_abort_youngest = false;
  bool do_connect = false;
  bool do_ping = false;
  ClientServer_IInetConnectionManager_t* connection_manager_p =
    CLIENTSERVER_CONNECTIONMANAGER_SINGLETON::instance ();

  // sanity check(s)
  ACE_ASSERT (connector_);
  ACE_ASSERT (connection_manager_p);

  const Client_IConnector_t::CONFIGURATION_T& configuration_r =
    connector_->getR ();
  unsigned int number_of_connections_i = 0;

  connection_manager_p->lock ();
  number_of_connections_i = connection_manager_p->count ();
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    switch (mode_)
    {
      case ACTION_NORMAL:
      {
        if (!number_of_connections_i)
          goto continue_;

        // grab a (random) connection handler
        // *PORTABILITY*: outside glibc, this is not very portable
        // *TODO*: use STL random funcionality instead
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        index_i =
          (ACE_OS::rand_r (&Common_Tools::randomSeed_) %
           number_of_connections_i);
#else
        result = ::random_r (&randomState_, &index_i);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ::random_r(): \"%s\", returning\n")));
          goto continue_;
        } // end IF
        index_i = (index_i % number_of_connections_i);
#endif // ACE_WIN32 || ACE_WIN64
        connection_2 = connection_manager_p->operator[] (index_i);
        if (!connection_2)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection #%d/%d, returning\n"),
                      index_i, number_of_connections_i));
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

            // grab a (random) connection handler
            // *PORTABILITY*: outside glibc, this is not very portable
            // *TODO*: use STL funcionality instead
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            index_i =
              (ACE_OS::rand_r (&Common_Tools::randomSeed_) % number_of_connections_i);
#else
            result = ::random_r (&randomState_, &index_i);
            if (result == -1)
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ::random_r(): \"%s\", returning\n")));
              goto continue_;
            } // end IF
            index_i = (index_i % number_of_connections_i);
#endif // ACE_WIN32 || ACE_WIN64
            connection_p = connection_manager_p->operator[] (index_i);
            if (!connection_p)
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to retrieve connection #%d/%d, returning\n"),
                          index_i, number_of_connections_i));
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
        float probability_f = static_cast<float> (randomGenerator_ ()) / 100.0F;

        if ((number_of_connections_i > 0) &&
            (probability_f <= NET_CLIENT_U_TEST_ABORT_PROBABILITY))
          do_abort_youngest = true;

        // allow some probability_f for opening connections in between
        probability_f = static_cast<float> (randomGenerator_ ()) / 100.0F;
        if (probability_f <= NET_CLIENT_U_TEST_CONNECT_PROBABILITY)
          do_connect = true;

        // ping the server

        // sanity check
        if (!number_of_connections_i ||
            ((number_of_connections_i == 1) && do_abort_youngest))
          break;

        // grab a (random) connection handler
        // *PORTABILITY*: outside glibc, this is not very portable...
        // *TODO*: use STL funcionality instead
        //        std::uniform_int_distribution<int> distribution (0, number_of_connections_i - 1);
        //        index_i = distribution (randomGenerator_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        index_i =
          (ACE_OS::rand_r (&Common_Tools::randomSeed_) % number_of_connections_i);
#else
        result = ::random_r (&randomState_, &index_i);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ::random_r(): \"%s\", returning\n")));
          goto continue_;
        } // end IF
        index_i = (index_i % number_of_connections_i);
#endif // ACE_WIN32 || ACE_WIN64
        connection_2 = connection_manager_p->operator[] (index_i);
        if (!connection_2)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection #%d/%d, returning\n"),
                      index_i, number_of_connections_i));
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
  } // end lock scope

continue_:
  connection_manager_p->unlock ();

  // -------------------------------------

  if (do_abort)
  { ACE_ASSERT (connection_p);
    try {
      connection_p->close ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::close(), returning\n")));

      // clean up
      connection_p->decrease ();
      if (connection_2)
        connection_2->decrease ();

      return;
    }

    // clean up
    connection_p->decrease ();
    connection_p = NULL;
  } // end IF

  if (do_abort_oldest)
    connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST);

  if (do_abort_youngest)
    connection_manager_p->abort (NET_CONNECTION_ABORT_STRATEGY_RECENT_MOST);

  if (do_connect)
  {
    ACE_INET_Addr peer_address;
    switch (connector_->transportLayer ())
    {
      case NET_TRANSPORTLAYER_TCP:
        peer_address =
          configuration_r.socketHandlerConfiguration.socketConfiguration_2.address;
        break;
      case NET_TRANSPORTLAYER_UDP:
        peer_address =
          configuration_r.socketHandlerConfiguration.socketConfiguration_3.peerAddress;
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    connector_->transportLayer ()));

        // clean up
        if (connection_2)
          connection_2->decrease ();

        return;
      }
    } // end SWITCH

    ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
    ACE_ASSERT (connector_);
    try {
      handle_h = connector_->connect (peer_address);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnector_t::connect(%s), returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));

      // clean up
      if (connection_2)
        connection_2->decrease ();

      return;
    }
    if (handle_h == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnector::connect(%s), returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));

      // clean up
      if (connection_2)
        connection_2->decrease ();

      return;
    } // end IF
    ClientServer_InetConnectionManager_t::ICONNECTION_T* iconnection_p =
      NULL;
    if (connector_->useReactor ())
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_p =
        connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle_h));
#else
      iconnection_p =
        connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle_h));
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
    else
    {
      // step1: wait for the connection to register with the manager
      // *TODO*: avoid these tight loops
      ACE_Time_Value deadline =
        (COMMON_TIME_NOW +
         ACE_Time_Value (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0));
      // *TODO*: this may not be accurate/applicable for/to all protocols
      do
      {
        // *TODO*: avoid these tight loops
        iconnection_p = connection_manager_p->get (peer_address,
                                                   true);
        if (iconnection_p)
          break;
      } while (COMMON_TIME_NOW < deadline);
    } // end ELSE
    if (!iconnection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));

      // clean up
      if (connection_2)
        connection_2->decrease ();

      return;
    } // end IF
    iconnection_p->decrease ();
    iconnection_p = NULL;
  } // end IF

  if (do_ping)
  { ACE_ASSERT (connection_2);
    Net_IPing* iping_p = dynamic_cast<Net_IPing*> (connection_2);
    if (!iping_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Net_IPing>(0x%@), returning\n"),
                  connection_2));

      // clean up
      connection_2->decrease ();

      return;
    } // end IF

    try {
      iping_p->ping ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IPing::ping(), returning\n")));

      // clean up
      connection_2->decrease ();

      return;
    }

    // clean up
    connection_2->decrease ();
    connection_2 = NULL;
  } // end IF
}
