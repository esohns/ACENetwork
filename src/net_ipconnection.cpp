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
#include "stdafx.h"

#include "net_ipconnection.h"

#include "net_defines.h"
#include "net_macros.h"

//Net_IPMulticastConnection::Net_IPMulticastConnection (Net_IIPConnectionManager_t* interfaceHandle_in)
// : inherited (interfaceHandle_in)
// , inherited2 (ACE_SOCK_Dgram_Mcast::DEFOPTS)
// , joined_ (false)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::Net_IPMulticastConnection"));

//// *TODO*: move this to net_ipconnectionhandler.cpp
//std::string address = ACE_TEXT_ALWAYS_CHAR (NET_DEFAULT_IP_MULTICAST_ADDRESS);
//address += ':';
//std::ostringstream converter;
//converter << NET_DEFAULT_PORT;
//address += converter.str ();
//if (address_.set (ACE_TEXT (address.c_str ()), AF_INET) == -1)
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
//if (inherited2::open (address_,
//                      NULL,
//                      1) == -1)
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::open(): \"%m\", continuing\n")));

//}

//Net_IPMulticastConnection::~Net_IPMulticastConnection ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::~Net_IPMulticastConnection"));

//// *TODO*: move this to net_ipconnectionhandler.cpp
//if (joined_)
//  if (inherited2::leave (address_,
//                         NULL) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::leave(): \"%m\", continuing\n")));
//if (inherited2::close () == -1)
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::close(): \"%m\", continuing\n")));

//}

//int
//Net_IPMulticastConnection::open (void* args_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::open"));

//  int result = -1;

//  // step1: init/start stream, tweak socket, register reading data with reactor
//  // , ...
//  result = inherited::open (args_in);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_SocketConnectionBase_T::open(): \"%m\", aborting")));

//    return -1;
//  } // end IF

//// *TODO*: move this to net_ipconnectionhandler.cpp
//int result = -1;

//u_short port_number = configuration_in.peerAddress.get_port_number ();
//if (address_.get_port_number () != port_number)
//{
//  if (joined_)
//  {
//    result = leave (address_,
//                    NULL);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::leave(): \"%m\", continuing\n")));
//    joined_ = false;
//  } // end IF
//  result = inherited2::close ();
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::close(): \"%m\", continuing\n")));
//} // end IF
//address_ = configuration_in.peerAddress;
//result = inherited2::open (address_,
//                           NULL,
//                           1);
//if (result == -1)
//{
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::open(): \"%m\", aborting\n")));

//  return false;
//} // end IF
//result = inherited2::join (address_,
//                           1,
//                           NULL);
//if (result == -1)
//{
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_SOCK_Dgram_Mcast::join(): \"%m\", aborting\n")));

//  return false;
//} // end IF
//joined_ = true;

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string local_address;
//  ACE_INET_Addr local_SAP, remote_SAP;
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

//    return -1;
//  }
//  // *TODO*: retrieve netlink socket information
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
//              ACE_TEXT ("registered connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
//              this, reinterpret_cast<unsigned int> (handle),
//              ACE_TEXT (localAddress.c_str ()),
//              ACE_TEXT (buffer),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("registered connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
//              this, handle,
//              ACE_TEXT (local_address.c_str ()),
//              ACE_TEXT (buffer),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif

//  return 0;
//}

//int
//Net_IPMulticastConnection::close (u_long flags_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::close"));

//  int result = -1;

//  result = inherited::close (flags_in);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_SocketConnectionBase_T::close(): \"%m\", aborting")));

//    return -1;
//  } // end IF

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_INET_Addr address1, address2;
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

//  Net_ConnectionBase_T::finalize ();

//  // *PORTABILITY*
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered connection [%@/%u] (total: %u)\n"),
//              this, reinterpret_cast<unsigned int> (handle),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered connection [%@/%d] (total: %d)\n"),
//              this, handle,
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif

//  return 0;
//}

//void
//Net_IPMulticastConnection::info (ACE_HANDLE& handle_out,
//                         ACE_INET_Addr& localSAP_out,
//                         ACE_INET_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::info"));

//  handle_out = inherited::SVC_HANDLER_T::get_handle ();
//  if (inherited::get_local_addr (localSAP_out) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
//  remoteSAP_out = inherited::configuration_.socketConfiguration.peerAddress;
//}

//unsigned int
//Net_IPMulticastConnection::id () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::id"));

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  return *static_cast<unsigned int*> (inherited::SVC_HANDLER_T::get_handle ());
//#else
//  return static_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ());
//#endif
//}

