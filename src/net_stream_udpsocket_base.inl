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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <linux/errqueue.h>
#include <netinet/ip_icmp.h>
#endif

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::Net_StreamUDPSocketBase_T ()
 : inherited ()
 , writeBuffer_ (NULL)
 , sendLock_ ()
 , serializeOutput_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::Net_StreamUDPSocketBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  if (unlikely (writeBuffer_))
    writeBuffer_->release ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->streamConfiguration);

  int result = -1;
  bool handle_reactor = false;
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
  serializeOutput_ =
      configuration_p->streamConfiguration->configuration_.serializeOutput;

  // step1: initialize/tweak socket, ...
  // *TODO*: remove type inferences
  result = inherited::open (&configuration_p->socketHandlerConfiguration);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
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

  // step4: register with the reactor ?
  if (likely (!inherited::writeOnly_))
  {
    ACE_Reactor* reactor_p = inherited::reactor ();
    ACE_ASSERT (reactor_p);

    result = reactor_p->register_handler (this,
                                          ACE_Event_Handler::READ_MASK);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::register_handler(READ_MASK): \"%m\", aborting\n")));
      goto error;
    } // end IF
    handle_reactor = true;
  } // end IF
  // *NOTE*: registered with the reactor (READ_MASK) at this point
  //         --> data may start arriving at handle_input ()

//   // ...register for writes (WRITE_MASK) as well
//   if (reactor_p->register_handler (this,
//                                    ACE_Event_Handler::WRITE_MASK) == -1)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to ACE_Reactor::register_handler(WRITE_MASK): \"%m\", aborting\n")));
//     goto error;
//   } // end IF

  // *NOTE*: let the reactor manage this handler
  // *WARNING*: this has some implications (see close() below)
  // *TODO*: remove type inference
  if (likely (!configuration_p->socketHandlerConfiguration.useThreadPerConnection))
    this->decrease ();

  return 0;

error:
  if (handle_reactor)
  {
    ACE_Reactor* reactor_p = inherited::reactor ();
    ACE_ASSERT (reactor_p);

    result = reactor_p->remove_handler (this,
                                        (ACE_Event_Handler::READ_MASK |
                                         ACE_Event_Handler::DONT_CALL));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(): \"%m\", continuing\n")));
  } // end IF

  if (handle_socket)
    result = inherited::handle_close (inherited::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);

  return -1;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  // [*NOTE*: override the default behavior of a ACE_Svc_Handler, which would
  //          call handle_close() AGAIN]

  // *NOTE*: this method will be invoked
  // - by any worker after returning from svc()
  //    --> in this case, this should be a NOP (triggered from handle_close(),
  //        which was invoked by the reactor) - override the default behavior of
  //        a ACE_Svc_Handler, which would call handle_close() AGAIN
  // - by the connector/acceptor when open() fails (e.g. too many connections !)
  //    --> shutdown

  bool close = false;
  switch (arg_in)
  {
    // called by:
    // - any worker from ACE_Task_Base on clean-up
    // - acceptor/connector if there are too many connections (i.e. open()
    //   returned -1)
    case NORMAL_CLOSE_OPERATION:
    {
      // check specifically for the first case
      result = ACE_OS::thr_equal (ACE_Thread::self (),
                                  inherited::last_thread ());
      if (unlikely (result))
      {
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

      // too many connections: invoke inherited default behavior
      // --> simply fall through to the next case
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. too many connections)
    // *NOTE*: this eventually calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION:
    {
      //ACE_HANDLE handle =
      //  ((arg_in == NET_CLOSE_REASON_INITIALIZATION) ? ACE_INVALID_HANDLE
      //                                               : inherited::get_handle ());
      close = true;

      break;
    }
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      // step1: shutdown operations, release any connection resources
      close = true;

      ////  step2: release the socket handle
      //// *IMPORTANT NOTE*: wakes up any reactor thread(s) that may be working on
      ////                   the handle
      //if (handle != ACE_INVALID_HANDLE)
      //{
      //  int result_2 = ACE_OS::closesocket (handle);
      //  if (result_2 == -1)
      //  {
      //    int error = ACE_OS::last_error ();
      //    // *TODO*: on Win32, ACE_OS::close (--> ::CloseHandle) throws an
      //    //         exception, so this looks like a resource leak...
      //    if (error != ENOTSOCK) //  Win32 (failed to connect: timed out)
      //      ACE_DEBUG ((LM_ERROR,
      //                  ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
      //                  reinterpret_cast<size_t> (handle)));

      //    result = -1;
      //  } // end IF
      //  //    inherited::handle (ACE_INVALID_HANDLE);
      //} // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %u), aborting\n"),
                  arg_in));
      break;
    }
  } // end SWITCH

  if (likely (close))
  {
    //ACE_HANDLE handle =
    //  ((arg_in == NET_CLOSE_REASON_INITIALIZATION) ? ACE_INVALID_HANDLE
    //                                               : inherited::get_handle ());
    ACE_HANDLE handle = inherited::get_handle ();
    // *NOTE*: may 'delete this'
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  handle, ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle, ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
    } // end IF
  } // end IF

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  int result = -1;
  ssize_t bytes_received = -1;
  ACE_INET_Addr peer_address;
  const ConfigurationType& configuration_r = this->getR ();
  bool enqueue = true;
  ACE_Message_Block* buffer_p = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_r.streamConfiguration);

  // read a datagram from the socket
  // *TODO*: remove type inferences
  buffer_p =
    allocateMessage (configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize);
  if (unlikely (!buffer_p))
  { ACE_ASSERT (configuration_r.messageAllocator);
    if (configuration_r.messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize,
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
    if (!buffer_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u byte(s), aborting\n"),
                  configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (buffer_p);

  // read a datagram from the socket
  bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (),   // buffer
                                          buffer_p->capacity (), // buffer size
                                          peer_address,          // peer address
                                          0);                    // flags
  //bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (),                 // buf
  //                                        inherited2::configuration_->PDUSize, // n
  //                                        0,                                   // flags
  //                                        NULL);                               // timeout
  switch (bytes_received)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET) &&
          (error != EPIPE)      &&      // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != EBADF)      &&
          (error != ENOTSOCK)   &&
          (error != ECONNABORTED)) // <-- connection abort()ed locally
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to SocketType::recv(): \"%m\", returning\n")));

      // clean up
      buffer_p->release ();

      return -1;
    }
    // *** GOOD CASES ***
    case 0:
    {
      // clean up
      buffer_p->release ();

      return -1;
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[0x%@]: received %d byte(s)\n"),
                  handle_in,
                  bytes_received));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[%d]: received %d byte(s)\n"),
                  handle_in,
                  bytes_received));
