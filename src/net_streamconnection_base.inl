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
#include "ace/Global_Macros.h"
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"

#include "common.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::Net_StreamConnectionBase_T (bool managed_in)
 : inherited ()
 , inherited2 (managed_in)
 , stream_ ()
 , sendLock_ ()
 , writeBuffer_ (NULL)
 , notify_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::Net_StreamConnectionBase_T"));

}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::~Net_StreamConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::~Net_StreamConnectionBase_T"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);

  // wait for any worker(s)
  // *TODO*: remove type inference
  if (unlikely (inherited2::configuration_->useThreadPerConnection))
  {
    result = ACE_Task_Base::wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                  id ()));
  } // end IF

  if (unlikely (writeBuffer_))
    writeBuffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::open"));

  bool handle_manager = false;
  bool handle_stream = false;
  ICONNECTOR_T* iconnector_p = NULL;
  ILISTENER_T* ilistener_p = NULL;
  int result = -1;
  AddressType local_SAP, peer_SAP;

  // *TODO*: remove type inferences
  const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  typename StreamType::SESSION_DATA_T* session_data_p = NULL;

  // step0: initialize connection base
  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener

  // *TODO*: remove type inference
  ConfigurationType* configuration_p = NULL;
  switch (this->transportLayer ())
  {
    case NET_TRANSPORTLAYER_TCP:
    {
      switch (this->role ())
      {
        case NET_ROLE_CLIENT:
        {
          iconnector_p = static_cast<ICONNECTOR_T*> (arg_in);
          ACE_ASSERT (iconnector_p);
          configuration_p =
              &const_cast<ConfigurationType&> (iconnector_p->getR ());
          break;
        }
        case NET_ROLE_SERVER:
        {
          ilistener_p = static_cast<ILISTENER_T*> (arg_in);
          ACE_ASSERT (ilistener_p);
          configuration_p =
              &const_cast<ConfigurationType&> (ilistener_p->getR_2 ());
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: invalid/unknown role (was: %d), aborting\n"),
                      id (),
                      this->role ()));
          goto error;
        }
      } // end SWITCH
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      iconnector_p = static_cast<ICONNECTOR_T*> (arg_in);
      ACE_ASSERT (iconnector_p);
      configuration_p =
          &const_cast<ConfigurationType&> (iconnector_p->getR ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), aborting\n"),
                  id (),
                  this->transportLayer ()));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (configuration_p);
  if (unlikely (!inherited2::initialize (*configuration_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_ConnectionBase_T::initialize(): \"%m\", aborting\n"),
                id ()));
    goto error;
  } // end IF

  // step1: initialize/tweak socket, register reading data with reactor, ...
  // *TODO*: remove type inference
  result = inherited::open (configuration_p);
  if (unlikely (result == -1))
  {
    // *NOTE*: this can happen when the connection handle is still registered
    //         with the reactor (i.e. the reactor is still processing events on
    //         a file descriptor that has been closed and is now being reused by
    //         the system)
    // *NOTE*: more likely, this happened because the (select) reactor is out of
    //         "free" (read) slots
    int error = ACE_OS::last_error ();
    ACE_UNUSED_ARG (error);

    // *NOTE*: perhaps max# connections has been reached
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to HandlerType::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  info (inherited2::state_.handle,
        local_SAP, peer_SAP);

  // step2: register with the connection manager ?
  if (likely (inherited2::isManaged_))
  {
    if (unlikely (!inherited2::register_ ()))
    {
      // *NOTE*: (most probably) maximum number of connections has been reached
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
      goto error;
    } // end IF
    handle_manager = true;
  } // end IF

  // step2: initialize/start stream
  // step2a: connect the stream head message queue with the reactor notification
  //         pipe ?
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration_);
  //if (!inherited2::configuration_->socketHandlerConfiguration.useThreadPerConnection)
  //  inherited2::configuration_->streamConfiguration_->configuration_.notificationStrategy =
  //    &(inherited::notificationStrategy_);
  if (unlikely (!stream_.initialize (*(inherited2::configuration_->streamConfiguration_),
                                     inherited2::state_.handle)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF
  if (unlikely (!stream_.initialize_2 (&(inherited::notificationStrategy_),
                                       ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF

  // update session data
  // *TODO*: remove type inferences
  session_data_container_p = &stream_.getR ();
  ACE_ASSERT (session_data_container_p);
  session_data_p =
    &const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->getR ());
  ACE_ASSERT (session_data_p->lock);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *(session_data_p->lock), -1);
    session_data_p->connectionStates.insert (std::make_pair (inherited2::state_.handle,
                                                             &(inherited2::state_)));
  } // end lock scope

  // step2d: start stream
  stream_.start ();
  if (unlikely (!stream_.isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to start processing stream, aborting\n"),
                id ()));
    goto error;
  } // end IF
  handle_stream = true;

//  // step2: start a worker ?
//  if (inherited::configuration_.streamConfiguration.useThreadPerConnection)
//  {
//    ACE_thread_t thread_ids[1];
//    thread_ids[0] = 0;
//    ACE_hthread_t thread_handles[1];
//    thread_handles[0] = 0;
//    char thread_name[BUFSIZ];
//    ACE_OS::memset (thread_name, 0, sizeof (thread_name));
//    ACE_OS::strcpy (thread_name,
//                    ACE_TEXT_ALWAYS_CHAR (NET_CONNECTION_HANDLER_THREAD_NAME));
//    const char* thread_names[1];
//    thread_names[0] = thread_name;
//    result = inherited::activate ((THR_NEW_LWP      |
//                                   THR_JOINABLE     |
//                                   THR_INHERIT_SCHED),                    // flags
//                                  1,                                      // # threads
//                                  0,                                      // force spawning
//                                  ACE_DEFAULT_THREAD_PRIORITY,            // priority
//                                  NET_CONNECTION_HANDLER_THREAD_GROUP_ID, // group id
//                                  NULL,                                   // corresp. task --> use 'this'
//                                  thread_handles,                         // thread handle(s)
//                                  NULL,                                   // thread stack(s)
//                                  NULL,                                   // thread stack size(s)
//                                  thread_ids,                             // thread id(s)
//                                  thread_names);                          // thread name(s)
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Task_Base::activate(): \"%m\", aborting\n")));
//      return -1;
//    } // end IF
//  } // end IF

  inherited2::state_.status = NET_CONNECTION_STATUS_OK;

  return 0;

