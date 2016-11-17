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

#include <ace/Synch.h>
#include "net_client_timeouthandler.h"

#include <ace/Log_Msg.h>

#include "common_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_defines.h"

Test_U_Client_TimeoutHandler::Test_U_Client_TimeoutHandler (ActionMode_t mode_in,
                                                            unsigned int maximumNumberOfConnections_in,
                                                            const ACE_INET_Addr& remoteSAP_in,
                                                            Test_U_IConnector_t* connector_in)
 : inherited (NULL,                           // default reactor
              ACE_Event_Handler::LO_PRIORITY) // priority
 , alternatingModeState_ (ALTERNATING_STATE_CONNECT)
 , connector_ (connector_in)
 , lock_ ()
 , maximumNumberOfConnections_ (maximumNumberOfConnections_in)
 , mode_ (mode_in)
 , peerAddress_ (remoteSAP_in)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , randomStateInitializationBuffer_ ()
 , randomState_ ()
#endif
 , randomDistribution_ (1, 100)
 , randomEngine_ ()
 , randomGenerator_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Client_TimeoutHandler::Test_U_Client_TimeoutHandler"));

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
#endif
  randomGenerator_ = std::bind (randomDistribution_, randomEngine_);
}

Test_U_Client_TimeoutHandler::~Test_U_Client_TimeoutHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Client_TimeoutHandler::~Test_U_Client_TimeoutHandler"));

}

void
Test_U_Client_TimeoutHandler::mode (ActionMode_t mode_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Client_TimeoutHandler::mode"));

  ACE_ASSERT (mode_in < ACTION_MAX);

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    mode_ = mode_in;
  } // end lock scope
}

Test_U_Client_TimeoutHandler::ActionMode_t
Test_U_Client_TimeoutHandler::mode () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Client_TimeoutHandler::mode"));

  // initialize return value(s)
  ActionMode_t result = ACTION_INVALID;

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    result = mode_;
  } // end lock scope

  return result;
}

int
Test_U_Client_TimeoutHandler::handle_timeout (const ACE_Time_Value& tv_in,
                                              const void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Client_TimeoutHandler::handle_timeout"));

  int result = -1;

  ACE_UNUSED_ARG (tv_in);
  ACE_UNUSED_ARG (arg_in);

  int index = 0;
  Test_U_InetConnectionManager_t::CONNECTION_T* abort_connection_p = NULL;
  Test_U_InetConnectionManager_t::CONNECTION_T* ping_connection_p = NULL;
  bool do_abort = false;
  bool do_abort_oldest = false;
  bool do_abort_youngest = false;
  bool do_connect = false;
  bool do_ping = false;
  Test_U_IInetConnectionManager_t* connection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->lock ();
  unsigned int number_of_connections = connection_manager_p->count ();

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    switch (mode_)
    {
      case ACTION_NORMAL:
      {
        if (number_of_connections == 0)
        {
          connection_manager_p->unlock ();
          return 0;
        } // end IF

        // grab a (random) connection handler
        // *PORTABILITY*: outside glibc, this is not very portable...
        // *TODO*: use STL funcionality instead
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        index =
          (ACE_OS::rand_r (&Common_Tools::randomSeed_) % number_of_connections);
#else
        result = ::random_r (&randomState_, &index);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ::random_r(): \"%s\", aborting\n")));

          // clean up
          connection_manager_p->unlock ();

          return -1;
        } // end IF
        index = (index % number_of_connections);
#endif
        ping_connection_p = connection_manager_p->operator[] (index);
        if (!ping_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection #%d/%d, aborting\n"),
                      index, number_of_connections));

          // clean up
          connection_manager_p->unlock ();

          return -1;
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
                (number_of_connections >= maximumNumberOfConnections_))
              do_abort_oldest = true;

            do_connect = true;

            break;
          }
          case ALTERNATING_STATE_ABORT:
          {
            // sanity check
            if (number_of_connections == 0)
              break; // nothing to do...

            // grab a (random) connection handler
            // *PORTABILITY*: outside glibc, this is not very portable...
            // *TODO*: use STL funcionality instead
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            index =
              (ACE_OS::rand_r (&Common_Tools::randomSeed_) % number_of_connections);
#else
            result = ::random_r (&randomState_, &index);
            if (result == -1)
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ::random_r(): \"%s\", aborting\n")));

              // clean up
              connection_manager_p->unlock ();

              return -1;
            } // end IF
            index = (index % number_of_connections);