#endif

      // adjust write pointer
      buffer_p->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // drop the buffer into the queue for processing
  // *NOTE*: the queue assumes ownership of the buffer
  if (likely (enqueue))
    result = inherited::putq (buffer_p, NULL);
  else
  {
    // clean up
    buffer_p->release ();

    result = 0;
  } // end ELSE
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));

    // clean up
    buffer_p->release ();

    return -1;
  } // end IF

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  int result = -1;
  ssize_t bytes_sent = -1;

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be dispatching the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  // *IMPORTANT NOTE*: the ACE documentation (books) explicitly claims that
  //                   measures are in place to prevent concurrent dispatch of
  //                   the same handler for a specific handle by different
  //                   threads (*TODO*: find reference). If this is indeed true,
  //                   this test may be removed (just make sure this holds for
  //                   the reactor implementation in general AND the specific
  //                   dispatch mechanism of (piped) reactor notifications)
  // *TODO*: remove type inferences
  if (unlikely (serializeOutput_))
  {
    result = sendLock_.acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return -1; // <-- remove 'this' from dispatch
    } // end IF
  } // end IF

  if (likely (!writeBuffer_))
  {
    // send next data chunk from the queue
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    // been notified to the reactor
    result = inherited::getq (writeBuffer_, NULL);
    if (unlikely (result == -1))
    {
      // *NOTE*: a number of issues can occur here:
      //         - connection has been closed in the meantime
      //         - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN) &&  // <-- connection has been closed in the meantime
          (error != ESHUTDOWN)) // <-- queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
      goto release;
    } // end IF
  } // end IF
  ACE_ASSERT (writeBuffer_);
  result = 0;

  // finished ?
  // *TODO*: remove type inferences
  if (writeBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
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

    // clean up
    writeBuffer_->release ();
    writeBuffer_ = NULL;

    result = -1; // <-- remove 'this' from dispatch

    goto release;
  } // end IF

  // place data into the socket
  bytes_sent =
      inherited::peer_.send (writeBuffer_->rd_ptr (), // data
                             writeBuffer_->length (), // #bytes to send
                             inherited::address_,     // peer address
                             0);//,                   // flags
                             //NULL);                   // timeout
  switch (bytes_sent)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET)   &&
          (error != ECONNABORTED) &&
          (error != EPIPE)        && // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != ENOTSOCK)     &&
          (error != EBADF))          // <-- connection abort()ed locally
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::send(%s,%u) (handle was: 0x%@): \"%m\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::address_).c_str ()),
                    writeBuffer_->length (),
                    handle_in));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::send(%s,%u) (handle was: %d): \"%m\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::address_).c_str ()),
                    writeBuffer_->length (),
                    handle_in));
#endif
      } // end IF
#if defined (ACE_LINUX)
      if (inherited::errorQueue_)
        processErrorQueue ();
#endif

      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
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
      if (unlikely (writeBuffer_->length ()))
        break; // there's more data

      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      break;
    }
  } // end SWITCH

  // immediately reschedule sending ?
  //  if ((writeBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
  //  {
  //    if (inherited::reactor ()->cancel_wakeup (this,
  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
  //  } // end IF
  //  else
  if (unlikely (writeBuffer_))
    result = 1;
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

release:
  if (unlikely (serializeOutput_))
  {
    int result_2 = sendLock_.release ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                       ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  int result = -1;
  ACE_Reactor* reactor_p = inherited::reactor ();
  ACE_ASSERT (reactor_p);

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
      if (unlikely (!configuration_r.socketHandlerConfiguration.useThreadPerConnection))
      {
        // *IMPORTANT NOTE*: in a multithreaded environment, in particular when
        //                   using a multithreaded reactor, there may still be
        //                   in-flight notifications being dispatched at this
        //                   stage. In that case, do not rely on releasing all
        //                   handler resources "manually", use reference
        //                   counting instead.
        //                   --> this just speeds things up a little.
        result =
            reactor_p->purge_pending_notifications (this,
                                                    ACE_Event_Handler::ALL_EVENTS_MASK);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::purge_pending_notifications(0x%@,ALL_EVENTS_MASK): \"%m\", continuing\n"),
                      this));
      } // end IF

      break;
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close() called for unknown reasons (handle: 0x%@, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close() called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#endif

      break;
    }
  } // end SWITCH

  // step3: deregister from the reactor ?
  if (likely (!inherited::writeOnly_ &&
              (handle_in != ACE_INVALID_HANDLE)))
  {
    result =
        reactor_p->remove_handler (handle_in,
                                   (mask_in |
                                    ACE_Event_Handler::DONT_CALL));
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@,%d): \"%m\", continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d,%d): \"%m\", continuing\n"),
                  handle_in,
                  mask_in));
