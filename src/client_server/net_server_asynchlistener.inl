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

#if defined (_MSC_VER)
#include <crtdefs.h>
#endif // _MSC_VER

#include "ace/Default_Constants.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/WIN32_Proactor.h"
#else
#include "ace/POSIX_Asynch_IO.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "common.h"
#include "common_tools.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_connection_configuration.h"
#include "net_itransportlayer.h"
#include "net_macros.h"

#include "net_server_defines.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::Net_Server_AsynchListener_T ()
 : inherited ()
 , configuration_ (NULL)
 , isInitialized_ (false)
 , isListening_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::Net_Server_AsynchListener_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::~Net_Server_AsynchListener_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::~Net_Server_AsynchListener_T"));

  int result = -1;

  if (unlikely (isListening_))
  {
    result = inherited::cancel ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Acceptor::cancel(): \"%m\", continuing")));
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::validate_connection (const ACE_Asynch_Accept::Result& result_in,
                                                                const ACE_INET_Addr& remoteAddress_in,
                                                                const ACE_INET_Addr& localAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::validate_connection"));

  ACE_UNUSED_ARG (localAddress_in);

  int result = result_in.success ();

  // success ?
  if (unlikely (result == 0))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Acceptor::accept(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (remoteAddress_in).c_str ()),
                ACE_TEXT (ACE_OS::strerror (static_cast<int> (result_in.error ())))));

  return ((result == 1) ? 0 : -1);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::accept (size_t bytesToRead_in,
                                                   const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::accept"));

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Asynch_Acceptor.cpp

  int result = -1;

  // The space_needed calculation is drive by needs of Windows. POSIX doesn't
  // need to extra 16 bytes, but it doesn't hurt.
  size_t space_needed = sizeof (sockaddr_in) + 16;
#if defined (ACE_HAS_IPV6)
  if (PF_INET6 == this->addr_family_)
    space_needed = sizeof (sockaddr_in6) + 16;
#endif /* ACE_HAS_IPV6 */
  space_needed = (2 * space_needed) + bytesToRead_in;

  // Create a new message block big enough for the addresses and data
  ACE_Message_Block* message_block_p = NULL;
  // sanity check(s)
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  if (likely (configuration_->messageAllocator))
  {
    typename StreamType::MESSAGE_T* message_p =
      static_cast<typename StreamType::MESSAGE_T*> (configuration_->messageAllocator->malloc (space_needed));
    message_block_p = message_p;
  } // end IF
  else
  { ACE_ASSERT (false); // this needs to be typed
    ACE_NEW_NORETURN (message_block_p,
                      ACE_Message_Block (space_needed,
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
  } // end IF
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return -1;
  } // end IF

//  // Create a new socket for the connection
//  ACE_HANDLE accept_handle = ACE_INVALID_HANDLE;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  accept_handle = ACE_OS::socket (configuration_.addressFamily, // domain
//                                  SOCK_STREAM,                  // type
//                                  0,                            // protocol
//                                  NULL,                         // protocol info
//                                  0,                            // group
//                                  WSA_FLAG_OVERLAPPED);         // flags
//  //if (accept_handle != ACE_INVALID_HANDLE)
//  //{
//  //  ACE_Proactor* proactor_p = ACE_Proactor::instance ();
//  //  ACE_ASSERT (proactor_p);
//  //  result = proactor_p->register_handle (accept_handle, NULL);
//  //  if (result == -1)
//  //  {
//  //    ACE_DEBUG ((LM_ERROR,
//  //                ACE_TEXT ("failed to ACE_Proactor::register_handle(0x%@): \"%m\", aborting\n"),
//  //                accept_handle));
//  //
//  //    // clean up
//  //    message_block_p->release ();
//
//  //    return -1;
//  //  } // end IF
//  //} // end IF
//#else
//  accept_handle = ACE_OS::socket (configuration_.addressFamily, // domain
//                                  SOCK_STREAM,                  // type
//                                  0);                           // protocol
//#endif
//  if (accept_handle == ACE_INVALID_HANDLE)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_OS::socket(%d,%d,0): \"%m\", aborting\n"),
//                configuration_.addressFamily,
//                SOCK_STREAM));
//
//    // clean up
//    message_block_p->release ();
//
//    return -1;
//  } // end IF

  // Initiate asynchronous accept(s)
  ACE_Asynch_Accept& asynch_accept_r = inherited::asynch_accept ();
  ILISTENER_T* listener_p = this;
  const void* act_p = (act_in ? act_in : listener_p);
  // *TODO*: remove type inference
  result =
    asynch_accept_r.accept (*message_block_p,                    // message block
                            bytesToRead_in,                      // bytes to read initially
                            //accept_handle,                       // new connection handle
                            ACE_INVALID_HANDLE,
                            act_p,                               // ACT
                            0,                                   // priority
                            COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL, // (real-time) signal
                            //this->addr_family_,                // address family
                            ACE_ADDRESS_FAMILY_INET);                 
                            //configuration_->addressFamily);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Accept::accept(): \"%m\", aborting\n")));
    message_block_p->release (); message_block_p = NULL;
//    result = ACE_OS::closesocket (accept_handle);
//    if (result == -1)
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
//                  accept_handle));
//#else
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
//                  accept_handle));
//#endif
    return -1;
  } // end IF

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
bool
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::initialize"));

  // *TODO*: remove type inference
  //addressFamily_ = configuration_in.addressFamily;
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isInitialized_ = true;

  return true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::open (const AddressType& listenAddress_in,
                                                 size_t numberOfBytesToRead_in,
                                                 bool passAddresses_in,
                                                 int backLog_in,
                                                 int reuseAddr_in,
                                                 ACE_Proactor* proactor_in,
                                                 bool validateConnections_in,
                                                 int reissueAccept_in,
                                                 int numberOfInitialAccepts_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::open"));

  int result = -1;
  static AddressType SAP_any (ACE_sap_any_cast (const AddressType&));
  bool close_accept = false;
  int number_of_initial_accepts = numberOfInitialAccepts_in;

  inherited::proactor (proactor_in);
  inherited::pass_addresses (passAddresses_in);
  inherited::bytes_to_read (numberOfBytesToRead_in);
  inherited::validate_new_connection (validateConnections_in);
  inherited::reissue_accept (reissueAccept_in);
  // *WARNING*: cannot currently set this in the base class
  //this->addr_family_ = address.get_type ();

  // Create the listener socket
  // *NOTE*: some socket options need to be set before accept()ing
  //         --> set these here
  int address_type = listenAddress_in.get_type ();
  ACE_HANDLE listen_handle = ACE_OS::socket (address_type, // domain
                                             SOCK_STREAM,  // type
                                             0);           // protocol
  if (unlikely (listen_handle == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(%d,%d,0): \"%m\", aborting\n"),
                address_type, SOCK_STREAM));
    return -1;
  } // end IF
  // *NOTE*: implicitly calls ACE_Asynch_Accept::open(),
  //         see Asynch_Acceptor.cpp:183
  inherited::set_handle (listen_handle);

  // Initialize the ACE_Asynch_Accept
  ACE_Asynch_Accept& asynch_accept_r = inherited::asynch_accept ();
  close_accept = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: is this really necessary  (see above) ?
  result = asynch_accept_r.open (*this,         // handler
                                 listen_handle, // socket handle
                                 NULL,          // completion key
                                 proactor_in);  // proactor handle
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Accept::open(0x%@): \"%m\", aborting\n"),
                listen_handle));
    goto close;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  if (likely (reuseAddr_in))
  {
    // Reuse the address
    int optval = 1;
    result = ACE_OS::setsockopt (listen_handle,
                                 SOL_SOCKET,
                                 SO_REUSEADDR,
                                 reinterpret_cast<const char*> (&optval),
                                 sizeof (int));
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                  listen_handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                  listen_handle));
#endif // ACE_WIN32 || ACE_WIN64
      goto close;
    } // end IF
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  // enable SIO_LOOPBACK_FAST_PATH on Win32 ?
  if ((address_type == ACE_ADDRESS_FAMILY_INET) &&
      listenAddress_in.is_loopback ()           &&
      NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH)
    if (unlikely (!Net_Common_Tools::setLoopBackFastPath (listen_handle)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLoopBackFastPath(0x%@): \"%m\", aborting\n"),
                  listen_handle));
      goto close;
    } // end IF
