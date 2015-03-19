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

#include "net_defines.h"
#include "net_macros.h"

template <typename SessionDataType,
          typename HandlerType>
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::Net_UDPConnection_T (ICONNECTION_MANAGER_T* interfaceHandle_in)
 : inherited (interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::Net_UDPConnection_T"));

}

template <typename SessionDataType,
          typename HandlerType>
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::Net_UDPConnection_T ()
 : inherited (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::Net_UDPConnection_T"));

}

template <typename SessionDataType,
          typename HandlerType>
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::~Net_UDPConnection_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::~Net_UDPConnection_T"));

}

template <typename SessionDataType,
          typename HandlerType>
int
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::open (void* args_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::open"));

  int result = -1;

  // step1: init/start stream, tweak socket, register reading data with reactor
  // , ...
  result = inherited::open (args_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketConnectionBase_T::open(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  ACE_INET_Addr local_SAP, remote_SAP;
  try
  {
    info (handle,
          local_SAP,
          remote_SAP);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), aborting\n")));
    return -1;
  }
  if (local_SAP.addr_to_string (buffer,
                                sizeof (buffer)) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  if (remote_SAP.addr_to_string (buffer,
                                 sizeof (buffer)) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, reinterpret_cast<unsigned int> (handle),
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, handle,
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#endif

  return 0;
}

template <typename SessionDataType,
          typename HandlerType>
int
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::close (u_long flags_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::close"));

  int result = -1;

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr address1, address2;
  try
  {
    info (handle,
          address1,
          address2);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), continuing\n")));
  }

  void* this_p = this;
  int num_connections =
      (inherited::manager_ ? (inherited::manager_->numConnections () - 1) : -1);
  // *WARNING*: this invokes 'delete this' !
  result = inherited::close ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketConnectionBase_T::close(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [%@/%u] (total: %u)\n"),
              this_p, reinterpret_cast<unsigned int> (handle),
              num_connections));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("closed connection [%@/%d] (total: %d)\n"),
              this_p, handle,
              num_connections));
#endif

  return 0;
}

template <typename SessionDataType,
          typename HandlerType>
bool
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::initialize (Net_ClientServerRole_t role_in,
                                              const Net_SocketConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::initialize"));

  ACE_UNUSED_ARG (role_in);
  ACE_UNUSED_ARG (configuration_in);

  //  typename inherited::CONNECTION_BASE_T* connection_base_p = this;
  //  return connection_base_p->initialize (role_in,
  //                                        configuration_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (true);

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

template <typename SessionDataType,
          typename HandlerType>
void
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::finalize"));

  int result = -1;

  result = close (0);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_UDPConnection_T::close(0): \"%m\", returning\n")));
}

template <typename SessionDataType,
          typename HandlerType>
void
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::info (ACE_HANDLE& handle_out,
                                        ACE_INET_Addr& localSAP_out,
                                        ACE_INET_Addr& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::info"));

  handle_out = inherited::SVC_HANDLER_T::get_handle ();
  if (inherited::peer_.get_local_addr (localSAP_out) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  remoteSAP_out = inherited::configuration_.socketConfiguration.peerAddress;
}

template <typename SessionDataType,
          typename HandlerType>
unsigned int
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return *static_cast<unsigned int*> (inherited::SVC_HANDLER_T::get_handle ());
#else
  return static_cast<unsigned int> (inherited::SVC_HANDLER_T::get_handle ());
#endif
}

template <typename SessionDataType,
          typename HandlerType>
void
Net_UDPConnection_T<SessionDataType,
                    HandlerType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::dump_state"));

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

//template <typename SessionDataType,
//          typename HandlerType>
//int
//Net_UDPConnection_T<SessionDataType>::handle_output (ACE_HANDLE handle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::handle_output"));

//  ACE_UNUSED_ARG (handle_in);

//  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be
//  // dispatching the reactor notification queue concurrently (most notably,
//  // ACE_TP_Reactor) --> enforce proper serialization
//  if (inherited::serializeOutput_)
//    inherited::sendLock_.acquire ();

//  if (inherited::currentWriteBuffer_ == NULL)
//  {
//    // send next data chunk from the stream...
//    // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
//    // been notified to the reactor
//    int result = -1;
//    //if (!inherited::myUserData.useThreadPerConnection)
//      result = inherited::stream_.get (inherited::currentWriteBuffer_,
//                                       const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
//    //else
//    //  result = inherited2::getq (inherited::currentWriteBuffer_,
//    //                             const_cast<ACE_Time_Value*> (&ACE_Time_Value::zero));
//    if (result == -1)
//    {
//      // *IMPORTANT NOTE*: a number of issues can occur here:
//      // - connection has been closed in the meantime
//      // - queue has been deactivated
//      int error = ACE_OS::last_error ();
//      if ((error != EAGAIN) ||  // <-- connection has been closed in the meantime
//          (error != ESHUTDOWN)) // <-- queue has been deactivated
//        ACE_DEBUG ((LM_ERROR,
//                    (inherited::configuration_.streamConfiguration.useThreadPerConnection ? ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")
//                                                                                          : ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n"))));

