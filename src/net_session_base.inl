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

#include "ace/Log_Msg.h"

#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_defines.h"
#include "net_macros.h"

#include "net_client_common_tools.h"

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::Net_SessionBase_T ()
 : configuration_ (NULL)
 , isAsynch_ (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_PROACTOR)
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::~Net_SessionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::~Net_SessionBase_T"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    // *TODO*: remove type inference
    if (!state_.connections.empty ())
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%u remaining connection(s) in session dtor, continuing\n"),
                  state_.connections.size ()));

    typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;
    for (Net_ConnectionIdsIterator_t iterator = state_.connections.begin ();
         iterator != state_.connections.end ();
         ++iterator)
    {
      iconnection_p =
          CONNECTION_MANAGER_SINGLETON_T::instance ()->get (*iterator);
      if (!iconnection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve connection handle (id was: %d), continuing\n"),
                    *iterator));
        continue;
      } // end IF
      iconnection_p->abort ();
      iconnection_p->decrease (); iconnection_p = NULL;
    } // end FOR
  } // end lock scope
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
  isAsynch_ = (configuration_in.dispatch == COMMON_EVENT_DISPATCH_PROACTOR);

  return true;
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::connect"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->connectionConfiguration);

  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  ConnectorType connector (true);

  // *NOTE*: this memory is cleaned up when the session closes (see also:
  //         Net_SessionBase_T::close())
  typename ConnectionConfigurationType::STREAM_CONFIGURATION_T* stream_configuration_p =
    NULL;
  ACE_NEW_NORETURN (stream_configuration_p,
                    typename ConnectionConfigurationType::STREAM_CONFIGURATION_T ());
  ACE_ASSERT (stream_configuration_p);
  *stream_configuration_p =
    *static_cast<ConnectionConfigurationType*> (configuration_->connectionConfiguration)->streamConfiguration;

  typename ConnectionConfigurationType::STREAM_CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T* module_handler_configuration_p =
    NULL;
  for (typename ConnectionConfigurationType::STREAM_CONFIGURATION_T::ITERATOR_T iterator = stream_configuration_p->begin ();
       iterator != stream_configuration_p->end ();
       ++iterator)
  {
    ACE_NEW_NORETURN (module_handler_configuration_p,
                      typename ConnectionConfigurationType::STREAM_CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T ());
    ACE_ASSERT (module_handler_configuration_p);
    *module_handler_configuration_p = *(*iterator).second.second;
    (*iterator).second.second = module_handler_configuration_p;
  } // end FOR

  ConnectionConfigurationType* connection_configuration_p = NULL;
  ACE_NEW_NORETURN (connection_configuration_p,
                    ConnectionConfigurationType ());
  ACE_ASSERT (connection_configuration_p);
  *connection_configuration_p =
    *static_cast<ConnectionConfigurationType*> (configuration_->connectionConfiguration);
  connection_configuration_p->streamConfiguration = stream_configuration_p;

  struct Net_UserData user_data_s; // *TODO*: make this generic
  typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;
  Net_ConnectionId_t id_i;
  Net_SessionConnectionConfigurationsIterator_t iterator;
  std::pair<Net_SessionConnectionConfigurationsIterator_t, bool> result_s;

  handle_h =
    Net_Client_Common_Tools::connect (connector,
                                      *connection_configuration_p,
                                      user_data_s,
                                      address_in,
                                      true,
                                      true);
  if (handle_h == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to connect to %s: \"%m\", returning\n"),
               ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    goto error;
  } // end IF
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    CONNECTION_MANAGER_SINGLETON_T::instance ()->get (reinterpret_cast<Net_ConnectionId_t> (handle_h));
#else
    CONNECTION_MANAGER_SINGLETON_T::instance ()->get (static_cast<Net_ConnectionId_t> (handle_h));
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!iconnection_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s: \"%m\", returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
    goto error;
  } // end IF
  id_i = iconnection_p->id ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%u: connected to %s...\n"),
              id_i,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));

  // clean up ?
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    iterator = configuration_->connectionConfigurations.find (id_i);
    if (unlikely (iterator != configuration_->connectionConfigurations.end ()))
    {
      ConnectionConfigurationType* connection_configuration_2 =
        static_cast<ConnectionConfigurationType*> ((*iterator).second);
      ACE_ASSERT (connection_configuration_2->streamConfiguration);
      for (typename ConnectionConfigurationType::STREAM_CONFIGURATION_T::ITERATOR_T iterator_2 = connection_configuration_2->streamConfiguration->begin ();
           iterator_2 != connection_configuration_2->streamConfiguration->end ();
           ++iterator_2)
        delete (*iterator_2).second.second;
      delete connection_configuration_2->streamConfiguration;
      delete connection_configuration_2;
      configuration_->connectionConfigurations.erase (iterator);
    } // end IF

    result_s =
      configuration_->connectionConfigurations.insert (std::make_pair (id_i,
                                                                       connection_configuration_p));
    ACE_ASSERT (result_s.second);
  } // end lock scope

  iconnection_p->decrease (); iconnection_p = NULL;

  return;

