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
#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename SocketType,
          typename ConfigurationType>
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       SocketType,
                       ConfigurationType>::Net_UDPSocketHandler_T ()
 : inherited ()
 , inherited2 (ACE_Thread_Manager::instance (), // no specific thread manager
               NULL,                            // no specific message queue
               ACE_Reactor::instance ())        // default reactor
 , inherited3 (ACE_Reactor::instance (),      // reactor
               this,                          // event handler
               ACE_Event_Handler::WRITE_MASK) // handle output only
 , address_ ()
#if defined (ACE_LINUX)
 , errorQueue_ (NET_SOCKET_DEFAULT_ERRORQUEUE)
#endif // ACE_LINUX
 , writeHandle_ (ACE_INVALID_HANDLE)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::Net_UDPSocketHandler_T"));

}

template <ACE_SYNCH_DECL,
          typename SocketType,
          typename ConfigurationType>
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       SocketType,
                       ConfigurationType>::~Net_UDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::~Net_UDPSocketHandler_T"));

  int result = -1;

  if (unlikely (writeHandle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::closesocket (writeHandle_);
    if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n")));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename SocketType,
          typename ConfigurationType>
int
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       SocketType,
                       ConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::open"));

  // sanity check(s)
  ACE_ASSERT (arg_in);

  static ACE_INET_Addr inet_addr_sap_any (ACE_sap_any_cast (const ACE_INET_Addr&));
  ConfigurationType* configuration_p = NULL;
  ACE_INET_Addr source_SAP;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  int result = -1;
#if defined (ACE_LINUX)
  unsigned short port_number = 0;
  bool handle_privileges = false;
#endif // ACE_LINUX
  bool handle_sockets = false;

  configuration_p = reinterpret_cast<ConfigurationType*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (configuration_p);

  // step0: configure addresses
  if (unlikely (configuration_p->sourcePort))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    struct _GUID interface_identifier;
#else
    std::string interface_identifier;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    std::string interface_identifier;
#endif // ACE_WIN32 || ACE_WIN64
    interface_identifier =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      Net_Common_Tools::IPAddressToInterface_2 (configuration_p->peerAddress);
#else
      Net_Common_Tools::IPAddressToInterface (configuration_p->peerAddress);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
      Net_Common_Tools::IPAddressToInterface (configuration_p->peerAddress);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
#else
    if (unlikely (interface_identifier.empty ()))
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    if (unlikely (interface_identifier.empty ()))
#endif // ACE_WIN32 || ACE_WIN64
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::IPAddressToInterface(%s), aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->peerAddress).c_str ())));
      goto error;
    } // end IF
    ACE_INET_Addr gateway_address;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (unlikely (!Net_Common_Tools::interfaceToIPAddress_2 (interface_identifier,
                                                             source_SAP,
                                                             gateway_address)))
#else
    if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                                           source_SAP,
                                                           gateway_address)))
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                                           source_SAP,
                                                           gateway_address)))
#endif // ACE_WIN32 || ACE_WIN64
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(%s): \"%m\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(%s): \"%m\", aborting\n"),
                  ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(%s): \"%m\", aborting\n"),
                  ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF
    source_SAP.set_port_number (static_cast<u_short> (configuration_p->sourcePort),
                                1);
  } // end IF

  // step1: open socket ?
  // *NOTE*: there are two distinct scenarios:
  //         - read-write: inherited2::PEER_STREAM maintains the inbound socket
  //                       handle; iff the outbound data is to have a specific
  //                       source port, open an additional 'bound' socket
  //                       (writeHandle_).
  //                       Note that as 'this' handles both input and output,
  //                       inherited2 has no definitive handle
  //                       --> maintain one (possibly two) handle(s)
  //         - write-only: (re)use inherited2::PEER_STREAM. Set writeHandle_
  //                       to inherited2::PEER_STREAM
  //                       --> maintain one handle
