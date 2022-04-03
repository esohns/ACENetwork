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
#include "ace/Proactor.h"

#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::Net_TCPConnectionBase_T (bool managed_in)
 : inherited (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPConnectionBase_T::Net_TCPConnectionBase_T"));

}

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPConnectionBase_T::handle_input"));

  int result = -1;
  ssize_t bytes_received = -1;
  ACE_Message_Block* message_block_p = NULL;

  // sanity check
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration);

  // *TODO*: remove type inferences
  unsigned int pdu_size_i =
    inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->defaultBufferSize +
    inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->paddingBytes;
  message_block_p = inherited::allocateMessage (pdu_size_i);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to allocateMessage(%u), aborting\n"),
                this->id (),
                pdu_size_i));
    return -1; // <-- remove 'this' from dispatch
  } // end IF
  message_block_p->size (inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->defaultBufferSize);

  // read some data from the socket
//retry:
  bytes_received =
    inherited::peer_.recv (message_block_p->wr_ptr (),                                                              // buffer
                           inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->defaultBufferSize, // #bytes to read
                           0);                                                                                      // flags
  switch (bytes_received)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      // [- data buffer is empty (SSL)]
      int error = ACE_OS::last_error ();
      if (error == EWOULDBLOCK)      // 11: SSL_read() failed (buffer is empty)
        goto continue_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (//(error != EPIPE)  && // 9    : write() on a close()d socket *TODO*
          (error != ENOTSOCK) && // 10038: local close()
          (error != ECONNRESET)) // 10054: peer closed the connection
#else
      if ((error != EPIPE)        && // 32 : connection reset by peer (write)
          // -----------------------------
          (error != EBADF)        && // 9  : *TODO*
          (error != ENOTSOCK)     && // 88 : *TODO*
          (error != ECONNABORTED) && // 103: connection abort()ed locally
          (error != ECONNRESET))     // 104: connection reset by peer (read)
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_(SSL_)SOCK_Stream::recv() (handle was: 0x%@): \"%m\", aborting\n"),
                    id (),
                    handle_in));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_(SSL_)SOCK_Stream::recv() (handle was: %d): \"%m\", aborting\n"),
                    this->id (),
                    handle_in));
#endif // ACE_WIN32 || ACE_WIN64

      // *TODO*: remove type inference
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::state_.lock, -1);
        if (inherited::state_.status == NET_CONNECTION_STATUS_OK)
          inherited::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
      } // end lock scope

      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::state_.lock, -1);
        if (inherited::state_.status == NET_CONNECTION_STATUS_OK)
          inherited::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
      } // end lock scope

      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    default:
    { // update statistic
      inherited::state_.statistic.receivedBytes += bytes_received;

      // adjust write pointer
      message_block_p->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // enqueue the buffer into the stream for processing
  // *NOTE*: the stream assumes ownership of the buffer
  result = inherited::stream_.put (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u/%s: failed to ACE_Stream::put(): \"%m\", aborting\n"),
                  this->id (),
                  ACE_TEXT (inherited::stream_.name ().c_str ())));

    // clean up
    message_block_p->release ();

    return -1; // <-- remove 'this' from dispatch
  } // end IF

continue_:
  return 0;
}

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPConnectionBase_T::handle_output"));

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);

  int result = -1;
  ssize_t bytes_sent = 0;

  // *NOTE*: in a threaded environment, workers could be dispatching the reactor
  //         notification queue concurrently (most notably, ACE_TP_Reactor)
  //         --> enforce proper serialization here
  // *IMPORTANT NOTE*: the ACE documentation (books) explicitly claims that
  //                   measures are in place to prevent concurrent dispatch of
  //                   the same handler for a specific handle by different
  //                   threads (*TODO*: find reference). If this is indeed true,
  //                   this test may be removed (just make sure this holds for
  //                   both:
  //                   - the socket dispatch implementation
  //                   - the (piped) notification dispatch implementation
  // *TODO*: remove type inferences
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::sendLock_, -1);
  if (unlikely (inherited::writeBuffer_))
    goto continue_;

  // dequeue next chunk from the queue/stream
  // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
  //                   been notified to the reactor
  result =
    (unlikely (inherited::CONNECTION_BASE_T::configuration_->useThreadPerConnection) ? inherited::getq (inherited::writeBuffer_, NULL)
                                                                                     : inherited::stream_.get (inherited::writeBuffer_, NULL));
  if (unlikely (result == -1))
  {
    // *NOTE*: a number of issues can occur here:
    //         - connection has been closed in the meantime
    //         - queue has been deactivated
    int error = ACE_OS::last_error ();
    if ((error != EAGAIN)   && // 11   : connection has been closed
        (error != ESHUTDOWN))  // 10058: queue has been deactivated
      ACE_DEBUG ((LM_ERROR,
                  (inherited::CONNECTION_BASE_T::configuration_->useThreadPerConnection ? ACE_TEXT ("%u: failed to ACE_Task::getq(): \"%m\", aborting\n")
                                                                                        : ACE_TEXT ("%u: failed to ACE_Stream::get(): \"%m\", aborting\n")),
                  this->id ()));
    return -1; // <-- remove 'this' from dispatch
  } // end IF