error:
  if (handle_stream)
    stream_.stop (true,  // wait for completion ?
                  false, // wait for upstream (if any) ?
                  true); // locked access ?

  if (handle_manager)
    inherited2::deregister ();

  inherited2::state_.handle = ACE_INVALID_HANDLE;
  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

  return -1;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::close"));

  int result = -1;

  // [*NOTE*: hereby override the default behavior of a ACE_Svc_Handler,
  // which would call handle_close() AGAIN]

  // *NOTE*: this method will be invoked
  // - by any worker after returning from svc()
  //    --> in this case, this should be a NOP (triggered from handle_close(),
  //        which was invoked by the reactor) - override the default
  //        behavior of a ACE_Svc_Handler, which would call handle_close() AGAIN
  // - by the connector when connect() fails (e.g. connection refused)
  // - by the connector/acceptor when open() fails (e.g. too many connections !)
  inherited2::state_.status = NET_CONNECTION_STATUS_CLOSED;

  switch (arg_in)
  {
    // called by:
    // - any worker from ACE_Task_Base on clean-up
    // - connector when connect() fails (e.g. connection refused)
    // - acceptor/connector when initialization fails (i.e. open() returned -1
    //   due to e.g. too many connections)
    case NORMAL_CLOSE_OPERATION:
    {
      // check specifically for the first case
      result = ACE_OS::thr_equal (ACE_Thread::self (),
                                  inherited::last_thread ());
      if (result)
      {
        // --> worker thread --> NOP
//       if (inherited::module ())
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("\"%s\" worker thread (id: %t) joining...\n"),
//                     ACE_TEXT (inherited::name ())));
//       else
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("worker thread (id: %t) joining...\n")));
        result = 0;

        break;
      } // end IF

      // *WARNING*: falls through !
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. cannot connect, too many connections, ...)
    // *NOTE*: this (eventually) calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION:
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      result = inherited::close (arg_in);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to HandlerType::close(%u): \"%m\", continuing\n"),
                    id (),
                    arg_in));
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid argument (was: %u), aborting\n"),
                  id (),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
}

//template <ACE_SYNCH_DECL,
//          typename HandlerType,
//          typename AddressType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename SocketConfigurationType,
//          typename HandlerConfigurationType,
//          typename ListenerConfigurationType,
//          typename StreamType,
//          typename StreamStatusType,
//          typename TimerManagerType,
//          typename UserDataType>
//int
//Net_StreamConnectionBase_T<ACE_SYNCH_USE,
//                           HandlerType,
//                           AddressType,
//                           ConfigurationType,
//                           StateType,
//                           StatisticContainerType,
//                           SocketConfigurationType,
//                           HandlerConfigurationType,
//                           ListenerConfigurationType,
//                           StreamType,
//                           StreamStatusType,
//                           TimerManagerType,
//                           UserDataType>::handle_input (ACE_HANDLE handle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::handle_input"));
//
//  int result = -1;
//  ssize_t bytes_received = -1;
//  ACE_Message_Block* message_block_p = NULL;
//
//  // sanity check
//  ACE_ASSERT (inherited2::configuration_);
//  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
//
//  // read some data from the socket
//  // *TODO*: remove type inference
//  message_block_p =
//    allocateMessage (inherited2::configuration_->streamConfiguration->allocatorConfiguration_.defaultBufferSize);
//  if (unlikely (!message_block_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%u: failed to allocateMessage(%u), aborting\n"),
//                id (),
//                inherited2::configuration_->streamConfiguration->allocatorConfiguration_.defaultBufferSize));
//    return -1; // <-- remove 'this' from dispatch
//  } // end IF
//
//  // read some data from the socket
//retry:
//  bytes_received =
//    inherited::peer_.recv (message_block_p->wr_ptr (),   // buffer
//                           message_block_p->capacity (), // #bytes to read
//                           0);                           // flags
//  switch (bytes_received)
//  {
//    case -1:
//    {
//      // *IMPORTANT NOTE*: a number of issues can occur here:
//      // - connection reset by peer
//      // - connection abort()ed locally
//      // [- data buffer is empty (SSL)]
//      int error = ACE_OS::last_error ();
//      if (error == EWOULDBLOCK)      // 11: SSL_read() failed (buffer is empty)
//        goto retry;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (//(error != EPIPE)        && // 9    : write() on a close()d socket *TODO*
//          (error != ECONNABORTED) && // 10053: local close()
//          (error != ECONNRESET))     // 10054: peer closed the connection
//#else
//      if ((error != EPIPE)        && // 32: connection reset by peer (write)
//          // -------------------------------------------------------------------
//          (error != EBADF)        && // 9
//          (error != ENOTSOCK)     && // 88
//          (error != ECONNABORTED) && // 103: connection abort()ed locally
//          (error != ECONNRESET))     // 104: connection reset by peer (read)
//#endif
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%u: failed to ACE_(SSL_)SOCK_Stream::recv() (handle was: 0x%@): \"%m\", aborting\n"),
//                    id (),
//                    handle_in));
//#else
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%u: failed to ACE_(SSL_)SOCK_Stream::recv() (handle was: %d): \"%m\", aborting\n"),
//                    id (),
//                    handle_in));
//#endif
//
//      // clean up
//      message_block_p->release ();
//
//      // *TODO*: remove type inference
//      if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
//        inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
//
//      return -1; // <-- remove 'this' from dispatch
//    }
//    // *** GOOD CASES ***
//    case 0:
//    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%u: socket (handle was: 0x%@) has been closed by the peer\n"),
//                  id (),
//                  handle_in));
//#else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%u: socket (handle was: %d) has been closed by the peer\n"),
//                  id (),
//                  handle_in));
//#endif
//
//      // clean up
//      message_block_p->release ();
//
//      // *TODO*: remove type inference
//      if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
//        inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
//
//      return -1; // <-- remove 'this' from dispatch
//    }
//    default:
//    {
////       ACE_DEBUG ((LM_DEBUG,
////                   ACE_TEXT ("[%d]: received %u bytes\n"),
////                   handle_in,
////                   bytes_received));
//
//      // adjust write pointer
//      message_block_p->wr_ptr (static_cast<size_t> (bytes_received));
//
//      break;
//    }
//  } // end SWITCH
//
//  // push the buffer onto the stream for processing
//  // *NOTE*: the stream assumes ownership of the buffer
//  result = stream_.put (message_block_p, NULL);
//  if (unlikely (result == -1))
//  {
//    int error = ACE_OS::last_error ();
//    if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%u: failed to ACE_Stream::put(): \"%m\", aborting\n"),
//                  id ()));
//
//    // clean up
//    message_block_p->release ();
//
//    return -1; // <-- remove 'this' from dispatch
//  } // end IF
//
//  return result;
//}

