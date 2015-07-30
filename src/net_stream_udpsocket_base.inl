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
#include "ace/OS.h"
#include "ace/Svc_Handler.h"

#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                      unsigned int statisticCollectionInterval_in)
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

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  int result = -1;

  // clean up
  // *TODO*: remove type inferences
  if (inherited2::configuration_.streamConfiguration.module)
  {
    Stream_Module_t* module_p =
      stream_.find (inherited2::configuration_.streamConfiguration.module->name ());
    if (module_p)
    {
      result =
        stream_.remove (inherited2::configuration_.streamConfiguration.module->name (),
                        ACE_Module_Base::M_DELETE_NONE);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
    } // end IF
    if (inherited2::configuration_.streamConfiguration.deleteModule)
      delete inherited2::configuration_.streamConfiguration.module;
  } // end IF

  if (currentWriteBuffer_)
    currentWriteBuffer_->release ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::open (void* args_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  int result = -1;

  // step0: initialize this
  // *TODO*: find a better way to do this
  serializeOutput_ =
    inherited2::configuration_.streamConfiguration.serializeOutput;

  // step1: open / tweak socket, ...
  result = inherited::open (args_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  //// *NOTE*: as soon as this returns, data starts arriving at handle_input()

  // step2: initialize/start stream
  // *TODO*: remove type inferences
  const typename StreamType::SESSION_DATA_T* session_data_p = NULL;
  // step2a: connect stream head message queue with the reactor notification
  // pipe ?
  if (!inherited2::configuration_.streamConfiguration.useThreadPerConnection)
  {
    // *IMPORTANT NOTE*: enable the reference counting policy, as this will
    // be registered with the reactor several times (1x READ_MASK, nx
    // WRITE_MASK); therefore several threads MAY be dispatching notifications
    // (yes, even concurrently; myLock enforces the proper sequence order, see
    // handle_output()) on the SAME handler. When the socket closes, the event
    // handler should thus not be destroyed() immediately, but simply purge any
    // pending notifications (see handle_close()) and de-register; after the
    // last active notification has been dispatched, it will be safely deleted
    inherited::reference_counting_policy ().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);

    // *NOTE*: must use 'this' (inherited2:: does not work here for some
    //         strange reason)...
    inherited2::configuration_.streamConfiguration.notificationStrategy =
        &this->notificationStrategy_;
  } // end IF
  // step2b: clone/initialize final module (if any)
  if (inherited2::configuration_.streamConfiguration.module)
  {
    // step2ba: clone final module ?
    if (inherited2::configuration_.streamConfiguration.module)
    {
      Stream_IModule_t* imodule_p =
        dynamic_cast<Stream_IModule_t*> (inherited2::configuration_.streamConfiguration.module);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
        return -1;
      } // end IF
      Stream_Module_t* clone_p = NULL;
      try
      {
        clone_p = imodule_p->clone ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_IModule::clone(), continuing\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
      }
      if (!clone_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_IModule::clone(), aborting\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
        return -1;
      }
      inherited2::configuration_.streamConfiguration.module =
        clone_p;
      inherited2::configuration_.streamConfiguration.deleteModule =
        true;
    } // end IF
    // *TODO*: step2bb: initialize final module
  } // end IF
  // step2c: initialize stream
  // *TODO*: this clearly is a design glitch
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited2::configuration_.streamConfiguration.sessionID =
    reinterpret_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ()); // (== socket handle)
#else
  inherited2::configuration_.streamConfiguration.sessionID =
    static_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ()); // (== socket handle)
