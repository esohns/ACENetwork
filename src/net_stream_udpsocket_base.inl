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

#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/Svc_Handler.h>

#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                      const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited ()
 , inherited2 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , currentWriteBuffer_ (NULL)
 , sendLock_ ()
 , serializeOutput_ (false)
 , stream_ (ACE_TEXT_ALWAYS_CHAR (NET_STREAM_DEFAULT_NAME))
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::Net_StreamUDPSocketBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  if (currentWriteBuffer_)
    currentWriteBuffer_->release ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->streamConfiguration);

  int result = -1;
  bool handle_manager = false;
  bool handle_reactor = false;
  bool handle_socket = false;
  //const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
  //  NULL;
  //const typename StreamType::SESSION_DATA_T* session_data_p = NULL;
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
  serializeOutput_ = configuration_p->streamConfiguration->serializeOutput;

  // step1: open / tweak socket, ...
  // *TODO*: remove type inferences
  result = inherited::open (configuration_p->socketHandlerConfiguration);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  handle_socket = true;

  // step2: register with the connection manager (if any)
  // *IMPORTANT NOTE*: register with the connection manager FIRST, otherwise
  //                   a race condition might occur when using multi-threaded
  //                   proactors/reactors
  if (!inherited2::registerc ())
  {
    // *NOTE*: perhaps max# connections has been reached
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    goto error;
  } // end IF
  handle_manager = true;

  // step3: initialize/start stream

  // step3a: connect stream head message queue with the reactor notification
  //         pipe ?
  // *TODO*: remove type inferences
  if (!configuration_p->streamConfiguration->useThreadPerConnection)
    configuration_p->streamConfiguration->notificationStrategy =
        &(inherited::notificationStrategy_);

  // step3c: initialize stream
  // *TODO*: Note how the session ID is simply set to the socket handle. This
  //         may not work in some scenarios (e.g. when a connection handles
  //         several consecutive sessions, and/or each session needs a reference
  //         to its' own specific and/or 'unique' ID...)
  // *TODO*: remove type inferences
  configuration_p->streamConfiguration->sessionID =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    reinterpret_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
#else
    static_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