//template <ACE_SYNCH_DECL,
//          typename HandlerType,
//          typename AddressType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename SocketConfigurationType,
//          typename HandlerConfigurationType,
//          typename ListenerConfigurationType,
//          typename StreamType,
//          typename StreamStatusType,
//          typename TimerManagerType,
//          typename UserDataType>
//int
//Net_StreamConnectionBase_T<ACE_SYNCH_USE,
//                           HandlerType,
//                           AddressType,
//                           ConfigurationType,
//                           StateType,
//                           StatisticContainerType,
//                           SocketConfigurationType,
//                           HandlerConfigurationType,
//                           ListenerConfigurationType,
//                           StreamType,
//                           StreamStatusType,
//                           TimerManagerType,
//                           UserDataType>::handle_output (ACE_HANDLE handle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::handle_output"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited2::configuration_);
//
//  int result = -1;
//  ssize_t bytes_sent = 0;
//
//  // *NOTE*: in a threaded environment, workers could be dispatching the reactor
//  //         notification queue concurrently (most notably, ACE_TP_Reactor)
//  //         --> enforce proper serialization here
//  // *IMPORTANT NOTE*: the ACE documentation (books) explicitly claims that
//  //                   measures are in place to prevent concurrent dispatch of
//  //                   the same handler for a specific handle by different
//  //                   threads (*TODO*: find reference). If this is indeed true,
//  //                   this test may be removed (just make sure this holds for
//  //                   both:
//  //                   - the socket dispatch implementation
//  //                   - the (piped) notification dispatch implementation
//  // *TODO*: remove type inferences
//  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, sendLock_, -1);
//  if (unlikely (writeBuffer_))
//    goto continue_;
//
//  // dispatch next chunk from the queue/stream
//  // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
//  //                   been notified to the reactor
//  result =
//    (unlikely (inherited2::configuration_->socketHandlerConfiguration.useThreadPerConnection) ? inherited::getq (writeBuffer_, NULL)
//                                                                                              : stream_.get (writeBuffer_, NULL));
//  if (unlikely (result == -1))
//  {
//    // *NOTE*: a number of issues can occur here:
//    //         - connection has been closed in the meantime
//    //         - queue has been deactivated
//    int error = ACE_OS::last_error ();
//    if ((error != EAGAIN)   && // 11   : connection has been closed
//        (error != ESHUTDOWN))  // 10058: queue has been deactivated
//      ACE_DEBUG ((LM_ERROR,
//                  (inherited2::configuration_->socketHandlerConfiguration.useThreadPerConnection ? ACE_TEXT ("%u: failed to ACE_Task::getq(): \"%m\", aborting\n")
//                                                                                                 : ACE_TEXT ("%u: failed to ACE_Stream::get(): \"%m\", aborting\n")),
//                  id ()));
//    return -1; // <-- remove 'this' from dispatch
//  } // end IF
//continue_:
//  ACE_ASSERT (writeBuffer_);
//
//  // finished ?
//  if (unlikely (inherited2::configuration_->socketHandlerConfiguration.useThreadPerConnection &&
//                (writeBuffer_->msg_type () == ACE_Message_Block::MB_STOP)))
//  {
////       ACE_DEBUG ((LM_DEBUG,
////                   ACE_TEXT ("[%u]: finished sending\n"),
////                   peer_.get_handle ()));
//
//    // clean up
//    writeBuffer_->release ();
//    writeBuffer_ = NULL;
//
//    return -1; // <-- remove 'this' from dispatch
//  } // end IF
//
//  // place data into the socket
//  // *TODO*: the iovec-based implementation kept blocking in
//  //         ACE::handle_write_ready(), i.e. obviously currently does not work
//  //         with multi-threaded (thread pool) reactors...
//  //         --> use 'traditional' method for now
//  //size_t bytes_transferred = 0;
//  //bytes_sent =
//  //    inherited::peer_.send_n (writeBuffer_, // buffer
//  //                             NULL,                // timeout
//  //                             &bytes_transferred); // bytes transferred
//  bytes_sent =
//    inherited::peer_.send_n (writeBuffer_->rd_ptr (),                    // data
//                             static_cast<int> (writeBuffer_->length ()), // #bytes to send
//                             static_cast<int> (0));                      // flags
//  switch (bytes_sent)
//  {
//    case -1:
//    {
//      // *NOTE*: a number of issues can occur here:
//      //         - connection reset by peer
//      //         - connection abort()ed locally
//      int error = ACE_OS::last_error ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (//(error != EPIPE)        && // 9    : write() on a close()d socket *TODO*
//          (error != ECONNABORTED) && // 10053: local close()
//          (error != ECONNRESET))     // 10054: peer closed the connection
//#else
//      if ((error != EPIPE)        && // <-- connection reset by peer
//          // -------------------------------------------------------------------
//          (error != EBADF)        &&
//          (error != ENOTSOCK)     &&
//          (error != ECONNABORTED) && // <-- connection abort()ed locally
//          (error != ECONNRESET))
//#endif
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%u: failed to ACE_SOCK_Stream::send_n() (handle was: 0x%@): \"%m\", aborting\n"),
//                    id (),
//                    handle_in));
//#else
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%u: failed to ACE_SOCK_Stream::send_n() (handle was: %d): \"%m\", aborting\n"),
//                    id (),
//                    handle_in));
//#endif
//
//      // clean up
//      writeBuffer_->release ();
//      writeBuffer_ = NULL;
//
//      // *TODO*: remove type inference
//      if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
//        inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
//
//      return -1; // <-- remove 'this' from dispatch
//    }
//    // *** GOOD CASES ***
//    case 0:
//    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%u: socket (handle was: 0x%@) has been closed by the peer\n"),
//                  id (),
//                  handle_in));
//#else
//      ACE_DEBUG ((LM_DEBUG,
//                  id (),
//                  ACE_TEXT ("%u: socket (handle was: %d) has been closed by the peer\n"),
//                  handle_in));
//#endif
//
//      // clean up
//      writeBuffer_->release ();
//      writeBuffer_ = NULL;
//
//      // *TODO*: remove type inference
//      if (likely (inherited2::state_.status == NET_CONNECTION_STATUS_OK))
//        inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
//
//      return -1; // <-- remove 'this' from dispatch
//    }
//    default:
//    {
////#if defined (ACE_WIN32) || defined (ACE_WIN64)
////      ACE_DEBUG ((LM_DEBUG,
////                  ACE_TEXT ("0x%@: sent %u bytes\n"),
////                  handle_in,
////                  bytes_sent));
////#else
////      ACE_DEBUG ((LM_DEBUG,
////                  ACE_TEXT ("%d: sent %u bytes\n"),
////                  handle_in,
////                  bytes_sent));
////#endif
//
//      // finished with this buffer ?
//      writeBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
//      if (unlikely (writeBuffer_->length () > 0))
//        break; // there's more data
//
//      // clean up
//      writeBuffer_->release ();
//      writeBuffer_ = NULL;
//
//      result = 0;
//
//      break;
//    }
//  } // end SWITCH
//
//  // immediately reschedule handler ?
//  if (unlikely (writeBuffer_))
//    result = 1; // <-- reschedule 'this' immediately
//
//  return result;
//}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                        ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::handle_close"));

  int result = -1;

  // step0a: set state
  // *IMPORTANT NOTE*: set the state early to avoid deadlock in
  //                   waitForCompletion() (see below), which may be implicitly
  //                   invoked by stream_.finished()
  // *TODO*: remove type inference
  if ((inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED) && // initialization failed ?
      (inherited2::state_.status != NET_CONNECTION_STATUS_CLOSED))
    inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;

  // step0b: notify stream ?
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock (), -1);
    if (likely (notify_ &&
                (inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED)))
    {
      notify_ = false;
      stream_.notify (STREAM_SESSION_MESSAGE_DISCONNECT,
                      true);
    } // end IF

    // step1: shut down the processing stream
    stream_.flush (false,  // do not flush inbound data
                   false,  // do not flush session messages
                   false); // flush upstream (if any)
    stream_.idle ();
    stream_.stop (true,  // wait for worker(s) (if any)
                  false, // wait for upstream (if any)
                  true); // locked access ?
  } // end lock scope

  switch (mask_in)
  {
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed (receive failed)
//    case ACE_Event_Handler::ACCEPT_MASK:
    case ACE_Event_Handler::WRITE_MASK:      // --> socket has been closed (send failed) (DevPoll)
    case ACE_Event_Handler::EXCEPT_MASK:     // --> socket has been closed (send failed)
    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused) /
                                             //     accept failed (e.g. too many connections) /
                                             //     select failed (EBADF see Select_Reactor_T.cpp) /
                                             //     user abort
    {
      // sanity check(s)
      ACE_ASSERT (inherited2::configuration_);

      // step1: signal completion and wait for all processing
      // *IMPORTANT NOTE*: when the socket closes, any dispatching threads
      //                   currently servicing the socket handle will call
      //                   handle_close()
//      stream_.flush (false,  // flush inbound ?
//                     false,  // flush session messages ?
//                     false); // flush upstream ?
//      stream_.wait (true,   // wait for worker(s) (if any)
//                    false,  // wait for upstream (if any)
//                    false); // wait for downstream (if any)

      // step2: purge any pending (!) notifications ?
      // *TODO*: remove type inference
      if (likely (!inherited2::configuration_->useThreadPerConnection))
      { // *IMPORTANT NOTE*: in a multithreaded environment, in particular when
        //                   using a multithreaded reactor, there may still be
        //                   in-flight notifications being dispatched at this
        //                   stage. In that case, do not rely on releasing all
        //                   handler resources "manually", use reference
        //                   counting instead
        //                   --> this just speeds things up a little
        ACE_Reactor* reactor_p = inherited::reactor ();
        ACE_ASSERT (reactor_p);
        result =
            reactor_p->purge_pending_notifications (this,
                                                    ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: failed to ACE_Reactor::purge_pending_notifications(0x%@,%d): \"%m\", continuing\n"),
                      id (),
                      this,
                      ACE_Event_Handler::ALL_EVENTS_MASK));
        //else if (result > 0)
        //  ACE_DEBUG ((LM_DEBUG,
        //              ACE_TEXT ("flushed %d notifications (handle was: %d)\n"),
        //              result,
        //              handle_in));
      } // end IF

      break;
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: Net_StreamTCPSocketBase_T::handle_close(0x%@,%d) called for unknown reasons --> check implementation !, continuing\n"),
                  id (),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: Net_StreamTCPSocketBase_T::handle_close(%d,%d) called for unknown reasons --> check implementation !, continuing\n"),
                  id (),
                  handle_in,
                  mask_in));