#endif
  if (!stream_.initialize (inherited2::configuration_.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    return -1;
  } // end IF
  session_data_p = &stream_.sessionData ();
  const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
      &const_cast<StateType&> (inherited2::state ());
  //stream_.dump_state ();
  // *NOTE*: as soon as this returns, data starts arriving at
  // handle_output()/msg_queue()
  stream_.start ();
  if (!stream_.isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    return -1;
  } // end IF

  // step3: register with the reactor
  ACE_Reactor* reactor_p = inherited::reactor ();
  ACE_ASSERT (reactor_p);
  result = reactor_p->register_handler (this,
                                        ACE_Event_Handler::READ_MASK);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Reactor::register_handler(READ_MASK): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // *NOTE*: registered with the reactor (READ_MASK) at this point

//   // ...register for writes (WRITE_MASK) as well
//   if (reactor ()->register_handler (this,
//                                     ACE_Event_Handler::WRITE_MASK) == -1)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to ACE_Reactor::register_handler(WRITE_MASK): \"%m\", aborting\n")));
//     return -1;
//   } // end IF

  // step4: register with the connection manager (if any)
  if (!inherited2::registerc ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    return -1;
  } // end IF

  // *NOTE*: let the reactor manage this handler...
  // *WARNING*: this has some implications (see close() below)
  if (!inherited2::configuration_.streamConfiguration.useThreadPerConnection)
    inherited::remove_reference ();

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::close"));

  int result = -1;

  // [*NOTE*: hereby we override the default behavior of a ACE_Svc_Handler,
  // which would call handle_close() AGAIN]

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
                    ACE_TEXT ("failed to HandlerType::close(%u): \"%m\", aborting\n"),
                    arg_in));

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

      //  step2: release the socket handle
      if (handle != ACE_INVALID_HANDLE)
      {
        int result_2 = ACE_OS::closesocket (handle);
        if (result_2 == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                      handle));
      } // end IF

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

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  // read a datagram from the socket
  bool enqueue = true;
  ACE_Message_Block* buffer_p =
    allocateMessage (inherited2::configuration_.streamConfiguration.bufferSize);
  if (!buffer_p)
  {
    if (inherited2::configuration_.streamConfiguration.messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  inherited2::configuration_.streamConfiguration.bufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (inherited2::configuration_.streamConfiguration.bufferSize,
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
                  inherited2::configuration_.streamConfiguration.bufferSize));
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
    // *TODO*: remove type inferences
    const typename StreamType::SESSION_DATA_T& session_data_r =
        stream_.sessionData ();
    if (session_data_r.lock)
    {
      result = session_data_r.lock->acquire ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
    } // end IF
    const_cast<typename StreamType::SESSION_DATA_T&> (session_data_r).currentStatistic.droppedMessages++;
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  int result = -1;
  ssize_t bytes_sent = -1;

  ACE_UNUSED_ARG (handle_in);

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be
  // dispatching the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
  if (serializeOutput_)
    sendLock_.acquire ();

  if (!currentWriteBuffer_)
  {
    // send next data chunk from the stream...
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    // been notified to the reactor
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
                    (inherited2::configuration_.streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
                                                                                                               : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));

      // clean up
      if (serializeOutput_)
        sendLock_.release ();

      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (currentWriteBuffer_);

  // finished ?
  if (inherited2::configuration_.streamConfiguration.useThreadPerConnection &&
      currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
  {
    currentWriteBuffer_->release ();
    currentWriteBuffer_ = NULL;

    //       ACE_DEBUG ((LM_DEBUG,
    //                   ACE_TEXT ("[%u]: finished sending...\n"),
    //                   peer_.get_handle ()));

    // clean up
    if (serializeOutput_)
      sendLock_.release ();

    return -1;
  } // end IF

  bytes_sent =
      inherited::peer_.send (currentWriteBuffer_->rd_ptr (),                             // data
                             currentWriteBuffer_->length (),                             // bytes to send
                             inherited2::configuration_.socketConfiguration.peerAddress, // peer address
                             0);                                                         // flags
  switch (bytes_sent)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET) &&
          (error != ECONNABORTED) &&
          (error != EPIPE) &&      // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != ENOTSOCK) &&
          (error != EBADF))        // <-- connection abort()ed locally
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SOCK_Stream::send(): \"%m\", aborting\n")));

      // clean up
      currentWriteBuffer_->release ();
      currentWriteBuffer_ = NULL;
      if (serializeOutput_)
        sendLock_.release ();

      return -1;
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
      if (serializeOutput_)
        sendLock_.release ();

      return -1;
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
    // clean up
    if (serializeOutput_)
      sendLock_.release ();

    return 1;
  } // end IF
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

  // clean up
  if (serializeOutput_)
    sendLock_.release ();

  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  int result = -1;

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
      if (!inherited2::configuration_.streamConfiguration.useThreadPerConnection)
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

  // step3: invoke base-class maintenance
  result = inherited::handle_close (handle_in,
                                     mask_in);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to SocketHandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, mask_in));

  // step4: deregister with the connection manager (if any)
  inherited2::deregister ();

  // step5: release a reference
  // *IMPORTANT NOTE*: may 'delete this'
  //inherited2::decrease ();
  this->decrease ();

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::collect"));

  try
  {
    return stream_.collect (data_out);
  }
  catch (...)
  {
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::report"));

  try
  {
    return stream_.report ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                 AddressType& localSAP_out,
                                                                 AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  if (inherited::peer_.get_local_addr (localSAP_out) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  remoteSAP_out =
    inherited2::configuration_.socketConfiguration.peerAddress;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
unsigned int
Net_StreamUDPSocketBase_T<HandlerType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ());
#else
  return static_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ());
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr local_inet_address, peer_inet_address;
  info (handle,
        local_inet_address,
        peer_inet_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  if (local_inet_address.addr_to_string (buffer,
                                         sizeof (buffer)) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string peer_address;
  if (peer_inet_address.addr_to_string (buffer,
                                        sizeof (buffer)) == -1)
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

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_.streamConfiguration.messageAllocator)
  {
allocate:
    try
    {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_.streamConfiguration.messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_.streamConfiguration.messageAllocator->block ())
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
    if (inherited2::configuration_.streamConfiguration.messageAllocator)
    {
      if (inherited2::configuration_.streamConfiguration.messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                      unsigned int statisticCollectionInterval_in)
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

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  // clean up
  if (inherited2::configuration_.streamConfiguration.module)
  {
    Stream_Module_t* module_p =
      stream_.find (inherited2::configuration_.streamConfiguration.module->name ());
    if (module_p)
    {
      int result =
        stream_.remove (inherited2::configuration_.streamConfiguration.module->name (),
                        ACE_Module_Base::M_DELETE_NONE);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
    } // end IF
    if (inherited2::configuration_.streamConfiguration.cloneModule ||
        inherited2::configuration_.streamConfiguration.deleteModule)
      delete inherited2::configuration_.streamConfiguration.module;
  } // end IF

  if (currentWriteBuffer_)
    currentWriteBuffer_->release ();
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::open (void* args_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  int result = -1;

  // step0: initialize this
  // *TODO*: find a better way to do this
  serializeOutput_ =
   inherited2::configuration_.streamConfiguration.serializeOutput;

  // step1: open / tweak socket, ...
  result = inherited::open (args_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  //// *NOTE*: as soon as this returns, data starts arriving at handle_input()

  // step2: initialize/start stream
  // *TODO*: remove type inferences
  const typename StreamType::SESSION_DATA_T* session_data_p = NULL;

  // step2a: connect stream head message queue with the reactor notification
  // pipe ?
  if (!inherited2::configuration_.streamConfiguration.useThreadPerConnection)
  {
    // *IMPORTANT NOTE*: enable the reference counting policy, as this will
    // be registered with the reactor several times (1x READ_MASK, nx
    // WRITE_MASK); therefore several threads MAY be dispatching notifications
    // (yes, even concurrently; myLock enforces the proper sequence order, see
    // handle_output()) on the SAME handler. When the socket closes, the event
    // handler should thus not be destroyed() immediately, but simply purge any
    // pending notifications (see handle_close()) and de-register; after the
    // last active notification has been dispatched, it will be safely deleted
    inherited::reference_counting_policy ().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);

    // *NOTE*: must use 'this' for some strange reason (inherited:: does not
    //         work here (GCC 4.9.2)...
#if defined (__GNUC__)
    inherited2::configuration_.streamConfiguration.notificationStrategy =
        &this->notificationStrategy_;
#else
    inherited2::configuration_.streamConfiguration.notificationStrategy =
        &inherited::notificationStrategy_;
#endif
  } // end IF
  // step2b: clone/initialize final module (if any)
  if (inherited2::configuration_.streamConfiguration.module)
  {
    // step2ba: clone final module ?
    if (inherited2::configuration_.streamConfiguration.module)
    {
      Stream_IModule_t* imodule_p =
        dynamic_cast<Stream_IModule_t*> (inherited2::configuration_.streamConfiguration.module);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
        return -1;
      } // end IF
      Stream_Module_t* clone_p = NULL;
      try
      {
        clone_p = imodule_p->clone ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_IModule::clone(), aborting\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
        return -1;
      }
      if (!clone_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_IModule::clone(), aborting\n"),
                    ACE_TEXT (inherited2::configuration_.streamConfiguration.module->name ())));
        return -1;
      }
      inherited2::configuration_.streamConfiguration.module = clone_p;
      inherited2::configuration_.streamConfiguration.deleteModule = true;
    } // end IF
    // *TODO*: step2bb: initialize final module
  } // end IF
  // step2c: initialize stream
  unsigned int session_id = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  session_id =
      reinterpret_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ()); // (== socket handle)
#else
  session_id =
      static_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ()); // (== socket handle)
