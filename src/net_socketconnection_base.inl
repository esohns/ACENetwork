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

//#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
//#include "ace/Netlink_Addr.h"
#include "ace/Svc_Handler.h"

#include "net_common.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
Net_SocketConnectionBase_T<SocketHandlerType,
                           ITransportLayerType,
                           ConfigurationType,
                           SocketHandlerConfigurationType,
                           UserDataType,
                           SessionDataType,
                           StatisticContainerType>::Net_SocketConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                unsigned int statisticsCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticsCollectionInterval_in)
 //, inherited2 ()
 //, inherited3 (interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::Net_SocketConnectionBase_T"));

}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
Net_SocketConnectionBase_T<SocketHandlerType,
                           ITransportLayerType,
                           ConfigurationType,
                           SocketHandlerConfigurationType,
                           UserDataType,
                           SessionDataType,
                           StatisticContainerType>::~Net_SocketConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::~Net_SocketConnectionBase_T"));

  // wait for our worker (if any)
  if (SocketHandlerType::CONNECTION_BASE_T::configuration_.streamConfiguration.useThreadPerConnection)
    if (inherited::wait () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
void
Net_SocketConnectionBase_T<SocketHandlerType,
                           ITransportLayerType,
                           ConfigurationType,
                           SocketHandlerConfigurationType,
                           UserDataType,
                           SessionDataType,
                           StatisticContainerType>::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::ping"));

  inherited::stream_.ping ();
}

//template <typename SocketHandlerType,
//          typename ITransportLayerType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename StatisticContainerType>
//int
//Net_SocketConnectionBase_T<SocketHandlerType,
//                           ITransportLayerType,
//                           ConfigurationType,
//                           SessionDataType,
//                           StatisticContainerType>::svc (void)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::svc"));

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("(%t) worker (connection: %u) starting...\n"),
//              inherited::get_handle ()));

//  int result = -1;
//  ssize_t bytes_sent = 0;
//  while (true)
//  {
//    if (!inherited::currentWriteBuffer_)
//    {
//      result = inherited::stream_.get (inherited::currentWriteBuffer_,
//                                       NULL); // block
//      if (result == -1)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));

//        return -1;
//      } // end IF
//    } // end IF

//    // finished ?
//    if (inherited::currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
//    {
//      // clean up
//      inherited::currentWriteBuffer_->release ();
//      inherited::currentWriteBuffer_ = NULL;

////       ACE_DEBUG ((LM_DEBUG,
////                   ACE_TEXT ("[%u]: finished sending...\n"),
////                   peer_.get_handle ()));

//      break;
//    } // end IF

//    // put some data into the socket...
//    bytes_sent =
//        inherited::peer_.send (inherited::currentWriteBuffer_->rd_ptr (),
//                               inherited::currentWriteBuffer_->length (),
//                               0, // default behavior
//                               //MSG_DONTWAIT, // don't block
//                               NULL); // block if necessary...
//    switch (bytes_sent)
//    {
//      case -1:
//      {
//        // connection reset by peer/broken pipe ? --> not an error
//        int error = ACE_OS::last_error ();
//        if ((error != ECONNRESET) &&
//            (error != ENOTSOCK) &&
//            (error != EPIPE))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_SOCK_Stream::send(): \"%m\", returning\n")));

//        // clean up
//        inherited::currentWriteBuffer_->release ();
//        inherited::currentWriteBuffer_ = NULL;

//        // nothing to do but wait for the shutdown signal (see above)...
//        break;
//      }
//      // *** GOOD CASES ***
//      case 0:
//      {
////        ACE_DEBUG ((LM_DEBUG,
////                    ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
////                    inherited::peer_.get_handle ()));

//        // clean up
//        inherited::currentWriteBuffer_->release ();
//        inherited::currentWriteBuffer_ = NULL;

//        // nothing to do but wait for the shutdown signal (see above)...
//        break;
//      }
//      default:
//      {
////        ACE_DEBUG ((LM_DEBUG,
////                    ACE_TEXT ("[%u]: sent %u bytes...\n"),
////                    peer_.get_handle (),
////                    bytes_sent));