#endif
      break;
    }
  } // end SWITCH

  // step3: invoke base class maintenance
  bool deregister = inherited2::isRegistered_;
  // *IMPORTANT NOTE*: retain the socket handle:
  //                   - otherwise this fails for the usecase "accept failed"
  //                     (see above)
  //                   - until deregistered from the manager
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                id (),
                handle_in,
                mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                id (),
                handle_in,
                mask_in));
#endif
  } // end IF
  inherited::set_handle (handle_in); // reset for debugging purposes

  // step4: deregister with the connection manager (if any)
  if (likely (deregister))
    inherited2::deregister ();

  return result;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: 0x%@]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: %d]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#endif
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::info (ACE_HANDLE& handle_out,
                                                AddressType& localSAP_out,
                                                AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::info"));

  int result = -1;
  int error = 0;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  handle_out = inherited::get_handle ();
#else
  handle_out = inherited::get_handle ();
#endif

  result = inherited::peer_.get_local_addr (localSAP_out);
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: TCP: socket already closed
#else
    if (error != EBADF)    // 9    : Linux: socket already closed
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_SOCK::get_local_addr(): \"%m\", continuing\n"),
                  id ()));
  } // end IF
  result = inherited::peer_.get_remote_addr (remoteSAP_out);
  // *NOTE*: peer may have disconnected already --> not an error
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENOTSOCK) && // 10038: (TCP) socket already closed
        (error != ENOTCONN))   // 10057: (UDP) socket is not connected
