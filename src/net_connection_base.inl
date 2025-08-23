/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#include "common_time_common.h"

#include "net_common_tools.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_ConnectionBase_T<ACE_SYNCH_USE,
                     AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     UserDataType>::Net_ConnectionBase_T (bool managed_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , configuration_ (NULL)
 , connector_ (NULL)
 , listener_ (NULL)
 , state_ ()
 , isManaged_ (managed_in)
 , isRegistered_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

  // initialize configuration/user data ?
  if (likely (isManaged_))
  {
    typename CONNECTION_MANAGER_T::INTERFACE_T* manager_p =
        CONNECTION_MANAGER_T::SINGLETON_T::instance ();
    ACE_ASSERT (manager_p);
    try {
      // get (default) configuration/user data from the connection manager
      // *TODO*: remove type inference
      manager_p->get (configuration_,
                      state_.userData);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::get(), continuing\n")));
    }
    ACE_ASSERT (configuration_);
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_ConnectionBase_T<ACE_SYNCH_USE,
                     AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     UserDataType>::update (const ACE_Time_Value& interval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::update"));

  ACE_UNUSED_ARG (interval_in);

  state_.statistic.previousBytes = state_.statistic.sentBytes +
                                   state_.statistic.receivedBytes;
  state_.statistic.previousTimeStamp = state_.statistic.timeStamp;
  state_.statistic.timeStamp = COMMON_TIME_NOW;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_ConnectionBase_T<ACE_SYNCH_USE,
                     AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     UserDataType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_ConnectionBase_T<ACE_SYNCH_USE,
                     AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     UserDataType>::register_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::register_"));

  // sanity check(s)
  ACE_ASSERT (!isRegistered_);

  // (try to) register with the connection manager
  typename CONNECTION_MANAGER_T::INTERFACE_T* manager_p =
      CONNECTION_MANAGER_T::SINGLETON_T::instance ();
  ACE_ASSERT (manager_p);
  try {
    isRegistered_ = manager_p->register_ (this);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnectionManager_T::registerc(), continuing\n")));
  }
  if (unlikely (!isRegistered_))
  {
    // *NOTE*: most probable reason: maximum number of connections has been
    //         reached, or currently not accepting new connections
    return false;
  } // end IF

  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  AddressType local_address, remote_address;
  try {
    this->info (handle_h,
                local_address, remote_address);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection_T::info(), continuing\n")));
  }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [0x%@/%d]: %s %s %s (total: %d)\n"),
              this, reinterpret_cast<int> (handle_h),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address, false, false).c_str ()),
              (remote_address.is_any () ? ACE_TEXT ("<--") : (local_address.is_any () ? ACE_TEXT ("-->") : ACE_TEXT ("<-->"))),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remote_address, false, false).c_str ()),
              manager_p->count ()));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [%@/%d]: %s %s %s (total: %d)\n"),
              this, handle_h,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address, false, false).c_str ()),
              (remote_address.is_any () ? ACE_TEXT ("<--") : (local_address.is_any () ? ACE_TEXT ("-->") : ACE_TEXT ("<-->"))),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remote_address, false, false).c_str ()),
              manager_p->count ()));
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_ConnectionBase_T<ACE_SYNCH_USE,
                     AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     UserDataType>::deregister ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::deregister"));

  // sanity check(s)
  if (unlikely (!isRegistered_))
    return;

  typename CONNECTION_MANAGER_T::INTERFACE_T* manager_p =
    CONNECTION_MANAGER_T::SINGLETON_T::instance ();
  ACE_ASSERT (manager_p);

  // (try to) de-register with the connection manager
  isRegistered_ = false;
  // *IMPORTANT NOTE*: may delete 'this'
  try {
    manager_p->deregister (this);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnectionManager_T::deregister(), continuing\n")));
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [0x%@/%d] (total: %u)\n"),
              this, reinterpret_cast<int> (state_.handle),
              manager_p->count ()));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [%@/%d] (total: %d)\n"),
              this, state_.handle,
              manager_p->count ()));
#endif // ACE_WIN32 || ACE_WIN64

  if (connector_ || listener_)
  {
    // notify the (UDP-) connector (server-side only !)
    if (connector_)
      connector_->disconnect (state_.handle);

    // notify the listener (server-side only !)
    if (listener_)
      listener_->disconnect (state_.handle);
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
ACE_Message_Block*
Net_ConnectionBase_T<ACE_SYNCH_USE,
                     AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

//  if (inherited::configuration_->messageAllocator)
  if (likely (configuration_->messageAllocator))
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (configuration_->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (unlikely (!message_block_p &&
                  !configuration_->messageAllocator->block ()))
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      ACE_Message_Block (requestedSize_in,
                                         ACE_Message_Block::MB_DATA,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                         ACE_Time_Value::zero,
                                         ACE_Time_Value::max_time,
                                         NULL,
                                         NULL));
  if (unlikely (!message_block_p))
  {
    if (configuration_->messageAllocator)
    {
      if (configuration_->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}
