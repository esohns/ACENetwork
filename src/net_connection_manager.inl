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

#include "ace/Guard_T.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"

#include "common_timer_manager_common.h"

#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::Net_Connection_Manager_T ()
 : resetTimeoutHandler_ (this)
 , resetTimeoutHandlerId_ (-1)
 , resetTimeoutInterval_ (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000)
 , configurationLock_ ()
 , connections_ ()
 , connectionsLock_ ()
 , connectionsCondition_ (connectionsLock_)
 , isActive_ (true)
 , isInitialized_ (false)
 , maximumNumberOfConnections_ (NET_CONNECTION_MAXIMUM_NUMBER_OF_OPEN)
 , configuration_ (NULL)
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::Net_Connection_Manager_T"));

}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::~Net_Connection_Manager_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::~Net_Connection_Manager_T"));

  if (unlikely (resetTimeoutHandlerId_ != -1))
  {
    Common_ITimer_Manager_t* timer_interface_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
    const void* act_p = NULL;
    int result = timer_interface_p->cancel_timer (resetTimeoutHandlerId_,
                                                  &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  resetTimeoutHandlerId_));
  } // end IF

  bool do_abort = false;
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    if (!unlikely (connections_.is_empty ()))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%u remaining connection(s) in dtor, continuing\n"),
                  connections_.size ()));
      do_abort = true;
    } // end IF
  } // end lock scope
  if (unlikely (do_abort))
    abort ();
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::initialize (unsigned int maximumNumberOfConnections_in,
                                                    const ACE_Time_Value& visitInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::initialize"));

  resetTimeoutInterval_ = visitInterval_in;

  maximumNumberOfConnections_ = maximumNumberOfConnections_in;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::lock (bool block_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::lock"));

  ACE_UNUSED_ARG (block_in);

  int result = configurationLock_.acquire ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));

  return (result == 0);
}
template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
int
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::unlock (bool unblock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::unlock"));

  ACE_UNUSED_ARG (unblock_in);

  int result = configurationLock_.release ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::set (const ConfigurationType& configuration_in,
                                             UserDataType* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::set"));

  ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, configurationLock_);

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  userData_ = userData_in;

  isInitialized_ = true;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::get (ConfigurationType*& configuration_out,
                                             UserDataType*& userData_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // *NOTE*: this is called by managed (!) connections to retrieve their
  //         respective configuration; (hopefully) the globalLock_ is being held
  //         by the initiating thread (*NOTE*: this might very well be a
  //         different thread for asynch connections)
  //         --> do NOT try to grab the lock here !
  configuration_out = configuration_;
  userData_out = userData_;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_IConnection_T<AddressType,
                  //ConfigurationType,
                  StateType,
                  StatisticContainerType>*
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::operator[] (unsigned int index_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::operator[]"));

  ICONNECTION_T* connection_p = NULL;

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_, NULL);
    unsigned int index = 0;
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance (), index++)
      if (index == index_in)
        break;
    if (unlikely (!connection_p))
      return NULL;
    connection_p->increase ();
  } // end lock scope

  return connection_p;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_IConnection_T<AddressType,
                  StateType,
                  StatisticContainerType>*
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::get (ACE_HANDLE handle_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  ICONNECTION_T* connection_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_, NULL);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (connection_p->id () == reinterpret_cast<Net_ConnectionId_t> (handle_in))
#else
      if (connection_p->id () == static_cast<Net_ConnectionId_t> (handle_in))
#endif // ACE_WIN32 || ACE_WIN64
        break;
      connection_p = NULL;
    } // end FOR
    if (likely (connection_p))
      connection_p->increase (); // increase reference count
  } // end lock scope

  return connection_p;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_IConnection_T<AddressType,
                  StateType,
                  StatisticContainerType>*
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::get (const AddressType& address_in,
                                             bool isPeerAddress_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  ICONNECTION_T* connection_p = NULL;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_, NULL);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance ())
    {
      connection_p->info (handle,
                          local_address,
                          peer_address);
      if ((isPeerAddress_in && (peer_address == address_in)) ||
          (!isPeerAddress_in && (local_address == address_in)))
        break;

      connection_p = NULL;
    } // end FOR
    if (likely (connection_p))
      connection_p->increase (); // increase reference count
  } // end lock scope

  return connection_p;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::register_ (ICONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::register_"));

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_, false);
    if (unlikely (!isActive_ || // --> (currently) rejecting new connections
                  (connections_.size () >= maximumNumberOfConnections_)))
      return false;

    try {
      connection_in->increase ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::increase(), aborting\n")));
      return false;
    }
    if (!unlikely (connections_.insert_tail (connection_in)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_DLList::insert_tail(): \"%m\", aborting\n")));
      connection_in->decrease ();
      return false;
    } // end IF
  } // end lock scope

  return true;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::deregister (ICONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::deregister"));

  ICONNECTION_T* connection_p = NULL;
  bool found = false;
  int result = -1;

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
         iterator.next (connection_p);
         iterator.advance ())
      if (unlikely (connection_p == connection_in))
      {
        found = true;
        result = iterator.remove ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_DLList_Iterator::remove(): \"%m\", continuing\n")));
        break;
      } // end IF
    if (unlikely (!found))
    {
      // *NOTE*: most probably cause: handle already deregistered (--> check
      //         implementation !)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection (id was: %u) handle (was: 0x%@) not found, returning\n"),
                  connection_in->id (),
                  connection_in));
      return;
    } // end IF
    ACE_ASSERT (connection_p);

    // clean up
    try {
      connection_in->decrease ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::decrease(): \"%m\", continuing\n")));
    }

    // signal any waiters
    result = connectionsCondition_.broadcast ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