#else
    if ((error != EBADF) &&    // 9    : Linux: socket already closed
        (error != ENOTCONN))   // 107  : Linux: perhaps UDP ?
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_SOCK::get_remote_addr(): \"%m\", continuing\n"),
                  id ()));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::close"));

  int result = -1;

  // step1: stop the stream; no new outbound data will be dispatched
  stream_.stop (false, // wait for completion ?
                false, // recurse upstream (if any) ?
                true); // locked access ?

  //  step2: wake up any open read operation; it will release the connection
  ACE_HANDLE handle = inherited::get_handle ();
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::closesocket (handle);
    if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                  id (),
                  handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                  id (),
                  handle));
#endif
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  stream_.wait (waitForThreads_in,
                false,             // wait for upstream ?
                false);            // wait for downstream ?

  // *NOTE*: all data has been dispatched to the reactor (i.e. kernel)

  // step2: wait for the kernel to place the data onto the wire
  // *TODO*: platforms may implement different methods by which this can be
  //         established (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  // *TODO*: remove type inference
  if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
  {
    ACE_HANDLE handle = inherited::get_handle ();
    ACE_ASSERT (handle != ACE_INVALID_HANDLE);
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::set (enum Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::set"));

  bool result = false;

  // sanity check(s)
  SocketConfigurationType socket_configuration;
  //// *TODO*: remove type inference
  //if (configuration_.socketConfiguration)
  //  socket_configuration = *configuration_.socketConfiguration;

  try {
    result = initialize (this->dispatch (),
                         role_in,
                         socket_configuration);
                         //*configuration_.socketConfiguration);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::send"));

  int result = -1;
  Stream_Module_t* module_p = NULL;
  Stream_Task_t* task_p = NULL;

  // *NOTE*: feed the data into the stream (queue of the head module 'reader'
  //         task)
  module_p = stream_.head ();
  ACE_ASSERT (module_p);
  task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  //result = task_p->reply (message_inout, NULL);
  result = task_p->put (message_inout, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: failed to ACE_Task::put(): \"%m\", returning\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                module_p->name ()));
    goto error;
  } // end IF

  return;

error:
  message_inout->release (); message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
bool
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           //UserDataType>::get () const
                           TimerManagerType,
                           UserDataType>::wait (StreamStatusType state_in,
                                                const ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::wait"));

  Stream_Module_t* top_module_p = NULL;
  int result = stream_.top (top_module_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                id ()));
    return false;
  } // end IF
  ACE_ASSERT (top_module_p);

  Stream_IStateMachine_t* istatemachine_p =
      dynamic_cast<Stream_IStateMachine_t*> (top_module_p->writer ());
  if (unlikely (!istatemachine_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: failed to dynamic_cast<Stream_IStateMachine_t>(0x%@), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                top_module_p->writer ()));
    return false;
  } // end IF

  bool result_2 = false;
  try {
    result_2 = istatemachine_p->wait (state_in,
                                      timeValue_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: caught exception in Stream_IStateMachine_t::wait(%d,%T#), continuing\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                state_in,
                timeValue_in));
  }

  return result_2;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
ACE_Message_Block*
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           SocketConfigurationType,
                           HandlerConfigurationType,
                           ListenerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           TimerManagerType,
                           UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration_);

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  id (),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator->block ())
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
    if (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                    id ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                  id ()));
  } // end IF

  return message_block_p;
}

/////////////////////////////////////////

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::Net_AsynchStreamConnectionBase_T (bool managed_in)
 : inherited ()
 , inherited2 (managed_in)
 , stream_ ()
 , notify_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::Net_AsynchStreamConnectionBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::act (const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::act"));

  // initialize this connection

  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener
  ConfigurationType* configuration_p = NULL;
  SocketConfigurationType* socket_configuration_p = NULL;
  const ICONNECTOR_T* iconnector_p = NULL;
  const ILISTENER_T* ilistener_p = NULL;
  switch (this->transportLayer ())
  {
    case NET_TRANSPORTLAYER_TCP:
    {
      switch (this->role ())
      {
        case NET_ROLE_CLIENT:
        {
          iconnector_p = static_cast<const ICONNECTOR_T*> (act_in);
          ACE_ASSERT (iconnector_p);
          configuration_p =
            &const_cast<ConfigurationType&> (iconnector_p->getR ());
          break;
        }
        case NET_ROLE_SERVER:
        {
          ilistener_p = static_cast<const ILISTENER_T*> (act_in);
          ACE_ASSERT (ilistener_p);
          configuration_p =
            &const_cast<ConfigurationType&> (ilistener_p->getR_2 ());
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: invalid/unknown role (was: %d), returning\n"),
                      id (),
                      this->role ()));
          return;
        }
      } // end SWITCH

      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      iconnector_p = static_cast<const ICONNECTOR_T*> (act_in);
      ACE_ASSERT (iconnector_p);
      configuration_p =
        &const_cast<ConfigurationType&> (iconnector_p->getR ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), returning\n"),
                  id (),
                  this->transportLayer ()));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (configuration_p);
  socket_configuration_p = configuration_p;

  if (unlikely (!inherited::initialize (*socket_configuration_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_SocketHandlerBase_T::initialize(): \"%m\", returning\n"),
                id ()));
    return;
  } // end IF
  if (!inherited2::isManaged_)
    if (unlikely (!inherited2::initialize (*configuration_p)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to Net_ConnectionBase_T::initialize(): \"%m\", returning\n"),
                  id ()));
      return;
    } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::open (ACE_HANDLE handle_in,
                                                      ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::open"));

  // sanity check(s)
  // *NOTE*: handle_in is ACE_INVALID_HANDLE for UDP connections
  //ACE_ASSERT (handle_in == ACE_INVALID_HANDLE);

  bool handle_manager = false;
  bool handle_stream = false;
  const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  typename StreamType::SESSION_DATA_T* session_data_p = NULL;
  AddressType local_SAP, peer_SAP;

  // *NOTE*: act() has been called already, Net_ConnectionBase has been
  //         initialized

  // step1: initialize/tweak socket, initialize I/O, ...
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::isInitialized_);
  inherited::open (handle_in,
                   messageBlock_in);
  if (unlikely (!inherited::isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to HandlerType::open (handle was: %u), aborting\n"),
                id (),
                handle_in));
    goto error;
  } // end IF
  this->info (inherited2::state_.handle,
              local_SAP, peer_SAP);

  // step2: register with the connection manager ?
  if (likely (inherited2::isManaged_))
  {
    if (unlikely (!inherited2::register_ ()))
    {
      // *NOTE*: (most probably) maximum number of connections has been reached
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
      goto error;
    } // end IF
    handle_manager = true;
  } // end IF

  // step2: initialize/start stream
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration_);
  // step2a: connect the stream head message queue with this handler; the queue
  //         will forward outbound data to handle_output ()
  //inherited2::configuration_->streamConfiguration_->configuration_.notificationStrategy =
  //  this;
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock ());
    if (unlikely (!stream_.initialize (*(inherited2::configuration_->streamConfiguration_),
                                       inherited2::state_.handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                  id (),
                  ACE_TEXT (stream_.name ().c_str ())));
      goto error;
    } // end IF
    if (unlikely (!stream_.initialize_2 (this,
                                         ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"))))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                  id (),
                  ACE_TEXT (stream_.name ().c_str ())));
      goto error;
    } // end IF

    // step2b: update session data
    // *TODO*: remove type inferences
    session_data_container_p = &stream_.getR ();
    session_data_p =
      &const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->getR ());
    ACE_ASSERT (session_data_p->lock);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);
      session_data_p->connectionStates.insert (std::make_pair (inherited2::state_.handle,
                                                              &(inherited2::state_)));
    } // end lock scope

    // step2c: start stream
    stream_.start ();
    if (unlikely (!stream_.isRunning ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to start processing stream, aborting\n"),
                  id ()));
      goto error;
    } // end IF
  } // end lock scope
  handle_stream = true;

  inherited2::state_.status = NET_CONNECTION_STATUS_OK;

  return;