#if defined (ACE_LINUX)
  port_number =
      (configuration_p->writeOnly ? (configuration_p->sourcePort ? source_SAP
                                                                               : inet_addr_sap_any)
                                         : configuration_p->listenAddress).get_port_number ();
  // (temporarily) elevate privileges to open system sockets
  if (unlikely (!configuration_p->writeOnly &&
                (port_number <= NET_ADDRESS_MAXIMUM_PRIVILEGED_PORT)))
  {
    if (!Common_Tools::switchUser (0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::switchUser(0): \"%m\", aborting\n")));
      goto error;
    } // end IF
    handle_privileges = true;
  } // end IF
#endif // ACE_LINUX
  result =
    inherited2::peer_.open ((configuration_p->writeOnly ? (configuration_p->sourcePort ? source_SAP
                                                                                       : inet_addr_sap_any)
                                                        : configuration_p->listenAddress), // local SAP
                            ACE_PROTOCOL_FAMILY_INET,                                                    // protocol family
                            0,                                                                           // protocol
                            1);                                                                          // reuse_addr
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketType::open(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString ((configuration_p->writeOnly ? (configuration_p->sourcePort ? source_SAP
                                                                                                                          : inet_addr_sap_any)
                                                                                           : configuration_p->listenAddress)).c_str ())));
    goto error;
  } // end IF
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::switchUser (static_cast<uid_t> (-1));
#endif // ACE_LINUX
  handle = inherited2::peer_.get_handle ();
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);
  if (likely (!configuration_p->writeOnly))
  { ACE_ASSERT (writeHandle_ == ACE_INVALID_HANDLE);
    writeHandle_ = ACE_OS::socket (AF_INET,    // family
                                   SOCK_DGRAM, // type
                                   0);         // protocol
    if (unlikely (writeHandle_ == ACE_INVALID_HANDLE))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(%d,%d,0): \"%m\", aborting\n"),
                  AF_INET, SOCK_DGRAM));

      // clean up
      result = inherited2::close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to SocketType::close(): \"%m\", continuing\n")));

      goto error;
    } // end IF

    // set source address ?
    if (unlikely (configuration_p->sourcePort))
    {
      source_SAP.set (static_cast<u_short> (configuration_p->sourcePort),
                      static_cast<ACE_UINT32> (INADDR_ANY),
                      1,
                      0);
      result =
        ACE_OS::bind (writeHandle_,
                      reinterpret_cast<struct sockaddr*> (source_SAP.get_addr ()),
                      source_SAP.get_addr_size ());
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::bind(0x%@,%s): \"%m\", aborting\n"),
                    writeHandle_,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (source_SAP).c_str ())));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::bind(%d,%s): \"%m\", aborting\n"),
                    writeHandle_,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (source_SAP).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
        goto error;
      } // end IF
    } // end IF
  } // end IF
  else
    writeHandle_ = handle;
  handle_sockets = true;

  // step1: connect ?
  if (unlikely (configuration_p->connect))
  {
    result =
      ACE_OS::connect (writeHandle_,
                       reinterpret_cast<struct sockaddr*> (configuration_p->peerAddress.get_addr ()),
                       configuration_p->peerAddress.get_addr_size ());
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::connect(0x%@,%s): \"%m\", aborting\n"),
                  writeHandle_,
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->peerAddress).c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::connect(%d,%s): \"%m\", aborting\n"),
                  writeHandle_,
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->peerAddress).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("0x%@: associated datagram socket to %s\n"),
//                handle_in,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->peerAddress).c_str ())));
//#else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%d: associated datagram socket to %s\n"),
//                handle_in,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->peerAddress).c_str ())));
//#endif
  } // end IF

  // *NOTE*: recvfrom()-ing datagrams larger than SO_RCVBUF will truncate the
  //         inbound datagram (MSG_TRUNC flag will be set)
  // *TODO*: remove type inferences
  if (likely (!configuration_p->writeOnly))
  {
    // step3a: tweak inbound socket
    if (likely (configuration_p->bufferSize))
      if (unlikely (!Net_Common_Tools::setSocketBuffer (handle,
                                                        SO_RCVBUF,
                                                        configuration_p->bufferSize)))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), aborting\n"),
                    handle, configuration_p->bufferSize));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), aborting\n"),
                    handle, configuration_p->bufferSize));
