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

template <typename AddressType,
          typename ConnectorType,
          typename ConnectionConfigurationType,
          typename UserDataType,
          typename ConnectionManagerType>
ACE_HANDLE
Net_Client_Common_Tools::connect (ConnectorType& connector_in,
                                  const ConnectionConfigurationType& configuration_in,
                                  const UserDataType& userData_in,
                                  const AddressType& address_in,
                                  bool wait_in,
                                  bool isPeerAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Common_Tools::connect"));

  ACE_HANDLE result = ACE_INVALID_HANDLE;

  typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
    ConnectionManagerType::SINGLETON_T::instance ();
  typename ConnectionManagerType::ICONNECTION_T* connection_p = NULL;

  // step1: initialize the connection manager
  iconnection_manager_p->set (configuration_in,
                              &const_cast<UserDataType&> (userData_in));

  // step2: initialize the connector
  if (unlikely (!connector_in.initialize (configuration_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector, aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF

  // step3: connect
  result = connector_in.connect (address_in);
  if (unlikely (result == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    return ACE_INVALID_HANDLE;
  } // end IF
  if (unlikely (!wait_in))
    return result;

  if (unlikely (connector_in.useReactor ()))
  {
    connection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (result));
#else
      iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (result));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  else
  {
    ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S,
                            0);
    ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
    enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
    typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;

    // step1: wait for the connection to establish
    typename ConnectorType::IASYNCH_CONNECTOR_T* iasynch_connector_p =
      dynamic_cast<typename ConnectorType::IASYNCH_CONNECTOR_T*> (&connector_in);
    ACE_ASSERT (iasynch_connector_p);
    int result_2 = iasynch_connector_p->wait (result,
                                              timeout); // relative- !
    if (unlikely (result_2))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnector::connect(%s): \"%s\" aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
                  ACE::sock_error (result_2)));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnector::connect(%s): \"%s\" aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
                  ACE_TEXT (ACE_OS::strerror (result_2))));
#endif // ACE_WIN32 || ACE_WIN64
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
    } while (COMMON_TIME_NOW < deadline);
    if (unlikely (!connection_p))
      goto continue_;

    // step3: wait for the connection to finish initializing
    // *TODO*: avoid these tight loops
    do
    {
      status = connection_p->status ();
      // *TODO*: break early upon failure too
      if (status == NET_CONNECTION_STATUS_OK)
        break;
    } while (COMMON_TIME_NOW < deadline);
    if (unlikely (status != NET_CONNECTION_STATUS_OK))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
      connection_p->decrease ();
      return ACE_INVALID_HANDLE;
    } // end IF

    // step4: wait for the connection stream (if any) to finish initializing
    istream_connection_p =
      dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_p);
    if (likely (istream_connection_p))
      istream_connection_p->wait (STREAM_STATE_RUNNING,
                                  NULL); // <-- block
  } // end ELSE
continue_:
  if (unlikely (!connection_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    return ACE_INVALID_HANDLE;
  } // end IF
  connection_p->decrease (); connection_p = NULL;

  return result;
}