//void
//Net_IPMulticastConnection::dump_state () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::dump_state"));

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_INET_Addr local_inet_address, peer_inet_address;
//  info (handle,
//        local_inet_address,
//        peer_inet_address);

//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string local_address;
//  if (local_inet_address.addr_to_string (buffer,
//                                         sizeof (buffer)) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//  else
//    local_address = buffer;
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string peer_address;
//  if (peer_inet_address.addr_to_string (buffer,
//                                        sizeof (buffer)) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//  else
//    peer_address = buffer;

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("connection [Id: %u [%u]]: \"%s\" <--> \"%s\"\n"),
//              id (), handle,
//              ACE_TEXT (local_address.c_str ()),
//              ACE_TEXT (peer_address.c_str ())));
//}

////int
////Net_IPMulticastConnection::handle_output (ACE_HANDLE handle_in)
////{
////  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::handle_output"));

////  ACE_UNUSED_ARG (handle_in);

////  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be
////  // dispatching the reactor notification queue concurrently (most notably,
////  // ACE_TP_Reactor) --> enforce proper serialization
////  if (inherited::serializeOutput_)
////    inherited::sendLock_.acquire ();

////  if (inherited::currentWriteBuffer_ == NULL)
////  {
////    // send next data chunk from the stream...
////    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
////    // been notified to the reactor
////    int result = -1;
////    //if (!inherited::myUserData.useThreadPerConnection)
////      result = inherited::stream_.get (inherited::currentWriteBuffer_,
////                                       const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
////    //else
////    //  result = inherited2::getq (inherited::currentWriteBuffer_,
////    //                             const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
////    if (result == -1)
////    {
////      // *IMPORTANT NOTE*: a number of issues can occur here:
////      // - connection has been closed in the meantime
////      // - queue has been deactivated
////      int error = ACE_OS::last_error ();
////      if ((error != EAGAIN) ||  // <-- connection has been closed in the meantime
////          (error != ESHUTDOWN)) // <-- queue has been deactivated
////        ACE_DEBUG ((LM_ERROR,
////                    (inherited::configuration_.streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
////                                                                                          : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));

////      // clean up
////      if (inherited::serializeOutput_)
////        inherited::sendLock_.release ();

////      return -1;
////    } // end IF
////  } // end IF
////  ACE_ASSERT (inherited::currentWriteBuffer_);

////  // finished ?
////  if (inherited::configuration_.streamConfiguration.useThreadPerConnection &&
////      inherited::currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
////  {
////    inherited::currentWriteBuffer_->release ();
////    inherited::currentWriteBuffer_ = NULL;

////    //       ACE_DEBUG ((LM_DEBUG,
////    //                   ACE_TEXT ("[%u]: finished sending...\n"),
////    //                   peer_.get_handle ()));

////    // clean up
////    if (inherited::serializeOutput_)
////      inherited::sendLock_.release ();

////    return -1;
////  } // end IF

////  // put some data into the socket...
////  ssize_t bytes_sent = inherited::send (inherited::currentWriteBuffer_->rd_ptr (),
////                                        inherited::currentWriteBuffer_->length (),
////                                        inherited::address_,
////                                        0);
////  switch (bytes_sent)
////  {
////    case -1:
////    {
////      // *IMPORTANT NOTE*: a number of issues can occur here:
////      // - connection reset by peer
////      // - connection abort()ed locally
////      int error = ACE_OS::last_error ();
////      if ((error != ECONNRESET) &&
////          (error != ECONNABORTED) &&
////          (error != EPIPE) &&      // <-- connection reset by peer
////          // -------------------------------------------------------------------
////          (error != ENOTSOCK) &&
////          (error != EBADF))        // <-- connection abort()ed locally
////          ACE_DEBUG ((LM_ERROR,
////                      ACE_TEXT ("failed to ACE_SOCK_Stream::send(): \"%m\", aborting\n")));

////      // clean up
////      inherited::currentWriteBuffer_->release ();
////      inherited::currentWriteBuffer_ = NULL;
////      if (inherited::serializeOutput_)
////        inherited::sendLock_.release ();

////      return -1;
////    }
////      // *** GOOD CASES ***
////    case 0:
////    {
////      //       ACE_DEBUG ((LM_DEBUG,
////      //                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
////      //                   handle_in));

////      // clean up
////      inherited::currentWriteBuffer_->release ();
////      inherited::currentWriteBuffer_ = NULL;
////      if (inherited::serializeOutput_)
////        inherited::sendLock_.release ();

////      return -1;
////    }
////    default:
////    {
////      //       ACE_DEBUG ((LM_DEBUG,
////      //                  ACE_TEXT ("[%u]: sent %u bytes...\n"),
////      //                  handle_in,
////      //                  bytes_sent));