error:
  if (iconnection_p)
  {
    iconnection_p->abort ();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF
  if (stream_configuration_p)
    for (typename ConnectionConfigurationType::STREAM_CONFIGURATION_T::ITERATOR_T iterator = stream_configuration_p->begin ();
         iterator != stream_configuration_p->end ();
         ++iterator)
      delete (*iterator).second.second;
  delete stream_configuration_p;
  delete connection_configuration_p;
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::disconnect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::disconnect"));

  ConnectionType* connection_p =
      CONNECTION_MANAGER_SINGLETON_T::instance ()->get (address_in);
  if (unlikely (!connection_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (peer address was: \"%s\") not found, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
    return;
  } // end IF
  connection_p->abort ();
  connection_p->decrease (); connection_p = NULL;
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::close (bool waitForCompletion_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::close"));

  // sanity check(s)
  ConnectionManagerType* connection_manager_p =
      CONNECTION_MANAGER_SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);

  typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;
  Net_ConnectionIds_t connection_ids_a;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    connection_ids_a = state_.connections;
    for (Net_ConnectionIdsIterator_t iterator = state_.connections.begin ();
         iterator != state_.connections.end ();
         ++iterator)
    {
      iconnection_p = connection_manager_p->get (*iterator);
      if (unlikely (!iconnection_p)) // mostly likely: different connection manager
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve connection handle (id was: %u), continuing\n"),
                    *iterator));
        continue;
      } // end IF
      iconnection_p->abort ();
      iconnection_p->decrease (); iconnection_p = NULL;

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("aborted connection (id was: %u)...\n"),
                  *iterator));
    } // end FOR

    if (waitForCompletion_in)
    {
      // step1: wait for connections to leave
      while (!state_.connections.empty ())
      {
        int result = condition_.wait ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Condition_Thread_Mutex::wait(): \"%m\", continuing\n")));
      } // end WHILE
    } // end IF
  } // end lock scope

  if (waitForCompletion_in)
  {
    // step2: now wait for connections to be gone entirely
    // *NOTE*: this step is needed to safely release all connection/stream
    //         configurations that are still around (see step3)
    for (Net_ConnectionIdsIterator_t iterator = connection_ids_a.begin ();
          iterator != connection_ids_a.end ();
          ++iterator)
      connection_manager_p->wait_2 (*iterator);

    // step3: clean up connection configurations (see connect()/disconnect())
    for (Net_SessionConnectionConfigurationsIterator_t iterator = configuration_->connectionConfigurations.begin ();
         iterator != configuration_->connectionConfigurations.end ();
         ++iterator)
    {
      ConnectionConfigurationType* connection_configuration_p =
        static_cast<ConnectionConfigurationType*> ((*iterator).second);
      ACE_ASSERT (connection_configuration_p->streamConfiguration);
      for (typename ConnectionConfigurationType::STREAM_CONFIGURATION_T::ITERATOR_T iterator_2 = connection_configuration_p->streamConfiguration->begin ();
            iterator_2 != connection_configuration_p->streamConfiguration->end ();
            ++iterator_2)
        delete (*iterator_2).second.second;
      delete connection_configuration_p->streamConfiguration;
      delete connection_configuration_p;
    } // end FOR
    configuration_->connectionConfigurations.clear ();
  } // end IF
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::wait ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::wait"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    while (!state_.connections.empty ())
    {
      int result = condition_.wait ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition_Thread_Mutex::wait(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope
}

//////////////////////////////////////////

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::connect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::connect"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    std::pair<Net_ConnectionIdsIterator_t, bool> result =
      state_.connections.insert (id_in);
    ACE_ASSERT (result.second);
  } // end lock scope

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("new connection (id: %u)...\n"),
  //            id_in));
}

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
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
                  ConnectionType,
                  ConnectionManagerType,
                  ConnectorType,
                  ConfigurationType,
                  StateType,
                  SessionInterfaceType>::disconnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SessionBase_T::disconnect"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  // step1: clean up connection configuration
  // *WARNING*: potential leakage here --> do it in the close(), after all (!)
  //            session connections have deregistered
  //Net_SessionConnectionConfigurationsIterator_t iterator =
  //  configuration_->connectionConfigurations.find (id_in);
  //ACE_ASSERT (iterator != configuration_->connectionConfigurations.end ());
  //ConnectionConfigurationType* connection_configuration_p =
  //  static_cast<ConnectionConfigurationType*> ((*iterator).second);
  //ACE_ASSERT (connection_configuration_p->streamConfiguration);
  //for (typename ConnectionConfigurationType::STREAM_CONFIGURATION_T::ITERATOR_T iterator_2 = connection_configuration_p->streamConfiguration->begin ();
  //     iterator_2 != connection_configuration_p->streamConfiguration->end ();
  //      ++iterator_2)
  //  delete (*iterator_2).second.second;
  //delete connection_configuration_p->streamConfiguration;
  //delete connection_configuration_p;
  //configuration_->connectionConfigurations.erase (iterator);

  // step2: remove connection from state
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    Net_ConnectionIdsIterator_t iterator_2 = state_.connections.find (id_in);
    ACE_ASSERT (iterator_2 != state_.connections.end ());
    state_.connections.erase (iterator_2);

    if (state_.connections.empty ())
    {
      int result = condition_.broadcast ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition_Thread_Mutex::broadcast(): \"%m\", continuing\n")));
    } // end IF
  } // end lock scope

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("connection lost (id was: %u)...\n"),
  //            id_in));
}