#endif // _WIN32_WINNT_WIN8
#endif // ACE_WIN32 || ACE_WIN64

  // If port is not specified, bind to any port.
  if (unlikely (listenAddress_in == SAP_any))
  //if (listenAddress_in.is_any ())
  {
    result = ACE::bind_port (listen_handle, // handle
                             INADDR_ANY,    // address
                             address_type); // family
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::bind_port(0x%@,%u,%d): \"%m\", aborting\n"),
                  listen_handle,
                  INADDR_ANY,
                  address_type));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::bind_port(%d,%u,%d): \"%m\", aborting\n"),
                  listen_handle,
                  INADDR_ANY,
                  address_type));
#endif // ACE_WIN32 || ACE_WIN64
      goto close;
    } // end IF
  } // end IF

  // Bind to the specified port.
  result =
    ACE_OS::bind (listen_handle,                                                     // handle
                  reinterpret_cast<struct sockaddr*> (listenAddress_in.get_addr ()), // address (handle)
                  listenAddress_in.get_size ());                                     // address length
  if (unlikely (result == -1))
  {
    //int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::bind(0x%@[\"%s\"]): \"%m\", aborting\n"),
                listen_handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (listenAddress_in).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::bind(%d[\"%s\"]): \"%m\", aborting\n"),
                listen_handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (listenAddress_in).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
    goto close;
  } // end IF

  // Start listening.
  result = ACE_OS::listen (listen_handle, backLog_in);
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::listen(0x%@[\"%s\"],%d): \"%m\", aborting\n"),
                listen_handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (listenAddress_in).c_str ()),
                backLog_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::listen(%d[\"%s\"],%d): \"%m\", aborting\n"),
                listen_handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (listenAddress_in).c_str ()),
                backLog_in));