////      // finished with this buffer ?
////      inherited::currentWriteBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
////      if (inherited::currentWriteBuffer_->length () > 0)
////        break; // there's more data

////      // clean up
////      inherited::currentWriteBuffer_->release ();
////      inherited::currentWriteBuffer_ = NULL;

////      break;
////    }
////  } // end SWITCH

////  // immediately reschedule sending ?
////  //  if ((currentWriteBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
////  //  {
////  //    if (inherited::reactor ()->cancel_wakeup (this,
////  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
////  //      ACE_DEBUG ((LM_ERROR,
////  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
////  //  } // end IF
////  //  else
////  if (inherited::currentWriteBuffer_ != NULL)
////  {
////    // clean up
////    if (inherited::serializeOutput_)
////      inherited::sendLock_.release ();

////    return 1;
////  } // end IF
////  //if (inherited::reactor ()->schedule_wakeup (this,
////  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
////  //  ACE_DEBUG ((LM_ERROR,
////  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

////  // clean up
////  if (inherited::serializeOutput_)
////    inherited::sendLock_.release ();

////  return 0;
////}

////int
////Net_IPMulticastConnection::handle_close (ACE_HANDLE handle_in,
////                                 ACE_Reactor_Mask mask_in)
////{
////  NETWORK_TRACE (ACE_TEXT ("Net_IPMulticastConnection::handle_close"));

////  switch (mask_in)
////  {
////    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
////    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused) /
////      //     accept failed (e.g. too many connections) /
////      //     select failed (EBADF see Select_Reactor_T.cpp)
////      //     asynch abort
////      break;
////    case ACE_Event_Handler::EXCEPT_MASK:
////      // *IMPORTANT NOTE*: the TP_Reactor dispatches notifications in parallel
////      // to regular socket events, thus several notifications may be in flight
////      // at the same time. In order to avoid confusion when the socket closes,
////      // proper synchronization is handled through the reference counting
////      // mechanism, i.e. the handler is only deleted after the last reference
////      // has been released. Still, handling notifications after this occurs
////      // will fail, and the reactor will invoke this method each time. As there
////      // is no need to go through the shutdown procedure several times, bail
////      // out early here
////      //if (handle_in == ACE_INVALID_HANDLE) // <-- notification has completed (!useThreadPerConnection)
////      //  ACE_DEBUG((LM_ERROR,
////      //             ACE_TEXT("notification completed, continuing\n")));
////      return 0;
////    default:
////      // *PORTABILITY*: this isn't entirely portable...
////#if defined (ACE_WIN32) || defined (ACE_WIN64)
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %u) --> check implementation !, continuing\n"),
////                  handle_in,
////                  mask_in));
////#else
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
////                  handle_in,
////                  mask_in));
////#endif
////      break;
////  } // end SWITCH

////  //// step1: connection shutting down --> signal any worker(s)
////  //if (inherited::configuration_.streamConfiguration.useThreadPerConnection)
////  //  shutdown ();

////  //  // step2: de-register this connection
////  //  if (inherited::manager_ &&
////  //      inherited::isRegistered_)
////  //  { // (try to) de-register with the connection manager
////  //    try
////  //    {
////  //      inherited::manager_->deregisterConnection (this);
////  //    }
////  //    catch (...)
////  //    {
////  //      ACE_DEBUG ((LM_ERROR,
////  //                  ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
////  //    }
////  //  } // end IF

////  // step3: invoke base-class maintenance
////  int result = inherited::handle_close (handle_in, // event handle
////                                        mask_in);  // event mask
////  if (result == -1)
////    // *PORTABILITY*: this isn't entirely portable...
////#if defined (ACE_WIN32) || defined (ACE_WIN64)
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%@, %d): \"%m\", continuing\n"),
////                handle_in,
////                mask_in));
////#else
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%d, %d): \"%m\", continuing\n"),
////                handle_in,
////                mask_in));
////#endif

////  return result;
////}

/////////////////////////////////////////

//Net_IPBroadcastConnection::Net_IPBroadcastConnection (Net_IIPConnectionManager_t* interfaceHandle_in)
// : inherited (interfaceHandle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPBroadcastConnection::Net_IPBroadcastConnection"));

// *TODO*: move this to net_ipconnectionhandler.cpp
//std::string address = ACE_TEXT_ALWAYS_CHAR (NET_DEFAULT_IP_BROADCAST_ADDRESS);
//address += ':';
//std::ostringstream converter;
//converter << NET_DEFAULT_PORT;
//address += converter.str();
//if (address_.set (ACE_TEXT(address.c_str ()), AF_INET) == -1)
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
//if (inherited2::open (address_,
//                      PF_INET,
//                      0,
//                      0,
//                      NULL) == -1)
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::open(): \"%m\", continuing\n")));

