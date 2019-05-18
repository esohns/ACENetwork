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

#include "net_common_tools.h"
#include "net_macros.h"

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     TimerManagerType,
                     UserDataType>::Net_ConnectionBase_T (bool managed_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , configuration_ (NULL)
 , state_ ()
 , isManaged_ (managed_in)
 , isRegistered_ (false)
 , statisticHandler_ (COMMON_STATISTIC_ACTION_COLLECT,
                      this,
                      true)
 , timerId_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

  // initialize configuration/user data
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

    // *TODO*: this must go into initialize () as well (un'managed' case)
    if (configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
    {
      // schedule regular statistic collection
      typename TimerManagerType::INTERFACE_T* itimer_manager_p =
          (configuration_->timerManager ? configuration_->timerManager
                                        : TimerManagerType::SINGLETON_T::instance ());
      ACE_ASSERT (itimer_manager_p);
      timerId_ =
        itimer_manager_p->schedule_timer (&statisticHandler_,                                            // event handler
                                          NULL,                                                          // asynchronous completion token
                                          COMMON_TIME_NOW + configuration_->statisticCollectionInterval, // first wakeup time
                                          configuration_->statisticCollectionInterval);                  // interval
      if (unlikely (timerId_ == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_ITimer::schedule_timer(%#T): \"%m\", continuing\n"),
                    &configuration_->statisticCollectionInterval));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("scheduled statistic timer (id: %d, interval: %#T)\n"),
                    timerId_,
                    &configuration_->statisticCollectionInterval));
#endif // _DEBUG
    } // end IF
  } // end IF
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     TimerManagerType,
                     UserDataType>::~Net_ConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::~Net_ConnectionBase_T"));

  int result = -1;

  // clean up timer, if necessary
  if (timerId_ != -1)
  {
    typename TimerManagerType::INTERFACE_T* itimer_manager_p =
        (configuration_ ? (configuration_->timerManager ? configuration_->timerManager
                                                        : TimerManagerType::SINGLETON_T::instance ())
                        : TimerManagerType::SINGLETON_T::instance ());
    ACE_ASSERT (itimer_manager_p);
    const void* act_p = NULL;
    result = itimer_manager_p->cancel_timer (timerId_,
                                             &act_p);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  timerId_));
#if defined (_DEBUG)
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled statistic timer (id was: %d)\n"),
                  timerId_));
#endif // _DEBUG
  } // end IF
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     TimerManagerType,
                     UserDataType>::register_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::register_"));

  // sanity check(s)
  ACE_ASSERT (!isRegistered_);

  typename CONNECTION_MANAGER_T::INTERFACE_T* manager_p =
      CONNECTION_MANAGER_T::SINGLETON_T::instance ();
  ACE_ASSERT (manager_p);

  // (try to) register with the connection manager
  try {
    isRegistered_ = manager_p->register_ (this);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnectionManager_T::registerc(), continuing\n")));
    isRegistered_ = false;
  }
  if (unlikely (!isRegistered_))
  {
    // *NOTE*: most probable reason: maximum number of connections has been
    //         reached
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("failed to Net_IConnectionManager_T::registerc(), aborting\n")));
    return false;
  } // end IF

#if defined (_DEBUG)
  AddressType local_address;
  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  AddressType remote_address;
  try {
    this->info (handle_h,
                local_address, remote_address);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection_T::info(), continuing\n")));
  }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [0x%@/0x%@]: %s %s %s (total: %d)\n"),
              this, handle_h,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              (remote_address.is_any () ? ACE_TEXT ("<--") : (local_address.is_any () ? ACE_TEXT ("-->") :ACE_TEXT ("<-->"))),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remote_address).c_str ()),
              manager_p->count ()));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [%@/%d]: %s %s %s (total: %d)\n"),
              this, handle_h,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              (remote_address.is_any () ? ACE_TEXT ("<--") : (local_address.is_any () ? ACE_TEXT ("-->") :ACE_TEXT ("<-->"))),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remote_address).c_str ()),
              manager_p->count ()));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     TimerManagerType,
                     UserDataType>::deregister ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::deregister"));

  // sanity check(s)
  if (unlikely (!isRegistered_))
    return;

  typename CONNECTION_MANAGER_T::INTERFACE_T* manager_p =
    CONNECTION_MANAGER_T::SINGLETON_T::instance ();
  ACE_ASSERT (manager_p);

#if defined (_DEBUG)
  AddressType local_address;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType remote_address;
  try {
    this->info (handle,
                local_address,
                remote_address);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection_T::info(), continuing\n")));
  }
  unsigned int number_of_connections = manager_p->count () - 1;
#endif // _DEBUG

  // (try to) de-register with the connection manager
  isRegistered_ = false;
  // *IMPORTANT NOTE*: may delete 'this'
  try {
    manager_p->deregister (this);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnectionManager_T::deregister(), continuing\n")));
  }

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [0x%@/0x%@] (total: %u)\n"),
              this, handle,
              number_of_connections));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [%@/%d] (total: %d)\n"),
              this, handle,
              number_of_connections));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
ACE_Message_Block*
Net_ConnectionBase_T<AddressType,
                     ConfigurationType,
                     StateType,
                     StatisticContainerType,
                     TimerManagerType,
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