#endif // ACE_WIN32 || ACE_WIN64
    goto close;
  } // end IF

  // For the number of <intial_accepts>.
  if (likely (number_of_initial_accepts == -1))
    number_of_initial_accepts = backLog_in;
  for (int i = 0; i < number_of_initial_accepts; i++)
  {
    // Initiate accepts.
    result = this->accept (numberOfBytesToRead_in, NULL);
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Server_AsynchListener_T::accept(0x%@): \"%m\", aborting\n"),
                  listen_handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Server_AsynchListener_T::accept(%d): \"%m\", aborting\n"),
                  listen_handle));
#endif // ACE_WIN32 || ACE_WIN64
      goto close;
    } // end IF
  } // end FOR

  return 0;

close:
  ACE_Errno_Guard errno_guard (errno);

  if (close_accept)
  {
    result = asynch_accept_r.cancel ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Accept::cancel(): \"%m\", continuing\n")));
  } // end IF

  result = ACE_OS::closesocket (listen_handle);
  if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                listen_handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                listen_handle));
#endif // ACE_WIN32 || ACE_WIN64
  inherited::handle (ACE_INVALID_HANDLE);

  return -1;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::start (ACE_thread_t& threadId_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::start"));

  // initialize return value(s)
  threadId_out = 0;

  int result = -1;

  // sanity check(s)
  if (unlikely (!isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (unlikely (isListening_))
    return; // nothing to do

  // not running --> start listening

  // sanity check(s)
  ACE_ASSERT (configuration_);

  // *TODO*: remove type inferences
  if (unlikely (configuration_->useLoopBackDevice))
  {
    result =
      configuration_->address.set (configuration_->address.get_port_number (), // port
                                   INADDR_LOOPBACK,                            // address
                                   1,                                          // encode ?
                                   0);                                         // map ?
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
      return;
    } // end IF
  } // end IF
  result =
    open (configuration_->address,    // local SAP
          0,                          // bytes to read
          1,                          // pass_addresses ?
          ACE_DEFAULT_ASYNCH_BACKLOG, // backlog
          1,                          // SO_REUSEADDR ?
          NULL,                       // proactor (use default)
          true,                       // validate new connections ?
          1,                          // reissue_accept ?
          -1);                        // number of initial accepts
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Server_AsynchListener_T::open(%s): \"%m\", returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->address).c_str ())));
    return;
  } // end IF
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: started listening: %s\n"),
              inherited::get_handle (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: started listening: %s\n"),
              inherited::get_handle (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

  isListening_ = true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::stop (bool waitForCompletion_in,
                                                 bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  if (unlikely (!isListening_))
    return; // nothing to do

  int result = -1;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = inherited::cancel ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Acceptor::cancel(): \"%m\", continuing\n")));
  result = ACE_OS::closesocket (inherited::handle ());
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
                reinterpret_cast<size_t> (inherited::handle ())));
  inherited::handle (ACE_INVALID_HANDLE);
  if (unlikely (false));
