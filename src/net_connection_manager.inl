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

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::Net_Connection_Manager_T ()
 : condition_ (lock_)
// , connections_ ()
// , configuration_ ()
 , isActive_ (true)
 , isInitialized_ (false)
 , maxNumConnections_ (NET_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS)
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::Net_Connection_Manager_T"));

  // init configuration data
  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::~Net_Connection_Manager_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::~Net_Connection_Manager_T"));

  // clean up
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  if (!connections_.is_empty ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%u connections still open --> check implementation !, continuing\n"),
                connections_.size ()));
    abortConnections ();
  } // end IF
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::initialize (unsigned int maxNumConnections_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::initialize"));

  maxNumConnections_ = maxNumConnections_in;

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("set maximum # connections: %u\n"),
//               maxNumConnections_));
}

//template <typename ConfigurationType,
//          typename StatisticContainerType>
//void
//Net_Connection_Manager_T<ConfigurationType,
//                         StatisticContainerType>::lock ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::lock"));
//
//	lock_.acquire ();
//}

//template <typename ConfigurationType,
//          typename StatisticContainerType>
//void
//Net_Connection_Manager_T<ConfigurationType,
//                         StatisticContainerType>::unlock ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::unlock"));
//
//	lock_.release ();
//}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::set (const ConfigurationType& configuration_in,
                                                    UserDataType* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::set"));

  configuration_ = configuration_in;
  userData_ = userData_in;

  isInitialized_ = true;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::get (ConfigurationType& configuration_out,
                                                    UserDataType*& userData_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::get"));

  configuration_out = configuration_;
  userData_out = userData_;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
Net_IConnection_T<ConfigurationType,
                  StatisticContainerType,
                  ITransportLayerType>*
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::operator[] (unsigned int sessionID_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::operator[]"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check(s)
  if (connections_.is_empty ())
    return NULL;

  CONNECTION_T* connection_p = NULL;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
       iterator.next (connection_p);
       iterator.advance ())
    if (connection_p->id () == sessionID_in)
      break;
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid session id (was: %u), aborting\n"),
                sessionID_in));
    return NULL;
  } // end IF

  // increase reference count
  connection_p->increase ();

  return connection_p;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::registerc (CONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::registerc"));

  ACE_ASSERT (isInitialized_);

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check(s)
  if (!isActive_)
    return false; // currently rejecting new connections...
  if (connections_.size () >= maxNumConnections_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("rejecting connection (maximum count [%u] has been reached), aborting\n"),
    //            maxNumConnections_));
    return false;
  } // end IF

  connection_in->increase ();
  connections_.insert_tail (connection_in);

  return true;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::deregister (CONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::deregister"));

  bool found = false;
  CONNECTION_T* connection_p = NULL;

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  for (CONNECTION_CONTAINER_ITERATOR_T iterator (connections_);
       iterator.next (connection_p);
       iterator.advance ())
    if (connection_p == connection_in)
    {
      found = true;
      iterator.remove ();

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

  // clean up
  connection_p->decrease ();

  // if there are no more connections, signal any waiters...
  if (connections_.is_empty () == 1)
    condition_.broadcast ();
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
unsigned int
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::numConnections () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::numConnections"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  return static_cast<unsigned int> (connections_.size ());
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::start"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  isActive_ = true;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::stop (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  if (lockedAccess_in)
    lock_.acquire ();
  isActive_ = false;
  if (lockedAccess_in)
    lock_.release ();
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  return isActive_;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::abortConnections ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortConnections"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check: anything to do ?
  if (connections_.is_empty ())
    return;

  unsigned int num_connections = connections_.size ();

  CONNECTION_T* connection_p = NULL;
//  for (ConnectionsIterator_t iterator (connections_);
//       iterator.next (connection_p);
//       iterator.advance ())
//  {
//    ACE_ASSERT (connection_p);
//    try
//    {
//      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection
//      connection_p->abort ();
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Net_IConnection::abort(), continuing")));
//    }
//  } // end FOR
  do
  {
    connection_p = NULL;
    if (connections_.get (connection_p, 0) == -1)
      break; // done

    ACE_ASSERT (connection_p);
    try
    {
      // *IMPORTANT NOTE*: implicitly invokes deregister()
      connection_p->finalize ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection_T::finalize(), continuing")));
    }
  } while (true);
  ACE_ASSERT (connections_.is_empty ());

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("closed %u connection(s)\n"),
              num_connections));
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::waitConnections () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::waitConnections"));

  // synch access to myConnections
  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

    while (connections_.is_empty () == 0)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting for (count: %d) connection(s) to leave...\n"),
                  connections_.size ()));

      condition_.wait ();
    } // end WHILE
  } // end lock scope

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("leaving...\n")));
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::abortOldestConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortOldestConnection"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check: anything to do ?
  if (connections_.is_empty () == 1)
    return;

  // close "oldest" connection --> list head
  CONNECTION_T* connection_p = NULL;
  if (connections_.get (connection_p) != -1)
  {
    try
    {
      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection from a
      // reactor thread, if any
      connection_p->finalize ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection::finalize(), continuing")));
    }
  } // end IF
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::abortNewestConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortNewestConnection"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check(s)
  if (connections_.empty ())
    return;

  // close "newest" connection
  ACE_Time_Value earliest = ACE_Time_Value::max_time;
  CONNECTION_CONTAINER_ITERATOR_T iterator;
  // *TODO*: this is really slow, sort the map by timestamp
  for (iterator = connections_.begin ();
       iterator != connections_.end ();
       iterator++)
    if ((*iterator).second.timestamp < earliest)
      earliest = (*iterator).second.timestamp;
  for (iterator = connections_.begin ();
       iterator != connections_.end ();
       iterator++)
    if ((*iterator).second.timestamp == earliest)
      break;
  ACE_ASSERT (iterator != connections_.end ());

  try
  { // *NOTE*: implicitly invokes deregisterConnection from a pro/reactor thread
    //         context (if any)
    (*iterator).second.connection->finalize ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection::finalize(), continuing")));
  }
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::collect"));

  // init result
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

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
                  ACE_TEXT ("caught exception in RPG_Common_IStatistic::collect(), continuing\n")));
    }

    data_out += temp;
  } // end FOR

  return true;
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::report"));

  // init result
  StatisticContainerType result;
  ACE_OS::memset (&result, 0, sizeof (result));

  // synch access to myConnections
  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

    // aggregate data from active connections
    if (!const_cast<SELF_T*> (this)->collect (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_IStatistic::collect(), aborting\n")));
      return;
    } // end IF

    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("*** RUNTIME STATISTICS ***\n--> [%u] Connection(s) <--\n # data messages: %u (avg.: %u)\ndata: %.0f (avg.: %.2f) bytes\n*** RUNTIME STATISTICS ***\\END\n"),
                connections_.size (),
                result.numDataMessages,
                (connections_.size () ? (result.numDataMessages / connections_.size ()) : 0),
                result.numBytes,
                (connections_.size () ? (result.numBytes / connections_.size ()) : 0.0)));
  } // end lock scope
}

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         UserDataType,
                         StatisticContainerType,
                         ITransportLayerType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::dump_state"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

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
                  ACE_TEXT ("caught exception in Net_IConnection::dump_state(), continuing")));
    }
  } // end FOR
}
