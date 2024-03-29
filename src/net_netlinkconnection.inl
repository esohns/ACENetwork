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

#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_NetlinkConnection_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::Net_NetlinkConnection_T (bool managed_in)
 : inherited (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::Net_NetlinkConnection_T"));

}

//template <typename HandlerType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename UserDataType>
//bool
//Net_NetlinkConnection_T<HandlerType,
//                        ConfigurationType,
//                        StateType,
//                        StatisticContainerType,
//                        StreamType,
//                        UserDataType>::initialize (Common_DispatchType dispatch_in,
//                                                   Net_ClientServerRole role_in,
//                                                   const Net_SocketConfiguration& configuration_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::initialize"));

//  if (!inherited::CONNECTION_BASE_T::initialize (dispatch_in,
//                                                 role_in,
//                                                 configuration_in))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(), aborting")));
//    return false;
//  } // end IF

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string local_address;
//  Net_Netlink_Addr local_SAP, remote_SAP;
//  try
//  {
//    info (handle,
//          local_SAP,
//          remote_SAP);
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), aborting")));

//    return false;
//  }
//  // *TODO*: find a replacement for ACE_INET_Addr::addr_to_string
////  if (local_SAP.addr_to_string (buffer,
////                                sizeof (buffer)) == -1)
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to ACE_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
//  local_address = buffer;
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
////  if (remote_SAP.addr_to_string (buffer,
////                                 sizeof (buffer)) == -1)
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to ACE_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

//  // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("registered connection [0x%@/0x%@]: (\"%s\") <--> (\"%s\") (total: %d)\n"),
//              this, handle,
//              ACE_TEXT (localAddress.c_str ()),
//              ACE_TEXT (buffer),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("registered netlink connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)\n"),
//              this, handle,
//              ACE_TEXT (local_address.c_str ()),
//              ACE_TEXT (buffer),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif

//  return true;
//}

//template <typename HandlerType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename UserDataType>
//void
//Net_NetlinkConnection_T<HandlerType,
//                        ConfigurationType,
//                        StateType,
//                        StatisticContainerType,
//                        StreamType,
//                        UserDataType>::finalize ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::finalize"));

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  Net_Netlink_Addr address1, address2;
//  try
//  {
//    info (handle,
//          address1,
//          address2);
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), continuing\n")));
//  }

//  inherited::CONNECTION_BASE_T::finalize ();

