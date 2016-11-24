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

#include <ace/Log_Msg.h>

#include "net_defines.h"
#include "net_macros.h"

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::Net_SessionBase_T ()
 : configuration_ (NULL)
 , connectionManager_ (NULL)
 , handlerConfiguration_ (NULL)
 , isAsynch_ (!NET_EVENT_USE_REACTOR)
 , lock_ ()
 , condition_ (lock_)
 , state_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::Net_SessionBase_T"));

}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::~Net_SessionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::~Net_SessionBase_T"));

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    // sanity check(s)
    ACE_ASSERT (connectionManager_);
    // *TODO*: remove type inference
    if (!state_.connections.empty ())
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%u remaining connection(s) in session dtor, continuing\n"),
                  state_.connections.size ()));

    typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;
    for (Net_ConnectionsIterator_t iterator = state_.connections.begin ();
         iterator != state_.connections.end ();
         ++iterator)
    {
      iconnection_p = connectionManager_->get (*iterator);
      if (!iconnection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve connection handle (id was: %d), continuing\n"),
                    *iterator));
        continue;
      } // end IF
      iconnection_p->close ();

      // clean up
      iconnection_p->decrease ();
      iconnection_p = NULL;
    } // end FOR
  } // end lock scope
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
bool
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  // *TODO*: remove type inferences
  connectionManager_ = configuration_in.connectionManager;
  handlerConfiguration_ =
      const_cast<ConfigurationType&> (configuration_in).socketHandlerConfiguration;
  isAsynch_ = !configuration_in.useReactor;

  return true;
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
void
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::connect"));

  ConnectorType connector (connectionManager_,
                           ACE_Time_Value::zero);
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // debug info
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = address_in.addr_to_string (buffer,
                                          sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  // step1: initialize connector
  typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector;
  ACE_ASSERT (handlerConfiguration_);
  if (!iconnector_p->initialize (*handlerConfiguration_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
    return;
  } // end IF

  ACE_Time_Value deadline;
  typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;

  // step2: try to connect
  handle = iconnector_p->connect (address_in);
  if (handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                buffer));
    return;
  } // end IF
  if (isAsynch_)
  {
    deadline =
        (COMMON_TIME_NOW +
         ACE_Time_Value (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT, 0));
    ACE_Time_Value delay (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_INTERVAL,
                          0);
    do
    {
      result = ACE_OS::sleep (delay);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &delay));

      // *TODO*: this does not work...
      iconnection_p = connectionManager_->get (address_in);
      if (iconnection_p)
        break; // done
    } while (COMMON_TIME_NOW < deadline);
  } // end ELSE
  else
    iconnection_p = connectionManager_->get (handle);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                buffer));
    return;
  } // end IF

  // step3a: wait for the connection to finish initializing
  // *TODO*: avoid tight loop here
  ACE_Time_Value initialization_timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT,
                                         0);
  deadline = COMMON_TIME_NOW + initialization_timeout;
  Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
  typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
  do
  {
    status = iconnection_p->status ();
    if (status == NET_CONNECTION_STATUS_OK) break;
  } while (COMMON_TIME_NOW < deadline);
  if (status != NET_CONNECTION_STATUS_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (to: \"%s\") failed to initialize (status was: %d), returning\n"),
                buffer,
                status));
    goto error;
  } // end IF
  // step3b: wait for the connection stream to finish initializing
  istream_connection_p =
    dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (iconnection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    goto error;
  } // end IF
  istream_connection_p->wait (STREAM_STATE_RUNNING,
                              NULL); // <-- block

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected to \"%s\": %d...\n"),
              buffer,
              iconnection_p->id ()));

  iconnection_p->decrease ();

  return;

error:
  if (iconnection_p)
  {
    iconnection_p->close ();
    iconnection_p->decrease ();
  } // end IF
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
void
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::disconnect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::disconnect"));

  // sanity check(s)
  ACE_ASSERT (connectionManager_);

  // debug info
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = address_in.addr_to_string (buffer,
                                          sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  ConnectionType* connection_p = connectionManager_->get (address_in);
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (peer was: \"%s\") not found, returning\n"),
                buffer));
    return;
  } // end IF

  connection_p->close ();
  connection_p->decrease ();
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
void
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::close (bool waitForCompletion_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::close"));

  int result = -1;

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    // sanity check(s)
    ACE_ASSERT (connectionManager_);

    typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;
    for (Net_ConnectionsIterator_t iterator = state_.connections.begin ();
         iterator != state_.connections.end ();
         ++iterator)
    {
      iconnection_p = connectionManager_->get (*iterator);
      if (!iconnection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve connection handle (id was: %d), continuing\n"),
                    *iterator));
        continue;
      } // end IF
      iconnection_p->close ();

      // clean up
      iconnection_p->decrease ();
      iconnection_p = NULL;
    } // end FOR

    if (waitForCompletion_in)
    {
      result = condition_.wait ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_CONDITION::wait(): \"%m\", continuing\n")));
      ACE_ASSERT (state_.connections.empty ());
    } // end IF
  } // end lock scope
}

//////////////////////////////////////////

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
void
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::connect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::connect"));

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    std::pair<Net_ConnectionsIterator_t, bool> result =
        state_.connections.insert (id_in);
    ACE_ASSERT (result.second);
  } // end lock scope

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("new connection (id was: %d)...\n"),
              id_in));
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType>
void
Net_SessionBase_T<AddressType,
                  ConnectionConfigurationType,
                  ConnectionStateType,
                  StatisticContainerType,
                  SocketConfigurationType,
                  HandlerConfigurationType,
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::disconnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::disconnect"));

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    Net_ConnectionsIterator_t iterator = state_.connections.find (id_in);
    ACE_ASSERT (iterator != state_.connections.end ());

    state_.connections.erase (iterator);
  } // end lock scope

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection lost (id was: %d)...\n"),
              id_in));
}