#endif
  if (!stream_.initialize (*configuration_p->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto error;
  } // end IF
  //session_data_container_p = stream_.get ();
  //ACE_ASSERT (session_data_container_p);
  //session_data_p = &session_data_container_p->get ();
  //// *TODO*: remove type inferences
  //const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
  //  &const_cast<StateType&> (inherited2::state ());
  //stream_.dump_state ();

  // step3d: start stream
  stream_.start ();
  if (!stream_.isRunning ())
  {
    // *NOTE*: most likely, this happened because the stream failed to
    //         initialize
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto error;
  } // end IF

  // step4: register with the reactor ?
  if (!inherited::writeOnly_)
  {
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
  if (!configuration_p->streamConfiguration->useThreadPerConnection)
    inherited2::decrease ();

  inherited2::initialize (*configuration_p);
  inherited2::state_.status = NET_CONNECTION_STATUS_OK;

  return 0;

error:
  if (handle_reactor)
  {
    result = reactor_p->remove_handler (this,
                                        (ACE_Event_Handler::READ_MASK |
                                         ACE_Event_Handler::DONT_CALL));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(): \"%m\", continuing\n")));
  } // end IF

  stream_.stop (true); // <-- wait for completion

  if (handle_socket)
    result = inherited::handle_close (inherited::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);

  if (handle_manager)
    inherited2::deregister ();

  // *TODO*: remove type inference
  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

  return -1;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::close (u_long arg_in)
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
      // check specifically for the first case...
      result = ACE_OS::thr_equal (ACE_Thread::self (),
                                  inherited::last_thread ());
      if (result)
      {
//       if (inherited::module ())
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("\"%s\" worker thread (ID: %t) leaving...\n"),
//                     ACE_TEXT (inherited::name ())));
//       else
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("worker thread (ID: %t) leaving...\n")));

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

  if (close)
  {
    //ACE_HANDLE handle =
    //  ((arg_in == NET_CLOSE_REASON_INITIALIZATION) ? ACE_INVALID_HANDLE
    //                                               : inherited::get_handle ());
    ACE_HANDLE handle = inherited::get_handle ();
    // *NOTE*: may 'delete this'
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // read a datagram from the socket
  bool enqueue = true;
  // *TODO*: remove type inferences
  ACE_Message_Block* buffer_p =
    allocateMessage (inherited2::configuration_->streamConfiguration->bufferSize);
  if (!buffer_p)
  {
    if (inherited2::configuration_->streamConfiguration->messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  inherited2::configuration_->streamConfiguration->bufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (inherited2::configuration_->streamConfiguration->bufferSize,
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
                  inherited2::configuration_->streamConfiguration->bufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (buffer_p);

  // read a datagram from the socket...
  ssize_t bytes_received = -1;
  ACE_INET_Addr peer_address;
  bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (), // buf
                                          buffer_p->size (),   // n
                                          peer_address,        // addr
                                          0);                  // flags
  //bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (), // buf
  //                                        buffer_p->size (),   // n
  //                                        0,                   // flags
  //                                        NULL);               // timeout
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
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//                   handle_in));

      // clean up
      buffer_p->release ();

      return -1;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: received %u bytes...\n"),
//                   handle_in,
//                   bytes_received));

      // adjust write pointer
      buffer_p->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // push the buffer onto the stream for processing
  // *NOTE*: the stream assumes ownership of the buffer
  int result = -1;
  if (enqueue)
    result = stream_.put (buffer_p);
  else
  {
    // *TODO*: remove type inferences
    const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
        stream_.get ();
    ACE_ASSERT (session_data_container_p);
    typename StreamType::SESSION_DATA_T& session_data_r =
        const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->get ());
    if (session_data_r.lock)
    {
      result = session_data_r.lock->acquire ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
    } // end IF
    session_data_r.currentStatistic.droppedFrames++;
    if (session_data_r.lock)
    {
      result = session_data_r.lock->release ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
    } // end IF

    // clean up
    buffer_p->release ();

    result = 0;
  } // end ELSE
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", aborting\n")));

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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = 0;
  ssize_t bytes_sent = -1;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration->socketConfiguration);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be dispatching the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  if (serializeOutput_)
  {
    result = sendLock_.acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return result;
    } // end IF
  } // end IF

  if (!currentWriteBuffer_)
  {
    // send next data chunk from the stream
    ACE_Message_Block* message_block_p = NULL;
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    // been notified to the reactor
    //if (!inherited::myUserData.useThreadPerConnection)
    result =
        stream_.get (message_block_p,
                     const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    //else
    //  result = inherited2::getq (inherited::currentWriteBuffer_,
    //                             const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    if (result == -1)
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection has been closed in the meantime
      // - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN) &&  // <-- connection has been closed in the meantime
          (error != ESHUTDOWN)) // <-- queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
        //ACE_DEBUG ((LM_ERROR,
        //            (inherited2::configuration_->streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
        //                                                                                    : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));
      goto clean;
    } // end IF

    currentWriteBuffer_ =
        dynamic_cast<typename StreamType::MESSAGE_T*> (message_block_p);
    if (!currentWriteBuffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<typename StreamType::MESSAGE_T*>(%@): \"%m\", aborting\n"),
                  message_block_p));
      goto clean;
    } // end IF
  } // end IF
  ACE_ASSERT (currentWriteBuffer_);

  // finished ?
  // *TODO*: remove type inferences
  if (inherited2::configuration_->streamConfiguration->useThreadPerConnection &&
      currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
  {
    currentWriteBuffer_->release ();
    currentWriteBuffer_ = NULL;

    //       ACE_DEBUG ((LM_DEBUG,
    //                   ACE_TEXT ("[%u]: finished sending...\n"),
    //                   peer_.get_handle ()));

    goto clean;
  } // end IF

  bytes_sent =
      inherited::peer_.send (currentWriteBuffer_->rd_ptr (), // data
                             currentWriteBuffer_->length (), // bytes to send
                             inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->address, // peer address
                             0);//,                                                    // flags
                             //NULL);                                                  // timeout
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
        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result =
            inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->address.addr_to_string (buffer,
                                                                                                                 sizeof (buffer),
                                                                                                                 1);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::send(\"%s\") (handle was: 0x%@): \"%m\", aborting\n"),
                    buffer,
                    inherited::peer_.get_handle ()));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::send(\"%s\") (handle was: %d): \"%m\", aborting\n"),
                    buffer,
                    inherited::peer_.get_handle ()));
#endif
      } // end IF
#if defined (ACE_LINUX)
      if (inherited::errorQueue_)
        processErrorQueue ();