//  // *PORTABILITY*
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered connection [0x%@/0x%@] (total: %u)\n"),
//              this, handle,
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered netlink connection [%@/%d] (total: %u)\n"),
//              this, handle,
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif
//}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_NetlinkConnection_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::info (ACE_HANDLE& handle_out,
                                             Net_Netlink_Addr& localSAP_out,
                                             Net_Netlink_Addr& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::info"));

  handle_out = inherited::peer_.get_handle ();
  if (inherited::peer_.get_local_addr (localSAP_out) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK::get_local_addr(): \"%m\", continuing\n")));
  if (inherited::peer_.get_remote_addr (remoteSAP_out) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK::get_remote_addr(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_ConnectionId_t
Net_NetlinkConnection_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::id"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Net_Netlink_Addr local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

  return static_cast<Net_ConnectionId_t> (handle);
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_NetlinkConnection_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Net_Netlink_Addr local_netlink_address, peer_netlink_address;
  info (handle,
        local_netlink_address,
        peer_netlink_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  result = local_netlink_address.addr_to_string (buffer,
                                                 sizeof (buffer),
                                                 1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = peer_netlink_address.addr_to_string (buffer,
                                                sizeof (buffer),
                                                1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [Id: %u [%u]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address.c_str ()),
              buffer));
}

//int
//Net_NetlinkConnection_T::svc (void)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::svc"));
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
////         ACE_DEBUG ((LM_DEBUG,
////                     ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
////                     peer_.get_handle ()));
//
//        // nothing to do but wait for our shutdown signal (see above)...
//        break;
//      }
//      default:
//      {
////         ACE_DEBUG ((LM_DEBUG,
////                     ACE_TEXT ("[%u]: sent %u bytes...\n"),
////                     peer_.get_handle (),
////                     bytes_sent));
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

//int
//Net_NetlinkConnection_T::open (void* arg_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::open"));
//
//  // step1: init/start stream, tweak socket, register reading data with reactor
//  // , ...
//  int result = inherited::open (arg_in);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("failed to Net_StreamSocketBase::open(): \"%m\", aborting\n")));
//    return -1;
//  } // end IF
//
//  // step2: start a worker ?
//  if (inherited::userData_.useThreadPerConnection)
//  {
//    ACE_thread_t thread_ids[1];
//    thread_ids[0] = 0;
//    ACE_hthread_t thread_handles[1];
//    thread_handles[0] = 0;
//    char thread_name[BUFSIZ];
//    ACE_OS::memset (thread_name, 0, sizeof (thread_name));
//    ACE_OS::strcpy (thread_name,
//                    NET_CONNECTION_HANDLER_THREAD_NAME);
//    const char* thread_names[1];
//    thread_names[0] = thread_name;
//    if (inherited::activate ((THR_NEW_LWP      |
//                              THR_JOINABLE     |
//                              THR_INHERIT_SCHED),                    // flags
//                             1,                                      // # threads
//                             0,                                      // force spawning
//                             ACE_DEFAULT_THREAD_PRIORITY,            // priority
//                             NET_CONNECTION_HANDLER_THREAD_GROUP_ID, // group id
//                             NULL,                                   // corresp. task --> use 'this'
//                             thread_handles,                         // thread handle(s)
//                             NULL,                                   // thread stack(s)
//                             NULL,                                   // thread stack size(s)
//                             thread_ids,                             // thread id(s)
//                             thread_names) == -1)                    // thread name(s)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Task_Base::activate(): \"%m\", aborting\n")));
//      return -1;
//    } // end IF
//  } // end IF
//
////  // step3: register this connection
////  if (inherited::manager_)
////  {
////    // (try to) register with the connection manager...
////    try
////    {
////      inherited::isRegistered_ = inherited::manager_->registerConnection (this);
////    }
////    catch (...)
////    {
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("caught exception in Net_IConnectionManager::registerConnection(), continuing\n")));
////    }
////    if (!inherited::isRegistered_)
////    {
////      // (most probably) too many connections...
////      ACE_OS::last_error (EBUSY);
////      return -1;
////    } // end IF
////  } // end IF
//
//  return 0;
//}

//int
//Net_NetlinkConnection_T::close (u_long arg_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::close"));
//  // [*NOTE*: hereby we override the default behavior of a ACE_Svc_Handler,
//  // which would call handle_close() AGAIN]
//
//  // *NOTE*: this method will be invoked
//  // - by any worker after returning from svc()
//  //    --> in this case, this should be a NOP (triggered from handle_close(),
//  //        which was invoked by the reactor) - we override the default
//  //        behavior of a ACE_Svc_Handler, which would call handle_close() AGAIN
//  // - by the connector/acceptor when open() fails (e.g. too many connections !)
//  //    --> shutdown
//
//  switch (arg_in)
//  {
//    // called by:
//    // - any worker from ACE_Task_Base on clean-up
//    // - acceptor/connector if there are too many connections (i.e. open()
//    //   returned -1)
//    case NORMAL_CLOSE_OPERATION:
//    {
//      // check specifically for the first case...
//      if (ACE_OS::thr_equal (ACE_Thread::self (),
//                             inherited::last_thread ()))
//      {
////       if (module ())
////         ACE_DEBUG((LM_DEBUG,
////                    ACE_TEXT ("\"%s\" worker thread (ID: %t) leaving...\n"),
////                    ACE_TEXT (name ())));
////       else
////         ACE_DEBUG ((LM_DEBUG,
////                     ACE_TEXT ("worker thread (ID: %t) leaving...\n")));
//
//        break;
//      } // end IF
//
//      // too many connections: invoke inherited default behavior
//      // --> simply fall through to the next case
//    }
//    // called by external (e.g. reactor) thread wanting to close the connection
//    // (e.g. too many connections)
//    // *NOTE*: this eventually calls handle_close() (see below)
//    case CLOSE_DURING_NEW_CONNECTION:
//      return inherited::close (arg_in);
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid argument: %u, returning\n"),
//                  arg_in));
//
//      break;
//    }
//  } // end SWITCH
//
//  return 0;
//}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_NetlinkConnection_T<HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                     ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::handle_close"));

  switch (mask_in)
  {
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused) /
      //     accept failed (e.g. too many connections) /
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

  //// step1: connection shutting down --> signal any worker(s)
  //if (inherited::userData_.useThreadPerConnection)
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
  int result = inherited::handle_close (handle_in, // event handle
                                        mask_in);   // event mask
  if (result == -1)
    // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%@, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%d, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#endif

  return result;
}