error:
  inherited2::state_.handle = ACE_INVALID_HANDLE;
  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

  if (handle_stream)
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock ());
    stream_.stop (true,  // wait for completion ?
                  true); // locked access ?
  } // end IF / lock scope

  if (handle_manager)
    inherited2::deregister ();
}

//template <typename HandlerType,
//          typename AddressType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename SocketConfigurationType,
//          typename HandlerConfigurationType,
//          typename ListenerConfigurationType,
//          typename StreamType,
//          typename StreamStatusType,
//          typename TimerManagerType,
//          typename UserDataType>
//int
//Net_AsynchStreamConnectionBase_T<HandlerType,
//                                 AddressType,
//                                 ConfigurationType,
//                                 StateType,
//                                 StatisticContainerType,
//                                 SocketConfigurationType,
//                                 HandlerConfigurationType,
//                                 ListenerConfigurationType,
//                                 StreamType,
//                                 StreamStatusType,
//                                 TimerManagerType,
//                                 UserDataType>::handle_output (ACE_HANDLE handle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_output"));
//
//  ACE_UNUSED_ARG (handle_in);
//
//  int result = -1;
//  int error = 0;
//  Common_IReferenceCount* ireference_count_p = this;
//  Stream_Base_t* stream_p = stream_.upStream ();
//  ACE_Message_Block* message_block_p = NULL;
//
//  // sanity check(s)
//  if (likely (!stream_p))
//    stream_p = &stream_;
//  ACE_ASSERT (stream_p);
//
////  if (!inherited::buffer_)
////  {
//    // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
//    //                   been notified
////    result = stream_p->get (inherited::buffer_, NULL);
//    result = stream_p->get (message_block_p, NULL);
//    if (unlikely (result == -1))
//    { // *NOTE*: most probable reason: socket has been closed by the peer, which
//      //         close()s the processing stream (see: handle_close()),
//      //         shutting down the message queue
//      error = ACE_OS::last_error ();
//      if (error != ESHUTDOWN) // 108: happens on Linux
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%u: failed to ACE_Stream::get(): \"%m\", aborting\n"),
//                    id ()));
//      return -1;
//    } // end IF
////  } // end IF
////  ACE_ASSERT (inherited::buffer_);
//    ACE_ASSERT (message_block_p);
//
//  // start (asynchronous) write
//  inherited2::increase ();
//  inherited::counter_.increase ();
//send:
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  result =
////    inherited::outputStream_.writev (*inherited::buffer_,                  // data
////                                     inherited::buffer_->length (),        // bytes to write
//      inherited::outputStream_.writev (*message_block_p,                     // data
//                                       message_block_p->length (),           // bytes to write
//                                       NULL,                                 // ACT
//                                       0,                                    // priority
//                                       COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
//#else
//  result =
////    inherited::outputStream_.write (*inherited::buffer_,                  // data
////                                    inherited::buffer_->length (),        // bytes to write
//      inherited::outputStream_.write (*message_block_p,                     // data
//                                      message_block_p->length (),           // bytes to write
//                                      NULL,                                 // ACT
//                                      0,                                    // priority
//                                      COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
//#endif
//  if (unlikely (result == -1))
//  {
//    error = ACE_OS::last_error ();
//    // *WARNING*: this could fail on multi-threaded proactors
//    if (error == EAGAIN) goto send; // 11   : happens on Linux
//    if ((error != ENOTSOCK)     &&  // 10038: happens on Win32
//        (error != ECONNABORTED) &&  // 10053: happens on Win32
//        (error != ECONNRESET)   &&  // 10054: happens on Win32
//        (error != ENOTCONN))        // 10057: happens on Win32
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%u: failed to ACE_Asynch_Write_Stream::writev(%u): \"%m\", aborting\n"),
//                  id (),
//                  message_block_p->length ()));
//
//    // clean up
////    inherited::buffer_->release ();
////    inherited::buffer_ = NULL;
//    message_block_p->release ();
//    inherited2::decrease ();
//    inherited::counter_.decrease ();
//  } // end IF
////  else if (result == 0)
////  {
////#if defined (ACE_WIN32) || defined (ACE_WIN64)
////    ACE_DEBUG ((LM_DEBUG,
////                ACE_TEXT ("0x%@: socket was closed\n"),
////                handle_in));
////#else
////    ACE_DEBUG ((LM_DEBUG,
////                ACE_TEXT ("%d: socket was closed\n"),
////                handle_in));
////#endif
////  } // end IF
//
//  return result;
//}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
int
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                              ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_close"));

  int result = -1;
  bool cancel_b = false;
  bool close_socket_b = false;

  // *IMPORTANT NOTE*: when control reaches here, the socket handle has already
  //                   gone away, i.e. no new data will be accepted by the
  //                   kernel / forwarded by the proactor
  //                   --> finish processing: flush all remaining outbound data
  //                       and wait for all workers within the stream
  //                   [--> cancel any pending asynchronous operations]

  // step0a: set state
  // *IMPORTANT NOTE*: set the state early to avoid deadlock in
  //                   waitForCompletion() (see below), which may be implicitly
  //                   invoked by stream_.finished()
  // *TODO*: remove type inference
  if ((inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED) && // initialization failed ?
      (inherited2::state_.status != NET_CONNECTION_STATUS_CLOSED))
  {
    inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
    cancel_b = true;
  } // end IF

  // step0b: notify stream ?
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock (), -1);
    if (likely (notify_ &&
                (inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED)))
    {
      notify_ = false;
      stream_.notify (STREAM_SESSION_MESSAGE_DISCONNECT,
                      true);
    } // end IF

    // step1: shut down the processing stream
    stream_.flush (false,  // do not flush inbound data
                   false,  // do not flush session messages
                   false); // flush upstream (if any)
    stream_.idle ();
    stream_.stop (true,  // wait for worker(s) (if any)
                  false, // wait for upstream (if any)
                  true); // locked access ?
  } // end lock scope

  // *NOTE*: pending socket operations are notified by the kernel and will
  //         return automatically
  // *TODO*: consider cancel()ling pending write operations

  // step2: invoke base-class maintenance
  // step2a: cancel pending operation(s) ?
  if (cancel_b)
    inherited::cancel ();

  // step2b: deregister with the connection manager (if any) ?