#endif

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      result = -1;
      goto clean;
    }
    // *** GOOD CASES ***
    case 0:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[%u]: socket was closed...\n"),
//                  inherited::peer_.get_handle ()));

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      result = -1;
      goto clean;
    }
    default:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%t: sent %u bytes (handle was: %u)...\n"),
//                  bytes_sent,
//                  inherited::peer_.get_handle ()));

      // finished with this buffer ?
      currentWriteBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
      if (currentWriteBuffer_->length () > 0)
        break; // there's more data

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      break;
    }
  } // end SWITCH

  // immediately reschedule sending ?
  //  if ((currentWriteBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
  //  {
  //    if (inherited::reactor ()->cancel_wakeup (this,
  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
  //  } // end IF
  //  else
  if (currentWriteBuffer_)
  {
    result = 1;
    goto clean;
  } // end IF
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

clean:
  if (serializeOutput_)
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

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
      // step1: wait for all workers within the stream (if any)
      stream_.stop (true); // <-- wait for completion

      // step2: purge any pending (!) notifications ?
      // *TODO*: remove type inference
      if (!inherited2::configuration_->streamConfiguration->useThreadPerConnection)
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
  if (!inherited::writeOnly_ &&
      (handle_in != ACE_INVALID_HANDLE))
  {
    result =
        reactor_p->remove_handler (handle_in,
                                   (mask_in |
                                    ACE_Event_Handler::DONT_CALL));
    if (result == -1)
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
  bool deregister = inherited2::isRegistered_;
  // *IMPORTANT NOTE*: use get_handle() here to pass proper handle
  //                   otherwise, this fails for the usecase "accept failed"
  //                   (see above)
  ACE_HANDLE handle = inherited::get_handle ();
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, mask_in));
  inherited::set_handle (handle); // used for debugging purposes only

  // step5: deregister with the connection manager (if any)
  if (deregister)
    inherited2::deregister ();

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                 AddressType& localSAP_out,
                                                                 AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration->socketConfiguration);

  int result = -1;

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out =
      inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->address;

  if (!inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly)
  {
    result = inherited::peer_.get_local_addr (localSAP_out);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_ConnectionId_t
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ());
#else
  return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ());
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Notification_Strategy*
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::notification ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::notification"));

  return &(inherited::notificationStrategy_);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