//void
//Net_NetlinkConnection_T::shutdown ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_NetlinkConnection_T::shutdown"));
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

//////////////////////////////////////////

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_AsynchNetlinkConnection_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::Net_AsynchNetlinkConnection_T (bool managed_in)
 : inherited (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::Net_AsynchNetlinkConnection_T"));

}

////int
////Net_AsynchNetlinkConnection_T::svc (void)
////{
////  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::svc"));
////
////  ACE_DEBUG((LM_DEBUG,
////             ACE_TEXT("(%t) worker (connection: %u) starting...\n"),
////             inherited::stream_.getSessionID ()));
////
////  ssize_t bytes_sent = 0;
////  while (true)
////  {
////    if (inherited::currentWriteBuffer_ == NULL)
////      if (inherited::stream_.get (inherited::currentWriteBuffer_, NULL) == -1) // block
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
////        return -1;
////      } // end IF
////
////    // finished ?
////    if (inherited::currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
////    {
////      inherited::currentWriteBuffer_->release ();
////      inherited::currentWriteBuffer_ = NULL;
////
//////       ACE_DEBUG ((LM_DEBUG,
//////                   ACE_TEXT ("[%u]: finished sending...\n"),
//////                   peer_.get_handle ()));
////
////      break;
////    } // end IF
////
////    // put some data into the socket...
////    bytes_sent = inherited::peer_.send (inherited::currentWriteBuffer_->rd_ptr (),
////                                        inherited::currentWriteBuffer_->length (),
////                                        0, // default behavior
////                                        //MSG_DONTWAIT, // don't block
////                                        NULL); // block if necessary...
////    switch (bytes_sent)
////    {
////      case -1:
////      {
////        // connection reset by peer/broken pipe ? --> not an error
////        int error = ACE_OS::last_error ();
////        if ((error != ECONNRESET) &&
////            (error != ENOTSOCK) &&
////            (error != EPIPE))
////          ACE_DEBUG ((LM_ERROR,
////                      ACE_TEXT ("failed to ACE_SOCK_Stream::send(): \"%m\", returning\n")));
////
////        inherited::currentWriteBuffer_->release ();
////        inherited::currentWriteBuffer_ = NULL;
////
////        // nothing to do but wait for our shutdown signal (see above)...
////        break;
////      }
////      // *** GOOD CASES ***
////      case 0:
////      {
////        inherited::currentWriteBuffer_->release ();
////        inherited::currentWriteBuffer_ = NULL;
////
//////         ACE_DEBUG ((LM_DEBUG,
//////                     ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//////                     peer_.get_handle ()));
////
////        // nothing to do but wait for our shutdown signal (see above)...
////        break;
////      }
////      default:
////      {
//////         ACE_DEBUG ((LM_DEBUG,
//////                     ACE_TEXT ("[%u]: sent %u bytes...\n"),
//////                     peer_.get_handle (),
//////                     bytes_sent));
////
////        // finished with this buffer ?
////        if (static_cast<size_t> (bytes_sent) == inherited::currentWriteBuffer_->length ())
////        {
////          // get the next one...
////          inherited::currentWriteBuffer_->release ();
////          inherited::currentWriteBuffer_ = NULL;
////        } // end IF
////        else
////        {
////          // there's more data --> adjust read pointer
////          inherited::currentWriteBuffer_->rd_ptr (bytes_sent);
////        } // end ELSE
////
////        break;
////      }
////    } // end SWITCH
////  } // end WHILE
////
////  //ACE_DEBUG ((LM_DEBUG,
////  //            ACE_TEXT ("(%t) worker (connection: %u) joining...\n"),
////  //            inherited::stream_.getSessionID ()));
////
////  return 0;
////}