//  Net_ConnectionId_t connection_id = id ();
  close_socket_b = (inherited2::count () == 1);
  if (likely (inherited2::isRegistered_))
    inherited2::deregister ();

  // step2c: release socket handle(s) ?
  result =
    (close_socket_b ? inherited::handle_close (handle_in,
                                               mask_in)
                    : 0);

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  this->info (handle,
              local_address,
              peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: 0x%@]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: %d]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::close"));

  inherited2::state_.status = NET_CONNECTION_STATUS_CLOSED;

  // step1: cancel i/o operation(s), release (write) socket handle, ...
  inherited::cancel ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  stream_.wait (waitForThreads_in,
                false,             // wait for upstream ?
                false);            // wait for downstream ?

  // *NOTE*: all data has been dispatched to the reactor (i.e. kernel)

  // step2: wait for the kernel to place the data onto the wire
  // *TODO*: platforms may implement different methods by which this can be
  //         established (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  // *TODO*: remove type inference
  if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
  {
    ACE_HANDLE handle = inherited::handle ();
    ACE_ASSERT (handle != ACE_INVALID_HANDLE);
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::set (enum Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::set"));

  bool result = false;

  // sanity check(s)
  SocketConfigurationType socket_configuration;
  //// *TODO*: remove type inference
  //if (configuration_.socketConfiguration)
  //  socket_configuration = *configuration_.socketConfiguration;

  try {
    result = initialize (this->dispatch (),
                         role_in,
                         socket_configuration);
                         //*configuration_.socketConfiguration);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
  }
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::send"));

  int result = -1;
  Stream_Module_t* module_p = NULL;
  Stream_Task_t* task_p = NULL;

  // *NOTE*: feed the data into the stream (queue of the head module 'reader'
  //         task)
  module_p = stream_.head ();
  ACE_ASSERT (module_p);
  task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  //result = task_p->reply (message_inout, NULL);
  result = task_p->put (message_inout, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s%s: failed to ACE_Task::put(): \"%m\", returning\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                module_p->name ()));
    goto error;
  } // end IF

  return;

error:
  message_inout->release (); message_inout = NULL;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::waitForIdleState () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::waitForIdleState"));

  // step1: wait for the streams' 'outbound' queue to turn idle
  stream_.idle ();

  // --> outbound stream data has been processed

  // step2: wait for any asynchronous operations to complete
  inherited::counter_.wait (0);

  // --> all data has been dispatched to the kernel (socket)
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
bool
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 //UserDataType>::get () const
                                 TimerManagerType,
                                 UserDataType>::wait (StreamStatusType state_in,
                                                      const ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::wait"));

  Stream_Module_t* top_module_p = NULL;
  int result = stream_.top (top_module_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                id ()));
    return false;
  } // end IF
  ACE_ASSERT (top_module_p);

  Stream_IStateMachine_t* istatemachine_p =
      dynamic_cast<Stream_IStateMachine_t*> (top_module_p->writer ());
  if (unlikely (!istatemachine_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: failed to dynamic_cast<Stream_IStateMachine_t>(0x%@), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                top_module_p->writer ()));
    return false;
  } // end IF

  bool result_2 = false;
  try {
    result_2 = istatemachine_p->wait (state_in,
                                      timeValue_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: caught exception in Stream_IStateMachine_t::wait(%d,%T#), continuing\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                state_in,
                timeValue_in));
  }

  return result_2;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
ACE_Message_Block*
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration_);

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  id (),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator->block ())
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
    if (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration_->configuration_.messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                    id ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                  id ()));
  } // end IF

  return message_block_p;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_read_stream"));

  int result = -1;

  // sanity check
  if (unlikely (!result_in.success ()))
  {
    // connection closed/reset (by peer) ? --> not an error
    unsigned long error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ERROR_NETNAME_DELETED)   && // 64  : peer close()
        (error != ERROR_OPERATION_ABORTED) && // 995 : local close()
        (error != ERROR_CONNECTION_ABORTED))  // 1236: local close()
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (handle was: 0x%@): \"%s\", continuing\n"),
                  id (), result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
    if ((error != EBADF)     && // 9  : local close(), happens on Linux
        (error != EPIPE)     && // 32 : happens on Linux
        (error != ECONNRESET))  // 104: happens on Linux
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (handle was: %d): \"%s\", continuing\n"),
                  id (), result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#endif
  } // end IF

  switch (static_cast<int> (result_in.bytes_transferred ()))
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      unsigned long error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely ((error != ERROR_NETNAME_DELETED)   &&  // 64  : peer close()
                    (error != ERROR_OPERATION_ABORTED) &&  // 995 : cancel()led
                    (error != ERROR_CONNECTION_ABORTED)))  // 1236: local close()
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to read from input stream (handle was: 0x%@): \"%s\", returning\n"),
                    id (), result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#else
      if (unlikely ((error != EBADF)     && // 9  : local close(), happens on Linux
                    (error != EPIPE)     && // 32 : happens on Linux
                    (error != ECONNRESET)))  // 104: happens on Linux
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to read from input stream (handle was: %d): \"%s\", returning\n"),
                    id (), result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#endif
      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%u: socket (handle was: 0x%@) has been closed\n"),
                  id (), result_in.handle ()));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%u: socket (handle was: %d) has been closed\n"),
                  id (), result_in.handle ()));