#endif
    } // end IF
  } // end IF

  // step4: invoke base-class maintenance
  // *IMPORTANT NOTE*: use get_handle() here to pass proper handle
  //                   otherwise, this fails for the usecase "accept failed"
  //                   (see above)
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

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::info (ACE_HANDLE& handle_out,
                                               AddressType& localSAP_out,
                                               AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  int result = -1;
  int error = 0;

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out.reset ();

  result = inherited::peer_.get_local_addr (localSAP_out);
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: socket already closed
#else
    if (error != EBADF)    //     9: Linux: socket already closed
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
  if (likely (inherited::writeOnly_))
    remoteSAP_out = inherited::address_;
  else
  {
    result = inherited::peer_.get_remote_addr (remoteSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ENOTSOCK) && // 10038: socket already closed
          (error != ENOTCONN))   // 10057: not connected
#else
      if ((error != EBADF) &&  //   9: Linux: socket already closed
          (error != ENOTCONN)) // 107: Linux: not connected
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram::get_remote_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end ELSE
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  result = this->close (NET_CONNECTION_CLOSE_REASON_USER_ABORT);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamUDPSocketBase_T::close(NET_CONNECTION_CLOSE_REASON_USER_ABORT): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::waitForCompletion"));

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

  // *NOTE*: all data has been dispatched to the reactor (i.e. kernel)

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

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [0x%@]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#endif
}

#if defined (ACE_LINUX)
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::processErrorQueue ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::processErrorQueue"));

  ssize_t result = -1;

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  struct iovec iovec_a[1];
  iovec_a[0].iov_base = buffer;
  iovec_a[0].iov_len = sizeof (buffer);
  AddressType socket_address;
  ACE_TCHAR buffer_2[BUFSIZ];
  ACE_OS::memset (buffer_2, 0, sizeof (buffer_2));
//  result = inherited::peer_.recv (iovec_a, 1,
//                                  socket_address,
//                                  MSG_ERRQUEUE);

  struct msghdr msghdr_s;
  msghdr_s.msg_iov = iovec_a;
  msghdr_s.msg_iovlen = 1;
#if defined (ACE_HAS_SOCKADDR_MSG_NAME)
  msghdr_s.msg_name =
      static_cast<struct sockaddr*> (socket_address.get_addr ());
#else
  msghdr_s.msg_name = static_cast<char*> (socket_address.get_addr ());
#endif /* ACE_HAS_SOCKADDR_MSG_NAME */
  msghdr_s.msg_namelen = socket_address.get_addr_size ();

#if defined (ACE_HAS_4_4BSD_SENDMSG_RECVMSG)
  msghdr_s.msg_control = buffer_2;
  msghdr_s.msg_controllen = sizeof (buffer_2);
#elif !defined ACE_LACKS_SENDMSG
  msghdr_s.msg_accrights = 0;
  msghdr_s.msg_accrightslen = 0;
#endif /* ACE_HAS_4_4BSD_SENDMSG_RECVMSG */

  result = ACE_OS::recvmsg (inherited::peer_.get_handle (),
                            &msghdr_s,
                            MSG_ERRQUEUE | MSG_WAITALL);
  if (result == -1)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::recv(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
//                inherited::peer_.get_handle ()));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%t: failed to ACE_OS::recvmsg(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
                inherited::peer_.get_handle ()));
    return;
  } // end IF
  socket_address.set_size (msghdr_s.msg_namelen);
  socket_address.set_type (static_cast<struct sockaddr_in*> (socket_address.get_addr ())->sin_family);

  struct sock_extended_err* sock_err_p = NULL;
  for (struct cmsghdr* cmsghdr_p = CMSG_FIRSTHDR (&msghdr_s);
       cmsghdr_p;
       cmsghdr_p = CMSG_NXTHDR (&msghdr_s, cmsghdr_p))
  {
    if ((cmsghdr_p->cmsg_level != SOL_IP) || // IPPROTO_IP
        (cmsghdr_p->cmsg_type  != IP_RECVERR))
      continue;

    sock_err_p =
        reinterpret_cast<struct sock_extended_err*> (CMSG_DATA (cmsghdr_p));
    if (!sock_err_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%t: failed to retrieve socket error: \"%m\", continuing\n"),
                  inherited::peer_.get_handle ()));
      continue;
    } // end IF

    switch (sock_err_p->ee_origin)
    {
      case SO_EE_ORIGIN_NONE:
        break;
      case SO_EE_ORIGIN_LOCAL:
        break;
      case SO_EE_ORIGIN_ICMP:
      {
        switch (sock_err_p->ee_type)
        {
          case ICMP_NET_UNREACH:
            break;
          case ICMP_HOST_UNREACH:
            break;
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%t: invalid/unknown ICMP error (was: %d), continuing\n"),
                        sock_err_p->ee_type));
            break;
          }
        } // end SWITCH
        break;
      }
      case SO_EE_ORIGIN_ICMP6:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: invalid/unknown error origin (was: %d), continuing\n"),
                    sock_err_p->ee_origin));
        break;
      }
    } // end SWITCH
  } // end FOR
}
#endif

