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

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"

#include "common_defines.h"

#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"
#include "net_macros.h"

template <typename SocketType>
Net_UDPSocketHandler_T<SocketType>::Net_UDPSocketHandler_T ()//MANAGER_T* manager_in)
 //: inherited ()
// : inherited (ACE_Reactor::instance (),       // default reactor
//              ACE_Event_Handler::LO_PRIORITY) // default priority
 : inherited2 (NULL,                     // no specific thread manager
               NULL,                     // no specific message queue
               ACE_Reactor::instance ()) // default reactor
 , notificationStrategy_ (ACE_Reactor::instance (),      // reactor
                          this,                          // event handler
                          ACE_Event_Handler::WRITE_MASK) // handle output only
// , manager_ (manager_in)
// , userData_ ()
// , isRegistered_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::Net_UDPSocketHandler_T"));

  // sanity check(s)
//  ACE_ASSERT (manager_);

  //  // register notification strategy ?
  //  inherited::msg_queue ()->notification_strategy (&notificationStrategy_);

  // init user data
  //ACE_OS::memset (&userData_, 0, sizeof (userData_));
//  if (manager_)
//  {
//    try
//    { // (try to) get user data from the connection manager
//      manager_->getConfiguration (userData_);
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Net_IConnectionManager::getConfiguration(), continuing\n")));
//    }
//  } // end IF
}

template <typename SocketType>
Net_UDPSocketHandler_T<SocketType>::~Net_UDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::~Net_UDPSocketHandler_T"));

  //if (manager_ && isRegistered_)
  //{ // (try to) de-register with connection manager
  //  try
  //  {
  //    manager_->deregisterConnection (this);
  //  }
  //  catch (...)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("caught exception in Net_IConnectionManager::deregisterConnection(), continuing\n")));
  //  }
  //} // end IF

//  // *IMPORTANT NOTE*: the streamed socket handler uses the stream head modules' queue
//  // --> this happens too late, as the stream/queue will have been deleted by now...
//  if (reactor ()->purge_pending_notifications (this, ACE_Event_Handler::ALL_EVENTS_MASK) == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Reactor::purge_pending_notifications(%@): \"%m\", continuing\n"),
//                this));

  //// need to close the socket (see handle_close() below) ?
  //if (inherited::reference_counting_policy ().value () ==
  //    ACE_Event_Handler::Reference_Counting_Policy::ENABLED)
  //  if (inherited::peer_.close () == -1)
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to ACE_SOCK_IO::close(): \"%m\", continuing\n")));
}

//ACE_Event_Handler::Reference_Count
//Net_UDPSocketHandler_T::add_reference (void)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::add_reference"));
//
//  inherited2::increase ();
//
//  //return inherited::add_reference ();
//  return 0;
//}
//
//ACE_Event_Handler::Reference_Count
//Net_UDPSocketHandler_T::remove_reference (void)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::remove_reference"));
//
//  // *NOTE*: may "delete this"
//  inherited2::decrease ();
//
//  //// *NOTE*: may "delete this"
//  //return inherited::remove_reference ();
//  return 0;
//}

template <typename SocketType>
int
Net_UDPSocketHandler_T<SocketType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::open"));

  int result = -1;

  // sanity check(s)
//  if (!manager_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid connection manager, aborting\n")));
//    return -1;
//  } // end IF
  ACE_ASSERT (arg_in);

  Net_SocketConfiguration_t* configuration_p =
      reinterpret_cast<Net_SocketConfiguration_t*> (arg_in);

  // step1: tweak socket
  if (configuration_p->bufferSize)
    if (!Net_Common_Tools::setSocketBuffer (SVC_HANDLER_T::get_handle (),
                                            SO_RCVBUF,
                                            configuration_p->bufferSize))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%u) (handle was: %d), aborting\n"),
                  configuration_p->bufferSize,
                  SVC_HANDLER_T::get_handle ()));
      return -1;
    } // end IF
  if (!Net_Common_Tools::setNoDelay (SVC_HANDLER_T::get_handle (),
                                     NET_DEFAULT_TCP_NODELAY))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(%s) (handle was: %d), aborting\n"),
                (NET_DEFAULT_TCP_NODELAY ? ACE_TEXT ("true")
                                         : ACE_TEXT ("false")),
                SVC_HANDLER_T::get_handle ()));
    return -1;
  } // end IF
  if (!Net_Common_Tools::setKeepAlive (SVC_HANDLER_T::get_handle (),
                                       NET_DEFAULT_TCP_KEEPALIVE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: %d), aborting\n"),
                (NET_DEFAULT_TCP_LINGER ? ACE_TEXT ("true")
                                        : ACE_TEXT ("false")),
                SVC_HANDLER_T::get_handle ()));
    return -1;
  } // end IF
  if (!Net_Common_Tools::setLinger (SVC_HANDLER_T::get_handle (),
                                    NET_DEFAULT_TCP_LINGER))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s) (handle was: %d), aborting\n"),
                ((NET_DEFAULT_TCP_LINGER > 0) ? ACE_TEXT ("true")
                                              : ACE_TEXT ("false")),
                SVC_HANDLER_T::get_handle ()));
    return -1;
  } // end IF

  // step2: register with the reactor
  if (reactor ()->register_handler (this,
                                    ACE_Event_Handler::READ_MASK) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Reactor::register_handler(READ_MASK): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // *NOTE*: we're registered with the reactor (READ_MASK) at this point

//   // ...register for writes (WRITE_MASK) as well
//   if (reactor ()->register_handler (this,
//                                     ACE_Event_Handler::WRITE_MASK) == -1)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to ACE_Reactor::register_handler(WRITE_MASK): \"%m\", aborting\n")));
//     return -1;
//   } // end IF

//  if (manager_)
//  {
//    // (try to) register with the connection manager...
//    // *WARNING*: as we register BEFORE the connection has fully opened, there
//    // is a small window for races...
//    try
//    {
//      isRegistered_ = manager_->registerConnection (this);
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Net_IConnectionManager::registerConnection(), continuing\n")));
//    }
//  } // end IF

  result = inherited::open (ACE_Addr::sap_any,        // local
                            ACE_PROTOCOL_FAMILY_INET, // protocol family
                            0,                        // protocol
                            0);                       // reuse_addr
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram::open(): \"%m\", aborting\n")));

    return -1;
  } // end IF

  return 0;
}