//        // finished with this buffer ?
//        if (static_cast<size_t> (bytes_sent) == inherited::currentWriteBuffer_->length ())
//        {
//          // get the next one...
//          inherited::currentWriteBuffer_->release ();
//          inherited::currentWriteBuffer_ = NULL;
//        } // end IF
//        else
//        {
//          // there's more data --> adjust read pointer
//          inherited::currentWriteBuffer_->rd_ptr (bytes_sent);
//        } // end ELSE

//        break;
//      }
//    } // end SWITCH
//  } // end WHILE

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("(%t) worker (connection: %u) joining...\n"),
//              inherited::get_handle ()));

//  return 0;
//}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
int
Net_SocketConnectionBase_T<SocketHandlerType,
                           ITransportLayerType,
                           ConfigurationType,
                           SocketHandlerConfigurationType,
                           UserDataType,
                           SessionDataType,
                           StatisticContainerType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::open"));

  int result = -1;
  ConfigurationType* configuration_p =
      reinterpret_cast<ConfigurationType*> (arg_in);

  // step0: initialize this connection (if there is no connection manager)
  if (!inherited::manager_)
  {
    // sanity check(s)
    ACE_ASSERT (configuration_p);

    if (!SocketHandlerType::CONNECTION_BASE_T::initialize (*configuration_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(): \"%m\", aborting\n")));
      return -1;
    } // end IF
  } // end IF

  // step1: init/start stream, tweak socket, register reading data with reactor
  // , ...
  result = inherited::open (arg_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to SocketHandlerType::open(): \"%m\", aborting\n")));
    return -1;
  } // end IF

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

  return 0;
}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
int
Net_SocketConnectionBase_T<SocketHandlerType,
                           ITransportLayerType,
                           ConfigurationType,
                           SocketHandlerConfigurationType,
                           UserDataType,
                           SessionDataType,
                           StatisticContainerType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::close"));

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

        break;
      } // end IF

      // too many connections: invoke inherited default behavior
      // --> simply fall through to the next case
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. too many connections)
    // *NOTE*: this (eventually) calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
      return inherited::close (arg_in);
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument: %u, returning\n"),
                  arg_in));

      break;
    }
  } // end SWITCH

  return 0;
}

//template <typename SocketHandlerType,
//          typename ITransportLayerType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename StatisticContainerType>
//int
//Net_SocketConnectionBase_T<SocketHandlerType,
//                           ITransportLayerType,
//                           ConfigurationType,
//                           SessionDataType,
//                           StatisticContainerType>::handle_close (ACE_HANDLE handle_in,
//                                                                   ACE_Reactor_Mask mask_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::handle_close"));

//  ACE_UNUSED_ARG (handle_in);

//  switch (mask_in)
//  {
//    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
//    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused) /
//      //     accept failed (e.g. too many connections) /
//      //     select failed (EBADF see Select_Reactor_T.cpp)
//      //     asynch abort
//      break;
//    case ACE_Event_Handler::EXCEPT_MASK:
//      // *IMPORTANT NOTE*: the TP_Reactor dispatches notifications in parallel
//      // to regular socket events, thus several notifications may be in flight
//      // at the same time. In order to avoid confusion when the socket closes,
//      // proper synchronization is handled through the reference counting
//      // mechanism, i.e. the handler is only deleted after the last reference
//      // has been released. Still, handling notifications after this occurs
//      // will fail, and the reactor will invoke this method each time. As there
//      // is no need to go through the shutdown procedure several times, bail
//      // out early here
//      //if (handle_in == ACE_INVALID_HANDLE) // <-- notification has completed (!useThreadPerConnection)
//      //  ACE_DEBUG((LM_ERROR,
//      //             ACE_TEXT("notification completed, continuing\n")));
//      return 0;
//    default:
//      // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %u) --> check implementation !, continuing\n"),
//                  ACE_Event_Handler::get_handle (),
//                  mask_in));
//#else
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
//                  ACE_Event_Handler::get_handle (),
//                  mask_in));
//#endif
//      break;
//  } // end SWITCH

//  //// step1: connection shutting down --> signal any worker(s)
//  //if (inherited::myConfiguration.useThreadPerConnection)
//  //  shutdown ();

