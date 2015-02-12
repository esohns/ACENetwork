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

#include "net_defines.h"
#include "net_macros.h"
#include "net_stream_common.h"

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::Net_StreamUDPSocketBase_T ()//MANAGER_T* manager_in)
 : inherited ()//manager_in)
 //, inherited2 ()
 //, userData_ ()
// , stream_ ()
 , currentReadBuffer_ (NULL)
// , sendLock_ ()
 , currentWriteBuffer_ (NULL)
 , serializeOutput_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::Net_StreamUDPSocketBase_T"));

}

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::~Net_StreamUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::~Net_StreamUDPSocketBase_T"));

  // clean up
  if (configuration_.module)
  {
    if (stream_.find (configuration_.module->name ()))
      if (stream_.remove (configuration_.module->name (),
                          ACE_Module_Base::M_DELETE_NONE) == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
                    ACE_TEXT (configuration_.module->name ())));

    if (configuration_.deleteModule)
      delete configuration_.module;
  } // end IF

  if (currentReadBuffer_)
    currentReadBuffer_->release ();
  if (currentWriteBuffer_)
    currentWriteBuffer_->release ();
}

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
int
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::open (const ConfigurationType& configuration_in,
                                                    const ACE_INET_Addr& peerAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::open"));

  configuration_ = configuration_in.streamSocketConfiguration;

  // step0: init this
  // *TODO*
  serializeOutput_ = configuration_.serializeOutput;

  // step1: init/start stream
  configuration_.sessionID = reinterpret_cast<unsigned int> (inherited::get_handle ()); // (== socket handle)
  // step1a: connect stream head message queue with the reactor notification
  // pipe ?
  if (!configuration_.useThreadPerConnection)
  {
    // *IMPORTANT NOTE*: enable the reference counting policy, as this will
    // be registered with the reactor several times (1x READ_MASK, nx
    // WRITE_MASK); therefore several threads MAY be dispatching notifications
    // (yes, even concurrently; myLock enforces the proper sequence order, see
    // handle_output()) on the SAME handler. When the socket closes, the event
    // handler should thus not be destroyed() immediately, but simply purge any
    // pending notifications (see handle_close()) and de-register; after the
    // last active notification has been dispatched, it will be safely deleted
    inherited2::reference_counting_policy ().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);
    // *IMPORTANT NOTE*: due to reference counting, the
    // ACE_Svc_Handle::shutdown() method will crash, as it references a
    // connection recycler AFTER removing the connection from the reactor (which
    // releases a reference). In the case that "this" is the final reference,
    // this leads to a crash. (see code)
    // --> avoid invoking ACE_Svc_Handle::shutdown()
    // --> this means that "manual" cleanup is necessary (see handle_close())
    //inherited2::closing_ = true;

    configuration_.notificationStrategy = &(inherited::myNotificationStrategy);
  } // end IF
  // step1b: init final module (if any)
  if (configuration_.module)
  {
    Net_IModule_t* imodule_handle = NULL;
    // need a downcast...
    imodule_handle = dynamic_cast<Net_IModule_t*> (configuration_.module);
    if (!imodule_handle)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<RPG_Stream_IModule> failed, aborting\n"),
                  ACE_TEXT (configuration_.module->name ())));
      return -1;
    } // end IF
    Net_Module_t* clone = NULL;
    try
    {
      clone = imodule_handle->clone ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in RPG_Stream_IModule::clone(), aborting\n"),
                  ACE_TEXT (configuration_.module->name ())));
      return -1;
    }
    if (!clone)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to RPG_Stream_IModule::clone(), aborting\n"),
                  ACE_TEXT (configuration_.module->name ())));
      return -1;
    }
    configuration_.module = clone;
    configuration_.deleteModule = true;
  } // end IF
  // step1c: init stream
  if (!stream_.init (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to init processing stream, aborting\n")));

    // clean up
    delete configuration_.module;
    configuration_.module = NULL;

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
  int result = inherited::open (ACE_Addr::sap_any,        // local
                                ACE_PROTOCOL_FAMILY_INET, // protocol family
                                0,                        // protocol
                                0);                       // reuse_addr
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram::open(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  //// *NOTE*: as soon as this returns, data starts arriving at handle_input()

  // *NOTE*: let the reactor manage this handler...
  // *WARNING*: this has some implications (see close() below)
  if (!configuration_.useThreadPerConnection)
    inherited2::remove_reference ();

  return 0;
}

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
int
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check
  ACE_ASSERT (currentReadBuffer_ == NULL);

  // read some data from the socket
  currentReadBuffer_ = allocateMessage (configuration_.bufferSize);
  if (currentReadBuffer_ == NULL)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                configuration_.bufferSize));

    return -1;
  } // end IF

  // read some data from the socket...
  ACE_INET_Addr peer_address;
  size_t bytes_received = inherited::recv (currentReadBuffer_->wr_ptr (),
                                           currentReadBuffer_->size (),
                                           peer_address,
                                           0);
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
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
int
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::handle_close (ACE_HANDLE handle_in,
                                                            ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::handle_close"));

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
      if (!configuration_.useThreadPerConnection)
        if (inherited2::reactor ()->purge_pending_notifications (this,
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

  //// invoke base-class maintenance
  //return inherited2::handle_close (handle_in,
  //                                 mask_in);
  return 0;
}

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
bool
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::collect (StatisticsContainerType& data_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::collect"));

  try
  {
    return stream_.collect (data_out);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in RPG_Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
void
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::report"));

  try
  {
    return stream_.report ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in RPG_Common_IStatistic::report(), aborting\n")));
  }
}

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
ACE_Message_Block*
Net_StreamUDPSocketBase_T<ConfigurationType,
                          StatisticsContainerType,
                          StreamType,
                          SocketType,
                          SocketHandlerType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamUDPSocketBase_T::allocateMessage"));

  ACE_ASSERT (configuration_.messageAllocator);

  // init return value(s)
  ACE_Message_Block* message_out = NULL;

  try
  {
    message_out = static_cast<ACE_Message_Block*> (configuration_.messageAllocator->malloc (requestedSize_in));
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