#endif
  // *TODO*: this clearly is a design glitch
  if (!stream_.initialize (session_id,
                           inherited2::configuration_.streamConfiguration,
                           inherited2::configuration_.protocolConfiguration,
                           inherited2::configuration_.streamSessionData))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    return -1;
  } // end IF
  session_data_p = &stream_.sessionData ();
  const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
      &const_cast<StateType&> (inherited2::state ());
  //stream_.dump_state ();
  // *NOTE*: as soon as this returns, data starts arriving at
  // handle_output()/msg_queue()
  stream_.start ();
  if (!stream_.isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    return -1;
  } // end IF

  // step3: register with the reactor
  ACE_Reactor* reactor_p = inherited::reactor ();
  ACE_ASSERT (reactor_p);
  if (reactor_p->register_handler (this,
                                   ACE_Event_Handler::READ_MASK) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Reactor::register_handler(READ_MASK): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // *NOTE*: registered with the reactor (READ_MASK) at this point

//   // ...register for writes (WRITE_MASK) as well
//   if (reactor_p->register_handler (this,
//                                    ACE_Event_Handler::WRITE_MASK) == -1)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to ACE_Reactor::register_handler(WRITE_MASK): \"%m\", aborting\n")));
//     return -1;
//   } // end IF

  // step4: register with the connection manager (if any)
  if (!inherited2::registerc ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    return -1;
  } // end IF

  // *NOTE*: let the reactor manage this handler...
  // *WARNING*: this has some implications (see close() below)
  if (!inherited2::configuration_.streamConfiguration.useThreadPerConnection)
    inherited::remove_reference ();

  return 0;
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                    ACE_TEXT ("failed to SocketHandlerType::close(): \"%m\", aborting\n")));

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

      //  step2: release the socket handle
      if (handle != ACE_INVALID_HANDLE)
      {
        int result_2 = ACE_OS::closesocket (handle);
        if (result_2 == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                      handle));
      } // end IF

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

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  // read a datagram from the socket
  bool enqueue = true;
  ACE_Message_Block* buffer_p =
   allocateMessage (inherited2::configuration_.streamConfiguration.bufferSize);
  if (!buffer_p)
  {
    if (inherited2::configuration_.streamConfiguration.messageAllocator->block ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  inherited2::configuration_.streamConfiguration.bufferSize));
      return -1;
    } // end IF

    // no buffer available --> drop datagram and continue
    enqueue = false;
    ACE_NEW_NORETURN (buffer_p,
                      ACE_Message_Block (inherited2::configuration_.streamConfiguration.bufferSize,
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
                  inherited2::configuration_.streamConfiguration.bufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (buffer_p);

  // read a datagram from the socket...
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
        const_cast<typename StreamType::STATE_T*> (stream_.getState ());
    state_p->currentStatistics.numDroppedMessages++;

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

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  ssize_t bytes_sent = -1;

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be
  // dispatching the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
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
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    // been notified to the reactor
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
                    (inherited2::configuration_.streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
                                                                                           : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));

      goto release;
    } // end IF
  } // end IF
  ACE_ASSERT (currentWriteBuffer_);

  // finished ?
  if (inherited2::configuration_.streamConfiguration.useThreadPerConnection &&
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
      if ((error != ECONNRESET) &&
          (error != ECONNABORTED) &&
          (error != EPIPE) &&      // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != ENOTSOCK) &&
          (error != EBADF))        // <-- connection abort()ed locally
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

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

  int result = -1;

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
      if (!inherited2::configuration_.streamConfiguration.useThreadPerConnection)
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
                ACE_TEXT ("failed to SocketHandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, mask_in));

  // step4: deregister with the connection manager (if any)
  inherited2::deregister ();

  // step5: release a reference
  // *IMPORTANT NOTE*: may 'delete this'
  this->decrease ();

  return result;
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::collect"));

  try
  {
    return stream_.collect (data_out);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::report"));

  try
  {
    return stream_.report ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                 AddressType& localSAP_out,
                                                                 AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::info"));

  int result = -1;

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  result = inherited::peer_.get_local_addr (localSAP_out);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  remoteSAP_out =
    inherited2::configuration_.socketConfiguration.peerAddress;
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
unsigned int
Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StatisticContainerType,
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ());
#else
  return static_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ());
#endif
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::stream () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::stream"));

  return stream_;
}

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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

template <typename HandlerConfigurationType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                          StreamType,
                          UserDataType,
                          ModuleConfigurationType,
                          ModuleHandlerConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_.streamConfiguration.messageAllocator)
  {
allocate:
    try
    {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_.streamConfiguration.messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_.streamConfiguration.messageAllocator->block ())
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
    if (inherited2::configuration_.streamConfiguration.messageAllocator)
    {
      if (inherited2::configuration_.streamConfiguration.messageAllocator->block ())
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