//template <typename HandlerType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename UserDataType>
//bool
//Net_AsynchNetlinkConnection_T<HandlerType,
//                              ConfigurationType,
//                              StateType,
//                              StatisticContainerType,
//                              StreamType,
//                              UserDataType>::initialize (Common_DispatchType dispatch_in,
//                                                         Net_ClientServerRole role_in,
//                                                         const Net_SocketConfiguration& configuration_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::initialize"));

//  if (!inherited::CONNECTION_BASE_T::initialize (dispatch_in,
//                                                 role_in,
//                                                 configuration_in))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_ConnectionBase_T::initialize(), aborting")));
//    return false;
//  } // end IF

//  int result = -1;
//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string local_address;
//  Net_Netlink_Addr local_SAP, remote_SAP;
//  try
//  {
//    this->info (handle,
//                local_SAP,
//                remote_SAP);
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), aborting")));
//    return false;
//  }
//  result = local_SAP.addr_to_string (buffer,
//                                     sizeof (buffer),
//                                     1);
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
//  local_address = buffer;
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  result = remote_SAP.addr_to_string (buffer,
//                                      sizeof (buffer),
//                                      1);
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

//  // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("registered connection [0x%@/0x%@]: (\"%s\") <--> (\"%s\") (total: %d)\n"),
//              this, handle,
//              ACE_TEXT (local_address.c_str ()),
//              buffer,
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("registered connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)\n"),
//              this, handle,
//              ACE_TEXT (local_address.c_str ()),
//              buffer,
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif

//  return true;
//}

//template <typename HandlerType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename UserDataType>
//void
//Net_AsynchNetlinkConnection_T<HandlerType,
//                              ConfigurationType,
//                              StateType,
//                              StatisticContainerType,
//                              StreamType,
//                              UserDataType>::finalize ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::finalize"));

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  Net_Netlink_Addr address1, address2;
//  try
//  {
//    this->info (handle,
//                address1,
//                address2);
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), continuing\n")));
//  }

//  typename inherited::CONNECTION_BASE_T::finalize ();

//  // *PORTABILITY*
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered netlink connection [%@/%u] (total: %u)\n"),
//              this, reinterpret_cast<unsigned int> (handle),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered netlink connection [%@/%d] (total: %d)\n"),
//              this, handle,
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif
//}

//template <typename HandlerType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename UserDataType>
//void
//Net_AsynchNetlinkConnection_T<HandlerType,
//                              ConfigurationType,
//                              StateType,
//                              StatisticContainerType,
//                              StreamType,
//                              UserDataType>::info (ACE_HANDLE& handle_out,
//                                                   Net_Netlink_Addr& localSAP_out,
//                                                   Net_Netlink_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::info"));