#endif // ACE_WIN32 || ACE_WIN64
        goto error;
      } // end IF

    // *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
    //                SO_LINGER
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (unlikely (!Net_Common_Tools::setLinger (handle,
                                                configuration_p->linger,
                                                -1)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%d,%s,-1), aborting\n"),
                  handle, (configuration_p->linger ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
      goto error;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  // *TODO*: remove type inferences
  address_ = configuration_p->peerAddress;

  // step3b: tweak outbound socket
  // *NOTE*: sendto()-ing datagrams larger than SO_SNDBUF will trigger errno
  //         EMSGSIZE (90)
  if (likely (configuration_p->bufferSize))
    if (unlikely (!Net_Common_Tools::setSocketBuffer (writeHandle_,
                                                      SO_SNDBUF,
                                                      configuration_p->bufferSize)))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_SNDBUF,%u), aborting\n"),
                  writeHandle_, configuration_p->bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_SNDBUF,%u), continuing\n"),
                  writeHandle_, configuration_p->bufferSize));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: (on Linux), packet fragmentation is off by default, so sendto()-ing
  //         datagrams larger than MTU will trigger errno EMSGSIZE (90)
  //         --> enable packet fragmentation
  if (unlikely (!Net_Common_Tools::setPathMTUDiscovery (writeHandle_,
                                                        IP_PMTUDISC_WANT)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::disablePathMTUDiscovery(%d,IP_PMTUDISC_WANT), aborting\n"),
                writeHandle_));
    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_LINUX)
  if (likely (errorQueue_))
  {
    if (likely (!configuration_p->writeOnly))
      if (unlikely (!Net_Common_Tools::enableErrorQueue (handle)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::enableErrorQueue() (handle was: %d), aborting\n"),
                    handle));
        goto error;
      } // end IF
    if (unlikely (!Net_Common_Tools::enableErrorQueue (writeHandle_)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::enableErrorQueue() (handle was: %d), aborting\n"),
                  writeHandle_));
      goto error;
    } // end IF
  } // end IF
#endif // ACE_LINUX

//  // debug info
//  unsigned int so_max_msg_size = Net_Common_Tools::getMaxMsgSize (handle);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket 0x%@: %u byte(s)\n"),
//              handle,
//              so_max_msg_size));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket %d: %u byte(s)\n"),
//              handle,
//              so_max_msg_size));
//#endif

  // step3: register with the reactor
  if (likely (!configuration_p->writeOnly))
  {
    result = inherited2::open (arg_in);
    if (unlikely (result == -1))
    {
      // *NOTE*: this can happen when the connection handle is still registered
      //         with the reactor (i.e. the reactor is still processing events on
      //         a file descriptor that has been closed and is now being reused by
      //         the system)
      // *NOTE*: more likely, this happened because the (select) reactor is out of
      //         "free" (read) slots
      int error = ACE_OS::last_error ();
      ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Svc_Handler::open(0x%@) (handle was: 0x%@): \"%m\", aborting\n"),
                  arg_in, handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Svc_Handler::open(0x%@) (handle was: %d): \"%m\", aborting\n"),
                  arg_in, handle));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF
  } // end IF

  // *NOTE*: registered with the reactor (READ_MASK) at this point

  return 0;

error:
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::switchUser (static_cast<uid_t> (-1));
#endif // ACE_LINUX
  if (handle_sockets)
  {
    result = inherited2::peer_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to SocketType::close(): \"%m\", continuing\n")));
    if (!configuration_p->writeOnly)
    { ACE_ASSERT (writeHandle_ != ACE_INVALID_HANDLE);
      result = ACE_OS::closesocket (writeHandle_);
      if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                    writeHandle_));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                    writeHandle_));
#endif // ACE_WIN32 || ACE_WIN64
      writeHandle_ = ACE_INVALID_HANDLE;
    } // end IF
  } // end IF

  return -1;
}

template <ACE_SYNCH_DECL,
          typename SocketType,
          typename ConfigurationType>