//      // clean up
//      if (inherited::serializeOutput_)
//        inherited::sendLock_.release ();

//      return -1;
//    } // end IF
//  } // end IF
//  ACE_ASSERT (inherited::currentWriteBuffer_);

//  // finished ?
//  if (inherited::configuration_.streamConfiguration.useThreadPerConnection &&
//      inherited::currentWriteBuffer_->msg_type () == ACE_Message_Block::MB_STOP)
//  {
//    inherited::currentWriteBuffer_->release ();
//    inherited::currentWriteBuffer_ = NULL;

//    //       ACE_DEBUG ((LM_DEBUG,
//    //                   ACE_TEXT ("[%u]: finished sending...\n"),
//    //                   peer_.get_handle ()));

//    // clean up
//    if (inherited::serializeOutput_)
//      inherited::sendLock_.release ();

//    return -1;
//  } // end IF

//  // put some data into the socket...
//  ssize_t bytes_sent = inherited::send (inherited::currentWriteBuffer_->rd_ptr (),
//                                        inherited::currentWriteBuffer_->length (),
//                                        inherited::address_,
//                                        0);
//  switch (bytes_sent)
//  {
//    case -1:
//    {
//      // *IMPORTANT NOTE*: a number of issues can occur here:
//      // - connection reset by peer
//      // - connection abort()ed locally
//      int error = ACE_OS::last_error ();
//      if ((error != ECONNRESET) &&
//          (error != ECONNABORTED) &&
//          (error != EPIPE) &&      // <-- connection reset by peer
//          // -------------------------------------------------------------------
//          (error != ENOTSOCK) &&
//          (error != EBADF))        // <-- connection abort()ed locally
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_SOCK_Stream::send(): \"%m\", aborting\n")));

//      // clean up
//      inherited::currentWriteBuffer_->release ();
//      inherited::currentWriteBuffer_ = NULL;
//      if (inherited::serializeOutput_)
//        inherited::sendLock_.release ();

//      return -1;
//    }
//      // *** GOOD CASES ***
//    case 0:
//    {
//      //       ACE_DEBUG ((LM_DEBUG,
//      //                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//      //                   handle_in));

//      // clean up
//      inherited::currentWriteBuffer_->release ();
//      inherited::currentWriteBuffer_ = NULL;
//      if (inherited::serializeOutput_)
//        inherited::sendLock_.release ();

//      return -1;
//    }
//    default:
//    {
//      //       ACE_DEBUG ((LM_DEBUG,
//      //                  ACE_TEXT ("[%u]: sent %u bytes...\n"),
//      //                  handle_in,
//      //                  bytes_sent));

//      // finished with this buffer ?
//      inherited::currentWriteBuffer_->rd_ptr (static_cast<size_t> (bytes_sent));
//      if (inherited::currentWriteBuffer_->length () > 0)
//        break; // there's more data

//      // clean up
//      inherited::currentWriteBuffer_->release ();
//      inherited::currentWriteBuffer_ = NULL;

//      break;
//    }
//  } // end SWITCH

//  // immediately reschedule sending ?
//  //  if ((currentWriteBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
//  //  {
//  //    if (inherited::reactor ()->cancel_wakeup (this,
//  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
//  //      ACE_DEBUG ((LM_ERROR,
//  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
//  //  } // end IF
//  //  else
//  if (inherited::currentWriteBuffer_ != NULL)
//  {
//    // clean up
//    if (inherited::serializeOutput_)
//      inherited::sendLock_.release ();

//    return 1;
//  } // end IF
//  //if (inherited::reactor ()->schedule_wakeup (this,
//  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

//  // clean up
//  if (inherited::serializeOutput_)
//    inherited::sendLock_.release ();

//  return 0;
//}

//template <typename SessionDataType,
//          typename HandlerType>
//int
//Net_UDPConnection_T<SessionDataType>::handle_close (ACE_HANDLE handle_in,
//                                                    ACE_Reactor_Mask mask_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnection_T::handle_close"));

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
//                  handle_in,
//                  mask_in));
//#else
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %u) --> check implementation !, continuing\n"),
//                  handle_in,
//                  mask_in));
//#endif
//      break;
//  } // end SWITCH

//  //// step1: connection shutting down --> signal any worker(s)
//  //if (inherited::configuration_.streamConfiguration.useThreadPerConnection)
//  //  shutdown ();

//  //  // step2: de-register this connection
//  //  if (inherited::manager_ &&
//  //      inherited::isRegistered_)
//  //  { // (try to) de-register with the connection manager
//  //    try
//  //    {
//  //      inherited::manager_->deregisterConnection (this);
//  //    }
//  //    catch (...)
//  //    {
//  //      ACE_DEBUG ((LM_ERROR,
//  //                  ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
//  //    }
//  //  } // end IF

