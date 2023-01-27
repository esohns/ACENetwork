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

  ConnectorType connector (true);
  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  //  typename ConnectorType::ICONNECTION_T* iconnection_p = NULL;
  struct Net_UserData user_data_s; // *TODO*: make this generic

  handle_h =
    Net_Client_Common_Tools::connect (connector,
                                      *static_cast<ConnectionConfigurationType*> (configuration_->connectionConfiguration),
                                      user_data_s,
                                      address_in,
                                      true,
                                      true);
  if (handle_h == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to connect to %s: \"%m\", returning\n"),
               ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    return;
  } // end IF
  //  iconnection_p =
  //#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //      CONNECTION_MANAGER_SINGLETON_T::instance ()->get (reinterpret_cast<Net_ConnectionId_t> (handle_h));
  //#else
  //      CONNECTION_MANAGER_SINGLETON_T::instance ()->get (static_cast<Net_ConnectionId_t> (handle_h));
  //#endif // ACE_WIN32 || ACE_WIN64
  //  if (!iconnection_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to connect to %s: \"%m\", returning\n"),
  //                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
  //    return;
  //  } // end IF

 //ACE_DEBUG ((LM_DEBUG,
 //            ACE_TEXT ("connected to %s: %u...\n"),
 //            ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
 //            iconnection_p->id ()));

  //  iconnection_p->decrease ();

  return;

//error:
//  if (iconnection_p)
//  {
//    iconnection_p->abort ();
//    iconnection_p->decrease (); iconnection_p = NULL;
//  } // end IF
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
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (peer address was: \"%s\") not found, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
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

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    for (Net_ConnectionIdsIterator_t iterator = state_.connections.begin ();
         iterator != state_.connections.end ();
         ++iterator)
    {
      iconnection_p =connection_manager_p->get (*iterator);
      if (unlikely (!iconnection_p)) // mostly likely: different connection manager
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve connection handle (id was: %d), continuing\n"),
                    *iterator));
        continue;
      } // end IF
      iconnection_p->abort ();
      iconnection_p->decrease (); iconnection_p = NULL;

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("aborted connection (id was: %d)...\n"),
                  *iterator));
    } // end FOR

    if (waitForCompletion_in)
    {
      int result = -1;
      while (!state_.connections.empty ())
      {
        result = condition_.wait ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Condition_Thread_Mutex::wait(): \"%m\", continuing\n")));
      } // end WHILE
    } // end IF
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

  int result = -1;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    while (!state_.connections.empty ())
    {
      result = condition_.wait ();
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
  //            ACE_TEXT ("new connection (id: %d)...\n"),
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

  Net_ConnectionIdsIterator_t iterator;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    iterator = state_.connections.find (id_in);
    ACE_ASSERT (iterator != state_.connections.end ());
    state_.connections.erase (iterator);

    if (state_.connections.empty ())
    {
      int result = condition_.broadcast ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition_Thread_Mutex::broadcast(): \"%m\", continuing\n")));
    } // end IF
  } // end lock scope

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("connection lost (id was: %d)...\n"),
  //            id_in));
}