int
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       SocketType,
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

      if (likely (handle_in != ACE_INVALID_HANDLE))
      {
        //result = inherited2::peer_.close ();
        result = inherited2::close ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      //ACE_TEXT ("failed to ACE_SOCK_IO::close (): %d, continuing\n")));
                      ACE_TEXT ("failed to ACE_Svc_Handler::close (): %d, continuing\n")));
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
#endif // ACE_WIN32 || ACE_WIN64
      break;
  } // end SWITCH

  return result;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       Net_SOCK_CODgram,
                       ConfigurationType>::Net_UDPSocketHandler_T ()
 : inherited ()
 , inherited2 (NULL,                     // no specific thread manager
               NULL,                     // no specific message queue
               ACE_Reactor::instance ()) // default reactor
 , inherited3 (ACE_Reactor::instance (),      // reactor
               this,                          // event handler
               ACE_Event_Handler::WRITE_MASK) // handle output only
 , address_ ()
#if defined (ACE_LINUX)
 , errorQueue_ (NET_SOCKET_DEFAULT_ERRORQUEUE)
#endif // ACE_LINUX
 , writeHandle_ (ACE_INVALID_HANDLE)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::Net_UDPSocketHandler_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       Net_SOCK_CODgram,
                       ConfigurationType>::~Net_UDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::~Net_UDPSocketHandler_T"));

  int result = -1;

  if (unlikely (writeHandle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::closesocket (writeHandle_);
    if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n")));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
int
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       Net_SOCK_CODgram,
                       ConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPSocketHandler_T::open"));

  // sanity check(s)
  ACE_ASSERT (arg_in);

  static ACE_INET_Addr inet_addr_sap_any (ACE_sap_any_cast (const ACE_INET_Addr&));
  ConfigurationType* configuration_p = NULL;
  ACE_INET_Addr source_SAP;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  int result = -1;
#if defined (ACE_LINUX)
  bool handle_privileges = false;
#endif // ACE_LINUX
  bool handle_sockets = false;

  configuration_p = reinterpret_cast<ConfigurationType*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (configuration_p);

  // step0: configure addresses
  if (unlikely (configuration_p->sourcePort))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    struct _GUID interface_identifier =
#else
    std::string interface_identifier =
#endif // _WIN32_WINNT_VISTA
#else
    std::string interface_identifier =
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      Net_Common_Tools::IPAddressToInterface_2 (configuration_p->peerAddress);
#else
      Net_Common_Tools::IPAddressToInterface (configuration_p->peerAddress);
#endif // _WIN32_WINNT_VISTA
#else
      Net_Common_Tools::IPAddressToInterface (configuration_p->peerAddress);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
#else
    if (unlikely (interface_identifier.empty ()))
#endif // _WIN32_WINNT_VISTA
#else
    if (unlikely (interface_identifier.empty ()))
#endif // ACE_WIN32 || ACE_WIN64
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::IPAddressToInterface(%s), aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->peerAddress).c_str ())));
      goto error;
    } // end IF
    ACE_INET_Addr gateway_address;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (unlikely (!Net_Common_Tools::interfaceToIPAddress_2 (interface_identifier,
                                                             source_SAP,
                                                             gateway_address)))
#else
    if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                                           source_SAP,
                                                           gateway_address)))
#endif // _WIN32_WINNT_VISTA
#else
    if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                                           source_SAP,
                                                           gateway_address)))
#endif // ACE_WIN32 || ACE_WIN64
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress_2(%s): \"%m\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(%s): \"%m\", aborting\n"),
                  ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(%s): \"%m\", aborting\n"),
                  ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF
    source_SAP.set_port_number (static_cast<u_short> (configuration_p->sourcePort),
                                1);
  } // end IF

  // step1: open socket ?
  // *NOTE*: there are two distinct scenarios:
  //         - read-write: inherited2::PEER_STREAM maintains the inbound socket
  //                       handle; iff the outbound data is to have a specific
  //                       source port, open an additional 'bound' socket
  //                       (writeHandle_).
  //                       Note that as 'this' handles both input and output,
  //                       inherited2 has no definitive handle
  //                       --> maintain one (possibly two) handle(s)
  //         - write-only: (re)use inherited2::PEER_STREAM. Set writeHandle_
  //                       to inherited2::PEER_STREAM
  //                       --> maintain one handle