const StreamType&
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::stream () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::stream"));

  return stream_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  result = this->close (NET_CONNECTION_CLOSE_REASON_USER_ABORT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamUDPSocketBase_T::close(NET_CONNECTION_CLOSE_REASON_USER_ABORT): \"%m\", continuing\n")));

  // *TODO*: remove type inference
  inherited2::state_.status = NET_CONNECTION_STATUS_CLOSED;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::waitForCompletion"));

  // wait for the stream to flush
  // --> all data has been dispatched (here: to the reactor/kernel)
  stream_.wait (waitForThreads_in,
                false,
                false);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::collect"));

  try {
    return stream_.collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::report"));

  try {
    return stream_.report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr local_inet_address, peer_inet_address;
  info (handle,
        local_inet_address,
        peer_inet_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  result = local_inet_address.addr_to_string (buffer,
                                              sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string peer_address;
  result = peer_inet_address.addr_to_string (buffer,
                                             sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    peer_address = buffer;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [Id: %u [%u]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (peer_address.c_str ())));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Message_Block*
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // *TODO*: remove type inferences
  if (inherited2::configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->messageAllocator->block ())
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
  if (!message_block_p)
  {
    if (inherited2::configuration_->streamConfiguration->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}

#if defined (ACE_LINUX)
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::processErrorQueue ()
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                      const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited ()
 , inherited2 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , currentWriteBuffer_ (NULL)
 , sendLock_ ()
 , serializeOutput_ (false)
 , stream_ (ACE_TEXT_ALWAYS_CHAR (NET_STREAM_DEFAULT_NAME))
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::Net_StreamUDPSocketBase_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  if (currentWriteBuffer_)
    currentWriteBuffer_->release ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->socketHandlerConfiguration);
  ACE_ASSERT (configuration_p->streamConfiguration);

  int result = -1;
  bool handle_manager = false;
  bool handle_reactor = false;
  bool handle_socket = false;
  //const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
  //  NULL;
  //const typename StreamType::SESSION_DATA_T* session_data_p = NULL;
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
  // *TODO*: remove type inferences
  serializeOutput_ = configuration_p->streamConfiguration->serializeOutput;

  // step1: open / tweak socket, ...
  result = inherited::open (configuration_p->socketHandlerConfiguration);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  handle_socket = true;

  // step2: register with the connection manager (if any)
  // *IMPORTANT NOTE*: register with the connection manager FIRST, otherwise
  //                   a race condition might occur when using multi-threaded
  //                   proactors/reactors
  if (!inherited2::registerc ())
  {
    // *NOTE*: perhaps max# connections has been reached
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    goto error;
  } // end IF
  handle_manager = true;

  // step3: initialize/start stream

  // step3a: connect stream head message queue with the reactor notification
  //         pipe ?
  // *TODO*: remove type inferences
  if (!configuration_p->streamConfiguration->useThreadPerConnection)
    configuration_p->streamConfiguration->notificationStrategy =
        &(inherited::notificationStrategy_);

  // step3c: initialize stream
  // *TODO*: Note how the session ID is simply set to the socket handle. This
  //         may not work in some scenarios (e.g. when a connection handles
  //         several consecutive sessions, and/or each session needs a reference
  //         to its' own specific and/or 'unique' ID...)
  // *TODO*: remove type inferences
  configuration_p->streamConfiguration->sessionID =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    reinterpret_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
#else
    static_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
#endif
  if (!stream_.initialize (*configuration_p->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto error;
  } // end IF
  //session_data_container_p = stream_.get ();
  //ACE_ASSERT (session_data_container_p);
  //session_data_p = &session_data_container_p->get ();
  //// *TODO*: remove type inferences
  //const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
  //  &const_cast<StateType&> (inherited2::state ());
  //stream_.dump_state ();

  // step3d: start stream
  stream_.start ();
  if (!stream_.isRunning ())
  {
    // *NOTE*: most likely, this happened because the stream failed to
    //         initialize
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto error;
  } // end IF

  // step4: register with the reactor ?
  if (!inherited::writeOnly_)
  {
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
  if (!configuration_p->streamConfiguration->useThreadPerConnection)
    inherited2::decrease ();

  inherited2::initialize (*configuration_p);
  // *TODO*: remove type inference
  inherited2::state_.status = NET_CONNECTION_STATUS_OK;

  return 0;

error:
  if (handle_reactor)
  {
    result = reactor_p->remove_handler (this,
                                        (ACE_Event_Handler::READ_MASK |
                                         ACE_Event_Handler::DONT_CALL));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(): \"%m\", continuing\n")));
  } // end IF

  stream_.stop (true); // <-- wait for completion

  if (handle_socket)
    result = inherited::handle_close (inherited::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);

  if (handle_manager)
    inherited2::deregister ();

  // *TODO*: remove type inference
  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

  return -1;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::close (u_long arg_in)
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
      // check specifically for the first case...
      result = ACE_OS::thr_equal (ACE_Thread::self (),
                                  inherited::last_thread ());
      if (result)
      {
//       if (inherited::module ())
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("\"%s\" worker thread (ID: %t) leaving...\n"),
//                     ACE_TEXT (inherited::name ())));
//       else
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("worker thread (ID: %t) leaving...\n")));

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

  if (close)
  {
    //ACE_HANDLE handle =
    //  ((arg_in == NET_CLOSE_REASON_INITIALIZATION) ? ACE_INVALID_HANDLE
    //                                               : inherited::get_handle ());
    ACE_HANDLE handle = inherited::get_handle ();
    // *NOTE*: may 'delete this'
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // read a datagram from the socket
  bool enqueue = true;
  // *TODO*: remove type inferences
  ACE_Message_Block* buffer_p =
    allocateMessage (inherited2::configuration_->streamConfiguration->bufferSize);
  if (!buffer_p)
  {
    if (inherited2::configuration_->streamConfiguration->messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  inherited2::configuration_->streamConfiguration->bufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (inherited2::configuration_->streamConfiguration->bufferSize,
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
                  inherited2::configuration_->streamConfiguration->bufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (buffer_p);

  // read a datagram from the socket
  ssize_t bytes_received = -1;
  bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (), // data buffer
                                          buffer_p->size (),   // space
                                          0,                   // flags
                                          NULL);               // timeout
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
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//                   handle_in));

      // clean up
      buffer_p->release ();

      return -1;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: received %u bytes...\n"),
//                   handle_in,
//                   bytes_received));

      // adjust write pointer
      buffer_p->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // push the buffer onto our stream for processing
  // *NOTE*: the stream assumes ownership of the buffer
  int result = -1;
  if (enqueue)
    result = stream_.put (buffer_p);
  else
  {
    // *TODO*: remove type inferences
    const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
        stream_.get ();
    ACE_ASSERT (session_data_container_p);
    typename StreamType::SESSION_DATA_T& session_data_r =
        const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->get ());
    if (session_data_r.lock)
    {
      result = session_data_r.lock->acquire ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
    } // end IF
    session_data_r.currentStatistic.droppedFrames++;
    if (session_data_r.lock)
    {
      result = session_data_r.lock->release ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
    } // end IF

    // clean up
    buffer_p->release ();

    result = 0;
  } // end ELSE
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", aborting\n")));

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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = 0;
  ssize_t bytes_sent = -1;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration->socketConfiguration);

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be dispatching the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  if (serializeOutput_)
  {
    result = sendLock_.acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return result;
    } // end IF
  } // end IF

  if (!currentWriteBuffer_)
  {
    // send next data chunk from the stream
    ACE_Message_Block* message_block_p = NULL;
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    // been notified to the reactor
    //if (!inherited::myUserData.useThreadPerConnection)
    result =
        stream_.get (message_block_p,
                     const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    //else
    //  result = inherited2::getq (inherited::currentWriteBuffer_,
    //                             const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    if (result == -1)
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection has been closed in the meantime
      // - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN) &&  // <-- connection has been closed in the meantime
          (error != ESHUTDOWN)) // <-- queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
        //ACE_DEBUG ((LM_ERROR,
        //            (inherited2::configuration_->streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
        //                                                                                    : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));
      goto clean;
    } // end IF

    currentWriteBuffer_ =
        dynamic_cast<typename StreamType::MESSAGE_T*> (message_block_p);
    if (!currentWriteBuffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<typename StreamType::MESSAGE_T*>(%@): \"%m\", aborting\n"),
                  message_block_p));
      goto clean;
    } // end IF
  } // end IF
  ACE_ASSERT (currentWriteBuffer_);

  // finished ?
  if (inherited2::configuration_->streamConfiguration->useThreadPerConnection &&
      currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
  {
    currentWriteBuffer_->release ();
    currentWriteBuffer_ = NULL;

    //       ACE_DEBUG ((LM_DEBUG,
    //                   ACE_TEXT ("[%u]: finished sending...\n"),
    //                   peer_.get_handle ()));

    goto clean;
  } // end IF

  bytes_sent =
      inherited::peer_.send (currentWriteBuffer_->rd_ptr (), // data
                             currentWriteBuffer_->length (), // bytes to send
                             0,                              // flags
                             NULL);                          // timeout
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
        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result =
            inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->address.addr_to_string (buffer,
                                                                                                                 sizeof (buffer),
                                                                                                                 1);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));