////  // step2: de-register this connection
////  if (inherited::manager_ &&
////      inherited::isRegistered_)
////  { // (try to) de-register with the connection manager
////    try
////    {
////      inherited::manager_->deregisterConnection (this);
////    }
////    catch (...)
////    {
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
////    }
////  } // end IF

//  // step3: invoke base-class maintenance
//  int result = inherited::handle_close (ACE_Event_Handler::get_handle (), // event handle
//                                        mask_in);     // event mask
//  if (result == -1)
//    // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%@, %d): \"%m\", continuing\n"),
//                ACE_Event_Handler::get_handle (),
//                mask_in));
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%d, %d): \"%m\", continuing\n"),
//                ACE_Event_Handler::get_handle (),
//                mask_in));
//#endif

//  return result;
//}

//template <typename SocketHandlerType,
//          typename ITransportLayerType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename StatisticContainerType>
//void
//Net_SocketConnectionBase_T<SocketHandlerType,
//                           ITransportLayerType,
//                           ConfigurationType,
//                           SessionDataType,
//                           StatisticContainerType>::shutdown ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_SocketConnectionBase_T::shutdown"));

//  ACE_Message_Block* stop_mb = NULL;
//  ACE_NEW_NORETURN (stop_mb,
//                    ACE_Message_Block (0,                                  // size
//                                       ACE_Message_Block::MB_STOP,         // type
//                                       NULL,                               // continuation
//                                       NULL,                               // data
//                                       NULL,                               // buffer allocator
//                                       NULL,                               // locking strategy
//                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
//                                       ACE_Time_Value::zero,               // execution time
//                                       ACE_Time_Value::max_time,           // deadline time
//                                       NULL,                               // data block allocator
//                                       NULL));                             // message allocator)
//  if (!stop_mb)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));
//    return;
//  } // end IF

//  if (inherited::putq (stop_mb, NULL) == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", continuing\n")));

//    stop_mb->release ();
//  } // end IF

//  // *NOTE*: defer waiting for any worker(s) to the dtor
//}

/////////////////////////////////////////

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
Net_AsynchSocketConnectionBase_T<SocketHandlerType,
                                 ITransportLayerType,
                                 ConfigurationType,
                                 SocketHandlerConfigurationType,
                                 UserDataType,
                                 SessionDataType,
                                 StatisticContainerType>::Net_AsynchSocketConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                            unsigned int statisticsCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticsCollectionInterval_in)
 //, inherited2 ()
 //, inherited3 (interfaceHandle_in)
 , configuration_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::Net_AsynchSocketConnectionBase_T"));

}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
Net_AsynchSocketConnectionBase_T<SocketHandlerType,
                                 ITransportLayerType,
                                 ConfigurationType,
                                 SocketHandlerConfigurationType,
                                 UserDataType,
                                 SessionDataType,
                                 StatisticContainerType>::~Net_AsynchSocketConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::~Net_AsynchSocketConnectionBase_T"));

}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
void
Net_AsynchSocketConnectionBase_T<SocketHandlerType,
                                 ITransportLayerType,
                                 ConfigurationType,
                                 SocketHandlerConfigurationType,
                                 UserDataType,
                                 SessionDataType,
                                 StatisticContainerType>::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::ping"));

  inherited::stream_.ping ();
}

//template <typename SocketHandlerType,
//          typename ITransportLayerType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename StatisticContainerType>
//bool
//Net_AsynchSocketConnectionBase_T<SocketHandlerType,
//                                 ITransportLayerType,
//                                 ConfigurationType,
//                                 SessionDataType,
//                                 StatisticContainerType>::collect (StatisticContainerType& data_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::collect"));
//
//  try
//  {
//    return inherited::stream_.collect (data_out);
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
//  }
//
//  return false;
//}
//
//template <typename SocketHandlerType,
//          typename ITransportLayerType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename StatisticContainerType>
//void
//Net_AsynchSocketConnectionBase_T<SocketHandlerType,
//                                 ITransportLayerType,
//                                 ConfigurationType,
//                                 SessionDataType,
//                                 StatisticContainerType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::report"));
//
//  try
//  {
//    return inherited::stream_.report ();
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_IStatistic::report(), continuing\n")));
//  }
//}