#if defined (ACE_LINUX)
  // (temporarily) elevate privileges to open system sockets
  // *TODO*: this is incomplete
  if (unlikely (!configuration_p->writeOnly &&
                (configuration_p->listenAddress.get_port_number () <= NET_ADDRESS_MAXIMUM_PRIVILEGED_PORT)))
  {
    if (!Common_Tools::switchUser (0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::switchUser(0): \"%m\", aborting\n")));
      goto error;
    } // end IF
    handle_privileges = true;
  } // end IF
#endif // ACE_LINUX
  result =
    inherited2::peer_.open (configuration_p->peerAddress,                                         // remote SAP
                            (configuration_p->writeOnly ? (configuration_p->sourcePort ? source_SAP
                                                                                                     : inet_addr_sap_any)
                                                               : configuration_p->listenAddress), // local SAP
                            ACE_PROTOCOL_FAMILY_INET,                                                    // protocol family
                            0,                                                                           // protocol
                            1);                                                                          // reuse_addr
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_CODgram::open(%s,%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->peerAddress).c_str ()),
                ACE_TEXT (Net_Common_Tools::IPAddressToString ((configuration_p->writeOnly ? (configuration_p->sourcePort ? source_SAP
                                                                                                                                        : inet_addr_sap_any)
                                                                                                  : configuration_p->listenAddress)).c_str ())));
    goto error;
  } // end IF
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::switchUser (static_cast<uid_t> (-1));
#endif // ACE_LINUX
  handle = inherited2::peer_.get_handle ();
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);
  if (likely (!configuration_p->writeOnly))
  { ACE_ASSERT (writeHandle_ == ACE_INVALID_HANDLE);
    writeHandle_ = ACE_OS::socket (AF_INET,    // family
                                   SOCK_DGRAM, // type
                                   0);         // protocol
    if (unlikely (writeHandle_ == ACE_INVALID_HANDLE))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(%d,%d,0): \"%m\", aborting\n"),
                  AF_INET, SOCK_DGRAM));

      // clean up
      result = inherited2::close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to SocketType::close(): \"%m\", continuing\n")));

      goto error;
    } // end IF

    // set source address ?
    if (unlikely (configuration_p->sourcePort))
    {
      source_SAP.set (static_cast<u_short> (configuration_p->sourcePort),
                      static_cast<ACE_UINT32> (INADDR_ANY),
                      1,
                      0);
      result =
        ACE_OS::bind (writeHandle_,
                      reinterpret_cast<struct sockaddr*> (source_SAP.get_addr ()),
                      source_SAP.get_addr_size ());
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        if (error != EINVAL) // 22: already bound
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_OS::bind(0x%@,%s): \"%m\", aborting\n"),
                      writeHandle_,
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (source_SAP).c_str ())));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_OS::bind(%d,%s): \"%m\", aborting\n"),
                      writeHandle_,
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (source_SAP).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
          goto error;
        } // end IF
      } // end IF
    } // end IF
  } // end IF
  else
    writeHandle_ = handle;
  handle_sockets = true;

  // *TODO*: remove type inferences
  address_ = configuration_p->peerAddress;

  // *NOTE*: recvfrom()-ing datagrams larger than SO_RCVBUF will truncate the
  //         inbound datagram (MSG_TRUNC flag will be set)
  // *TODO*: remove type inferences
  if (likely (!configuration_p->writeOnly))
  {
    // step3a: tweak inbound socket
    if (likely (configuration_p->bufferSize))
      if (unlikely (!Net_Common_Tools::setSocketBuffer (handle,
                                                        SO_RCVBUF,
                                                        configuration_p->bufferSize)))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), aborting\n"),
                    handle, configuration_p->bufferSize));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), aborting\n"),
                    handle, configuration_p->bufferSize));