//  // step3: invoke base-class maintenance
//  int result = inherited::handle_close (handle_in, // event handle
//                                        mask_in);  // event mask
//  if (result == -1)
//    // *PORTABILITY*: this isn't entirely portable...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%@, %d): \"%m\", continuing\n"),
//                handle_in,
//                mask_in));
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamSocketBase::handle_close(%d, %d): \"%m\", continuing\n"),
//                handle_in,
//                mask_in));
//#endif

//  return result;
//}

/////////////////////////////////////////

template <typename SessionDataType,
          typename HandlerType>
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::Net_AsynchUDPConnection_T (ICONNECTION_MANAGER_T* interfaceHandle_in)
 : inherited (interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::Net_AsynchUDPConnection_T"));

}

template <typename SessionDataType,
          typename HandlerType>
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::Net_AsynchUDPConnection_T ()
 : inherited (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::Net_AsynchUDPConnection_T"));

}

template <typename SessionDataType,
          typename HandlerType>
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::~Net_AsynchUDPConnection_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::~Net_AsynchUDPConnection_T"));

}

template <typename SessionDataType,
          typename HandlerType>
bool
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::initialize (Net_ClientServerRole_t role_in,
                                                    const Net_SocketConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::initialize"));

  ACE_UNUSED_ARG (role_in);
  ACE_UNUSED_ARG (configuration_in);

  //  typename inherited::CONNECTION_BASE_T* connection_base_p = this;
  //  return connection_base_p->initialize (role_in,
  //                                        configuration_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (true);

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

template <typename SessionDataType,
          typename HandlerType>
void
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::finalize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::finalize"));

  int result = -1;

  ACE_HANDLE handle = inherited::handle ();
  result = ACE_OS::closesocket (handle);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", returning\n"),
                handle));
}

template <typename SessionDataType,
          typename HandlerType>
void
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::info (ACE_HANDLE& handle_out,
                                              ACE_INET_Addr& localSAP_out,
                                              ACE_INET_Addr& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::info"));

  handle_out = ACE_Event_Handler::get_handle ();
  localSAP_out = inherited::localSAP_;
  remoteSAP_out = inherited::remoteSAP_;
}

template <typename SessionDataType,
          typename HandlerType>
unsigned int
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::id"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr local_inet_address, peer_inet_address;
  info (handle,
        local_inet_address,
        peer_inet_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return *static_cast<unsigned int*> (handle);
#else
  return static_cast<unsigned int> (handle);
#endif
}

template <typename SessionDataType,
          typename HandlerType>
void
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::dump_state"));

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
              ACE_TEXT ("connection [id: %u [%d]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (peer_address.c_str ())));
}

template <typename SessionDataType,
          typename HandlerType>
void
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::open (ACE_HANDLE handle_in,
                                              ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::open"));

  // step1: init/start stream, tweak socket, register reading data with reactor
  // , ...
  inherited::open (handle_in,
                   messageBlock_in);

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address;
  ACE_INET_Addr local_SAP, remote_SAP;
  try
  {
    info (handle,
          local_SAP,
          remote_SAP);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), returning\n")));

    return;
  }
  // *TODO*: retrieve netlink socket information
//  if (local_SAP.addr_to_string (buffer,
//                                sizeof (buffer)) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
  local_address = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
//  if (remote_SAP.addr_to_string (buffer,
//                                 sizeof (buffer)) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));

  // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [%@/%u]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, reinterpret_cast<unsigned int> (handle),
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered connection [%@/%d]: (\"%s\") <--> (\"%s\") (total: %d)...\n"),
              this, handle,
              ACE_TEXT (local_address.c_str ()),
              ACE_TEXT (buffer),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#endif
}

template <typename SessionDataType,
          typename HandlerType>
int
Net_AsynchUDPConnection_T<SessionDataType,
                          HandlerType>::handle_close (ACE_HANDLE handle_in,
                                                      ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnection_T::handle_close"));

  int result = -1;

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_INET_Addr address1, address2;
  try
  {
    info (handle,
          address1,
          address2);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ITransportLayer_T::info(), continuing\n")));
  }

  // step1: invoke base-class maintenance
  result = inherited::handle_close (handle_in, // event handle
                                    mask_in);  // event mask
  if (result == -1)
    // *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchSocketConnectionBase_T::handle_close(%@, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchSocketConnectionBase_T::handle_close(%d, %d): \"%m\", continuing\n"),
                handle_in,
                mask_in));
#endif

  // *PORTABILITY*
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [%@/%u] (total: %u)\n"),
              this, reinterpret_cast<unsigned int> (handle_in),
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("deregistered connection [%@/%d] (total: %d)\n"),
              this, handle_in,
              (inherited::manager_ ? inherited::manager_->numConnections ()
                                   : -1)));
#endif

  return result;
}