//template <typename SocketHandlerType,
//          typename ConfigurationType,
//          typename StatisticContainerType>
//int
//Net_AsynchSocketConnectionBase_T::svc (void)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::svc"));
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("(%t) worker (connection: %u) starting...\n"),
//              inherited::stream_.getSessionID ()));
//
//  ssize_t bytes_sent = 0;
//  while (true)
//  {
//    if (inherited::currentWriteBuffer_ == NULL)
//      if (inherited::stream_.get (inherited::currentWriteBuffer_, NULL) == -1) // block
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
//        return -1;
//      } // end IF
//
//    // finished ?
//    if (inherited::currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
//    {
//      inherited::currentWriteBuffer_->release ();
//      inherited::currentWriteBuffer_ = NULL;
//
////       ACE_DEBUG ((LM_DEBUG,
////                   ACE_TEXT ("[%u]: finished sending...\n"),
////                   peer_.get_handle ()));
//
//      break;
//    } // end IF
//
//    // put some data into the socket...
//    bytes_sent = inherited::peer_.send (inherited::currentWriteBuffer_->rd_ptr (),
//                                        inherited::currentWriteBuffer_->length (),
//                                        0, // default behavior
//                                        //MSG_DONTWAIT, // don't block
//                                        NULL); // block if necessary...
//    switch (bytes_sent)
//    {
//      case -1:
//      {
//        // connection reset by peer/broken pipe ? --> not an error
//        int error = ACE_OS::last_error ();
//        if ((error != ECONNRESET) &&
//            (error != ENOTSOCK) &&
//            (error != EPIPE))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_SOCK_Stream::send(): \"%m\", returning\n")));
//
//        inherited::currentWriteBuffer_->release ();
//        inherited::currentWriteBuffer_ = NULL;
//
//        // nothing to do but wait for our shutdown signal (see above)...
//        break;
//      }
//      // *** GOOD CASES ***
//      case 0:
//      {
//        inherited::currentWriteBuffer_->release ();
//        inherited::currentWriteBuffer_ = NULL;
//
////         ACE_DEBUG((LM_DEBUG,
////                    ACE_TEXT("[%u]: socket was closed by the peer...\n"),
////                    peer_.get_handle ()));
//
//        // nothing to do but wait for our shutdown signal (see above)...
//        break;
//      }
//      default:
//      {
////         ACE_DEBUG ((LM_DEBUG,
////                    ACE_TEXT ("[%u]: sent %u bytes...\n"),
////                    peer_.get_handle (),
////                    bytes_sent));
//
//        // finished with this buffer ?
//        if (static_cast<size_t> (bytes_sent) == inherited::currentWriteBuffer_->length ())
//        {
//          // get the next one...
//          inherited::currentWriteBuffer_->release ();
//          inherited::currentWriteBuffer_ = NULL;
//        } // end IF
//        else
//        {
//          // there's more data --> adjust read pointer
//          inherited::currentWriteBuffer_->rd_ptr (bytes_sent);
//        } // end ELSE
//
//        break;
//      }
//    } // end SWITCH
//  } // end WHILE
//
//  //ACE_DEBUG ((LM_DEBUG,
//  //            ACE_TEXT ("(%t) worker (connection: %u) joining...\n"),
//  //            inherited::stream_.getSessionID ()));
//
//  return 0;
//}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
void
Net_AsynchSocketConnectionBase_T<SocketHandlerType,
                                 ITransportLayerType,
                                 ConfigurationType,
                                 SocketHandlerConfigurationType,
                                 UserDataType,
                                 SessionDataType,
                                 StatisticContainerType>::open (ACE_HANDLE handle_in,
                                                                ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::open"));

  // step0: initialize this connection (if there is no connection manager)
  if (!inherited::manager_)
  {
    ACE_ASSERT (configuration_);
//    SocketHandlerType* socket_handler_p = this;
//    typename SocketHandlerType::CONNECTION_BASE_T* connection_base_p =
//     socket_handler_p;
    if (!SocketHandlerType::CONNECTION_BASE_T::initialize (*configuration_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(): \"%m\", returning\n")));

      return;
    } // end IF
  } // end IF

  // step1: init/start stream, tweak socket, register reading data with reactor
  // , ...
  inherited::open (handle_in,
                   messageBlock_in);
}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
void
Net_AsynchSocketConnectionBase_T<SocketHandlerType,
                                 ITransportLayerType,
                                 ConfigurationType,
                                 SocketHandlerConfigurationType,
                                 UserDataType,
                                 SessionDataType,
                                 StatisticContainerType>::act (const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::act"));

  configuration_ =
      reinterpret_cast<ConfigurationType*> (const_cast<void*> (act_in));
}