/////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::Net_StreamUDPSocketBase_T ()
 : inherited ()
 , writeBuffer_ (NULL)
 , sendLock_ ()
 , serializeOutput_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::Net_StreamUDPSocketBase_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  if (unlikely (writeBuffer_))
    writeBuffer_->release ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->streamConfiguration);

  int result = -1;
  bool handle_reactor = false;
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
  // *TODO*: remove type inferences
  serializeOutput_ =
    configuration_p->streamConfiguration->configuration_.serializeOutput;

  // step1: open / tweak socket, ...
  result = inherited::open (&configuration_p->socketHandlerConfiguration);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
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

  // step4: register with the reactor ?
  if (likely (!inherited::writeOnly_))
  {
    ACE_Reactor* reactor_p = inherited::reactor ();
    ACE_ASSERT (reactor_p);

    result = reactor_p->register_handler (this,
                                          ACE_Event_Handler::READ_MASK);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::register_handler(READ_MASK): \"%m\", aborting\n")));
      goto error;
    } // end IF
    handle_reactor = true;
  } // end IF
  // *NOTE*: registered with the reactor (READ_MASK) at this point
  //         --> data may start arriving at handle_input ()

//   // ...register for writes (WRITE_MASK) as well
//   if (reactor_p->register_handler (this,
//                                    ACE_Event_Handler::WRITE_MASK) == -1)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to ACE_Reactor::register_handler(WRITE_MASK): \"%m\", aborting\n")));
//     goto error;
//   } // end IF

  // *NOTE*: let the reactor manage this handler...
  // *WARNING*: this has some implications (see close() below)
  if (likely (!configuration_p->socketHandlerConfiguration.useThreadPerConnection))
    this->decrease ();

  return 0;

error:
  if (handle_reactor)
  {
    ACE_Reactor* reactor_p = inherited::reactor ();
    ACE_ASSERT (reactor_p);

    result = reactor_p->remove_handler (this,
                                        (ACE_Event_Handler::READ_MASK |
                                         ACE_Event_Handler::DONT_CALL));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(): \"%m\", continuing\n")));
  } // end IF

  if (handle_socket)
    result = inherited::handle_close (inherited::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);

  return -1;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  // [*NOTE*: override the default behavior of a ACE_Svc_Handler, which would
  //          call handle_close() AGAIN]

  // *NOTE*: this method will be invoked
  // - by any worker after returning from svc()
  //    --> in this case, this should be a NOP (triggered from handle_close(),
  //        which was invoked by the reactor) - override the default behavior of
  //        a ACE_Svc_Handler, which would call handle_close() AGAIN
  // - by the connector/acceptor when open() fails (e.g. too many connections !)
  //    --> shutdown

  bool close = false;
  switch (arg_in)
  {
    // called by:
    // - any worker from ACE_Task_Base on clean-up
    // - acceptor/connector if there are too many connections (i.e. open()
    //   returned -1)
    case NORMAL_CLOSE_OPERATION:
    {
      // check specifically for the first case
      result = ACE_OS::thr_equal (ACE_Thread::self (),
                                  inherited::last_thread ());
      if (unlikely (result))
      {
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

      // too many connections: invoke inherited default behavior
      // --> simply fall through to the next case
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. too many connections)
    // *NOTE*: this eventually calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION:
    {
      close = true;

      break;
    }
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      // step1: shutdown operations, release any connection resources
      close = true;

      ////  step2: release the socket handle
      //// *IMPORTANT NOTE*: wakes up any reactor thread(s) that may be working on
      ////                   the handle
      //if (handle != ACE_INVALID_HANDLE)
      //{
      //  int result_2 = ACE_OS::closesocket (handle);
      //  if (result_2 == -1)
      //  {
      //    int error = ACE_OS::last_error ();
      //    // *TODO*: on Win32, ACE_OS::close (--> ::CloseHandle) throws an
      //    //         exception, so this looks like a resource leak...
      //    if (error != ENOTSOCK) //  Win32 (failed to connect: timed out)
      //      ACE_DEBUG ((LM_ERROR,
      //                  ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
      //                  reinterpret_cast<size_t> (handle)));

      //    result = -1;
      //  } // end IF
      //  //    inherited::handle (ACE_INVALID_HANDLE);
      //} // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %u), aborting\n"),
                  arg_in));
      break;
    }
  } // end SWITCH

  if (likely (close))
  {
    //ACE_HANDLE handle =
    //  ((arg_in == NET_CLOSE_REASON_INITIALIZATION) ? ACE_INVALID_HANDLE
    //                                               : inherited::get_handle ());
    ACE_HANDLE handle = inherited::get_handle ();
    // *NOTE*: may 'delete this'
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_UDPSocketHandler_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  handle, ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_UDPSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle, ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
    } // end IF
  } // end IF

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  int result = -1;
  ssize_t bytes_received = -1;
  ACE_INET_Addr peer_address;
  const ConfigurationType& configuration_r = this->getR ();
  bool enqueue = true;
  ACE_Message_Block* buffer_p = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_r.streamConfiguration);

  // read a datagram from the socket
  // *TODO*: remove type inferences
  buffer_p =
    allocateMessage (configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize);
  if (unlikely (!buffer_p))
  { ACE_ASSERT (configuration_r.messageAllocator);
    if (configuration_r.messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize,
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
    if (!buffer_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u byte(s), aborting\n"),
                  configuration_r.streamConfiguration->allocatorConfiguration_.defaultBufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (buffer_p);

  // read a datagram from the socket
  bytes_received =
    inherited::peer_.recv (buffer_p->wr_ptr (),   // buffer
                           buffer_p->capacity (), // buffer size
                           0,                     // flags
                           NULL);                 // timeout
  switch (bytes_received)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET) &&
          (error != EPIPE) &&      // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != EBADF) &&
          (error != ENOTSOCK) &&
          (error != ECONNABORTED)) // <-- connection abort()ed locally
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to SocketType::recv(): \"%m\", returning\n")));

      // clean up
      buffer_p->release ();

      return -1;
    }
    // *** GOOD CASES ***
    case 0:
    {
      // clean up
      buffer_p->release ();

      return -1;
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[0x%@]: received %d byte(s)\n"),
                  handle_in,
                  bytes_received));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[%d]: received %d byte(s)\n"),
                  handle_in,
                  bytes_received));
