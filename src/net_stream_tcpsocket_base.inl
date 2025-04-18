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
#include "ace/Reactor.h"
#include "ace/Svc_Handler.h"
#include "ace/Thread.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::Net_StreamTCPSocketBase_T ()
 : inherited ()
 , readBuffer_ (NULL)
 , writeBuffer_ (NULL)
 , sendLock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::Net_StreamTCPSocketBase_T"));

}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::~Net_StreamTCPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::~Net_StreamTCPSocketBase_T"));

  if (readBuffer_)
    readBuffer_->release ();
  if (writeBuffer_)
    writeBuffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->streamConfiguration);

  int result = -1;
  bool handle_socket = false;

  // step0: initialize this
  // *IMPORTANT NOTE*: enable the reference counting policy, as this will
  //                   be registered with the reactor several times
  //                   (1x READ_MASK, nx WRITE_MASK); therefore several threads
  //                   MAY be dispatching notifications (yes, even concurrently;
  //                   lock_ enforces the proper sequence order, see
  //                   handle_output()) on the SAME handler. When the socket
  //                   closes, the event handler should thus not be destroyed()
  //                   immediately, but simply purge any pending notifications
  //                   (see handle_close()) and de-register; after the last
  //                   active notification has been dispatched, it will be
  //                   safely deleted
  inherited::reference_counting_policy ().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);
  // *IMPORTANT NOTE*: due to reference counting, the
  //                   ACE_Svc_Handle::shutdown() method will crash, as it
  //                   references a connection recycler AFTER removing the
  //                   connection from the reactor (which releases a reference).
  //                   In the case that "this" is the final reference, this
  //                   leads to a crash (see code)
  //                   --> avoid invoking ACE_Svc_Handle::shutdown()
  //                   --> this means that "manual" cleanup is necessary
  //                       (see handle_close())
  inherited::closing_ = true;
  // *TODO*: find a better way to do this
  //serializeOutput_ =
  //  configuration_p->streamConfiguration->configuration_.serializeOutput;

  // step1: initialize/tweak socket, register I/O handle with the reactor, ...
  // *NOTE*: as soon as this returns, data starts arriving at handle_input()
  result =
    inherited::open (&configuration_p->socketHandlerConfiguration);
  if (result == -1)
  {
    // *NOTE*: this can happen when the connection handle is still registered
    //         with the reactor (i.e. the reactor is still processing events on
    //         a file descriptor that has been closed and is now being reused by
    //         the system)
    // *NOTE*: more likely, this happened because the (select) reactor is out of
    //         "free" (read) slots
    int error = ACE_OS::last_error ();
    ACE_UNUSED_ARG (error);

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  handle_socket = true;

  //// step2: register with the connection manager (if any)
  //// *IMPORTANT NOTE*: register with the connection manager FIRST, otherwise
  ////                   a race condition might occur when using multi-threaded
  ////                   proactors/reactors
  //if (unlikely (!inherited2::registerc ()))
  //{
  //  // *NOTE*: perhaps max# connections has been reached
  //  //ACE_DEBUG ((LM_ERROR,
  //  //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
  //  goto error;
  //} // end IF
  //handle_manager = true;

  return 0;

error:
  if (handle_socket)
    result = inherited::handle_close (inherited::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);

  return -1;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::close"));

  int result = -1;

  // [*NOTE*: override the default behavior of a ACE_Svc_Handler,
  // which would call handle_close() AGAIN]

  // this method will be invoked:
  // - by any worker after returning from svc()
  //    --> in this case, this should be a NOP (triggered from handle_close(),
  //        which was invoked by the reactor) - override the default
  //        behavior of a ACE_Svc_Handler, which would call handle_close() AGAIN
  // - by the connector when connect() fails (e.g. connection refused)
  // - by the connector/acceptor when open() fails (e.g. too many connections !)
  switch (arg_in)
  {
    // called by:
    // - any worker from ACE_Task_Base on clean-up
    // - connector when connect() fails (e.g. connection refused)
    // - acceptor/connector when initialization fails (i.e. open() returned -1
    //   due to e.g. too many connections)
    case NORMAL_CLOSE_OPERATION:
    {
      // check specifically for the first case...
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
    case CLOSE_DURING_NEW_CONNECTION: // e.g. connect()ion refused
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION: // initialization (i.e. open()) failed
    {
      ACE_HANDLE handle = inherited::get_handle ();

      // shutdown operations, release any connection resources
      result = handle_close (handle,
                             ACE_Event_Handler::ALL_EVENTS_MASK);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                    handle, ACE_Event_Handler::ALL_EVENTS_MASK));

      break;
    }
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      ACE_HANDLE handle = inherited::get_handle ();

      // shutdown operations, release any connection resources
      result = handle_close (handle,
                             ACE_Event_Handler::ALL_EVENTS_MASK);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                    handle, ACE_Event_Handler::ALL_EVENTS_MASK));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %u), aborting\n"),
                  arg_in));
    }
  } // end SWITCH

  return result;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_input"));

  int result = -1;
  ssize_t bytes_received = -1;
  const ConfigurationType& configuration_r = this->getR ();

  // sanity check
  ACE_ASSERT (configuration_r.streamConfiguration);
  ACE_ASSERT (!readBuffer_);

  // read some data from the socket
  // *TODO*: remove type inference
  size_t pdu_size_i =
    configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize +
    configuration_r.streamConfiguration->allocatorConfiguration_.paddingBytes;
  readBuffer_ = this->allocateMessage (pdu_size_i);
  if (unlikely (!readBuffer_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                pdu_size_i));
    return -1; // <-- remove 'this' from dispatch
  } // end IF
  readBuffer_->size (configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize);

  // read some data from the socket
