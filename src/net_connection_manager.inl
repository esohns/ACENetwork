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

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::Net_Connection_Manager_T ()
 : condition_ (lock_)
 , configuration_ ()
 , connections_ ()
 , isActive_ (true)
 , isInitialized_ (false)
 , maxNumConnections_ (NET_CONNECTION_MAXIMUM_NUMBER_OF_OPEN)
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::Net_Connection_Manager_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::~Net_Connection_Manager_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::~Net_Connection_Manager_T"));

  bool do_abort = false;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    if (!connections_.is_empty ())
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("connections still open --> check implementation, continuing\n")));
      do_abort = true;
    } // end IF
  } // end lock scope

  if (do_abort)
    abort ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::initialize (unsigned int maxNumConnections_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::initialize"));

  maxNumConnections_ = maxNumConnections_in;

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("set maximum # connections: %u\n"),
//               maxNumConnections_));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::lock ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::lock"));

  int result = lock_.acquire ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::unlock ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::unlock"));

  int result = lock_.release ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::set (const ConfigurationType& configuration_in,
                                             UserDataType* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::set"));

  //ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  configuration_ = configuration_in;
  userData_ = userData_in;

  isInitialized_ = true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::get (ConfigurationType& configuration_out,
                                             UserDataType*& userData_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  //ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  configuration_out = configuration_;
  userData_out = userData_;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_IConnection_T<AddressType,
                  ConfigurationType,
                  StateType,
                  StatisticContainerType>*
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::operator[] (unsigned int index_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::operator[]"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  ICONNECTION_T* connection_p = NULL;
  unsigned int index = 0;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
       iterator.next (connection_p);
       iterator.advance (), index++)
    if (index == index_in)
      break;
  if (!connection_p)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid index (was: %u), aborting\n"),
//                index_in));
    return NULL;
  } // end IF

  // increase reference count
  connection_p->increase ();

  return connection_p;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_IConnection_T<AddressType,
                  ConfigurationType,
                  StateType,
                  StatisticContainerType>*
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::get (ACE_HANDLE handle_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  ICONNECTION_T* result = NULL;

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
       iterator.next (result);
       iterator.advance ())
#if defined (ACE_WIN32) || defined (ACE_WIN64)
       if (result->id () == reinterpret_cast<unsigned int> (handle_in))
#else
       if (result->id () == static_cast<unsigned int> (handle_in))
#endif
         break;
  if (result)
    result->increase (); // increase reference count
//  else
//  {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("connection not found (handle was: %@), aborting\n"),
//                handle_in));
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("connection not found (handle was: %d), aborting\n"),
//                handle_in));
//#endif
//  } // end ELSE

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
Net_IConnection_T<AddressType,
                  ConfigurationType,
                  StateType,
                  StatisticContainerType>*
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::get (const AddressType& peerAddress_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  ICONNECTION_T* result = NULL;

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  { // synch access
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
         iterator.next (result);
         iterator.advance ())
    {
      result->info (handle,
                    local_address,
                    peer_address);
      if (peer_address == peerAddress_in)
        break;
    } // end FOR
    if (result)
      result->increase (); // increase reference count
    else
    {
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      int result_2 = peerAddress_in.addr_to_string (buffer,
                                                    sizeof (buffer),
                                                    1);
      if (result_2 == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection not found (address was: \"%s\"), aborting\n"),
                  buffer));
    } // end ELSE
  } // end lock scope

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::registerc (ICONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::registerc"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  if (!isActive_ || // --> currently rejecting new connections...
      (connections_.size () >= maxNumConnections_))
    return false;

  try
  {
    connection_in->increase ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection_T::increase(): \"%m\", aborting\n")));
    return false;
  }
  if (!connections_.insert_tail (connection_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_DLList::insert_tail(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::deregister (ICONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::deregister"));

  bool found = false;
  ICONNECTION_T* connection_p = NULL;
  int result = -1;

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
       iterator.next (connection_p);
       iterator.advance ())
    if (connection_p == connection_in)
    {
      found = true;
      result = iterator.remove ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_DLList_Iterator::remove(): \"%m\", continuing\n")));
      break;
    } // end IF
  if (!found)
  {
    // *IMPORTANT NOTE*: when a connection attempt fails, the reactor close()s
    // the connection although it was never open()ed
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection handle %@ not found (id was: %u), returning\n"),
                connection_in,
                connection_in->id ()));
    return;
  } // end IF
  ACE_ASSERT (connection_p);

  // clean up
  try
  {
    connection_in->decrease ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection_T::decrease(): \"%m\", continuing\n")));
  }

  // if there are no more connections, signal any waiters...
  if (connections_.is_empty () == 1)
  {
    result = condition_.broadcast ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));
  } // end IF
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
unsigned int
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::numConnections () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::numConnections"));

  unsigned int result = 0;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    result = static_cast<unsigned int> (connections_.size ());
  } // end lock scope

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::start"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  isActive_ = true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
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
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Recursive_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF
  isActive_ = false;
  if (lockedAccess_in)
  {
    result = lock_.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Recursive_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  bool result = false;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    result = isActive_;
  } // end lock scope

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abort"));

  int result = -1;
  unsigned int closed_connections = 0;
  ICONNECTION_T* connection_p = NULL;

  // *WARNING*: when using single-threaded reactors, close()ing connections
  //            inside the lock scope may lead to deadlock
