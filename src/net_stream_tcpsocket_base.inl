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

#include "stream_common.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
Net_StreamTCPSocketBase_T<ConfigurationType,
                          SessionDataType,
                          StatisticsContainerType,
                          StreamType,
                          SocketHandlerType>::Net_StreamTCPSocketBase_T ()//MANAGER_T* manager_in)
 : inherited ()//manager_in)
 //, configuration_ (NULL)
// , stream_ ()
 , currentReadBuffer_ (NULL)
// , sendLock_ ()
 , currentWriteBuffer_ (NULL)
 //, serializeOutput_ (false)
 , state_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::Net_StreamTCPSocketBase_T"));

}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
Net_StreamTCPSocketBase_T<ConfigurationType,
                          SessionDataType,
                          StatisticsContainerType,
                          StreamType,
                          SocketHandlerType>::~Net_StreamTCPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::~Net_StreamTCPSocketBase_T"));

  if (configuration_.streamConfiguration.module)
  {
    if (stream_.find (configuration_.streamConfiguration.module->name ()))
      if (stream_.remove (configuration_.streamConfiguration.module->name (),
                          ACE_Module_Base::M_DELETE_NONE) == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
                    ACE_TEXT (configuration_.streamConfiguration.module->name ())));

    if (configuration_.streamConfiguration.deleteModule)
      delete configuration_.streamConfiguration.module;
  } // end IF

  if (currentReadBuffer_)
    currentReadBuffer_->release ();
  if (currentWriteBuffer_)
    currentWriteBuffer_->release ();
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
int
Net_StreamTCPSocketBase_T<ConfigurationType,
                          SessionDataType,
                          StatisticsContainerType,
                          StreamType,
                          SocketHandlerType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::open"));

  // sanity check(s)
  //ACE_ASSERT (!inherited2::configuration_);
  ACE_ASSERT (state_);

  //configuration_ = reinterpret_cast<ConfigurationType*> (arg_in);

  // step0: init this
  //// *TODO*: find a better way to do this
  //serializeOutput_ = configuration_->streamConfiguration.serializeOutput;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  state_->sessionID = *static_cast<unsigned int*> (inherited::get_handle ()); // (== socket handle)
#else
  state_->sessionID = static_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
#endif

  // step1: init/start stream
  // step1a: connect stream head message queue with the reactor notification
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
    // *IMPORTANT NOTE*: due to reference counting, the
    // ACE_Svc_Handle::shutdown() method will crash, as it references a
    // connection recycler AFTER removing the connection from the reactor (which
    // releases a reference). In the case that "this" is the final reference,
    // this leads to a crash. (see code)
    // --> avoid invoking ACE_Svc_Handle::shutdown()
    // --> this means that "manual" cleanup is necessary (see handle_close())
    inherited::closing_ = true;

    configuration_.streamConfiguration.notificationStrategy =
        &(inherited::notificationStrategy_);
  } // end IF
  // step1b: init final module (if any)
  if (configuration_.streamConfiguration.module)
  {
    Stream_IModule_t* imodule_handle = NULL;
    // need a downcast...
    imodule_handle =
        dynamic_cast<Stream_IModule_t*> (configuration_.streamConfiguration.module);
    if (!imodule_handle)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule_t*> failed, aborting\n"),
                  ACE_TEXT (configuration_.streamConfiguration.module->name ())));

      return -1;
    } // end IF
    Common_Module_t* clone = NULL;
    try
    {
      clone = imodule_handle->clone ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IModule::clone(), aborting\n"),
                  ACE_TEXT (configuration_.streamConfiguration.module->name ())));

      return -1;
    }
    if (!clone)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_IModule::clone(), aborting\n"),
                  ACE_TEXT (configuration_.streamConfiguration.module->name ())));

      return -1;
    }
    configuration_.streamConfiguration.module = clone;
    configuration_.streamConfiguration.deleteModule = true;
  } // end IF
  // step1c: init stream
  if (!stream_.init (state_->sessionID,
                     configuration_.streamConfiguration,
                     configuration_.protocolConfiguration,
                     configuration_.streamUserData))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to init processing stream, aborting\n")));

    return -1;
  } // end IF
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

  // step2: tweak socket, register I/O handle with the reactor, ...
  // *NOTE*: as soon as this returns, data starts arriving at handle_input()
  int result = inherited::open (arg_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_TCPSocketHandler::open(): \"%m\", aborting\n")));

    return -1;
  } // end IF

  // *NOTE*: let the reactor manage this handler...
  // *WARNING*: this has some implications (see close() below)
  if (!configuration_.streamConfiguration.useThreadPerConnection)
    inherited::remove_reference ();

  return 0;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