template <typename SocketHandlerType,
          typename ITransportLayerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType>
int
Net_AsynchSocketConnectionBase_T<SocketHandlerType,
                                 ITransportLayerType,
                                 ConfigurationType,
                                 SocketHandlerConfigurationType,
                                 UserDataType,
                                 SessionDataType,
                                 StatisticContainerType>::handle_close (ACE_HANDLE handle_in,
                                                                        ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;

  switch (mask_in)
  {
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> asynch abort
      //     select failed (EBADF see Select_Reactor_T.cpp)
      //     asynch abort
      break;
    case ACE_Event_Handler::EXCEPT_MASK:
      // *IMPORTANT NOTE*: the TP_Reactor dispatches notifications in parallel
      // to regular socket events, thus several notifications may be in flight
      // at the same time. In order to avoid confusion when the socket closes,
      // proper synchronization is handled through the reference counting
      // mechanism, i.e. the handler is only deleted after the last reference
      // has been released. Still, handling notifications after this occurs
      // will fail, and the reactor will invoke this method each time. As there
      // is no need to go through the shutdown procedure several times, bail
      // out early here
      //if (handle_in == ACE_INVALID_HANDLE) // <-- notification has completed (!useThreadPerConnection)
      //  ACE_DEBUG((LM_ERROR,
      //             ACE_TEXT("notification completed, continuing\n")));
      return 0;
    default:
      // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %u) --> check implementation !, continuing\n"),
                  inherited::handle (),
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
                  inherited::handle (),
                  mask_in));
#endif
      break;
  } // end SWITCH

  //// step1: connection shutting down --> signal any worker(s)
  //if (inherited::myConfiguration.useThreadPerConnection)
  //  shutdown ();

//  // step2: de-register this connection
//  if (inherited::manager_ &&
//      inherited::isRegistered_)
//  { // (try to) de-register with the connection manager
//    try
//    {
//      inherited::manager_->deregisterConnection (this);
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
//    }
//  } // end IF

  // step3: invoke base-class maintenance
  result = inherited::handle_close (inherited::handle (), // event handle
                                    mask_in);             // event mask
  if (result == -1)
    // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%@, %d): \"%m\", continuing\n"),
                inherited::handle (),
                mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%d, %d): \"%m\", continuing\n"),
                inherited::handle (),
                mask_in));
#endif

  return result;
}

//template <typename SocketHandlerType,
//          typename ConfigurationType,
//          typename StatisticContainerType>
//void
//Net_AsynchSocketConnectionBase_T::shutdown ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchSocketConnectionBase_T::shutdown"));
//
//  ACE_Message_Block* stop_mb = NULL;
//  ACE_NEW_NORETURN (stop_mb,
//                    ACE_Message_Block (0,                                  // size
//                                       ACE_Message_Block::MB_STOP,         // type
//                                       NULL,                               // continuation
//                                       NULL,                               // data
//                                       NULL,                               // buffer allocator
//                                       NULL,                               // locking strategy
//                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
//                                       ACE_Time_Value::zero,               // execution time
//                                       ACE_Time_Value::max_time,           // deadline time
//                                       NULL,                               // data block allocator
//                                       NULL));                             // message allocator)
//  if (!stop_mb)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));
//    return;
//  } // end IF
//
//  if (inherited::putq (stop_mb, NULL) == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", continuing\n")));
//
//    stop_mb->release ();
//  } // end IF
//
//  // *NOTE*: defer waiting for any worker(s) to the dtor
//}