retry:
  bytes_received =
    inherited::peer_.recv (readBuffer_->wr_ptr (),                                                         // buffer
                           configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize, // #bytes to read
                           0);                                                                             // flags
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
        goto retry;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (//(error != EPIPE)        && // 9    : write() on a close()d socket *TODO*
          (error != ECONNABORTED) && // 10053: local close()
          (error != ECONNRESET))     // 10054: peer closed the connection
#else
      if ((error != EPIPE)        && // 32: connection reset by peer (write)
          // -------------------------------------------------------------------
          (error != EBADF)        && // 9
          (error != ENOTSOCK)     && // 88
          (error != ECONNABORTED) && // 103: connection abort()ed locally
          (error != ECONNRESET))     // 104: connection reset by peer (read)
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_(SSL_)SOCK_Stream::recv(%d): \"%m\", aborting\n"),
                    handle_in));

      // clean up
      readBuffer_->release ();
      readBuffer_ = NULL;

      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("0x%@: socket was closed by the peer\n"),
                  handle_in));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%d: socket was closed by the peer\n"),
                  handle_in));
#endif

      // clean up
      readBuffer_->release ();
      readBuffer_ = NULL;

      return -1; // <-- remove 'this' from dispatch
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[0x%@]: received %d bytes\n"),
                  handle_in,
                  bytes_received));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[%d]: received %d bytes\n"),
                  handle_in,
                  bytes_received));
#endif

      // adjust write pointer
      readBuffer_->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // drop the buffer into the queue for processing
  // *NOTE*: the queue assumes ownership of the buffer
  result = inherited::putq (readBuffer_, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));

    // clean up
    readBuffer_->release ();
    readBuffer_ = NULL;

    return -1; // <-- remove 'this' from dispatch
  } // end IF
  readBuffer_ = NULL;

  return result;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_output"));

  int result = -1;
  ssize_t bytes_sent = -1;

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
  //                   - the dispatch implementation of (piped) notifications
  // *TODO*: remove type inferences
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, sendLock_, -1);

  if (likely (!writeBuffer_))
  {
    // send next data chunk from the queue
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    //                   been notified to the reactor
    result = inherited::getq (writeBuffer_, NULL);
    if (unlikely (result == -1))
    {
      // *NOTE*: a number of issues can occur here:
      //         - connection has been closed in the meantime
      //         - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN)   && // 11   : connection has been closed
          (error != ESHUTDOWN))  // 10058: queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
      return -1; // <-- remove 'this' from dispatch
    } // end IF
  } // end IF
  ACE_ASSERT (writeBuffer_);
  result = 0;
  if (unlikely (writeBuffer_->msg_type () == ACE_Message_Block::MB_STOP))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("[0x%@]: finished sending\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("[%d]: finished sending\n"),
                handle_in));
#endif
    writeBuffer_->release (); writeBuffer_ = NULL;
    return -1; // <-- remove 'this' from dispatch
  } // end IF

  // place data into the socket
  // *TODO*: the iovec-based implementation kept blocking in
  //         ACE::handle_write_ready(), i.e. obviously currently does not work
  //         with multi-threaded (thread pool) reactors...
  //         --> use 'traditional' method for now
  //size_t bytes_transferred = 0;
  //bytes_sent =
  //    inherited::peer_.send_n (writeBuffer_, // buffer
  //                             NULL,                // timeout
  //                             &bytes_transferred); // bytes transferred
  bytes_sent =
    inherited::peer_.send_n (writeBuffer_->rd_ptr (),                    // data
                             static_cast<int> (writeBuffer_->length ()), // #bytes to send
                             (int)0);                                    // flags
  switch (bytes_sent)
  {
    case -1:
    {
      // *NOTE*: a number of issues can occur here:
      //         - connection reset by peer
      //         - connection abort()ed locally
      int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (//(error != EPIPE)        && // 9    : write() on a close()d socket *TODO*
          (error != ECONNABORTED) && // 10053: local close()
          (error != ECONNRESET))     // 10054: peer closed the connection
#else
      if ((error != EPIPE)        && // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != EBADF)        &&
          (error != ENOTSOCK)     &&
          (error != ECONNABORTED) && // <-- connection abort()ed locally
          (error != ECONNRESET))
#endif
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Stream::send_n(%u) (handle was: 0x%@): \"%m\", aborting\n"),
                    writeBuffer_->length (),
                    handle_in));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Stream::send_n(%u) (handle was: %d): \"%m\", aborting\n"),
                    writeBuffer_->length (),
                    handle_in));
