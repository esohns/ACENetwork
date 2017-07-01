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
#include "common_tools.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"
#include "net_macros.h"

template <typename SocketType,
          typename ConfigurationType>
Net_UDPSocketHandler_T<SocketType,
                       ConfigurationType>::Net_UDPSocketHandler_T ()
 : inherited ()
 , inherited2 (NULL,                     // no specific thread manager
               NULL,                     // no specific message queue
               ACE_Reactor::instance ()) // default reactor
 , address_ ()
#if defined (ACE_LINUX)
 , errorQueue_ (NET_SOCKET_DEFAULT_ERRORQUEUE)
#endif
 , notificationStrategy_ (ACE_Reactor::instance (),      // reactor
                          this,                          // event handler
                          ACE_Event_Handler::WRITE_MASK) // handle output only
 , writeOnly_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::Net_UDPSocketHandler_T"));

}

template <typename SocketType,
          typename ConfigurationType>
Net_UDPSocketHandler_T<SocketType,
                       ConfigurationType>::~Net_UDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::~Net_UDPSocketHandler_T"));

}

template <typename SocketType,
          typename ConfigurationType>
int
Net_UDPSocketHandler_T<SocketType,
                       ConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::open"));

  int result = -1;
#if defined (ACE_LINUX)
  bool handle_privileges = false;
#endif
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // sanity check(s)
  ACE_ASSERT (arg_in);
  ConfigurationType* configuration_p =
    reinterpret_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);

  // *TODO*: remove type inferences
  address_ = configuration_p->socketConfiguration.address;
  writeOnly_ = configuration_p->socketConfiguration.writeOnly;

  // step1: open socket ?
  // *NOTE*: even when this is a write-only connection
  //         (configuration_p->socketConfiguration->writeOnly), the base class
  //         still requires a valid handle to open the output stream
  if (!writeOnly_)
  {
    // sanity check(s)
    // *TODO*: remove type inferences
    ACE_ASSERT (configuration_p->listenerConfiguration);

    address_ =
      configuration_p->listenerConfiguration->socketHandlerConfiguration.socketConfiguration.address;
  } // end IF

#if defined (ACE_LINUX)
  // (temporarily) elevate priviledges to open system sockets
  if (address_.get_port_number () <= NET_ADDRESS_MAXIMUM_PRIVILEGED_PORT)
  {
    if (!Common_Tools::setRootPrivileges ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setRootPrivileges(): \"%m\", continuing\n")));
    handle_privileges = true;
  } // end IF
#endif
  ACE_INET_Addr inet_address =
      (writeOnly_ ? ACE_INET_Addr (static_cast<u_short> (0),
                                   static_cast<ACE_UINT32> (INADDR_ANY))
                  : address_);
  result = inherited2::peer_.open (inet_address,             // SAP
                                   ACE_PROTOCOL_FAMILY_INET, // protocol family
                                   0,                        // protocol
                                   1);                       // reuse_addr
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketType::open(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address).c_str ())));
    goto error;
  } // end IF
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::dropRootPrivileges ();
#endif
  handle = inherited2::get_handle ();
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);

  // step2: connect ?
  // *TODO*: remove type inference
  if (configuration_p->socketConfiguration.connect)
  {
    ACE_INET_Addr associated_address =
        (writeOnly_ ? address_
                    : ACE_INET_Addr (static_cast<u_short> (0),
                                     static_cast<ACE_UINT32> (INADDR_ANY)));
    result =
        ACE_OS::connect (handle,
                         reinterpret_cast<struct sockaddr*> (associated_address.get_addr ()),
                         associated_address.get_addr_size ());
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::connect(0x%@,\"%s\"): \"%m\", aborting\n"),
                  handle,
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (associated_address).c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::connect(%d,\"%s\"): \"%m\", aborting\n"),
                  handle,
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (associated_address).c_str ())));
#endif
      goto error;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: associated to \"%s\"\n"),
                handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (associated_address).c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: associated to \"%s\"\n"),
                handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (associated_address).c_str ())));
#endif
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("0x%@: (initial) MTU between \"%s\" and \"%s\": %u byte(s)...\n"),
//                handle,
//                buffer, buffer_2,
//                Net_Common_Tools::getMTU (handle)));
//#else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%d: (initial) MTU between \"%s\" and \"%s\": %u byte(s)...\n"),
//                handle,
//                buffer, buffer_2,
//                Net_Common_Tools::getMTU (handle)));
//#endif

  // *NOTE*: recvfrom()-ing datagrams larger than SO_RCVBUF will truncate the
  //         inbound datagram (MSG_TRUNC flag will be set)
  // *TODO*: remove type inferences
  if (!writeOnly_)
  {
    // step3: tweak inbound socket
    if (configuration_p->socketConfiguration.bufferSize)
      if (!Net_Common_Tools::setSocketBuffer (handle,
                                              SO_RCVBUF,
                                              configuration_p->socketConfiguration.bufferSize))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                    handle,
                    configuration_p->socketConfiguration.bufferSize));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), continuing\n"),
                    handle,
                    configuration_p->socketConfiguration.bufferSize));