#endif
      break;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%d]: received %u bytes\n"),
//                   result.handle (),
//                   result.bytes_transferred ()));

      // push the buffer onto the stream for processing
      result = stream_.put (&result_in.message_block (), NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u/%s: failed to ACE_Stream::put(): \"%m\", returning\n"),
                    id (), ACE_TEXT (stream_.name ().c_str ())));
        break;
      } // end IF

      // start next read
      if (unlikely (!inherited::initiate_read ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to Net_IAsynchSocketHandler::initiate_read(): \"%m\", returning\n"),
                    id ()));
        break;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ().release ();

  Net_ConnectionId_t id = this->id ();
  // *WARNING*: most likely 'delete''s 'this'
  result =
      handle_close (result_in.handle (),
                    ((result_in.bytes_transferred () == 0) ? ACE_Event_Handler::READ_MASK // peer closed the connection
                                                           : ACE_Event_Handler::ALL_EVENTS_MASK));
  if (unlikely (result == -1))
  {
//    error = ACE_OS::last_error ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if ((error != ENOENT)                  && // 2   :
//        (error != ENOMEM)                  && // 12  : [server: local close()], [client: peer reset ()]
//        (error != ERROR_IO_PENDING)        && // 997 :
//        (error != ERROR_CONNECTION_ABORTED))  // 1236: [client: local close()]
//#else
//    if (error == EINPROGRESS) result = 0; // --> AIO_CANCELED
//    if ((error != ENOENT)     && // 2  :
//        (error != EPIPE)      && // 32 : Linux [client: remote close()]
//        (error != EINPROGRESS))  // 115: happens on Linux
//#endif
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n"),
                id));
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::handle_read_dgram (const ACE_Asynch_Read_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_read_dgram"));

  int result = -1;
  bool close = false;

  // sanity check
  if (unlikely (!result_in.success ()))
  {
    // connection closed/reset ? --> not an error
    unsigned long error = result_in.error ();
    if ((error != EBADF)                   && // 9    : local close (), happens on Linux
        (error != EPIPE)                   && // 32
        (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
        (error != ECONNRESET))                // 10054: happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (0x%@): \"%s\", aborting\n"),
                  id (),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (%d): \"%s\", aborting\n"),
                  id (),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#endif
  } // end IF

  switch (static_cast<int> (result_in.bytes_transferred ()))
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      unsigned long error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely ((error != EPIPE)                   && // 32
                    (error != ERROR_OPERATION_ABORTED) && // 995  : cancel()led
                    (error != ECONNRESET)              && // 10054
                    (error != EHOSTDOWN)))                // 10064
#else
      if (unlikely ((error != EBADF)                   && // 9    : local close (), happens on Linux
                    (error != EPIPE)                   && // 32
                    (error != ECONNRESET)))               // 10054: happens on Win32
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: failed to read from input stream (0x%@): \"%s\", aborting\n"),
                      id (),
                      result_in.handle (),
                      ACE::sock_error (static_cast<int> (error))));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to read from input stream (%d): \"%s\", aborting\n"),
                    id (),
                    result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#endif
      close = true;
      break;
    }
    // *** GOOD CASES ***
    case 0:
      break;
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%d]: received %u bytes\n"),
//                   result_in.handle (),
//                   result_in.bytes_transferred ()));

      // push the buffer into the stream for processing
      result = stream_.put (result_in.message_block (), NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u/%s: failed to ACE_Stream::put(): \"%m\", returning\n"),
                    id (),
                    ACE_TEXT (stream_.name ().c_str ())));
        close = true;
        break;
      } // end IF

      // start next read
      if (unlikely (!inherited::initiate_read ()))
      {
        int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if ((error != ENXIO)               && // happens on Win32
            (error != EFAULT)              && // *TODO*: happens on Win32
            (error != ERROR_UNEXP_NET_ERR) && // *TODO*: happens on Win32
            (error != ERROR_NETNAME_DELETED)) // happens on Win32
#else
        if (error)
#endif
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: failed to Net_IAsynchSocketHandler::initiate_read(): \"%m\", returning\n"),
                      id ()));
        close = true;
        break;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ()->release ();

  if (unlikely (close))
  {
    result = handle_close (result_in.handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  result_in.handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(%d,%d): \"%m\", continuing\n"),
                  result_in.handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
  } // end IF

  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_write_stream"));

  //// sanity check(s)
  //ACE_ASSERT (!inherited::partialWrite_);

  // *NOTE*: the message block is released by the base class
  inherited::handle_write_stream (result_in);

  //// partial write ?
  //if (inherited::partialWrite_)
  //    goto continue_;

//  // reschedule ?
//  if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
//  {
//    // *TODO*: put the buffer back into the queue and call handle_output()
//    // *IMPORTANT NOTE*: this will fail if any buffers have been
//    //                   dispatched in the meantime
//    //                   --> i.e. works for single-threaded proactors only
//
////    result = handle_output (result_in.handle ());
////    if (result == -1)
////    { // *NOTE*: most probable reason: socket has been closed by the peer, which
////      //         close()s the processing stream (see: handle_close()),
////      //         shutting down the message queue
////      int error = ACE_OS::last_error ();
////#if defined (ACE_WIN32) || defined (ACE_WIN64)
////      if ((error != ENOTSOCK)   && // 10038: happens on Win32
////          (error != ECONNRESET) && // 10054: happens on Win32
////          (error != ENOTCONN))     // 10057: happens on Win32
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("failed to ACE_Event_Handler::handle_output(0x%@): \"%m\", continuing\n"),
////                    handle));
////#else
////      if (error != ESHUTDOWN) // 108: happens on Linux
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("failed to ACE_Event_Handler::handle_output(%d): \"%m\", continuing\n"),
////                    handle));
////#endif
////    } // end IF
//  } // end IF

//continue_:
  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ListenerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 ListenerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 TimerManagerType,
                                 UserDataType>::handle_write_dgram (const ACE_Asynch_Write_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_write_dgram"));

  inherited::handle_write_dgram (result_in);

  this->decrease ();
}