//  handle_out = ACE_IPC_SAP::get_handle ();
//  localSAP_out = inherited::localSAP_;
//  remoteSAP_out = inherited::remoteSAP_;
//}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_ConnectionId_t
Net_AsynchNetlinkConnection_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::id"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Net_Netlink_Addr local_netlink_address, peer_netlink_address;
  this->info (handle,
              local_netlink_address,
              peer_netlink_address);

  return static_cast<Net_ConnectionId_t> (handle);
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_AsynchNetlinkConnection_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Net_Netlink_Addr local_netlink_address, peer_netlink_address;
  this->info (handle,
              local_netlink_address,
              peer_netlink_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  result = local_netlink_address.addr_to_string (buffer,
                                                 sizeof (buffer),
                                                 1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = peer_netlink_address.addr_to_string (buffer,
                                                sizeof (buffer),
                                                1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [Id: %u [%u]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address.c_str ()),
              buffer));
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_AsynchNetlinkConnection_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::open (ACE_HANDLE handle_in,
                                                   ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::open"));

  // step1: initialize/start stream, tweak socket, register reading data with
  //        reactor, ...
  inherited::open (handle_in,
                   messageBlock_in);

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  Net_Netlink_Addr local_SAP, remote_SAP;
  try {
    this->info (handle,
                local_SAP,
                remote_SAP);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), returning\n")));

    return;
  }
  result = local_SAP.addr_to_string (buffer,
                                     sizeof (buffer),
                                     1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
  local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = remote_SAP.addr_to_string (buffer,
                                      sizeof (buffer),
                                      1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

  // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered netlink connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, reinterpret_cast<unsigned int> (handle),
              ACE_TEXT (local_address.c_str ()),
              buffer,
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered netlink connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, handle,
              ACE_TEXT (local_address.c_str ()),
              buffer,
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#endif
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_AsynchNetlinkConnection_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                           ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchNetlinkConnection_T::handle_close"));

  int result = -1;

  switch (mask_in)
  {
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused)
                                             //     accept failed (e.g. too many connections)
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

  //// step1: connection shutting down --> signal any worker(s)
  //if (inherited::myUserData.useThreadPerConnection)
  //  shutdown();

  //  // step2: de-register this connection
  //  if (inherited::myManager &&
  //      inherited::myIsRegistered)
  //  { // (try to) de-register with the connection manager
  //    try
  //    {
  //      inherited::myManager->deregisterConnection(this);
  //    }
  //    catch (...)
  //    {
  //      ACE_DEBUG((LM_ERROR,
  //                 ACE_TEXT("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
  //    }
  //  } // end IF

  // step3: invoke base-class maintenance
  result = inherited::handle_close (handle_in, // event handle
                                    mask_in);  // event mask
  if (result == -1)
    // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%@, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%d, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#endif

  return result;
}

////void
////Net_AsynchNetlinkConnection_T::shutdown()
////{
////  NETWORK_TRACE(ACE_TEXT("Net_AsynchNetlinkConnection_T::shutdown"));
////
////  ACE_Message_Block* stop_mb = NULL;
////  ACE_NEW_NORETURN(stop_mb,
////                   ACE_Message_Block(0,                                  // size
////                                     ACE_Message_Block::MB_STOP,         // type
////                                     NULL,                               // continuation
////                                     NULL,                               // data
////                                     NULL,                               // buffer allocator
////                                     NULL,                               // locking strategy
////                                     ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
////                                     ACE_Time_Value::zero,               // execution time
////                                     ACE_Time_Value::max_time,           // deadline time
////                                     NULL,                               // data block allocator
////                                     NULL));                             // message allocator)
////  if (!stop_mb)
////  {
////    ACE_DEBUG((LM_ERROR,
////               ACE_TEXT("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));
////    return;
////  } // end IF
////
////  if (inherited::putq(stop_mb, NULL) == -1)
////  {
////    ACE_DEBUG((LM_ERROR,
////               ACE_TEXT("failed to ACE_Task::putq(): \"%m\", continuing\n")));
////
////    stop_mb->release();
////  } // end IF
////
////  // *NOTE*: defer waiting for any worker(s) to the dtor
////}