unsigned int
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::count () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::count"));

  unsigned int result = 0;

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_, 0);
    result = static_cast<unsigned int> (connections_.size ());
  } // end lock scope

  return result;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::start (ACE_Time_Value* timeout_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::start"));

  ACE_UNUSED_ARG (timeout_in);

  int result = -1;
  Common_ITimer_Manager_t* timer_interface_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  const void* act_p = NULL;

  // (re-)schedule the visitor interval timer
  if (unlikely (resetTimeoutHandlerId_ != -1))
  {
    result = timer_interface_p->cancel_timer (resetTimeoutHandlerId_,
                                              &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  resetTimeoutHandlerId_));
    resetTimeoutHandlerId_ = -1;
  } // end IF
  resetTimeoutHandlerId_ =
    timer_interface_p->schedule_timer (&resetTimeoutHandler_,  // event handler handle
                                       NULL,                   // asynchronous completion token
                                       ACE_Time_Value::zero,   // first wakeup time
                                       resetTimeoutInterval_); // interval
  if (unlikely (resetTimeoutHandlerId_ == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                &resetTimeoutInterval_));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("scheduled visitor interval timer (id: %d, interval: %#T)\n"),
              resetTimeoutHandlerId_,
              &resetTimeoutInterval_));

  isActive_ = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::stop (bool waitForCompletion_in,
                                              bool)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  isActive_ = false;

  if (unlikely (resetTimeoutHandlerId_ != -1))
  {
    Common_ITimer_Manager_t* timer_interface_p =
        COMMON_TIMERMANAGER_SINGLETON::instance ();
    const void* act_p = NULL;
    int result = timer_interface_p->cancel_timer (resetTimeoutHandlerId_,
                                                  &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  resetTimeoutHandlerId_));
    resetTimeoutHandlerId_ = -1;
  } // end IF

  if (waitForCompletion_in)
    wait (true); // N/A
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::abort (enum Net_Connection_AbortStrategy strategy_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abort"));

  switch (strategy_in)
  {
    case NET_CONNECTION_ABORT_STRATEGY_RECENT_LEAST:
      return abortLeastRecent ();
    case NET_CONNECTION_ABORT_STRATEGY_RECENT_MOST:
      return abortMostRecent ();
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown strategy (was: %d), returning\n"),
                  strategy_in));
      break;
    }
  } // end SWITCH
}
template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::abort (bool waitForCompletion_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abort"));

  ICONNECTION_T* connection_p = NULL;
  CONNECTION_CONTAINER_T connections_a;
  bool is_first_b = true;

begin:
  // step1: gather a set of open connection handles
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
      connection_p->increase ();

      if (unlikely (!connections_a.insert_tail (connection_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_DLList::insert_tail(): \"%m\", returning\n")));
        connection_p->decrease ();
        return;
      } // end IF
    } // end FOR
  } // end lock scope
  if (unlikely (connections_a.is_empty ()))
    return;

  // step2: close all connections
  connection_p = NULL;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_a);
       iterator.next (connection_p);
       iterator.advance ())
  { ACE_ASSERT (connection_p);
    try {
      connection_p->abort ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Net_IConnection_T::abort(), continuing\n"),
                  connection_p->id ()));
    }
    connection_p->decrease (); connection_p = NULL;
  } // end FOR
  if (is_first_b)
  {
    is_first_b = false;

    if (likely (connections_a.size ()))
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("aborted %u connection(s)\n"),
                  connections_a.size ()));
  } // end IF
  if (unlikely (waitForCompletion_in))
  {
    connections_a.reset ();
    goto begin;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::wait (bool) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::wait"));

  int result = -1;

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    while (!connections_.is_empty ())
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting for %u connection(s)...\n"),
                  connections_.size ()));
      result = connectionsCondition_.wait ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_CONDITION::wait(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::wait_2 (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::wait_2"));

  int result = -1;
  ICONNECTION_T* connection_p = NULL;
  bool handle_found_b;
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    while (!connections_.is_empty ())
    {
      handle_found_b = false;
      for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
           iterator.next (connection_p);
           iterator.advance ())
      { ACE_ASSERT (connection_p);
        handle_found_b = (connection_p->id () == handle_in);
        if (handle_found_b)
          break; // connection is still around
      } // end FOR
      if (!handle_found_b)
        break; // connection is gone

      result = connectionsCondition_.wait ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_CONDITION::wait(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::abortLeastRecent ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortLeastRecent"));

  ICONNECTION_T* connection_p = NULL;
  int result = -1;

  // *NOTE*: when using single-threaded reactors, close()ing the connection
  //         inside the lock scope may lead to deadlock
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    if (unlikely (connections_.is_empty ()))
      return;

    // close "oldest" connection --> list head
    result = connections_.get (connection_p, 0);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_DLList::get(0): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (connection_p);
    connection_p->increase ();
  } // end lock scope
  ACE_ASSERT (connection_p);

  try {
    connection_p->abort ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_IConnection_T::abort(), continuing\n"),
                connection_p->id ()));
  }
  connection_p->decrease ();
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::abortMostRecent ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortMostRecent"));

  ICONNECTION_T* connection_p = NULL;
  int result = -1;

  // *NOTE*: when using single-threaded reactors, close()ing the connection
  //         inside the lock scope may lead to deadlock
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    if (unlikely (connections_.is_empty ()))
      return;

    // close "newest" connection --> list tail
    CONNECTION_CONTAINER_REVERSEITERATOR_T iterator (connections_);
    result = iterator.next (connection_p);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_DLList_Reverse_Iterator::next(): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (connection_p);
    connection_p->increase ();
  } // end lock scope
  ACE_ASSERT (connection_p);

  try {
    connection_p->abort ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_IConnection_T::abort(), continuing\n"),
                connection_p->id ()));
  }
  connection_p->decrease ();
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::collect"));

  // initialize return values
  ACE_OS::memset (&data_out, 0, sizeof (StatisticContainerType));

  // aggregate statistic data
  StatisticContainerType statistic_s;
  // *WARNING*: this assumes the caller is holding the lock !
  ICONNECTION_T* connection_p = NULL;
  // *NOTE*: called from report () only !
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_, false);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
      ACE_OS::memset (&statistic_s, 0, sizeof (StatisticContainerType));
      try { // collect information
        connection_p->collect (statistic_s);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStatistic::collect(), continuing\n")));
      }

      data_out += statistic_s;
      connection_p = NULL;
    } // end FOR
  } // end lock scope

  return true;
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::reset ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::reset"));

  ICONNECTION_T* connection_p = NULL;
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
      try { // dump connection information
        connection_p->update (resetTimeoutInterval_);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IStatistic_T::update(), continuing\n")));
      }
      connection_p = NULL;
    } // end FOR
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::report"));

  // initialize result
  StatisticContainerType statistic_s;
  ACE_OS::memset (&statistic_s, 0, sizeof (StatisticContainerType));

  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);
  size_t number_of_connections_i = 0;

  // step1: aggregate data from active connections
  if (unlikely (!this_p->collect (statistic_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IStatistic::collect(), returning\n")));
    return;
  } // end IF

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    number_of_connections_i = connections_.size ();
  } // end lock scope

  // step2: dump statistic information
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** CONNECTION STATISTIC ***\n--> [%B] connection(s) <--\n sent data: \t%Q\t(avg.: %.2f)\n received data: %Q\t(avg.: %.2f)\n msg./s: \t%.2f \tB/s: %.2f\n*** CONNECTION STATISTIC ***\\END\n"),
              number_of_connections_i,
              statistic_s.sentBytes,
              (number_of_connections_i ? (statistic_s.sentBytes / static_cast<float> (number_of_connections_i)) : 0.0F),
              statistic_s.receivedBytes,
              (number_of_connections_i ? (statistic_s.receivedBytes / static_cast<float> (number_of_connections_i)) : 0.0F),
              statistic_s.messagesPerSecond, statistic_s.bytesPerSecond));
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::dump_state"));

  ICONNECTION_T* connection_p = NULL;

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, connectionsLock_);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
      try {
        connection_p->dump_state ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
      }
      connection_p = NULL;
    } // end FOR
  } // end lock scope
}