#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_IO::send(\"%s\") (handle was: 0x%@): \"%m\", aborting\n"),
                    buffer,
                    inherited::peer_.get_handle ()));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_IO::send(\"%s\") (handle was: %d): \"%m\", aborting\n"),
                    buffer,
                    inherited::peer_.get_handle ()));
#endif
      } // end IF
#if defined (ACE_LINUX)
      if (inherited::errorQueue_)
        processErrorQueue ();
#endif

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      result = -1;
      goto clean;
    }
    // *** GOOD CASES ***
    case 0:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[%u]: socket was closed...\n"),
//                  inherited::peer_.get_handle ()));

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      result = -1;
      goto clean;
    }
    default:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%t: sent %u bytes (handle was: %u)...\n"),
//                  bytes_sent,
//                  inherited::peer_.get_handle ()));

      // finished with this buffer ?
      currentWriteBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
      if (currentWriteBuffer_->length () > 0)
        break; // there's more data

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      break;
    }
  } // end SWITCH

  // immediately reschedule sending ?
  //  if ((currentWriteBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
  //  {
  //    if (inherited::reactor ()->cancel_wakeup (this,
  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
  //  } // end IF
  //  else
  if (currentWriteBuffer_)
  {
    result = 1;
    goto clean;
  } // end IF
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

