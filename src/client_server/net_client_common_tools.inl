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

#include "ace/Time_Value.h"

#include "common_time_common.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename ConnectorType>
ACE_HANDLE
Net_Client_Common_Tools::connect (ConnectorType& connector_in,
                                  const typename ConnectorType::CONFIGURATION_T& configuration_in,
                                  const typename ConnectorType::USERDATA_T& userData_in,
                                  const typename ConnectorType::ADDRESS_T& address_in,
                                  bool wait_in,
                                  bool isPeerAddress_in,
                                  unsigned int retries_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Common_Tools::connect"));

  ACE_HANDLE result = ACE_INVALID_HANDLE;

  typename ConnectorType::CONNECTION_MANAGER_T::INTERFACE_T* iconnection_manager_p =
    ConnectorType::CONNECTION_MANAGER_T::SINGLETON_T::instance ();
  typename ConnectorType::CONNECTION_MANAGER_T::ICONNECTION_T* connection_p =
    NULL;
  typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_in;
  ACE_Time_Value deadline, timeout;
  enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
  typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;

  iconnection_manager_p->lock (true);

  // step1: initialize the connection manager
  iconnection_manager_p->set (configuration_in,
                              &const_cast<typename ConnectorType::USERDATA_T&> (userData_in));

  // step2: initialize the connector
  if (unlikely (!iconnector_p->initialize (configuration_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector, aborting\n")));
    iconnection_manager_p->unlock (false);
    return ACE_INVALID_HANDLE;
  } // end IF

  // step3: connect
retry:
  result = iconnector_p->connect (address_in);
  if (unlikely (result == ACE_INVALID_HANDLE))
  {
    if (retries_in--)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to connect to %s, retrying\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
      goto retry;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
    iconnection_manager_p->unlock (false);
    return ACE_INVALID_HANDLE;
  } // end IF
  if (unlikely (!wait_in))
  {
    iconnection_manager_p->unlock (false);
    return result;
  } // end IF

  if (unlikely (iconnector_p->useReactor ()))
  {
    connection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (result));
#else
      iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (result));
#endif // ACE_WIN32 || ACE_WIN64
    if (unlikely (!connection_p))
      goto continue_;
  } // end IF
  else
  {
    timeout.sec (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S);
    deadline = ACE_OS::gettimeofday () + timeout;

    // step1: wait for the connection to establish
    typename ConnectorType::IASYNCH_CONNECTOR_T* iasynch_connector_p =
      dynamic_cast<typename ConnectorType::IASYNCH_CONNECTOR_T*> (&connector_in);
    ACE_ASSERT (iasynch_connector_p);
    int result_2 = iasynch_connector_p->wait (result,
                                              timeout); // relative- !
    if (unlikely (result_2))
    {
      if (retries_in--)
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("failed to Net_IConnector::wait(%s): \"%s\", retrying\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ()),
                    ACE_OS::strerror (result_2)));
        iasynch_connector_p->abort ();
        goto retry;
      } // end IF
      ACE_DEBUG (((result_2 == ETIME ? LM_WARNING: LM_ERROR),
                  ACE_TEXT ("failed to Net_IConnector::wait(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ()),
                  ACE_OS::strerror (result_2)));
      iasynch_connector_p->abort ();
      iconnection_manager_p->unlock (false);
      return ACE_INVALID_HANDLE;
    } // end IF

    // step2: wait for the connection to register with the manager
    // *TODO*: avoid tight loop here
    do
    {
      connection_p = iconnection_manager_p->get (address_in,
                                                 isPeerAddress_in);
      if (likely (connection_p))
        break;
    } while (ACE_OS::gettimeofday () < deadline);
    if (unlikely (!connection_p))
      goto continue_;
  } // end ELSE

  timeout.sec (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT_S);
  deadline = ACE_OS::gettimeofday () + timeout;

  // step3: wait for the connection to finish initializing
  // *TODO*: avoid these tight loops
  do
  {
    status = connection_p->status ();
    if (status > NET_CONNECTION_STATUS_INITIALIZING)
      break;
  } while (ACE_OS::gettimeofday () < deadline);
  if (unlikely (status != NET_CONNECTION_STATUS_OK))
  {
    if (retries_in--)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to connect to %s, retrying\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
      connection_p->decrease (); connection_p = NULL;
      goto retry;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
    connection_p->decrease ();
    iconnection_manager_p->unlock (false);
    return ACE_INVALID_HANDLE;
  } // end IF

  // step4: wait for the connection stream (if any) to finish initializing
  istream_connection_p =
    dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_p);
  if (likely (istream_connection_p))
    istream_connection_p->wait (STREAM_STATE_RUNNING,
                                NULL); // <-- block

continue_:
  if (unlikely (!connection_p))
  {
    if (retries_in--)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to connect to %s, retrying\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
      goto retry;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
    iconnection_manager_p->unlock (false);
    return ACE_INVALID_HANDLE;
  } // end IF
  connection_p->decrease (); connection_p = NULL;
  iconnection_manager_p->unlock (false);

  return result;
}