template <typename SocketType>
int
Net_UDPSocketHandler_T<SocketType>::handle_close (ACE_HANDLE handle_in,
                                                  ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::handle_close"));

  //// sanity check(s)
  //// *IMPORTANT NOTE*: this handles failed connects (e.g. connection refused)
  //// as well... (see below). This may change in the future, so keep the
  //// alternate implementation
  //if (inherited::reference_counting_policy().value() ==
  //    ACE_Event_Handler::Reference_Counting_Policy::DISABLED)
  //  return inherited::handle_close (handle_in, mask_in); // --> shortcut

  // init return value
  int result = 0;

  // *IMPORTANT NOTE*: due to reference counting, the
  // ACE_Svc_Handle::shutdown() method will crash, as it references a
  // connection recycler AFTER removing the connection from the reactor (which
  // releases a reference). In the case that "this" is the final reference,
  // this leads to a crash. (see code)
  // --> avoid invoking ACE_Svc_Handle::shutdown()
  // --> this means that "manual" cleanup is necessary (see below)

  // *IMPORTANT NOTE*: due to reference counting, the base-class function is a
  // NOP (see code) --> this means that clean up is necessary on:
  // - connect failed (e.g. connection refused)
  // - accept failed (e.g. too many connections)
  // - ... ?

  //  bool already_deleted = false;
  switch (mask_in)
  {
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
      break;
    case ACE_Event_Handler::EXCEPT_MASK:
      //if (handle_in == ACE_INVALID_HANDLE) // <-- notification has completed (!useThreadPerConnection)
      //  ACE_DEBUG((LM_ERROR,
      //             ACE_TEXT("notification completed, continuing\n")));
      break;
    case ACE_Event_Handler::ALL_EVENTS_MASK: // - connect failed (e.g. connection refused) /
                                             // - accept failed (e.g. too many connections) /
                                             // - select failed (EBADF see Select_Reactor_T.cpp) /
                                             // - asynch abort
                                             // - ... ?
    {
      if (handle_in != ACE_INVALID_HANDLE)
      {
        // *TODO*: select case ?
        break;
      } // end IF
      // *TODO*: validate (failed) connect/accept case
//      else if (!myIsRegistered)
//      {
//        // (failed) connect/accept case

//        // *IMPORTANT NOTE*: when a connection attempt fails, the reactor
//        // close()s the connection although it was never open()ed; in that case
//        // there is no valid socket handle
//        ACE_HANDLE handle = get_handle ();
//        if (handle == ACE_INVALID_HANDLE)
//        {
//          // (failed) connect case

//          // clean up
//          decrease ();

//          break;
//        } // end IF

//        // (failed) accept case

//        // *IMPORTANT NOTE*: when an accept fails (e.g. too many connections),
//        // this may have been open()ed, so proper clean up will:
//        // - de-register from the reactor (decreases reference count)
//        // - close the socket (--> done in dtor (see above))
//      } // end ELSE IF

      // (failed) accept / asynch abort case

      ACE_ASSERT (inherited2::reactor ());
      result = inherited2::reactor ()->remove_handler (this,
                                                       (mask_in |
                                                        ACE_Event_Handler::DONT_CALL));
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@, %d), continuing\n"),
                    this,
                    mask_in));

      break;
    }
    default:
// *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %d) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %d) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#endif
      break;
  } // end SWITCH

  return result;
}

//void
//Net_UDPSocketHandler_T::info(ACE_HANDLE& handle_out,
//                               ACE_INET_Addr& localSAP_out,
//                               ACE_INET_Addr& remoteSAP_out) const
//{
//  NETWORK_TRACE(ACE_TEXT("Net_UDPSocketHandler_T::info"));

