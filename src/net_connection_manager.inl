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

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::Net_Connection_Manager_T ()
 : condition_ (lock_)
 //, connections_ ()
 //, configuration_ ()
 , isActive_ (true)
 , isInitialized_ (false)
 , maxNumConnections_ (NET_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS)
 , sessionData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::Net_Connection_Manager_T"));

  // init configuration data
  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
  //ACE_OS::memset (&sessionData_, 0, sizeof (sessionData_));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::~Net_Connection_Manager_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::~Net_Connection_Manager_T"));

  // clean up
  {
    // synch access to myConnections
    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

    if (!connections_.is_empty ())
    {
      // *NOTE*: we should NEVER get here !
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%u connections still open --> check implementation !, continuing\n"),
                  connections_.size ()));

      abortConnections ();
    } // end IF
  } // end lock scope
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::initialize (unsigned int maxNumConnections_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::initialize"));

  maxNumConnections_ = maxNumConnections_in;

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("set maximum # connections: %u\n"),
//               maxNumConnections_));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::set (const ConfigurationType& configuration_in,
                                                        const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::set"));

  configuration_ = configuration_in;
  sessionData_ = sessionData_in;

  isInitialized_ = true;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::start"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  isActive_ = true;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::stop (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  if (lockedAccess_in)
    lock_.acquire ();
  isActive_ = false;
  if (lockedAccess_in)
    lock_.release ();
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  return isActive_;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::getData (ConfigurationType& configuration_out,
                                                            SessionDataType& sessionData_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::getData"));

  ACE_ASSERT (isInitialized_);

  configuration_out = configuration_;
  sessionData_out = sessionData_;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::registerConnection (CONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::registerConnection"));

  ACE_ASSERT (isInitialized_);

  // synch access to myConnections
  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

    if (!isActive_)
      return false; // currently rejecting new connections...

    if (connections_.size () >= maxNumConnections_)
    {
      // max reached
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("rejecting connection (maximum count [%u] has been reached), aborting\n"),
      //            maxNumConnections_));

      return false;
    } // end IF

    connection_in->increase ();
    connections_.insert_tail (connection_in);
  } // end lock scope

  return true;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::deregisterConnection (const CONNECTION_T* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::deregisterConnection"));

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
                ACE_TEXT ("failed to remove connection (%@): not found, aborting\n"),
                connection_in));

    return;
  } // end IF

  // clean up
  connection_p->decrease ();

  // if there are no more connections, signal any waiters...
  if (connections_.is_empty () == 1)
    condition_.broadcast ();
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::abortConnections ()
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
      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection
      connection_p->finalize ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection::finalize(), continuing")));
    }
  } while (true);
  ACE_ASSERT (connections_.is_empty ());

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("aborted %u connection(s)\n"),
              num_connections));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::waitConnections () const
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
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
unsigned int
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::numConnections () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::numConnections"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  return static_cast<unsigned int> (connections_.size ());
}

//template <typename ConfigurationType,
//          typename StatisticsContainerType>
//void
//Net_Connection_Manager_T<ConfigurationType,
//                         StatisticsContainerType>::lock ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::lock"));
//	
//	lock_.acquire ();
//}

//template <typename ConfigurationType,
//          typename StatisticsContainerType>
//void
//Net_Connection_Manager_T<ConfigurationType,
//                         StatisticsContainerType>::unlock ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::unlock"));
//	
//	lock_.release ();
//}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
const Net_IConnection_T<ITransportLayerType,
                        StatisticsContainerType>*
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::operator[] (unsigned int index_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::operator[]"));

  // init result
  CONNECTION_T* return_value = NULL;

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check
  if (connections_.is_empty () ||
      (index_in > connections_.size ()))
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("out of bounds (index was: %u), aborting"),
//                index_in));

    return NULL;
  } // end IF

  CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
  for (unsigned int i = 0;
       iterator.next (return_value) && (i < index_in);
       iterator.advance (), i++) {} // end FOR

  // increase reference count
  return_value->increase ();

  return return_value;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::abortOldestConnection ()
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
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::abortNewestConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortNewestConnection"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check: anything to do ?
  if (connections_.is_empty () == 1)
    return;

  // close "newest" connection --> list tail
  CONNECTION_T* connection_p = NULL;
  CONNECTION_CONTAINER_REVERSEITERATOR_T iterator (connections_);
  if (iterator.next (connection_p) == 1)
  {
    try
    {
      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection from a reactor thread, if any
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
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::collect (StatisticsContainerType& data_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::collect"));

  // init result
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  StatisticsContainerType temp;
  // aggregate statistical data
  // *WARNING*: this assumes we're holding our lock !
  CONNECTION_T* connection_p = NULL;
  for (CONNECTION_CONTAINER_ITERATOR_T iterator (const_cast<CONNECTION_CONTAINER_T&> (connections_));
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
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::report"));

  // init result
  StatisticsContainerType result;
  ACE_OS::memset (&result, 0, sizeof (result));

  // synch access to myConnections
  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

    // aggregate data from active connections
    if (!collect (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to RPG_Common_IStatistic::collect(), aborting\n")));

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
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         ITransportLayerType,
                         StatisticsContainerType>::dump_state () const
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