#endif
      } // end IF

    // *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
    //                SO_LINGER
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (!Net_Common_Tools::setLinger (handle,
                                      configuration_p->socketConfiguration.linger,
                                      -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%d,%s,-1), returning\n"),
                  handle,
                  (configuration_p->socketConfiguration.linger ? ACE_TEXT ("true")
                                                               : ACE_TEXT ("false"))));
      goto error;
    } // end IF
#endif
  } // end IF
  // step4: tweak outbound socket
  // *NOTE*: sendto()-ing datagrams larger than SO_SNDBUF will trigger errno
  //         EMSGSIZE (90)
  if (configuration_p->socketConfiguration.bufferSize)
    if (!Net_Common_Tools::setSocketBuffer (handle,
                                            SO_SNDBUF,
                                            configuration_p->socketConfiguration.bufferSize))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->socketConfiguration.bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_SNDBUF,%u), continuing\n"),
                  handle,
                  configuration_p->socketConfiguration.bufferSize));
#endif
    } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: (on Linux), packet fragmentation is off by default, so sendto()-ing
  //         datagrams larger than MTU will trigger errno EMSGSIZE (90)
  //         --> enable packet fragmentation
  if (!Net_Common_Tools::setPathMTUDiscovery (handle,
                                              IP_PMTUDISC_WANT))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::disablePathMTUDiscovery(%d,%d), aborting\n"),
                handle,
                IP_PMTUDISC_WANT));
    goto error;
  } // end IF
#endif

#if defined (ACE_LINUX)
  if (errorQueue_)
    if (!Net_Common_Tools::enableErrorQueue (handle))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::enableErrorQueue() (handle was: %d), aborting\n"),
                  handle));
      goto error;
    } // end IF
#endif

//  // debug info
//  unsigned int so_max_msg_size = Net_Common_Tools::getMaxMsgSize (handle);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket 0x%@: %u byte(s)...\n"),
//              handle,
//              so_max_msg_size));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket %d: %u byte(s)...\n"),
//              handle,
//              so_max_msg_size));
//#endif

  return 0;

error:
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::dropRootPrivileges ();
#endif

  return -1;
}

template <typename SocketType,
          typename ConfigurationType>
int
Net_UDPSocketHandler_T<SocketType,
                       ConfigurationType>::handle_close (ACE_HANDLE handle_in,
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

  // initialize return value
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
      // *TODO*: validate (failed) connect/accept case
//      if (!isRegistered_)
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

      // asynch abort case

      if (handle_in != ACE_INVALID_HANDLE)
      {
        result = inherited2::peer_.close ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SOCK_IO::close (): %d, continuing\n")));
      } // end IF

//      ACE_Reactor* reactor_p = inherited2::reactor ();
//      ACE_ASSERT (reactor_p);
//      result =
//        reactor_p->remove_handler (handle_in,
//                                   (mask_in |
//                                    ACE_Event_Handler::DONT_CALL));
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@/%d, %d), continuing\n"),
//                    this, handle_in,
//                    mask_in));
      break;
    }
    default:
// *PORTABILITY*: this isn't entirely portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: 0x%@, mask: %d) --> check implementation !, continuing\n"),
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

//////////////////////////////////////////

template <typename ConfigurationType>
Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                       ConfigurationType>::Net_UDPSocketHandler_T ()
 : inherited ()
 , inherited2 (NULL,                     // no specific thread manager
               NULL,                     // no specific message queue
               ACE_Reactor::instance ()) // default reactor
 , address_ ()
#if defined (ACE_LINUX)
 , errorQueue_ (NET_SOCKET_DEFAULT_ERRORQUEUE)
#endif
 , notificationStrategy_ (ACE_Reactor::instance (),      // reactor
                          this,                          // event handler
                          ACE_Event_Handler::WRITE_MASK) // handle output only
 , writeOnly_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::Net_UDPSocketHandler_T"));

}

template <typename ConfigurationType>
Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                       ConfigurationType>::~Net_UDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::~Net_UDPSocketHandler_T"));

}

template <typename ConfigurationType>
int
Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                       ConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::open"));

  int result = -1;
#if defined (ACE_LINUX)
  bool handle_privileges = false;
#endif
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // sanity check(s)
  ACE_ASSERT (arg_in);
  ConfigurationType* configuration_p =
    reinterpret_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);

  // *TODO*: remove type inferences
  address_ = configuration_p->socketConfiguration.address;
  writeOnly_ = configuration_p->socketConfiguration.writeOnly;

  // step1: open socket ?
  // *NOTE*: even when this is a write-only connection
  //         (configuration_p->socketConfiguration->writeOnly), the base class
  //         still requires a valid handle to open the output stream
//  ACE_INET_Addr local_SAP = ACE_Addr::sap_any;
  ACE_INET_Addr local_SAP (static_cast<u_short> (0),
                           static_cast<ACE_UINT32> (INADDR_ANY));
  if (!writeOnly_)
  {
    // sanity check(s)
    ACE_ASSERT (configuration_p->listenerConfiguration);

    // *TODO*: remove type inference
    local_SAP =
      configuration_p->listenerConfiguration->socketHandlerConfiguration.socketConfiguration.address;
  } // end IF