//}

//Net_IPBroadcastConnection::~Net_IPBroadcastConnection ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPBroadcastConnection::~Net_IPBroadcastConnection"));
//
//// *TODO*: move this to net_ipconnectionhandler.cpp
//if (inherited2::close () == -1)
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::close(): \"%m\", continuing\n")));
//
//}

//void
//Net_IPBroadcastConnection::open (ACE_HANDLE handle_in,
//                                 ACE_Message_Block& messageBlock_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPBroadcastConnection::open"));

//  // step1: init/start stream, tweak socket, register reading data with reactor
//  // , ...
//  inherited::open (handle_in,
//                   messageBlock_in);

//// *TODO*: move this to net_ipconnectionhandler.cpp
//int result = -1;

//u_short port_number = configuration_in.peerAddress.get_port_number ();
//if (address_.get_port_number () != port_number)
//{
//  result = inherited2::close ();
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::close(): \"%m\", continuing\n")));
//} // end IF
//address_ = configuration_in.peerAddress;
//result = inherited2::open (address_,
//                           PF_INET,
//                           0,
//                           0,
//                           NULL);
//if (result == -1)
//{
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to ACE_SOCK_Dgram_Bcast::open(): \"%m\", aborting\n")));

//  return false;
//} // end IF

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string local_address;
//  ACE_INET_Addr local_SAP, remote_SAP;
//  try
//  {
//    info (handle,
//          local_SAP,
//          remote_SAP);
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), returning")));

//    return;
//  }
//  // *TODO*: retrieve netlink socket information
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
//              ACE_TEXT ("registered connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
//              this, reinterpret_cast<unsigned int> (handle),
//              ACE_TEXT (localAddress.c_str ()),
//              ACE_TEXT (buffer),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("registered connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
//              this, handle,
//              ACE_TEXT (local_address.c_str ()),
//              ACE_TEXT (buffer),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif
//}

//int
//Net_IPBroadcastConnection::handle_close (ACE_HANDLE handle_in,
//                                       ACE_Reactor_Mask mask_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPBroadcastConnection::handle_close"));

//  int result = -1;

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_INET_Addr address1, address2;
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

//  // step1: invoke base-class maintenance
//  result = inherited::handle_close (handle_in, // event handle
//                                    mask_in);  // event mask
//  if (result == -1)
//    // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_AsynchSocketConnectionBase_T::handle_close(%@, %d): \"%m\", continuing\n"),
//                handle_in,
//                mask_in));
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_AsynchSocketConnectionBase_T::handle_close(%d, %d): \"%m\", continuing\n"),
//                handle_in,
//                mask_in));
//#endif

//  // *PORTABILITY*
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered connection [%@/%u] (total: %u)\n"),
//              this, reinterpret_cast<unsigned int> (handle_in),
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("deregistered connection [%@/%d] (total: %d)\n"),
//              this, handle_in,
//              (inherited::manager_ ? inherited::manager_->numConnections ()
//                                   : -1)));
//#endif

//  return result;
//}

//void
//Net_IPBroadcastConnection::info (ACE_HANDLE& handle_out,
//                               ACE_INET_Addr& localSAP_out,
//                               ACE_INET_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPBroadcastConnection::info"));

//  handle_out = ACE_Event_Handler::get_handle ();
//  localSAP_out = inherited::localSAP_;
//  remoteSAP_out = inherited::remoteSAP_;
//}

//unsigned int
//Net_IPBroadcastConnection::id () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPBroadcastConnection::id"));

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_INET_Addr local_inet_address, peer_inet_address;
//  info (handle,
//        local_inet_address,
//        peer_inet_address);

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  return *static_cast<unsigned int*> (handle);
//#else
//  return static_cast<unsigned int> (handle);
//#endif
//}

//void
//Net_IPBroadcastConnection::dump_state () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_IPBroadcastConnection::dump_state"));

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_INET_Addr local_inet_address, peer_inet_address;
//  info (handle,
//        local_inet_address,
//        peer_inet_address);

//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string local_address;
//  if (local_inet_address.addr_to_string (buffer,
//                                         sizeof (buffer)) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//  else
//    local_address = buffer;
//  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  std::string peer_address;
//  if (peer_inet_address.addr_to_string (buffer,
//                                        sizeof (buffer)) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//  else
//    peer_address = buffer;

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("connection [id: %u [%d]]: \"%s\" <--> \"%s\"\n"),
//              id (), handle,
//              ACE_TEXT (local_address.c_str ()),
//              ACE_TEXT (peer_address.c_str ())));
//}