#endif
      } // end IF
#if defined (ACE_LINUX)
      if (inherited::errorQueue_)
        this->processErrorQueue ();
#endif
      writeBuffer_->release (); writeBuffer_ = NULL;
      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("0x%@: socket was closed by the peer\n"),
                  handle_in));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%d: socket was closed by the peer\n"),
                  handle_in));
#endif
      writeBuffer_->release (); writeBuffer_ = NULL;
      return -1; // <-- remove 'this' from dispatch
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("0x%@: sent %d bytes\n"),
                  handle_in,
                  bytes_sent));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%d: sent %d bytes\n"),
                  handle_in,
                  bytes_sent));
#endif

      // finished with this buffer ?
      writeBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
      if (unlikely (writeBuffer_->length () > 0))
        break; // there's more data

      writeBuffer_->release (); writeBuffer_ = NULL;

      break;
    }
  } // end SWITCH

  // immediately reschedule handler ?
  //  if ((writeBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
  //  {
  //    if (inherited::reactor ()->cancel_wakeup (this,
  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
  //  } // end IF
  //  else
  if (unlikely (writeBuffer_))
    result = 1; // <-- reschedule 'this' immediately
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

  return result;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                       ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_close"));

  int result = -1;

  // *IMPORTANT NOTE*: when control reaches here, the socket handle has already
  //                   gone away, i.e. no new data will be accepted by the
  //                   kernel / forwarded by the proactor
  //                   --> finish processing: flush all remaining outbound data

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
      const ConfigurationType& configuration_r = this->getR ();

      // step2: purge any pending (!) notifications ?
      // *TODO*: remove type inference
      if (likely (!configuration_r.socketHandlerConfiguration.useThreadPerConnection))
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
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::purge_pending_notifications(0x%@,ALL_EVENTS_MASK): \"%m\", continuing\n"),
                      this));
        //else if (result > 0)
        //  ACE_DEBUG ((LM_DEBUG,
        //              ACE_TEXT ("flushed %d notifications (handle was: %u)\n"),
        //              result,
        //              handle_in));
      } // end IF

      break;
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_close() called for unknown reasons (handle: 0x%@, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_close() called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#endif
      break;
    }
  } // end SWITCH

  // step3: invoke base class maintenance
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
                ACE_TEXT ("failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                handle_in, mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, mask_in));
#endif
  } // end IF
  inherited::set_handle (handle_in); // used for debugging purposes only

  return result;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
unsigned int
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::increase ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::increase"));

  ACE_Event_Handler::Reference_Count count = inherited::add_reference ();
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%@/%u: added, count: %d\n"), this, id (), count));

  return static_cast<unsigned int> (count);
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
unsigned int
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::decrease ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::decrease"));

  ACE_Event_Handler::Reference_Count count = inherited::remove_reference ();
  //  if (count)
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("%@/%u: removed, count: %d\n"), this_ptr, session_id, count));
  //  else
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("%@/%u: removed, count: %d --> deleted !\n"), this_ptr, session_id, count));

  return static_cast<unsigned int> (count);
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::info (ACE_HANDLE& handle_out,
                                               AddressType& localSAP_out,
                                               AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::info"));

  int result = -1;
  int error = 0;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  handle_out = inherited::get_handle ();
#else
  handle_out = inherited::get_handle ();
#endif

  result = inherited::peer_.get_local_addr (localSAP_out);
  if (result == -1)
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (error != EBADF) // 9: Linux: socket already closed
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
  result = inherited::peer_.get_remote_addr (remoteSAP_out);
  // *NOTE*: peer may have disconnected already --> not an error
  if (result == -1)
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (error != EBADF) // 9: Linux: socket already closed
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK::get_remote_addr(): \"%m\", continuing\n")));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::close"));

  int result = -1;

  result = close (NET_CONNECTION_CLOSE_REASON_USER_ABORT);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamTCPSocketBase_T::close(NET_CONNECTION_CLOSE_REASON_USER_ABORT): \"%m\", continuing\n")));
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamTCPSocketBase_T<ACE_SYNCH_USE,
                          HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          HandlerConfigurationType,
                          UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::waitForCompletion"));

  ACE_UNUSED_ARG (waitForThreads_in);

  int result = -1;
  ACE_Time_Value one_second (1, 0);

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  // step1: wait for the queue to flush
  while (!inherited::msg_queue_->is_empty ())
  {
    result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
  } // end WHILE

  // step2: wait for the kernel to place the data onto the wire
  // *TODO*: platforms may implement different methods by which this can be
  //         established (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  // *TODO*: remove type inference
//    ACE_HANDLE handle = inherited::get_handle ();
//    ACE_ASSERT (handle != ACE_INVALID_HANDLE);
//    bool no_delay = Net_Common_Tools::getNoDelay (handle);
//    Net_Common_Tools::setNoDelay (handle, true);
//    Net_Common_Tools::setNoDelay (handle, no_delay);
#endif
}
