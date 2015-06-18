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
#include "common_timer_manager.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::Net_Connection_Manager_T ()
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::~Net_Connection_Manager_T ()
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::initialize (unsigned int maxNumConnections_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::initialize"));

  maxNumConnections_ = maxNumConnections_in;

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("set maximum # connections: %u\n"),
//               maxNumConnections_));
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::lock ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::lock"));

  int result = lock_.acquire ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::unlock ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::unlock"));

  int result = lock_.release ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::set (const ConfigurationType& configuration_in,
                                           UserDataType* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::set"));

  configuration_ = configuration_in;
  userData_ = userData_in;

  isInitialized_ = true;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::get (ConfigurationType& configuration_out,
                                           UserDataType*& userData_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  configuration_out = configuration_;
  userData_out = userData_;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
Net_IConnection_T<AddressType,
                  ConfigurationType,
                  StatisticContainerType,
                  StreamType>*
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::operator[] (unsigned int index_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::operator[]"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  CONNECTION_T* connection_p = NULL;
  unsigned int index = 0;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
       iterator.next (connection_p);
       iterator.advance (), index++)
    if (index == index_in)
      break;
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid index (was: %u), aborting\n"),
                index_in));
    return NULL;
  } // end IF

  // increase reference count
  connection_p->increase ();

  return connection_p;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
Net_IConnection_T<AddressType,
                  ConfigurationType,
                  StatisticContainerType,
                  StreamType>*
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::get (ACE_HANDLE handle_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  CONNECTION_T* connection_p = NULL;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
       iterator.next (connection_p);
       iterator.advance ())
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (connection_p->id () == reinterpret_cast<unsigned int> (handle_in))
#else
    if (connection_p->id () == static_cast<unsigned int> (handle_in))
#endif
      break;
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid handle (was: %u), aborting\n"),
                handle_in));
    return NULL;
  } // end IF

  // increase reference count
  connection_p->increase ();

  return connection_p;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
bool
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::registerc (CONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::registerc"));

  ACE_ASSERT (isInitialized_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  if (!isActive_ || // --> currently rejecting new connections...
      (connections_.size () >= maxNumConnections_))
    return false;

  if (!connections_.insert_tail (connection_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_DLList::insert_tail(): \"%m\", aborting\n")));
    return false;
  } // end IF
  connection_in->increase ();

  return true;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::deregister (CONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::deregister"));

  bool found = false;
  CONNECTION_T* connection_p = NULL;
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
  connection_p->decrease ();

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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
unsigned int
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::numConnections () const
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::start"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  isActive_ = true;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::stop (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
bool
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::isRunning () const
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abort"));

  int result = -1;
  unsigned int closed_connections = 0;
  CONNECTION_T* connection_p = NULL;

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
                ACE_TEXT ("caught exception in Net_IConnection_T::close(), continuing\n")));
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::wait () const
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::abortOldestConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortOldestConnection"));

  int result = -1;
  CONNECTION_T* connection_p = NULL;

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
                ACE_TEXT ("caught exception in Net_IConnection_T::close(), continuing\n")));
  }
  connection_p->decrease ();
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::abortYoungestConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortYoungestConnection"));

  int result = -1;
  CONNECTION_T* connection_p = NULL;

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
                ACE_TEXT ("caught exception in Net_IConnection_T::close(), continuing\n")));
  }
  connection_p->decrease ();
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
bool
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::collect"));

  // initialize result
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  // *NOTE*: called from report () only !
//  //  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  StatisticContainerType temp;
  // aggregate statistical data
  // *WARNING*: this assumes we're holding our lock !
  CONNECTION_T* connection_p = NULL;
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
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::report"));

  // initialize result
  StatisticContainerType result;
  ACE_OS::memset (&result, 0, sizeof (result));

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    // aggregate data from active connections
    if (!const_cast<SELF_T*> (this)->collect (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_IStatistic::collect(), returning\n")));
      return;
    } // end IF

    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("*** RUNTIME STATISTICS ***\n--> [%u] Connection(s) <--\n# data messages: %u (avg.: %u)\ndata: %.0f (avg.: %.2f) bytes\n*** RUNTIME STATISTICS ***\\END\n"),
                connections_.size (),
                result.numDataMessages,
                (connections_.size () ? (result.numDataMessages / connections_.size ())
                                      : 0),
                result.numBytes,
                (connections_.size () ? (result.numBytes / connections_.size ())
                                      : 0.0)));
  } // end lock scope
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename StreamType>
void
Net_Connection_Manager_T<AddressType,
                         SocketConfigurationType,
                         ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         StreamType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::dump_state"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  CONNECTION_T* connection_p = NULL;
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
                  ACE_TEXT ("caught exception in Net_IConnection_T::dump_state(), continuing\n")));
    }
  } // end FOR
}
