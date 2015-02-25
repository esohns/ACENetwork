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
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::Net_Connection_Manager_T ()
 : condition_ (lock_)
 , maxNumConnections_ (NET_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS)
 //, connections_ ()
 //, configuration_ ()
 , sessionData_ ()
 , isInitialized_ (false)
 , isActive_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::Net_Connection_Manager_T"));

  // init configuration data
  ACE_OS::memset (&configuration_, 0, sizeof (configuration_));
  //ACE_OS::memset (&sessionData_, 0, sizeof (sessionData_));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
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
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::init (unsigned int maxNumConnections_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::init"));

  maxNumConnections_ = maxNumConnections_in;

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("set maximum # connections: %u\n"),
//               maxNumConnections_));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
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
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::start"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  isActive_ = true;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
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
          typename StatisticsContainerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::stop"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  return isActive_;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
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
          typename StatisticsContainerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::registerConnection (Connection_t* connection_in)
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

    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof(buffer));
    std::string localAddress;
    ACE_INET_Addr local_SAP, remote_SAP;
    try
    {
      connection_in->info (handle,
                           local_SAP,
                           remote_SAP);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection::info(), aborting")));

      return false;
    }
    if (local_SAP.addr_to_string (buffer,
                                  sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    localAddress = buffer;
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    if (remote_SAP.addr_to_string (buffer,
                                   sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

    // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("registered connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %u)...\n"),
                connection_in, reinterpret_cast<unsigned int> (handle),
                ACE_TEXT (localAddress.c_str ()),
                ACE_TEXT (buffer),
                connections_.size ()));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("registered connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %u)...\n"),
                connection_in, handle,
                ACE_TEXT (localAddress.c_str ()),
                ACE_TEXT (buffer),
                connections_.size ()));
#endif
  } // end lock scope

  return true;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::deregisterConnection (const Connection_t* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::deregisterConnection"));

  bool found = false;

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr address1, address2;
  try
  {
    connection_in->info (handle,
                         address1,
                         address2);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IConnection::info(), continuing\n")));
  }

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  Connection_t* connection = NULL;
  for (ConnectionsIterator_t iterator (connections_);
       iterator.next (connection);
       iterator.advance ())
    if (connection == connection_in)
    {
      found = true;

      iterator.remove ();

      // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("deregistered connection [%@/%u] (total: %u)\n"),
                  connection_in, reinterpret_cast<unsigned int> (handle),
                  connections_.size ()));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("deregistered connection [%@/%d] (total: %u)\n"),
                  connection_in, handle,
                  connections_.size ()));
#endif

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
  connection->decrease ();

  // if there are no more connections, signal any waiters...
  if (connections_.is_empty () == 1)
    condition_.broadcast ();
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::abortConnections ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortConnections"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check: anything to do ?
  if (connections_.is_empty ())
    return;

  unsigned int num_connections = connections_.size ();

  Connection_t* connection = NULL;
//  for (ConnectionsIterator_t iterator (connections_);
//       iterator.next (connection);
//       iterator.advance ())
//  {
//    ACE_ASSERT (connection);
//    try
//    {
//      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection
//      connection->abort ();
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Net_IConnection::abort(), continuing")));
//    }
//  } // end FOR
  do
  {
    connection = NULL;
    if (connections_.get (connection, 0) == -1)
      break; // done

    ACE_ASSERT (connection);
    try
    {
      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection
      connection->fini ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection::fini(), continuing")));
    }
  } while (true);
  ACE_ASSERT (connections_.is_empty ());

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("aborted %u connection(s)\n"),
              num_connections));
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
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
          typename StatisticsContainerType>
unsigned int
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
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
          typename StatisticsContainerType>
const typename Net_Connection_Manager_T<ConfigurationType,
                                        SessionDataType,
                                        StatisticsContainerType>::Connection_t*
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::operator[] (unsigned int index_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::operator[]"));

  // init result
  Connection_t* result = NULL;

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check
  if (connections_.is_empty () ||
      (index_in > connections_.size ()))
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("out of bounds (index was: %u), aborting"),
//                index_in));

    return result;
  } // end IF

  ConnectionsIterator_t iterator (const_cast<Connections_t&> (connections_));
  for (unsigned int i = 0;
       iterator.next (result) && (i < index_in);
       iterator.advance (), i++) {} // end FOR

  // increase reference count
  result->increase ();

  return result;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::abortOldestConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortOldestConnection"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check: anything to do ?
  if (connections_.is_empty () == 1)
    return;

  // close "oldest" connection --> list head
  Connection_t* connection = NULL;
  if (connections_.get (connection) != -1)
  {
    try
    {
      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection from a
      // reactor thread, if any
      connection->fini ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection::fini(), continuing")));
    }
  } // end IF
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::abortNewestConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::abortNewestConnection"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  // sanity check: anything to do ?
  if (connections_.is_empty () == 1)
    return;

  // close "newest" connection --> list tail
  Connection_t* connection = NULL;
  ConnectionsReverseIterator_t iterator (connections_);
  if (iterator.next (connection) == 1)
  {
    try
    {
      // *IMPORTANT NOTE*: implicitly invokes deregisterConnection from a reactor thread, if any
      connection->fini ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("caught exception in Net_IConnection::fini(), continuing")));
    }
  } // end IF
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
bool
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::collect (StatisticsContainerType& data_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::collect"));

  // init result
  ACE_OS::memset (&data_out, 0, sizeof (data_out));

  StatisticsContainerType temp;
  // aggregate statistical data
  // *WARNING*: this assumes we're holding our lock !
  Connection_t* connection = NULL;
  for (ConnectionsIterator_t iterator (const_cast<Connections_t&> (connections_));
       iterator.next (connection);
       iterator.advance ())
  {
    ACE_OS::memset (&temp, 0, sizeof (temp));
    try
    { // collect information
      connection->collect (temp);
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
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
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
          typename StatisticsContainerType>
void
Net_Connection_Manager_T<ConfigurationType,
                         SessionDataType,
                         StatisticsContainerType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Connection_Manager_T::dump_state"));

  // synch access to myConnections
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  Connection_t* connection = NULL;
  for (ConnectionsIterator_t iterator (const_cast<Connections_t&> (connections_));
       iterator.next (connection);
       iterator.advance ())
  {
    try
    { // dump connection information
      connection->dump_state ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IConnection::dump_state(), continuing")));
    }
  } // end FOR
}
