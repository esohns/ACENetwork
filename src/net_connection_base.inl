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
#include "ace/OS.h"

#include "common_timer_manager_common.h"

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
                     UserDataType>::Net_ConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                          const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (1,    // initial count
              true) // delete on zero ?
 , configuration_ (NULL)
 , state_ ()
 , isRegistered_ (false)
 , manager_ (interfaceHandle_in)
 , statisticHandler_ (ACTION_COLLECT,
                      this,
                      true)
 , timerId_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::Net_ConnectionBase_T"));

  // initialize configuration/user data
  if (manager_)
  {
    try {
      // get (default) configuration/user data from the connection manager
      // *TODO*: remove type inference
      manager_->get (configuration_,
                     state_.userData);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnectionManager_T::get(), continuing\n")));
    }
  } // end IF
//  // register with the connection manager, if any
//  if (!registerc ())
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), continuing\n")));

  if (statisticCollectionInterval_in != ACE_Time_Value::zero)
  {
    // schedule regular statistic collection
    typename TimerManagerType::INTERFACE_T* itimer_manager_p =
        (configuration_ ? (configuration_->timerManager ? configuration_->timerManager
                                                        : TIMER_MANAGER_SINGLETON_T::instance ())
                        : TIMER_MANAGER_SINGLETON_T::instance ());
    ACE_ASSERT (itimer_manager_p);
    timerId_ =
      itimer_manager_p->schedule_timer (&statisticHandler_,                               // event handler
                                        NULL,                                             // asynchronous completion token
                                        COMMON_TIME_NOW + statisticCollectionInterval_in, // first wakeup time
                                        statisticCollectionInterval_in);                  // interval
    if (timerId_ == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::schedule_timer(%#T): \"%m\", continuing\n"),
                  &statisticCollectionInterval_in));
  //    else
  //      ACE_DEBUG ((LM_DEBUG,
  //                  ACE_TEXT ("scheduled statistic timer (id: %d, interval: %#T)\n"),
  //                  timerId_,
  //                  &statisticCollectionInterval_in));
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
                                                        : TIMER_MANAGER_SINGLETON_T::instance ())
                        : TIMER_MANAGER_SINGLETON_T::instance ());
    ACE_ASSERT (itimer_manager_p);
    const void* act_p = NULL;
    result = itimer_manager_p->cancel_timer (timerId_,
                                             &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  timerId_));
//    else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("cancelled statistic timer (id was: %d)\n"),
//                  timerId_));
  } // end IF

//  // deregister with the connection manager, if any
//  deregister ();
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
//#if defined (__GNUG__)
                     UserDataType>::registerc (ICONNECTION_T* connection_in)
//#else
//                     UserDataType>::registerc ()
//#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionBase_T::registerc"));

  ICONNECTION_T* iconnection_p = NULL;
  AddressType local_address, remote_address;

  // sanity check(s)
  ACE_ASSERT (!isRegistered_);
  if (!manager_)
    goto continue_; // nothing to do

  // (try to) register with the connection manager
  //iconnection_p = this;
//#if defined (__GNUG__)
  // *WORKAROUND*: see header
  iconnection_p = (connection_in ? connection_in : this);
//#endif
  try {
    isRegistered_ = manager_->registerc (iconnection_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnectionManager_T::registerc(), continuing\n")));
    isRegistered_ = false;
  }
  if (!isRegistered_)
  {
    // *NOTE*: most probable reason: maximum number of connections has been
    //         reached
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("failed to Net_IConnectionManager_T::registerc(), aborting\n")));
    return false;
  } // end IF

  try {
//#if defined (__GNUG__)
    // *WORKAROUND*: see header
    ACE_ASSERT (iconnection_p);
    iconnection_p->info (state_.handle,
                         local_address,
                         remote_address);
//#else
    //      this->info (state_.handle,
    //                  local_address,
    //                  remote_address);
//#endif
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection_T::info(), aborting\n")));
    return false;
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [0x%@/0x%@]: %s <--> %s (total: %d)\n"),
              this, state_.handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remote_address).c_str ()),
              manager_->count ()));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [%@/%d]: %s <--> %s (total: %d)\n"),
              this, state_.handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (remote_address).c_str ()),
              manager_->count ()));
#endif

continue_:
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
  if (!manager_ ||
      !isRegistered_)
    return;

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, remote_address;
  try {
    this->info (handle,
                local_address,
                remote_address);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection_T::info(), continuing\n")));
  }

  OWN_TYPE_T* this_p = this;
  unsigned int number_of_connections = manager_->count () - 1;

  // (try to) de-register with the connection manager
  isRegistered_ = false;
  // *IMPORTANT NOTE*: may delete 'this'
  try {
    manager_->deregister (this);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnectionManager_T::deregister(), continuing\n")));
  }

  // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [0x%@/0x%@] (total: %u)\n"),
              this_p, handle,
              number_of_connections));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [%@/%d] (total: %d)\n"),
              this_p, handle,
              number_of_connections));
#endif
}