#endif // ACE_WIN32 || ACE_WIN64
        goto error;
      } // end IF

    // *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
    //                SO_LINGER
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (unlikely (!Net_Common_Tools::setLinger (handle,
                                                configuration_p->linger,
                                                -1)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%d,%s,-1), aborting\n"),
                  handle, (configuration_p->linger ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
      goto error;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  // step3b: tweak outbound socket
  // *NOTE*: sendto()-ing datagrams larger than SO_SNDBUF will trigger errno
  //         EMSGSIZE (90)
  if (likely (configuration_p->bufferSize))
    if (unlikely (!Net_Common_Tools::setSocketBuffer (writeHandle_,
                                                      SO_SNDBUF,
                                                      configuration_p->bufferSize)))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_SNDBUF,%u), aborting\n"),
                  writeHandle_, configuration_p->bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_SNDBUF,%u), continuing\n"),
                  writeHandle_, configuration_p->bufferSize));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: (on Linux), packet fragmentation is off by default, so sendto()-ing
  //         datagrams larger than MTU will trigger errno EMSGSIZE (90)
  //         --> enable packet fragmentation
  if (unlikely (!Net_Common_Tools::setPathMTUDiscovery (writeHandle_,
                                                        IP_PMTUDISC_WANT)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::disablePathMTUDiscovery(%d,IP_PMTUDISC_WANT), aborting\n"),
                writeHandle_));
    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_LINUX)
  if (errorQueue_)
  {
    if (likely (!configuration_p->writeOnly))
      if (!Net_Common_Tools::enableErrorQueue (handle))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::enableErrorQueue() (handle was: %d), aborting\n"),
                    handle));
        goto error;
      } // end IF
    if (unlikely (!Net_Common_Tools::enableErrorQueue (writeHandle_)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::enableErrorQueue() (handle was: %d), aborting\n"),
                  writeHandle_));
      goto error;
    } // end IF
  } // end IF
#endif // ACE_LINUX

//  // debug info
//  unsigned int so_max_msg_size = Net_Common_Tools::getMaxMsgSize (handle);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket 0x%@: %u byte(s)\n"),
//              handle,
//              so_max_msg_size));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("maximum message size for UDP socket %d: %u byte(s)\n"),
//              handle,
//              so_max_msg_size));
//#endif

  // step3: register with the reactor
  if (likely (!configuration_p->writeOnly))
  {
    result = inherited2::open (arg_in);
    if (unlikely (result == -1))
    {
      // *NOTE*: this can happen when the connection handle is still registered
      //         with the reactor (i.e. the reactor is still processing events on
      //         a file descriptor that has been closed and is now being reused by
      //         the system)
      // *NOTE*: more likely, this happened because the (select) reactor is out of
      //         "free" (read) slots
      int error = ACE_OS::last_error ();
      ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Svc_Handler::open(0x%@) (handle was: 0x%@): \"%m\", aborting\n"),
                  arg_in, handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Svc_Handler::open(0x%@) (handle was: %d): \"%m\", aborting\n"),
                  arg_in, handle));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF
  } // end IF

  // *NOTE*: registered with the reactor (READ_MASK) at this point

  return 0;

error:
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::switchUser (static_cast<uid_t> (-1));
#endif // ACE_LINUX
  if (handle_sockets)
  {
    result = inherited2::peer_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to SocketType::close(): \"%m\", continuing\n")));
    if (!configuration_p->writeOnly)
    { ACE_ASSERT (writeHandle_ != ACE_INVALID_HANDLE);
      result = ACE_OS::closesocket (writeHandle_);
      if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                    writeHandle_));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                    writeHandle_));
#endif // ACE_WIN32 || ACE_WIN64
      writeHandle_ = ACE_INVALID_HANDLE;
    } // end IF
  } // end IF

  return -1;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
int
Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                       Net_SOCK_CODgram,
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

      if (likely (handle_in != ACE_INVALID_HANDLE))
      {
        result = inherited2::peer_.close ();
        if (unlikely (result == -1))
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
#endif // ACE_WIN32 || ACE_WIN64
      break;
  } // end SWITCH

  return result;
}