begin:
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    if (connections_.is_empty () == 1)
      goto done;

    result = connections_.get (connection_p, 0);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_DLList::get(0): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (connection_p);
    connection_p->increase ();
  } // end lock scope

  ACE_ASSERT (connection_p);
  try
  {
    connection_p->close ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IICONNECTION_T::close(), continuing\n")));
  }
  connection_p->decrease ();
  closed_connections++;
  goto begin;

done:
  if (closed_connections)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed %u connection(s)\n"),
                closed_connections));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::wait () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::wait"));

  int result = -1;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    while (connections_.is_empty () == 0)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting for (count: %d) connection(s) to leave...\n"),
                  connections_.size ()));

      result = condition_.wait ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition::wait(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::abortLeastRecent ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortLeastRecent"));

  int result = -1;
  ICONNECTION_T* connection_p = NULL;

  // *WARNING*: when using single-threaded reactors, close()ing the connection
  //            inside the lock scope may lead to deadlock
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    // sanity check(s)
    if (connections_.is_empty () == 1)
      return;

    // close "oldest" connection --> list head
    result = connections_.get (connection_p, 0);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_DLList::get(0): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (connection_p);
    connection_p->increase ();
  } // end lock scope

  ACE_ASSERT (connection_p);
  try
  {
    connection_p->close ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IICONNECTION_T::close(), continuing\n")));
  }
  connection_p->decrease ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::abortMostRecent ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortMostRecent"));

  int result = -1;
  ICONNECTION_T* connection_p = NULL;

  // *WARNING*: when using single-threaded reactors, close()ing the connection
  //            inside the lock scope may lead to deadlock
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    // sanity check(s)
    if (connections_.is_empty () == 1)
      return;

    // close "newest" connection --> list tail
    CONNECTION_CONTAINER_REVERSEITERATOR_T iterator (connections_);
    result = iterator.next (connection_p);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_DLList_Reverse_Iterator::next(): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (connection_p);
    connection_p->increase ();
  } // end lock scope

  ACE_ASSERT (connection_p);
  try
  {
    connection_p->close ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IICONNECTION_T::close(), continuing\n")));
  }
  connection_p->decrease ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::collect"));

  // initialize result
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *NOTE*: called from report () only !
//  //  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  StatisticContainerType temp;
  // aggregate statistical data
  // *WARNING*: this assumes we're holding our lock !
  ICONNECTION_T* connection_p = NULL;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
       iterator.next (connection_p);
       iterator.advance ())
  {
    ACE_OS::memset (&temp, 0, sizeof (temp));
    try
    { // collect information
      connection_p->collect (temp);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IStatistic::collect(), continuing\n")));
    }

    data_out += temp;
  } // end FOR

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::report"));

  // initialize result
  StatisticContainerType result;
  ACE_OS::memset (&result, 0, sizeof (result));

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    // aggregate data from active connections
    if (!const_cast<OWN_TYPE_T*> (this)->collect (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_IStatistic::collect(), returning\n")));
      return;
    } // end IF

    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("*** RUNTIME STATISTICS ***\n--> [%u] connection(s) <--\n# data messages: %u (avg.: %u)\ndata: %.0f (avg.: %.2f) bytes\n*** RUNTIME STATISTICS ***\\END\n"),
                connections_.size (),
                result.dataMessages,
                (connections_.size () ? (result.dataMessages / connections_.size ())
                                      : 0),
                result.bytes,
                (connections_.size () ? (result.bytes / connections_.size ())
                                      : 0.0)));
  } // end lock scope
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename UserDataType>
void
Net_Connection_Manager_T<AddressType,
                         ConfigurationType,
                         StateType,
                         StatisticContainerType,
                         UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::dump_state"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  ICONNECTION_T* connection_p = NULL;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
       iterator.next (connection_p);
       iterator.advance ())
  {
    try
    { // dump connection information
      connection_p->dump_state ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IICONNECTION_T::dump_state(), continuing\n")));
    }
  } // end FOR
}
