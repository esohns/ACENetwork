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
 : condition_ (lock_)
 , connections_ ()
 , isActive_ (true)
 , isInitialized_ (false)
 , lock_ ()
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

  bool do_abort = false;

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
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
                         UserDataType>::initialize (unsigned int maximumNumberOfConnections_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::initialize"));

  maximumNumberOfConnections_ = maximumNumberOfConnections_in;

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("set maximum # connections: %u\n"),
//               maximumNumberOfConnections_));
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

  int result = lock_.acquire ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", aborting\n")));

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

  int result = lock_.release ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", aborting\n")));
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

  //ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);

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

  //ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);

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
                  ConfigurationType,
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

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, NULL);
    unsigned int index = 0;
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance (), index++)
      if (index == index_in)
        break;
    if (unlikely (!connection_p))
    {
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("invalid index (was: %u), aborting\n"),
      //                index_in));
      return NULL;
    } // end IF

    // increase reference count
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
                  ConfigurationType,
                  StateType,
                  StatisticContainerType>*
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::get (Net_ConnectionId_t id_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return get (reinterpret_cast<ACE_HANDLE> (id_in));
#else
  return get (static_cast<ACE_HANDLE> (id_in));
#endif
}

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_IConnection_T<AddressType,
                  ConfigurationType,
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

  // sanity check(s)
  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);

  ICONNECTION_T* connection_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, NULL);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (connection_p->id () == reinterpret_cast<Net_ConnectionId_t> (handle_in))
#else
      if (connection_p->id () == static_cast<Net_ConnectionId_t> (handle_in))
#endif
        break;
    } // end FOR
    if (likely (connection_p))
      connection_p->increase (); // increase reference count
    else
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection not found (handle was: 0x%@), aborting\n"),
                  handle_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection not found (handle was: %d), aborting\n"),
                  handle_in));
#endif
    } // end ELSE
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
                  ConfigurationType,
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

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, NULL);
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
    //else
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("connection not found (address was: \"%s\"), aborting\n"),
    //              ACE_TEXT (Net_Common_Tools::IPAddress2String (address_in).c_str ())));
    //} // end ELSE
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
                         UserDataType>::registerc (ICONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::registerc"));

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
    if (unlikely (!isActive_ || // --> currently rejecting new connections
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

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
         iterator.next (connection_p);
         iterator.advance ())
      if (connection_p == connection_in)
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

    // iff there are no more connections, signal any waiters
    if (connections_.is_empty ())
    {
      result = condition_.broadcast ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));
    } // end IF
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

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, 0);
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
void
Net_Connection_Manager_T<ACE_SYNCH_USE,
                         AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::start"));

  ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);

  isActive_ = true;
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
                                              bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);

  int result = -1;

  if (lockedAccess_in)
  {
    result = lock_.acquire ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", continuing\n")));
  } // end IF

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    isActive_ = false;
  } // end lock scope

  if (lockedAccess_in)
  {
    result = lock_.release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF
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
                         UserDataType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  bool result = false;

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
    result = isActive_;
  } // end lock scope

  return result;
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
                  ACE_TEXT ("invalid/unknown strategy (was; %d), returning\n"),
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
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
      connection_p->increase ();

      if (unlikely (!connections_a.insert_tail (connection_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_DLList::insert_tail(): \"%m\", returning\n")));

        // clean up
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
      connection_p->close ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Net_IConnection_T::close(), continuing\n"),
                  connection_p->id ()));
    }

    connection_p->decrease ();
    connection_p = NULL;
  } // end FOR
  // debug info
  if (is_first_b)
  {
    is_first_b = false;

    if (likely (connections_a.size ()))
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed %u connection(s)\n"),
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
                         UserDataType>::wait (bool waitForMessageQueue_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::wait"));

  ACE_UNUSED_ARG (waitForMessageQueue_in);

  int result = -1;

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    while (!connections_.is_empty ())
    { // debug info
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting for %u connection(s)...\n"),
                  connections_.size ()));

      result = condition_.wait ();
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
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
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
    connection_p->close ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_IConnection_T::close(), continuing\n"),
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
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
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
    connection_p->close ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_IConnection_T::close(), continuing\n"),
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

  // initialize result
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *NOTE*: called from report () only !
//  ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);

  StatisticContainerType statistic;
  // aggregate statistical data
  // *WARNING*: this assumes the caller is holding the lock !
  ICONNECTION_T* connection_p = NULL;
  //{ ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
      ACE_OS::memset (&statistic, 0, sizeof (statistic));
      try { // collect information
        connection_p->collect (statistic);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: caught exception in Common_IStatistic::collect(), continuing\n"),
                    connection_p->id ()));
      }

      data_out += statistic;
      connection_p = NULL;
    } // end FOR
  //} // end lock scope

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
                         UserDataType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::report"));

  // initialize result
  StatisticContainerType result;
  ACE_OS::memset (&result, 0, sizeof (result));

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    // aggregate data from active connections
    if (unlikely (!const_cast<OWN_TYPE_T*> (this)->collect (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_IStatistic::collect(), returning\n")));
      return;
    } // end IF

    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("*** CONNECTION STATISTIC ***\n--> [%u] connection(s) <--\n# data messages: %u (avg.: %u)\ndata: %.0f (avg.: %.2f) bytes\n*** CONNECTION STATISTIC ***\\END\n"),
                connections_.size (),
                result.dataMessages,
                (connections_.size () ? (result.dataMessages / connections_.size ())
                                      : 0),
                result.bytes,
                (connections_.size () ? (result.bytes / connections_.size ())
                                      : 0.0)));
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
                         UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::dump_state"));

  ICONNECTION_T* connection_p = NULL;
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (connection_p);
         iterator.advance ())
    { ACE_ASSERT (connection_p);
      try { // dump connection information
        connection_p->dump_state ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: caught exception in Net_IConnection_T::dump_state(), continuing\n"),
                    connection_p->id ()));
      }
      connection_p = NULL;
    } // end FOR
  } // end lock scope
}