clean:
  if (serializeOutput_)
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration->socketConfiguration);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

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
      // step1: wait for all workers within the stream (if any)
      stream_.stop (true); // <-- wait for completion

      // step2: purge any pending (!) notifications ?
      // *TODO*: remove type inference
      if (!inherited2::configuration_->streamConfiguration->useThreadPerConnection)
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
  if ((!inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly) &&
      (handle_in != ACE_INVALID_HANDLE))
  {
    result =
        reactor_p->remove_handler (handle_in,
                                   (mask_in |
                                    ACE_Event_Handler::DONT_CALL));
    if (result == -1)
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
  bool deregister = inherited2::isRegistered_;
  // *IMPORTANT NOTE*: use get_handle() here to pass proper handle
  //                   otherwise, this fails for the usecase "accept failed"
  //                   (see above)
  ACE_HANDLE handle = inherited::get_handle ();
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, mask_in));
  inherited::set_handle (handle); // used for debugging purposes only

  // step5: deregister with the connection manager (if any)
  if (deregister)
    inherited2::deregister ();

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                 AddressType& localSAP_out,
                                                                 AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration->socketConfiguration);

  int result = -1;

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out =
      inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->address;

  if (!inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly)
  {
    result = inherited::peer_.get_local_addr (localSAP_out);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_ConnectionId_t
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ());
#else
  return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ());
#endif
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Notification_Strategy*
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::notification ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::notification"));

  return &(inherited::notificationStrategy_);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