#endif

      // adjust write pointer
      buffer_p->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // drop the buffer into the queue for processing
  // *NOTE*: the queue assumes ownership of the buffer
  if (likely (enqueue))
    result = inherited::putq (buffer_p, NULL);
  else
  {
    // clean up
    buffer_p->release ();

    result = 0;
  } // end ELSE
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));

    // clean up
    buffer_p->release ();

    return -1;
  } // end IF

  return 0;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  int result = -1;
  ssize_t bytes_sent = -1;

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be dispatching the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  // *IMPORTANT NOTE*: the ACE documentation (books) explicitly claims that
  //                   measures are in place to prevent concurrent dispatch of
  //                   the same handler for a specific handle by different
  //                   threads (*TODO*: find reference). If this is indeed true,
  //                   this test may be removed (just make sure this holds for
  //                   the reactor implementation in general AND the specific
  //                   dispatch mechanism of (piped) reactor notifications)
  if (unlikely (serializeOutput_))
  {
    result = sendLock_.acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return -1; // <-- remove 'this' from dispatch
    } // end IF
  } // end IF

  if (likely (!writeBuffer_))
  {
    // send next data chunk from the stream
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    // been notified to the reactor
    result = inherited::getq (writeBuffer_, NULL);
    if (unlikely (result == -1))
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection has been closed in the meantime
      // - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN) &&  // <-- connection has been closed in the meantime
          (error != ESHUTDOWN)) // <-- queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
      goto release;
    } // end IF
  } // end IF
  ACE_ASSERT (writeBuffer_);
  result = 0;

  // finished ?
  if (writeBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
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

    // clean up
    writeBuffer_->release ();
    writeBuffer_ = NULL;

    result = -1; // <-- remove 'this' from dispatch

    goto release;
  } // end IF

  // place data into the socket
  bytes_sent =
      inherited::peer_.send (writeBuffer_->rd_ptr (), // data
                             writeBuffer_->length (), // #bytes to send
                             0,                       // flags
                             NULL);                   // timeout
  switch (bytes_sent)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET)   &&
          (error != ECONNABORTED) &&
          (error != EPIPE)        && // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != ENOTSOCK)     &&
          (error != EBADF))          // <-- connection abort()ed locally
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_IO::send(%s,%u) (handle was: 0x%@): \"%m\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::address_).c_str ()),
                    writeBuffer_->length (),
                    handle_in));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_IO::send(%s,%u) (handle was: %d): \"%m\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::address_).c_str ()),
                    writeBuffer_->length (),
                    handle_in));
#endif
      } // end IF
#if defined (ACE_LINUX)
      if (inherited::errorQueue_)
        processErrorQueue ();
#endif

      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
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
      if (unlikely (writeBuffer_->length ()))
        break; // there's more data

      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      break;
    }
  } // end SWITCH

  // immediately reschedule sending ?
  //  if ((writeBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
  //  {
  //    if (inherited::reactor ()->cancel_wakeup (this,
  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
  //  } // end IF
  //  else
  if (unlikely (writeBuffer_))
    result = 1;
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

release:
  if (unlikely (serializeOutput_))
  {
    int result_2 = sendLock_.release ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                       ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  int result = -1;
  ACE_Reactor* reactor_p = inherited::reactor ();
  ACE_ASSERT (reactor_p);

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
      if (unlikely (!configuration_r.socketHandlerConfiguration.useThreadPerConnection))
      {
        // *IMPORTANT NOTE*: in a multithreaded environment, in particular when
        //                   using a multithreaded reactor, there may still be
        //                   in-flight notifications being dispatched at this
        //                   stage. In that case, do not rely on releasing all
        //                   handler resources "manually", use reference
        //                   counting instead.
        //                   --> this just speeds things up a little.
        result =
            reactor_p->purge_pending_notifications (this,
                                                    ACE_Event_Handler::ALL_EVENTS_MASK);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::purge_pending_notifications(0x%@,ALL_EVENTS_MASK): \"%m\", continuing\n"),
                      this));
      } // end IF

      break;
    }
    default:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: 0x%@, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#endif
      break;
  } // end SWITCH

  // step3: deregister from the reactor ?
  if (likely ((!inherited::writeOnly_) &&
              (handle_in != ACE_INVALID_HANDLE)))
  {
    result =
        reactor_p->remove_handler (handle_in,
                                   (mask_in |
                                    ACE_Event_Handler::DONT_CALL));
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@,%d), continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d,%d), continuing\n"),
                  handle_in,
                  mask_in));