continue_:
  ACE_ASSERT (inherited::writeBuffer_);

  // place data into the socket
  // *TODO*: the iovec-based implementation kept blocking in
  //         ACE::handle_write_ready(), i.e. obviously currently does not work
  //         (with multi-threaded (thread pool) reactors...)
  //         --> use 'traditional' method for now
  //size_t bytes_transferred = 0;
  //bytes_sent =
  //    inherited::peer_.send_n (writeBuffer_,        // buffer
  //                             NULL,                // timeout
  //                             &bytes_transferred); // bytes transferred
  ACE_ASSERT (!inherited::writeBuffer_->cont ());
  bytes_sent =
    inherited::peer_.send_n (inherited::writeBuffer_->rd_ptr (),                    // data
                             static_cast<int> (inherited::writeBuffer_->length ()), // #bytes to send
                             static_cast<int> (0));                                 // flags
  switch (bytes_sent)
  {
    case -1:
    {
      // *NOTE*: a number of issues can occur here:
      //         - connection reset by peer
      //         - connection abort()ed locally
      int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely ((//(error != EPIPE)        && // 9    : write() on a close()d socket *TODO*
                     (error != ECONNABORTED) && // 10053: local close()
                     (error != ECONNRESET))))   // 10054: peer closed the connection
#else
      if (unlikely ((error != EPIPE)        && // <-- connection reset by peer
                     // -------------------------------------------------------------------
                     (error != EBADF)        &&
                     (error != ENOTSOCK)     &&
                     (error != ECONNABORTED) && // <-- connection abort()ed locally
                     (error != ECONNRESET)))
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_SOCK_Stream::send_n() (handle was: 0x%@): \"%m\", aborting\n"),
                    id (),
                    handle_in));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_SOCK_Stream::send_n() (handle was: %d): \"%m\", aborting\n"),
                    this->id (),
                    handle_in));
#endif // ACE_WIN32 || ACE_WIN64
      // *TODO*: remove type inference
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::state_.lock, -1);
        if (likely (inherited::state_.status == NET_CONNECTION_STATUS_OK))
          inherited::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
      } // end lock scope

      inherited::writeBuffer_->release (); inherited::writeBuffer_ = NULL;

      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
      // *TODO*: remove type inference
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::state_.lock, -1);
        if (likely (inherited::state_.status == NET_CONNECTION_STATUS_OK))
          inherited::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
      } // end lock scope

      inherited::writeBuffer_->release (); inherited::writeBuffer_ = NULL;

      return -1; // <-- remove 'this' from dispatch
    }
    default:
    { // update statistic
      inherited::state_.statistic.sentBytes += bytes_sent;

      // finished with this buffer ?
      inherited::writeBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
      if (unlikely (inherited::writeBuffer_->length () > 0))
        break; // there's more data
      inherited::writeBuffer_->release (); inherited::writeBuffer_ = NULL;
      result = 0;
      break;
    }
  } // end SWITCH

  // immediately reschedule handler ?
  if (unlikely (inherited::writeBuffer_))
    result = 1; // <-- reschedule 'this' immediately

  // *NOTE*: relinquish a reference (see: Net_StreamConnectionBase_T::notify ())
  inherited::decrease ();

  return result;
}