const StreamType&
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::stream () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::stream"));

  return stream_;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  result = this->close (NET_CONNECTION_CLOSE_REASON_USER_ABORT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamUDPSocketBase_T::close(NET_CONNECTION_CLOSE_REASON_USER_ABORT): \"%m\", continuing\n")));

  // *TODO*: remove type inference
  inherited2::state_.status = NET_CONNECTION_STATUS_CLOSED;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::waitForCompletion"));

  // wait for the stream to flush
  // --> all data has been dispatched (here: to the reactor/kernel)
  stream_.wait (waitForThreads_in,
                false,
                false);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::collect"));

  try {
    return stream_.collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::report"));

  try {
    return stream_.report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr local_inet_address, peer_inet_address;
  info (handle,
        local_inet_address,
        peer_inet_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  result = local_inet_address.addr_to_string (buffer,
                                              sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string peer_address;
  result = peer_inet_address.addr_to_string (buffer,
                                             sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    peer_address = buffer;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [Id: %u [%u]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (peer_address.c_str ())));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Message_Block*
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // *TODO*: remove type inferences
  if (inherited2::configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->messageAllocator->block ())
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
  if (!message_block_p)
  {
    if (inherited2::configuration_->streamConfiguration->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}

#if defined (ACE_LINUX)
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                 HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::processErrorQueue ()
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

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                      const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited ()
 , inherited2 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , currentWriteBuffer_ (NULL)
 , sendLock_ ()
 , serializeOutput_ (false)
 , stream_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::Net_StreamUDPSocketBase_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  if (currentWriteBuffer_)
    currentWriteBuffer_->release ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->streamConfiguration);

  int result = -1;
  bool handle_manager = false;
  bool handle_reactor = false;
  bool handle_socket = false;
  const typename StreamType::SESSION_DATA_T* session_data_p = NULL;
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
      configuration_p->streamConfiguration->serializeOutput;

  // step1: open / tweak socket, ...
  // *TODO*: remove type inferences
  result = inherited::open (configuration_p->socketHandlerConfiguration);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  handle_socket = true;

  // step2: initialize/start stream

  // step2a: connect stream head message queue with the reactor notification
  //         pipe ?
  // *TODO*: remove type inferences
  if (!configuration_p->streamConfiguration->useThreadPerConnection)
    configuration_p->streamConfiguration->notificationStrategy =
        &(inherited::notificationStrategy_);

  // step2c: initialize stream
  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  configuration_p->streamConfiguration.sessionID =
    reinterpret_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
#else
  configuration_p->streamConfiguration->sessionID =
    static_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
#endif
  if (!stream_.initialize (*configuration_p->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto error;
  } // end IF
  session_data_p = &stream_.sessionData ();
  // *TODO*: remove type inferences
  const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
    &const_cast<StateType&> (inherited2::state ());
  //stream_.dump_state ();

  // step2d: start stream
  stream_.start ();
  if (!stream_.isRunning ())
  {
    // *NOTE*: most likely, this happened because the stream failed to
    //         initialize
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto error;
  } // end IF

  // step3: register with the connection manager (if any)
  // *IMPORTANT NOTE*: register with the connection manager FIRST, otherwise
  //                   a race condition might occur when using multi-threaded
  //                   proactors/reactors
  if (!inherited2::registerc ())
  {
    // *NOTE*: perhaps max# connections has been reached
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    goto error;
  } // end IF
  handle_manager = true;

  // step4: register with the reactor ?
  result = reactor_p->register_handler (this,
                                        ACE_Event_Handler::READ_MASK);
  if (result == -1)
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
  if (!inherited2::configuration_.streamConfiguration->useThreadPerConnection)
    inherited2::decrease ();

  inherited2::initialize (*configuration_p);
  // *TODO*: remove type inference
  inherited2::state_.status = NET_CONNECTION_STATUS_OK;

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

  stream_.stop (true); // <-- wait for completion

  if (handle_socket)
    result = inherited::handle_close (inherited::get_handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);

  if (handle_manager)
    inherited2::deregister ();

  // *TODO*: remove type inference
  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

  return -1;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::close (u_long arg_in)
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
      // check specifically for the first case...
      if (ACE_OS::thr_equal (ACE_Thread::self (),
                             inherited::last_thread ()))
      {
//       if (inherited::module ())
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("\"%s\" worker thread (ID: %t) leaving...\n"),
//                     ACE_TEXT (inherited::name ())));
//       else
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("worker thread (ID: %t) leaving...\n")));

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
      // step1: stop processing in/outbound data
      if (stream_.isRunning ())
        stream_.stop ();
      stream_.waitForCompletion ();

      // step2: close socket, deregister I/O handle with the reactor, ...
      result = inherited::close (arg_in);
      if (result == -1)
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
      if (result == -1)
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

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // read a datagram from the socket
  bool enqueue = true;
  ACE_Message_Block* buffer_p =
      allocateMessage (inherited2::configuration_->streamConfiguration->bufferSize);
  if (!buffer_p)
  {
    if (inherited2::configuration_.streamConfiguration->messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  inherited2::configuration_->streamConfiguration->bufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (inherited2::configuration_->streamConfiguration->bufferSize,
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
                  inherited2::configuration_->streamConfiguration->bufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (buffer_p);

  // read a datagram from the socket
  ssize_t bytes_received = -1;
  bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (), // buf
                                          buffer_p->size (),   // n
                                          0);                  // flags
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
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//                   handle_in));

      // clean up
      buffer_p->release ();

      return -1;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: received %u bytes...\n"),
//                   handle_in,
//                   bytes_received));

      // adjust write pointer
      buffer_p->wr_ptr (bytes_received);

      break;
    }
  } // end SWITCH

  // push the buffer onto our stream for processing
  // *NOTE*: the stream assumes ownership of the buffer
  int result = -1;
  if (enqueue)
    result = stream_.put (buffer_p);
  else
  {
    typename StreamType::STATE_T* state_p =
        &const_cast<typename StreamType::STATE_T&> (stream_.state ());
    ACE_ASSERT (state_p->currentSessionData);
    state_p->currentSessionData->currentStatistic.droppedMessages++;

    // clean up
    buffer_p->release ();

    result = 0;
  } // end ELSE
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", aborting\n")));

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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  int result = 0;
  ssize_t bytes_sent = -1;

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be dispatching the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor)
  //                   --> enforce proper serialization
  if (serializeOutput_)
  {
    result = sendLock_.acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return -1; // <-- deregister
    } // end IF
  } // end IF

  if (!currentWriteBuffer_)
  {
    // send next data chunk from the stream...
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has been
    //                   notified to the reactor
    //if (!inherited::myUserData.useThreadPerConnection)
    result =
        stream_.get (currentWriteBuffer_,
                     const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    //else
    //  result = inherited2::getq (inherited::currentWriteBuffer_,
    //                             const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    if (result == -1)
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection has been closed in the meantime
      // - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN) ||  // <-- connection has been closed in the meantime
          (error != ESHUTDOWN)) // <-- queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
        //ACE_DEBUG ((LM_ERROR,
        //            (inherited2::configuration_->streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
        //                                                                                    : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));

      goto release;
    } // end IF
  } // end IF
  ACE_ASSERT (currentWriteBuffer_);

  // finished ?
  if (inherited2::configuration_->streamConfiguration->useThreadPerConnection &&
      currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
  {
    // clean up
    currentWriteBuffer_->release ();
    currentWriteBuffer_ = NULL;

    //       ACE_DEBUG ((LM_DEBUG,
    //                   ACE_TEXT ("[%u]: finished sending...\n"),
    //                   peer_.get_handle ()));

    result = -1; // <-- deregister
    goto release;
  } // end IF

  bytes_sent =
      inherited::peer_.send (currentWriteBuffer_->rd_ptr (), // data
                             currentWriteBuffer_->length (), // bytes to send
                             0);                             // flags
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
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Stream::send(): \"%m\", aborting\n")));

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      result = -1; // <-- deregister
      break;
    }
      // *** GOOD CASES ***
    case 0:
    {
      //       ACE_DEBUG ((LM_DEBUG,
      //                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
      //                   handle_in));

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      result = -1; // <-- deregister
      break;
    }
    default:
    {
      //       ACE_DEBUG ((LM_DEBUG,
      //                  ACE_TEXT ("[%u]: sent %u bytes...\n"),
      //                  handle_in,
      //                  bytes_sent));

      // finished with this buffer ?
      currentWriteBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
      if (currentWriteBuffer_->length () > 0)
        break; // there's more data

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;

      break;
    }
  } // end SWITCH

  // immediately re-schedule handler ?
  if (currentWriteBuffer_)
    result = 1; // <-- re-schedule