#endif
            abort_connection_p = connection_manager_p->operator[] (index);
            if (!abort_connection_p)
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to retrieve connection #%d/%d, aborting\n"),
                          index, number_of_connections));

              // clean up
              connection_manager_p->unlock ();

              return -1;
            } // end IF

            do_abort = true;

            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("unknown/invalid alternating state (was: %d), aborting\n"),
                        alternatingModeState_));

            // clean up
            connection_manager_p->unlock ();

            return -1;
          }
        } // end SWITCH

        // cycle mode
        int temp = alternatingModeState_;
        alternatingModeState_ =
          static_cast<Test_U_Client_TimeoutHandler::AlternatingModeState_t> (++temp);
        if (alternatingModeState_ == ALTERNATING_STATE_MAX)
          alternatingModeState_ = ALTERNATING_STATE_CONNECT;

        break;
      }
      case ACTION_STRESS:
      {
        // allow some probability for closing connections in between
        float probability = static_cast<float> (randomGenerator_ ()) / 100.0F;

        if ((number_of_connections > 0) &&
            (probability <= NET_CLIENT_U_TEST_ABORT_PROBABILITY))
          do_abort_youngest = true;

        // allow some probability for opening connections in between
        probability = static_cast<float> (randomGenerator_ ()) / 100.0F;
        if (probability <= NET_CLIENT_U_TEST_CONNECT_PROBABILITY)
          do_connect = true;

        // ping the server

        // sanity check
        if ((number_of_connections == 0) ||
            ((number_of_connections == 1) && do_abort_youngest))
          break;

        // grab a (random) connection handler
        // *PORTABILITY*: outside glibc, this is not very portable...
        // *TODO*: use STL funcionality instead
        //        std::uniform_int_distribution<int> distribution (0, number_of_connections - 1);
        //        index = distribution (randomGenerator_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        index =
          (ACE_OS::rand_r (&Common_Tools::randomSeed_) % number_of_connections);
#else
        result = ::random_r (&randomState_, &index);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ::random_r(): \"%s\", aborting\n")));

          // clean up
          TEST_U_CONNECTIONMANAGER_SINGLETON::instance ()->unlock ();

          return -1;
        } // end IF
        index = (index % number_of_connections);
#endif
        ping_connection_p = connection_manager_p->operator[] (index);
        if (!ping_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection #%d/%d, aborting\n"),
                      index, number_of_connections));

          // clean up
          connection_manager_p->unlock ();

          return -1;
        } // end IF

        do_ping = true;

        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unknown/invalid mode (was: %d), aborting\n"),
                    mode_));

        // clean up
        connection_manager_p->unlock ();

        return -1;
      }
    } // end SWITCH
  } // end lock scope
  connection_manager_p->unlock ();

  // -------------------------------------

  if (do_abort)
  {
    ACE_ASSERT (abort_connection_p);
    try {
      abort_connection_p->close ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::close(), aborting\n")));

      // clean up
      abort_connection_p->decrease ();
      if (ping_connection_p)
        ping_connection_p->decrease ();

      return -1;
    }

    // clean up
    abort_connection_p->decrease ();
  } // end IF

  if (do_abort_oldest)
    connection_manager_p->abortLeastRecent ();

  if (do_abort_youngest)
    connection_manager_p->abortMostRecent ();

  if (do_connect)
  {
    ACE_ASSERT (connector_);
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    try {
      handle = connector_->connect (peerAddress_);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Client_IConnector_t::connect(), aborting\n")));

      // clean up
      if (ping_connection_p)
        ping_connection_p->decrease ();

      return -1;
    }
    if (handle == ACE_INVALID_HANDLE)
    {
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      int result_3 = peerAddress_.addr_to_string (buffer, sizeof (buffer));
      if (result_3 == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Client_IConnector::connect(\"%s\"), continuing\n"),
                  buffer));
    } // end IF
  } // end IF

  if (do_ping)
  {
    ACE_ASSERT (ping_connection_p);
    Net_ITransportLayer_t* transportlayer_p =
      dynamic_cast<Net_ITransportLayer_t*> (ping_connection_p);
    if (!transportlayer_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Net_ITransportLayer_t*> (0x%@), aborting\n"),
                  ping_connection_p));

      // clean up
      ping_connection_p->decrease ();

      return -1;
    } // end IF

    try {
      transportlayer_p->ping ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_ITransportLayer_t::ping(), aborting\n")));

      // clean up
      ping_connection_p->decrease ();

      return -1;
    }

    // clean up
    ping_connection_p->decrease ();
  } // end IF

  return 0;
}