#endif
    } // end IF
  } // end IF

  // step4: invoke base-class maintenance
  // *IMPORTANT NOTE*: use get_handle() here to pass proper handle
  //                   otherwise, this fails for the usecase "accept failed"
  //                   (see above)
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

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::info (ACE_HANDLE& handle_out,
                                               AddressType& localSAP_out,
                                               AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  int result = -1;
  int error = 0;

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out.reset ();

  result = inherited::peer_.get_local_addr (localSAP_out);
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: socket already closed
#else
    if (error != EBADF) // 9: Linux: socket already closed
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
  if (likely (inherited::writeOnly_))
    remoteSAP_out = inherited::address_;
  else
  {
    result = inherited::peer_.get_remote_addr (remoteSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ENOTSOCK) && // 10038: socket already closed
          (error != ENOTCONN))   // 10057: not connected
#else
      if ((error != EBADF) &&  //   9: Linux: socket already closed
          (error != ENOTCONN)) // 107: Linux: not connected
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram::get_remote_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end ELSE
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  result = this->close (NET_CONNECTION_CLOSE_REASON_USER_ABORT);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamUDPSocketBase_T::close(NET_CONNECTION_CLOSE_REASON_USER_ABORT): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::waitForCompletion"));

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

  // *NOTE*: all data has been dispatched to the reactor (i.e. kernel)

  // step2: wait for the kernel to place the data onto the wire
  // *TODO*: platforms may implement different methods by which this can be
  //         established (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  // *TODO*: remove type inference
  if (inherited::state_.status == NET_CONNECTION_STATUS_OK)
  {
    ACE_HANDLE handle = inherited::get_handle ();
    ACE_ASSERT (handle != ACE_INVALID_HANDLE);
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_inet_address, peer_inet_address;
  info (handle,
        local_inet_address,
        peer_inet_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [0x%@]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_inet_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_inet_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_inet_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_inet_address).c_str ())));
#endif
}

#if defined (ACE_LINUX)
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                 HandlerConfigurationType,
                                                 Net_SOCK_CODgram>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::processErrorQueue ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::processErrorQueue"));

  ssize_t result = -1;

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  struct iovec iovec_a[1];
  iovec_a[0].iov_base = buffer;
  iovec_a[0].iov_len = sizeof (buffer);
  AddressType socket_address;
  ACE_TCHAR buffer_2[BUFSIZ];
  ACE_OS::memset (buffer_2, 0, sizeof (buffer_2));
//  result = inherited::peer_.recv (iovec_a, 1,
//                                  socket_address,
//                                  MSG_ERRQUEUE);

  struct msghdr msghdr_s;
  msghdr_s.msg_iov = iovec_a;
  msghdr_s.msg_iovlen = 1;
#if defined (ACE_HAS_SOCKADDR_MSG_NAME)
  msghdr_s.msg_name =
      static_cast<struct sockaddr*> (socket_address.get_addr ());
#else
  msghdr_s.msg_name = static_cast<char*> (socket_address.get_addr ());
#endif /* ACE_HAS_SOCKADDR_MSG_NAME */
  msghdr_s.msg_namelen = socket_address.get_addr_size ();

#if defined (ACE_HAS_4_4BSD_SENDMSG_RECVMSG)
  msghdr_s.msg_control = buffer_2;
  msghdr_s.msg_controllen = sizeof (buffer_2);
#elif !defined ACE_LACKS_SENDMSG
  msghdr_s.msg_accrights = 0;
  msghdr_s.msg_accrightslen = 0;
#endif /* ACE_HAS_4_4BSD_SENDMSG_RECVMSG */

  result = ACE_OS::recvmsg (inherited::peer_.get_handle (),
                            &msghdr_s,
                            MSG_ERRQUEUE | MSG_WAITALL);
  if (result == -1)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::recv(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
//                inherited::peer_.get_handle ()));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%t: failed to ACE_OS::recvmsg(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
                inherited::peer_.get_handle ()));
    return;
  } // end IF
  socket_address.set_size (msghdr_s.msg_namelen);
  socket_address.set_type (static_cast<struct sockaddr_in*> (socket_address.get_addr ())->sin_family);

  struct sock_extended_err* sock_err_p = NULL;
  for (struct cmsghdr* cmsghdr_p = CMSG_FIRSTHDR (&msghdr_s);
       cmsghdr_p;
       cmsghdr_p = CMSG_NXTHDR (&msghdr_s, cmsghdr_p))
  {
    if ((cmsghdr_p->cmsg_level != SOL_IP) || // IPPROTO_IP
        (cmsghdr_p->cmsg_type  != IP_RECVERR))
      continue;

    sock_err_p =
        reinterpret_cast<struct sock_extended_err*> (CMSG_DATA (cmsghdr_p));
    if (!sock_err_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%t: failed to retrieve socket error: \"%m\", continuing\n"),
                  inherited::peer_.get_handle ()));
      continue;
    } // end IF

    switch (sock_err_p->ee_origin)
    {
      case SO_EE_ORIGIN_NONE:
        break;
      case SO_EE_ORIGIN_LOCAL:
        break;
      case SO_EE_ORIGIN_ICMP:
      {
        switch (sock_err_p->ee_type)
        {
          case ICMP_NET_UNREACH:
            break;
          case ICMP_HOST_UNREACH:
            break;
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%t: invalid/unknown ICMP error (was: %d), continuing\n"),
                        sock_err_p->ee_type));
            break;
          }
        } // end SWITCH
        break;
      }
      case SO_EE_ORIGIN_ICMP6:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: invalid/unknown error origin (was: %d), continuing\n"),
                    sock_err_p->ee_origin));
        break;
      }
    } // end SWITCH
  } // end FOR
}
#endif

/////////////////////////////////////////

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::Net_StreamUDPSocketBase_T ()
 : inherited ()
 , writeBuffer_ (NULL)
 , sendLock_ ()
 , serializeOutput_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::Net_StreamUDPSocketBase_T"));

}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  if (unlikely (writeBuffer_))
    writeBuffer_->release ();
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->streamConfiguration);

  int result = -1;
  bool handle_reactor = false;
  bool handle_socket = false;
  ACE_Reactor* reactor_p = inherited::reactor ();
  ACE_ASSERT (reactor_p);

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
  serializeOutput_ =
      configuration_p->streamConfiguration->configuration_.serializeOutput;

  // step1: initialize/tweak socket, ...
  // *TODO*: remove type inferences
  result = inherited::open (&configuration_p->socketHandlerConfiguration);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
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

  // step4: register with the reactor ?
  result = reactor_p->register_handler (this,
                                        ACE_Event_Handler::READ_MASK);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Reactor::register_handler(READ_MASK): \"%m\", aborting\n")));
    goto error;
  } // end IF
  handle_reactor = true;

  // *NOTE*: registered with the reactor (READ_MASK) at this point