//  ACE_INET_Addr remote_SAP = ACE_Addr::sap_any;
  ACE_INET_Addr remote_SAP (static_cast<u_short> (0),
                            static_cast<ACE_UINT32> (INADDR_ANY));
  // *TODO*: remove type inference
  if (writeOnly_)
    remote_SAP = address_;

#if defined (ACE_LINUX)
  // (temporarily) elevate priviledges to open system sockets
  u_short port_number = local_SAP.get_port_number ();
  if (port_number &&
      (port_number <= NET_ADDRESS_MAXIMUM_PRIVILEGED_PORT))
  {
    if (!Common_Tools::setRootPrivileges ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setRootPrivileges(): \"%m\", continuing\n")));
    handle_privileges = true;
  } // end IF
#endif

  result = inherited2::peer_.open (remote_SAP,               // remote SAP
                                   local_SAP,                // local SAP
                                   ACE_PROTOCOL_FAMILY_INET, // protocol family
                                   0,                        // protocol
                                   1);                       // reuse_addr
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_CODgram::open(\"%s\",\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (local_SAP).c_str ()),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (remote_SAP).c_str ())));
    goto error;
  } // end IF
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::dropRootPrivileges ();
#endif
  handle = inherited2::get_handle ();
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);

  // step3: tweak socket
  // *NOTE*: recvfrom()-ing datagrams larger than SO_RCVBUF will truncate the
  //         inbound datagram (MSG_TRUNC flag will be set)
  // *TODO*: remove type inferences
  if (configuration_p->socketConfiguration.bufferSize)
  {
    if (!Net_Common_Tools::setSocketBuffer (handle,
                                            SO_RCVBUF,
                                            configuration_p->socketConfiguration.bufferSize))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->socketConfiguration.bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->socketConfiguration.bufferSize));
#endif
    } // end IF
    // *NOTE*: sendto()-ing datagrams larger than SO_SNDBUF will trigger errno
    //         EMSGSIZE (90)
    if (!Net_Common_Tools::setSocketBuffer (handle,
                                            SO_SNDBUF,
                                            configuration_p->socketConfiguration.bufferSize))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->socketConfiguration.bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_SNDBUF,%u), continuing\n"),
                  handle,
                  configuration_p->socketConfiguration.bufferSize));
#endif
    } // end IF
  } // end IF

// *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
//                SO_LINGER
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (!Net_Common_Tools::setLinger (handle,
                                    configuration_p->socketConfiguration.linger,
                                    -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setLinger(%d,%s,-1), aborting\n"),
                handle,
                (configuration_p->socketConfiguration.linger ? ACE_TEXT ("true")
                                                             : ACE_TEXT ("false"))));
    goto error;
  } // end IF
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: (on Linux), packet fragmentation is off by default, so sendto()-ing
  //         datagrams larger than MTU will trigger errno EMSGSIZE (90)
  //         --> enable packet fragmentation
  if (!Net_Common_Tools::setPathMTUDiscovery (handle,
                                              IP_PMTUDISC_WANT))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::disablePathMTUDiscovery(%d,%d), aborting\n"),
                handle,
                IP_PMTUDISC_WANT));
    goto error;
  } // end IF
#endif

#if defined (ACE_LINUX)
  if (errorQueue_)
    if (!Net_Common_Tools::enableErrorQueue (handle))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::enableErrorQueue() (handle was: %d), aborting\n"),
                  handle));
      goto error;
    } // end IF
#endif

//  // debug info
//  unsigned int so_max_msg_size = Net_Common_Tools::getMaxMsgSize (handle);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket 0x%@: %u byte(s)...\n"),
//              handle,
//              so_max_msg_size));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket %d: %u byte(s)...\n"),
//              handle,
//              so_max_msg_size));
//#endif

  return 0;

error:
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::dropRootPrivileges ();
#endif

  return -1;
}

template <typename ConfigurationType>
int
Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                       ConfigurationType>::handle_close (ACE_HANDLE handle_in,
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

  // initialize return value
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
      // *TODO*: validate (failed) connect/accept case
//      if (!isRegistered_)
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

      // asynch abort case

      if (handle_in != ACE_INVALID_HANDLE)
      {
        result = inherited2::peer_.close ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SOCK_IO::close (): %d, continuing\n")));
      } // end IF

//      ACE_Reactor* reactor_p = inherited2::reactor ();
//      ACE_ASSERT (reactor_p);
//      result =
//        reactor_p->remove_handler (handle_in,
//                                   (mask_in |
//                                    ACE_Event_Handler::DONT_CALL));
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@/%d, %d), continuing\n"),
//                    this, handle_in,
//                    mask_in));
      break;
    }
    default:
// *PORTABILITY*: this isn't entirely portable
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: 0x%@, mask: %d) --> check implementation !, continuing\n"),
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