int
Net_StreamTCPSocketBase_T<ConfigurationType,
                          SessionDataType,
                          StatisticsContainerType,
                          StreamType,
                          SocketHandlerType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check
  ACE_ASSERT (currentReadBuffer_ == NULL);

  // read some data from the socket
  currentReadBuffer_ =
      allocateMessage (configuration_.streamConfiguration.bufferSize);
  if (currentReadBuffer_ == NULL)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                configuration_.streamConfiguration.bufferSize));

    return -1;
  } // end IF

  // read some data from the socket...
  size_t bytes_received = inherited::peer_.recv (currentReadBuffer_->wr_ptr (),
                                                 currentReadBuffer_->size ());
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
                    ACE_TEXT ("failed to ACE_SOCK_Stream::recv(): \"%m\", returning\n")));

      // clean up
      currentReadBuffer_->release ();
      currentReadBuffer_ = NULL;

      return -1;
    }
    // *** GOOD CASES ***
    case 0:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//                   handle_in));

      // clean up
      currentReadBuffer_->release ();
      currentReadBuffer_ = NULL;

      return -1;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: received %u bytes...\n"),
//                   handle_in,
//                   bytes_received));

      // adjust write pointer
      currentReadBuffer_->wr_ptr (bytes_received);

      break;
    }
  } // end SWITCH

  // push the buffer onto our stream for processing
  // *NOTE*: the stream assumes ownership of the buffer
  if (stream_.put (currentReadBuffer_) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", aborting\n")));

    // clean up
    currentReadBuffer_->release ();
    currentReadBuffer_ = NULL;

    return -1;
  } // end IF
  currentReadBuffer_ = NULL;

  return 0;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
int
Net_StreamTCPSocketBase_T<ConfigurationType,
                          SessionDataType,
                          StatisticsContainerType,
                          StreamType,
                          SocketHandlerType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be
  // dispatching the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
  if (configuration_.streamConfiguration.serializeOutput)
    sendLock_.acquire ();

  if (currentWriteBuffer_ == NULL)
  {
    // send next data chunk from the stream...
    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
    // been notified to the reactor
    int result = -1;
    if (!configuration_.streamConfiguration.useThreadPerConnection)
      result = stream_.get (currentWriteBuffer_,
                            const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    else
      result = inherited::getq (currentWriteBuffer_,
                                const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
    if (result == -1)
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection has been closed in the meantime
      // - queue has been deactivated
      int error = ACE_OS::last_error ();
      if ((error != EAGAIN) ||  // <-- connection has been closed in the meantime
          (error != ESHUTDOWN)) // <-- queue has been deactivated
        ACE_DEBUG ((LM_ERROR,
                    (configuration_.streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
                                                                               : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));

      // clean up
      if (configuration_.streamConfiguration.serializeOutput)
        sendLock_.release ();

      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (currentWriteBuffer_);

  // finished ?
  if (configuration_.streamConfiguration.useThreadPerConnection &&
      currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
  {
    currentWriteBuffer_->release ();
    currentWriteBuffer_ = NULL;

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: finished sending...\n"),
//                   peer_.get_handle ()));

    // clean up
    if (configuration_.streamConfiguration.serializeOutput)
      sendLock_.release ();

    return -1;
  } // end IF

  // put some data into the socket...
  ssize_t bytes_sent = inherited::peer_.send (currentWriteBuffer_->rd_ptr (),
                                              currentWriteBuffer_->length ());
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
      if (configuration_.streamConfiguration.serializeOutput)
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
      if (configuration_.streamConfiguration.serializeOutput)
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
  if (currentWriteBuffer_ != NULL)
  {
    // clean up
    if (configuration_.streamConfiguration.serializeOutput)
      sendLock_.release ();

    return 1;
  } // end IF
    //if (inherited::reactor ()->schedule_wakeup (this,
    //                                            ACE_Event_Handler::WRITE_MASK) == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

  // clean up
  if (configuration_.streamConfiguration.serializeOutput)
    sendLock_.release ();

  return 0;
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
int
Net_StreamTCPSocketBase_T<ConfigurationType,
                          SessionDataType,
                          StatisticsContainerType,
                          StreamType,
                          SocketHandlerType>::handle_close (ACE_HANDLE handle_in,
                                                            ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamTCPSocketBase_T::handle_close"));

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
      {
        stream_.stop ();
        stream_.waitForCompletion ();
      } // end IF

      // step2: purge any pending notifications ?
      // *IMPORTANT NOTE*: if called from a non-reactor context, or when using a
      // a multithreaded reactor, there may still be in-flight notifications
      // being dispatched at this stage, so this just speeds things up a little
      if (!configuration_.streamConfiguration.useThreadPerConnection)
        if (inherited::reactor ()->purge_pending_notifications (this,
                                                                ACE_Event_Handler::ALL_EVENTS_MASK) == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::purge_pending_notifications(%@): \"%m\", continuing\n"),
                      this));

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
  return inherited::handle_close (handle_in,
                                  mask_in);
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
ACE_Message_Block*
Net_StreamTCPSocketBase_T<ConfigurationType,
                          SessionDataType,
                          StatisticsContainerType,
                          StreamType,
                          SocketHandlerType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE(ACE_TEXT("Net_StreamTCPSocketBase_T::allocateMessage"));

  // init return value(s)
  ACE_Message_Block* message_out = NULL;

  try
  {
    message_out =
        static_cast<ACE_Message_Block*> (configuration_.streamConfiguration.messageAllocator->malloc (requestedSize_in));
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  }
  if (!message_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
}