//   // ...register for writes (WRITE_MASK) as well
//   if (reactor_p->register_handler (this,
//                                    ACE_Event_Handler::WRITE_MASK) == -1)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to ACE_Reactor::register_handler(WRITE_MASK): \"%m\", aborting\n")));
//     return -1;
//   } // end IF

  // *NOTE*: let the reactor manage this handler...
  // *WARNING*: this has some implications (see close() below)
  if (likely (!configuration_p->socketHandlerConfiguration.useThreadPerConnection))
    this->decrease ();

  return 0;

error:
  // clean up
  if (handle_reactor)
  {
    result = reactor_p->remove_handler (this,
                                        (ACE_Event_Handler::READ_MASK |
                                         ACE_Event_Handler::DONT_CALL));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(): \"%m\", continuing\n")));
  } // end IF

  if (handle_socket)
    result = inherited::handle_close (inherited::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);

  return -1;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  // [*NOTE*: override the default behavior of a ACE_Svc_Handler, which would
  //          call handle_close() AGAIN]

  // *NOTE*: this method will be invoked
  // - by any worker after returning from svc()
  //    --> in this case, this should be a NOP (triggered from handle_close(),
  //        which was invoked by the reactor) - we override the default
  //        behavior of a ACE_Svc_Handler, which would call handle_close() AGAIN
  // - by the connector/acceptor when open() fails (e.g. too many connections !)
  //    --> shutdown

  switch (arg_in)
  {
    // called by:
    // - any worker from ACE_Task_Base on clean-up
    // - acceptor/connector if there are too many connections (i.e. open()
    //   returned -1)
    case NORMAL_CLOSE_OPERATION:
    {
      // check specifically for the first case
      if (unlikely (ACE_OS::thr_equal (ACE_Thread::self (),
                                       inherited::last_thread ())))
      {
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

      // too many connections: invoke inherited default behavior
      // --> simply fall through to the next case
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. too many connections)
    // *NOTE*: this eventually calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION:
    {
      // step2: close socket, deregister I/O handle with the reactor, ...
      result = inherited::close (arg_in);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_NetlinkSocketHandler_T::close(): \"%m\", aborting\n")));

      break;
    }
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      // step1: shutdown operations
      ACE_HANDLE handle = inherited::SVC_HANDLER_T::get_handle ();
      // *NOTE*: may 'delete this'
      result = handle_close (handle,
                             ACE_Event_Handler::ALL_EVENTS_MASK);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_StreamUDPSocketBase_T::handle_close(): \"%m\", aborting\n")));

//      //  step2: release the socket handle
//      if (handle != ACE_INVALID_HANDLE)
//      {
//        int result_2 = ACE_OS::closesocket (handle);
//        if (result_2 == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
//                      reinterpret_cast<size_t> (handle)));
//      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %u), aborting\n"),
                  arg_in));
      break;
    }
  } // end SWITCH

  return result;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  int result = -1;
  ssize_t bytes_received = -1;
  ACE_INET_Addr peer_address;
  const ConfigurationType& configuration_r = this->getR ();
  bool enqueue = true;
  ACE_Message_Block* buffer_p = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_r.streamConfiguration);

  // read a datagram from the socket
  buffer_p =
      allocateMessage (configuration_r.streamConfiguration->allocatorConfiguration->defaultBufferSize);
  if (unlikely (!buffer_p))
  { ACE_ASSERT (configuration_r.messageAllocator);
    if (configuration_r.messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  configuration_r.streamConfiguration->allocatorConfiguration->defaultBufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (configuration_r.streamConfiguration->allocatorConfiguration->defaultBufferSize,
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
    if (!buffer_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ACE_Message_Block(%u), aborting\n"),
                  configuration_r.streamConfiguration->allocatorConfiguration->defaultBufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (buffer_p);

  // read a datagram from the socket
  bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (),   // buffer
                                          buffer_p->capacity (), // buffer size
                                          0);                    // flags
  switch (bytes_received)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET)  &&
          (error != EPIPE)       && // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != EBADF)       &&
          (error != ENOTSOCK)    &&
          (error != ECONNABORTED))  // <-- connection abort()ed locally
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to SocketType::recv(): \"%m\", returning\n")));

      // clean up
      buffer_p->release ();

      return -1;
    }
    // *** GOOD CASES ***
    case 0:
    {
      // clean up
      buffer_p->release ();

      return -1;
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[0x%@]: received %d byte(s)\n"),
                  handle_in,
                  bytes_received));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[%d]: received %d byte(s)\n"),
                  handle_in,
                  bytes_received));