//  //handle_out = reinterpret_cast<ACE_HANDLE>(myUserData.sessionID);
//  handle_out = peer_.get_handle();
//  if (inherited::peer_.get_local_addr(localSAP_out) == -1)
//  {
//    // *NOTE*: socket already closed ? --> not an error
//    int error = ACE_OS::last_error();
//    if ((error != EBADF) &&
//        (error != ENOTSOCK))
//      ACE_DEBUG((LM_ERROR,
//                 ACE_TEXT("failed to ACE_SOCK_Stream::get_local_addr(): \"%m\", continuing\n")));
//  } // end IF
//  if (inherited::peer_.get_remote_addr(remoteSAP_out) == -1)
//  {
//    // *NOTE*: socket already closed ? --> not an error
//    int error = ACE_OS::last_error();
//    if ((error != ENOTCONN) &&
//        (error != EBADF) &&
//        (error != ENOTSOCK))
//      ACE_DEBUG((LM_ERROR,
//                 ACE_TEXT("failed to ACE_SOCK_Stream::get_remote_addr(): \"%m\", continuing\n")));
//  } // end IF
//}

//void
//Net_UDPSocketHandler_T::close()
//{
//  NETWORK_TRACE(ACE_TEXT("Net_UDPSocketHandler_T::close"));

//  // *NOTE*: do NOT simply close the underlying socket

//  // de-register from the manager, close the stream, de-register from the
//  // reactor[, delete this]
//  close(NORMAL_CLOSE_OPERATION);

//  // *IMPORTANT NOTE*: if called from a non-reactor context, or when using a
//  // a multithreaded reactor, there may still be in-flight notifications and/or
//  // socket events being dispatched at this stage...
//}

//unsigned int
//Net_UDPSocketHandler_T::id() const
//{
//  NETWORK_TRACE(ACE_TEXT("Net_UDPSocketHandler_T::id"));

//  // *PORTABILITY*: this isn't entirely portable...
//#if !defined(ACE_WIN32) && !defined(ACE_WIN64)
//  return peer_.get_handle();
//#else
//  return reinterpret_cast<unsigned int>(peer_.get_handle());
//#endif
//}

//void
//Net_UDPSocketHandler_T::increase()
//{
//  NETWORK_TRACE(ACE_TEXT("Net_UDPSocketHandler_T::increase"));
//
//  inherited2::increase();
//  //inherited::add_reference();
//}
//
//void
//Net_UDPSocketHandler_T::decrease()
//{
//  NETWORK_TRACE(ACE_TEXT("Net_UDPSocketHandler_T::decrease"));
//
//  // *NOTE*: may "delete this"
//  inherited2::decrease();
//  //// *NOTE*: may "delete this"
//  //inherited::remove_reference();
//}

//void
//Net_UDPSocketHandler_T::dump_state() const
//{
//  NETWORK_TRACE(ACE_TEXT("Net_UDPSocketHandler_T::dump_state"));
//
//  ACE_TCHAR buffer[RPG_COMMON_BUFSIZE];
//  ACE_OS::memset(buffer, 0, sizeof(buffer));
//  ACE_INET_Addr address;
//  if (inherited::peer_.get_local_addr(address) == -1)
//  {
//    // *NOTE*: socket already closed ? --> not an error
//    int error = ACE_OS::last_error();
//    if ((error != EBADF) &&
//        (error != ENOTSOCK))
//      ACE_DEBUG((LM_ERROR,
//                 ACE_TEXT("failed to ACE_SOCK_Stream::get_local_addr(): \"%m\", continuing\n")));
//  } // end IF
//  else if (address.addr_to_string(buffer,
//                                  sizeof(buffer)) == -1)
//    ACE_DEBUG((LM_ERROR,
//               ACE_TEXT("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//  std::string local_address = buffer;
//
//  ACE_OS::memset(buffer, 0, sizeof(buffer));
//  if (inherited::peer_.get_remote_addr(address) == -1)
//  {
//    // *NOTE*: socket already closed ? --> not an error
//    int error = ACE_OS::last_error();
//    if ((error != ENOTCONN) &&
//        (error != EBADF) &&
//        (error != ENOTSOCK))
//      ACE_DEBUG((LM_ERROR,
//                 ACE_TEXT("failed to ACE_SOCK_Stream::get_remote_addr(): \"%m\", continuing\n")));
//  } // end IF
//  else if (address.addr_to_string(buffer,
//                                  sizeof(buffer)) == -1)
//    ACE_DEBUG((LM_ERROR,
//               ACE_TEXT("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//  std::string peer_address = buffer;
//
//  ACE_DEBUG((LM_DEBUG,
//             ACE_TEXT("socket [%d]: \"%s\" <--> \"%s\"\n"),
//             get_handle(),
//             ACE_TEXT(local_address.c_str()),
//             ACE_TEXT(peer_address.c_str())));
//}