#else
  const ACE_Asynch_Accept& asynch_accept_r = inherited::asynch_accept ();
  ACE_POSIX_Asynch_Accept* posix_asynch_accept_p = NULL;
  try {
    posix_asynch_accept_p =
      dynamic_cast<ACE_POSIX_Asynch_Accept*> (asynch_accept_r.implementation ());
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<ACE_POSIX_Asynch_Accept*>(%@) failed, aborting\n"),
                asynch_accept_r.implementation ()));
    posix_asynch_accept_p = NULL;
  }
  if (unlikely (!posix_asynch_accept_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<ACE_POSIX_Asynch_Accept*>(%@) failed, aborting\n"),
                asynch_accept_r.implementation ()));
    return;
  }
  result = posix_asynch_accept_p->close ();
  if (unlikely (result == -1))
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT ("failed to ACE_POSIX_Asynch_Accept::close(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
#if defined (_DEBUG)
  else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: stopped listening: %s\n"),
                inherited::get_handle (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->address).c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: stopped listening: %s\n"),
                inherited::get_handle (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

  isListening_ = false;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::handle_accept (const ACE_Asynch_Accept::Result &result)
{
  ACE_TRACE ("Net_Server_AsynchListener_T::handle_accept");

  int error = 0;
  ACE_HANDLE listen_handle = ACE_INVALID_HANDLE;
  HandlerType* handler_p = NULL;
  AddressType remote_address, local_address;

  if (unlikely (!result.success () ||
                (result.accept_handle () == ACE_INVALID_HANDLE)))
    error = 1;

  listen_handle = this->handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // In order to use accept handle with other Window Sockets 1.1
  // functions, we call the setsockopt function with the
  // SO_UPDATE_ACCEPT_CONTEXT option. This option initializes the
  // socket so that other Windows Sockets routines to access the
  // socket correctly.
  if (unlikely (!error &&
                ACE_OS::setsockopt (result.accept_handle (),
                                    SOL_SOCKET,
                                    SO_UPDATE_ACCEPT_CONTEXT,
                                    (char*)&listen_handle,
                                    sizeof (listen_handle)) == -1))
    error = 1;
#endif // ACE_WIN32 || ACE_WIN64

  // retrieve addresses ?
  if (likely (!error &&
              (this->validate_new_connection () || this->pass_addresses ())))
    this->parse_address (result,
                         remote_address,
                         local_address);

  // validate remote address
  if (unlikely (!error &&
                this->validate_new_connection () &&
                (this->validate_connection (result,
                                            remote_address, local_address) == -1)))
    error = 1;

  if (likely (!error))
  {
    handler_p = this->make_handler ();
    if (unlikely (!handler_p))
      error = 1;
  } // end IF

  if (likely (!error))
  {
    if (unlikely (!handler_p->proactor ()))
      handler_p->proactor (this->proactor ());

    if (likely (this->pass_addresses ()))
      handler_p->addresses (remote_address,
                            local_address);

    // *EDIT*: set role
//    Net_ILinkLayer_T<Net_TCPSocketConfiguration_t>* ilinklayer_p = ;
    handler_p->set (NET_ROLE_SERVER);

    // Pass the ACT
    if (unlikely (result.act ()))
      handler_p->act (result.act ());

    handler_p->handle (result.accept_handle ());

    handler_p->open (result.accept_handle (),
                     result.message_block ());
  } // end IF

  if (unlikely (error &&
                result.accept_handle () != ACE_INVALID_HANDLE))
    ACE_OS::closesocket (result.accept_handle ());
  result.message_block ().release ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (likely (this->should_reissue_accept ()               &&
              (listen_handle != ACE_INVALID_HANDLE)        &&
              (result.error () != ERROR_OPERATION_ABORTED)))
#else
  if (likely (this->should_reissue_accept ()        &&
              (listen_handle != ACE_INVALID_HANDLE) &&
              (result.error () != ECANCELED)))
#endif // ACE_WIN32
    this->accept (this->bytes_to_read (), result.act ());
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
HandlerType*
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            UserDataType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::make_handler"));

  // initialize return value(s)
  HandlerType* connection_p = NULL;

  // *TODO*: remove type inference
  ACE_NEW_NORETURN (connection_p,
                    HandlerType (true)); // managed ?
  if (unlikely (!connection_p))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return connection_p;
}