#endif

      // adjust write pointer
      buffer_p->wr_ptr (bytes_received);

      break;
    }
  } // end SWITCH

  // drop the buffer into the queue for processing
  // *NOTE*: the queue assumes ownership of the buffer
  if (likely (enqueue))
    result = inherited::putq (buffer_p, NULL);
  else
  {
    // clean up
    buffer_p->release ();

    result = 0;
  } // end ELSE
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));

    // clean up
    buffer_p->release ();

    return -1;
  } // end IF

  return 0;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  int result = -1;
  ssize_t bytes_sent = -1;

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be dispatching the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  // *IMPORTANT NOTE*: the ACE documentation (books) explicitly claims that
  //                   measures are in place to prevent concurrent dispatch of
  //                   the same handler for a specific handle by different
  //                   threads (*TODO*: find reference). If this is indeed true,
  //                   this test may be removed (just make sure this holds for
  //                   the reactor implementation in general AND the specific
  //                   dispatch mechanism of (piped) reactor notifications)
  // *TODO*: remove type inferences
  if (unlikely (serializeOutput_))
  {
    result = sendLock_.acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return -1; // <-- remove 'this' from dispatch
    } // end IF
  } // end IF

  if (likely (!writeBuffer_))
  {
    // send next data chunk from the queue
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has been
    //                   notified to the reactor
    result = inherited::getq (writeBuffer_, NULL);
    if (unlikely (result == -1))
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection has been closed in the meantime
      // - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN) ||  // <-- connection has been closed in the meantime
          (error != ESHUTDOWN)) // <-- queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
      goto release;
    } // end IF
  } // end IF
  ACE_ASSERT (writeBuffer_);
  result = 0;

  // finished ?
  if (writeBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("[%d]: finished sending\n"),
                handle_in));

    // clean up
    writeBuffer_->release ();
    writeBuffer_ = NULL;

    result = -1; // <-- remove 'this' from dispatch

    goto release;
  } // end IF

  // place data into the socket
  bytes_sent =
      inherited::peer_.send (writeBuffer_->rd_ptr (), // data
                             writeBuffer_->length (), // #bytes to send
                             0);                      // flags
  switch (bytes_sent)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET)   &&
          (error != ECONNABORTED) &&
          (error != EPIPE)        && // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != ENOTSOCK)     &&
          (error != EBADF))          // <-- connection abort()ed locally
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to Net_SOCK_Netlink::send(%s,%u) (handle was: %d): \"%m\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::address_).c_str ()),
                    writeBuffer_->length (),
                    handle_in));
      } // end IF
#if defined (ACE_LINUX)
      if (inherited::errorQueue_)
        processErrorQueue ();
#endif

      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
    }
      // *** GOOD CASES ***
    case 0:
    {
      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%d: sent %d bytes\n"),
                  handle_in,
                  bytes_sent));

      // finished with this buffer ?
      writeBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
      if (unlikely (writeBuffer_->length ()))
        break; // there's more data

      // clean up
      writeBuffer_->release ();
      writeBuffer_ = NULL;

      break;
    }
  } // end SWITCH

  // immediately re-schedule handler ?
  //  if ((writeBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
  //  {
  //    if (inherited::reactor ()->cancel_wakeup (this,
  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
  //  } // end IF
  //  else
  if (unlikely (writeBuffer_))
    result = 1; // <-- re-schedule
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

release:
  if (unlikely (serializeOutput_))
  {
    int result_2 = sendLock_.release ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                       ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  int result = -1;
  ACE_Reactor* reactor_p = inherited::reactor ();
  ACE_ASSERT (reactor_p);

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

      // step2: purge any pending notifications ?
      // *TODO*: remove type inference
      if (unlikely (!configuration_r.socketHandlerConfiguration.useThreadPerConnection))
      {
        // *IMPORTANT NOTE*: in a multithreaded environment, in particular when
        //                   using a multithreaded reactor, there may still be
        //                   in-flight notifications being dispatched at this
        //                   stage. In that case, do not rely on releasing all
        //                   handler resources "manually", use reference
        //                   counting instead.
        //                   --> this just speeds things up a little.
        result =
            reactor_p->purge_pending_notifications (this,
                                                    ACE_Event_Handler::ALL_EVENTS_MASK);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::purge_pending_notifications(%@): \"%m\", continuing\n"),
                      this));
      } // end IF

      break;
    }
    default:
      // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close() called for unknown reasons (handle: %@, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close() called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#endif
      break;
  } // end SWITCH

  result =
      reactor_p->remove_handler (handle_in,
                                 (mask_in |
                                  ACE_Event_Handler::DONT_CALL));
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@,%d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d,%d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#endif
  } // end IF

  // invoke base-class maintenance
  // *IMPORTANT NOTE*: use get_handle() here to pass proper handle
  //                   otherwise, this fails for the usecase "accept failed"
  //                   (see above)
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_NetlinkSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, mask_in));
  } // end IF
  inherited::set_handle (handle_in); // used for debugging purposes only

  return result;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::info (ACE_HANDLE& handle_out,
                                               Net_Netlink_Addr& localSAP_out,
                                               Net_Netlink_Addr& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  int result = -1;
  int error = 0;
  const ConfigurationType& configuration_r = this->getR ();

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out.reset ();

  if (likely (inherited::writeOnly_))
  {
    result = inherited::peer_.get_local_addr (localSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      if (error != EBADF) // 9: Linux: socket already closed
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Netlink::get_local_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end IF
  remoteSAP_out =
    configuration_r.socketHandlerConfiguration.socketConfiguration.address;
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  result = this->close (NET_CONNECTION_CLOSE_REASON_USER_ABORT);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamUDPSocketBase_T::close(NET_CONNECTION_CLOSE_REASON_USER_ABORT): \"%m\", continuing\n")));
}

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::waitForCompletion"));

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

  // *NOTE*: all data has been dispatched to the reactor (i.e. kernel)

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

template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          Net_Netlink_Addr,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          TimerManagerType,
                          SocketConfigurationType,
                          HandlerConfigurationType,
                          UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Net_Netlink_Addr local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address_string;
  result = local_address.addr_to_string (buffer,
                                         sizeof (buffer),
                                         1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address_string = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = peer_address.addr_to_string (buffer,
                                        sizeof (buffer),
                                        1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (local_address_string.c_str ()),
              buffer));
}
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