/////////////////////////////////////////

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_AsynchTCPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::Net_AsynchTCPConnectionBase_T (bool managed_in)
 : inherited (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPConnectionBase_T::Net_AsynchTCPConnectionBase_T"));

}

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_AsynchTCPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::open (ACE_HANDLE handle_in,
                                                   ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPConnectionBase_T::open"));

  int result = -1;

  // step1: initialize asynchronous I/O
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::state_.lock);
    inherited::state_.status = NET_CONNECTION_STATUS_INITIALIZING;
  } // end lock scope
  inherited::open (handle_in,
                   messageBlock_in);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::state_.lock);
    if (unlikely (inherited::state_.status != NET_CONNECTION_STATUS_OK))
      goto error;
  } // end lock scope

  // step2: process any data ?
  if (unlikely (messageBlock_in.length ()))
  {
    ACE_Message_Block* duplicate_p = messageBlock_in.duplicate ();
    if (!duplicate_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    // fake a result to emulate regular behavior
    ACE_Proactor* proactor_p = inherited::proactor ();
    ACE_ASSERT (proactor_p);
    ACE_Asynch_Read_Stream_Result_Impl* fake_result_p =
        proactor_p->create_asynch_read_stream_result (inherited::proxy (),                            // handler proxy
                                                      handle_in,                                      // socket handle
                                                      *duplicate_p,                                   // buffer
                                                      static_cast<u_long> (duplicate_p->capacity ()), // (maximum) #bytes to read
                                                      NULL,                                           // ACT
                                                      ACE_INVALID_HANDLE,                             // event
                                                      0,                                              // priority
                                                      COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);           // signal number
    if (!fake_result_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Proactor::create_asynch_read_stream_result: \"%m\", aborting\n")));
      goto error;
    } // end IF
    size_t bytes_transferred = duplicate_p->length ();
    // <complete> for Accept would have already moved the <wr_ptr>
    // forward; update it to the beginning position
    duplicate_p->wr_ptr (duplicate_p->wr_ptr () - bytes_transferred);
    // invoke ourselves (see handle_read_stream())
    fake_result_p->complete (bytes_transferred, // bytes read
                             1,                 // success
                             NULL,              // ACT
                             0);                // error

    // clean up
    delete fake_result_p;
  } // end IF
  ACE_ASSERT (this->count () >= 2); // connection manager, read operation
                                    //  (+ stream module(s))

  return;

error:
  result = inherited::handle_close (handle_in,
                                    ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: local close
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchStreamConnectionBase_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  handle_in, ACE_Event_Handler::ALL_EVENTS_MASK));
#else
    if (error != EBADF) // 9: local close
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchStreamConnectionBase_T::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle_in, ACE_Event_Handler::ALL_EVENTS_MASK));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  this->decrease ();
}

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_AsynchTCPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPConnectionBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  int retries_i = 0;
  ACE_Message_Block* message_block_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_Message_Block* message_block_2 = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  typename StreamType::ISTREAM_T::STREAM_T* stream_p =
      inherited::stream_.upstream (true);
  if (likely (!stream_p))
    stream_p = &(inherited::stream_);

  // sanity check(s)
  ACE_ASSERT (stream_p);

  // dequeue message from the stream
  // *IMPORTANT NOTE*: this should NEVER block as available outbound data has
  //                   been notified
  result = stream_p->get (message_block_p, NULL);
  if (unlikely (result == -1))
  { // *NOTE*: most probable reason: socket has been closed by the peer, which
    //         close()s the processing stream (see: handle_close()),
    //         shutting down the message queue
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 108
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u/%s: failed to ACE_Stream::get(): \"%m\", aborting\n"),
                  this->id (),
                  ACE_TEXT (inherited::stream_.name ().c_str ())));
    return -1;
  } // end IF
  ACE_ASSERT (message_block_p);

  // start (asynchronous) write
send:
  inherited::increase ();
  inherited::counter_.increase ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result =
    inherited::outputStream_.writev (*message_block_p,                     // data
                                     message_block_p->total_length (),     // #bytes to write
                                     NULL,                                 // asynchronous completion token
                                     0,                                    // priority
                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
#else
  message_block_2 = message_block_p->cont ();
  message_block_p->cont (NULL);
  result =
    inherited::outputStream_.write (*message_block_p,                     // data
                                    message_block_p->length (),           // #bytes to write
                                    NULL,                                 // asynchronous completion token
                                    0,                                    // priority
                                    COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    // *WARNING*: this could fail on multi-threaded proactors
    if (error == EAGAIN) // 11   : happens on Linux (e.g. no free aio slot ATM)
    {
      ++retries_i;
      if (retries_i <= NET_CONNECTION_DEFAULT_SEND_RETRIES)
        goto send;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENOTSOCK)     &&  // 10038: happens on Win32
        (error != ECONNABORTED) &&  // 10053: happens on Win32
        (error != ECONNRESET)   &&  // 10054: happens on Win32
        (error != ENOTCONN))        // 10057: happens on Win32
#else
    if ((error != ENOMEM))     // 12: Linux: local close
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_Asynch_Write_Stream::writev(%u): \"%m\", aborting\n"),
                  this->id (),
                  message_block_p->total_length ()));

    // clean up
    message_block_p->release ();
    inherited::decrease ();
    inherited::counter_.decrease ();

    return -1;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // send any continuation(s)
  if (unlikely (message_block_2))
  {
    message_block_p = message_block_2;
    goto send;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}