release:
  if (serializeOutput_)
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  switch (mask_in)
  {
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused) /
                                             //     accept failed (e.g. too many connections) /
                                             //     select failed (EBADF see Select_Reactor_T.cpp) /
                                             //     asynch abort
    {
      // step1: wait for all workers within the stream (if any)
      if (stream_.isRunning ())
        stream_.stop (true); // <-- wait for completion

      // step2: purge any pending notifications ?
      // *IMPORTANT NOTE*: if called from a non-reactor context, or when using a
      // a multithreaded reactor, there may still be in-flight notifications
      // being dispatched at this stage, so this just speeds things up a little
      // *TODO*: remove type inference
      if (!inherited2::configuration_.streamConfiguration->useThreadPerConnection)
      {
        ACE_Reactor* reactor_p = inherited::reactor ();
        ACE_ASSERT (reactor_p);
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
    case ACE_Event_Handler::EXCEPT_MASK:
      //if (handle_in == ACE_INVALID_HANDLE) // <-- notification has completed (!useThreadPerConnection)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("notification completed, continuing\n")));
      break;
    default:
      // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %u) --> check implementation !, continuing\n"),
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

  // invoke base-class maintenance
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_NetlinkSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, mask_in));

  // step4: deregister with the connection manager (if any)
  inherited2::deregister ();

  // step5: release a reference
  // *IMPORTANT NOTE*: may 'delete this'
  this->decrease ();

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                 AddressType& localSAP_out,
                                                                 AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited2::configuration_->socketHandlerConfiguration->socketConfiguration);

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
//  result = localSAP_out.set (static_cast<u_short> (0),
//                             static_cast<ACE_UINT32> (INADDR_NONE));
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::set(0, %d): \"%m\", continuing\n"),
//                INADDR_NONE));
  localSAP_out = ACE_Addr::sap_any;
  if (!inherited::writeOnly_)
  {
    result = inherited::peer_.get_local_addr (localSAP_out);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
  remoteSAP_out =
      inherited2::configuration_->socketHandlerConfiguration->socketConfiguration->address;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_ConnectionId_t
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::id"));

  return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ());
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Notification_Strategy*
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::notification ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::notification"));

  return &(inherited::notificationStrategy_);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
const StreamType&
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::stream () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::stream"));

  return stream_;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  result = this->close (NET_CONNECTION_CLOSE_REASON_USER_ABORT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamUDPSocketBase_T::close(NET_CONNECTION_CLOSE_REASON_USER_ABORT): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  //        --> all data has been dispatched (here: to the reactor/kernel)
  stream_.flush ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::collect"));

  try {
    return stream_.collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::report"));

  try {
    return stream_.report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
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
                ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address_string = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = peer_address.addr_to_string (buffer,
                                        sizeof (buffer),
                                        1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [Id: %u [%u]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address_string.c_str ()),
              buffer));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Message_Block*
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          HandlerConfigurationType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // *TODO*: remove type inferences
  if (inherited2::configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->messageAllocator->block ())
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
  if (!message_block_p)
  {
    if (inherited2::configuration_->streamConfiguration->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}

#endif
