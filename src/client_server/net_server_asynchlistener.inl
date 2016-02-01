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
#endif

#include "ace/Default_Constants.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/WIN32_Proactor.h"
#else
#include "ace/POSIX_Asynch_IO.h"
#endif

#include "common.h"
#include "common_tools.h"

#include "net_common.h"
#include "net_macros.h"

#include "net_server_defines.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::Net_Server_AsynchListener_T ()
 : inherited ()
 //, addressFamily_ (ACE_ADDRESS_FAMILY_INET)
 , configuration_ ()
 , handlerConfiguration_ ()
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
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::~Net_Server_AsynchListener_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::~Net_Server_AsynchListener_T"));

  int result = -1;

  if (isListening_)
  {
    result = inherited::cancel ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Acceptor::cancel(): \"%m\", continuing")));
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::validate_connection (const ACE_Asynch_Accept::Result& result_in,
                                                                const ACE_INET_Addr& remoteAddress_in,
                                                                const ACE_INET_Addr& localAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::validate_connection"));

  ACE_UNUSED_ARG (localAddress_in);

  int result = -1;

  // success ?
  result = result_in.success ();
  if (result == 0)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset(buffer, 0, sizeof (buffer));
    int result_2 = remoteAddress_in.addr_to_string (buffer,
                                                    sizeof (buffer),
                                                    1);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Acceptor::accept(\"%s\"): \"%s\", aborting\n"),
                buffer,
                ACE_TEXT (ACE_OS::strerror (static_cast<int> (result_in.error ())))));
  } // end IF

  return ((result == 1) ? 0 : -1);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
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
  // *TODO*: remove type inference
  if (configuration_.messageAllocator)
  {
    typename StreamType::PROTOCOL_DATA_T* message_p =
      static_cast<typename StreamType::PROTOCOL_DATA_T*> (configuration_.messageAllocator->malloc (space_needed));
    message_block_p = message_p;
  } // end IF
  else
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
  if (!message_block_p)
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
                            configuration_.addressFamily);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Accept::accept(): \"%m\", aborting\n")));

    // clean up
    message_block_p->release ();
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
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::should_reissue_accept ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::should_reissue_accept"));

  // default behavior: delegate to baseclass
  return inherited::should_reissue_accept ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
const HandlerConfigurationType&
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::get"));

  return handlerConfiguration_;
}

//template <typename HandlerType,
//          typename AddressType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StreamType,
//          typename HandlerConfigurationType,
//          typename UserDataType>
//bool
//Net_Server_AsynchListener_T<HandlerType,
//                            AddressType,
//                            ConfigurationType,
//                            StateType,
//                            StreamType,
//                            HandlerConfigurationType,
//                            UserDataType>::initialize (const HandlerConfigurationType& configuration_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::initialize"));
//
//  handlerConfiguration_ = configuration_in;
//
//  return true;
//}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::useReactor () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::useReactor"));

  return false;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.socketHandlerConfiguration);

  // *TODO*: remove type inference
  //addressFamily_ = configuration_in.addressFamily;
  configuration_ = configuration_in;
  handlerConfiguration_ = *configuration_in.socketHandlerConfiguration;
  isInitialized_ = true;

  return true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::open (const ACE_INET_Addr& listenAddress_in,
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
  static ACE_INET_Addr sa (ACE_sap_any_cast (const ACE_INET_Addr &));
  ACE_TCHAR buffer[BUFSIZ];
  bool close_accept = false;
  int nunber_of_initial_accepts = numberOfInitialAccepts_in;

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
  if (listen_handle == ACE_INVALID_HANDLE)
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
  if (result == -1)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Accept::open(0x%@): \"%m\", aborting\n"),
                listen_handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Accept::open(%d): \"%m\", aborting\n"),
                listen_handle));
#endif
    goto close;
  } // end IF
#endif

  if (reuseAddr_in)
  {
    // Reuse the address
    int optval = 1;
    result = ACE_OS::setsockopt (listen_handle,
                                 SOL_SOCKET,
                                 SO_REUSEADDR,
                                 reinterpret_cast<const char*> (&optval),
                                 sizeof (int));
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                  listen_handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                  listen_handle));
#endif
      goto close;
    } // end IF
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // enable SIO_LOOPBACK_FAST_PATH on Win32
  if ((address_type == ACE_ADDRESS_FAMILY_INET) &&
      listenAddress_in.is_loopback ()           &&
      NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH)
    if (!Net_Common_Tools::setLoopBackFastPath (listen_handle))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLoopBackFastPath(0x%@): \"%m\", aborting\n"),
                  listen_handle));
      goto close;
    } // end IF
#endif

  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = listenAddress_in.addr_to_string (buffer,
                                            sizeof (buffer),
                                            1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  // If port is not specified, bind to any port.
  if (listenAddress_in == sa)
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
#endif
      goto close;
    } // end IF
  } // end IF

  // Bind to the specified port.
  result =
    ACE_OS::bind (listen_handle,                                              // handle
                  reinterpret_cast<sockaddr*> (listenAddress_in.get_addr ()), // address (handle)
                  listenAddress_in.get_size ());                              // address length
  if (result == -1)
  {
    //int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::bind(0x%@[\"%s\"]): \"%m\", aborting\n"),
                listen_handle, buffer));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::bind(%d[\"%s\"]): \"%m\", aborting\n"),
                listen_handle, buffer));
#endif
    goto close;
  } // end IF

  // Start listening.
  result = ACE_OS::listen (listen_handle, backLog_in);
  if (result == -1)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::listen(0x%@[\"%s\"],%d): \"%m\", aborting\n"),
                listen_handle, buffer,
                backLog_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::listen(%d[\"%s\"],%d): \"%m\", aborting\n"),
                listen_handle, buffer,
                backLog_in));
#endif
    goto close;
  } // end IF

  // For the number of <intial_accepts>.
  if (nunber_of_initial_accepts == -1)
    nunber_of_initial_accepts = backLog_in;
  for (int i = 0; i < nunber_of_initial_accepts; i++)
  {
    // Initiate accepts.
    result = this->accept (numberOfBytesToRead_in, NULL);
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Server_AsynchListener_T::accept(0x%@): \"%m\", aborting\n"),
                  listen_handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Server_AsynchListener_T::accept(%d): \"%m\", aborting\n"),
                  listen_handle));
#endif
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
#endif
  inherited::handle (ACE_INVALID_HANDLE);

  return -1;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::isInitialized () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::isInitialized"));

  return isInitialized_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::initialize ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::initialize"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::start"));

  // sanity check(s)
  if (isListening_)
    return; // nothing to do...

  // sanity check: configured ?
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF

  // not running --> start listening
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;

  // *TODO*: remove type inferences
  if (configuration_.useLoopBackDevice)
  {
    result =
        configuration_.address.set (configuration_.address.get_port_number (), // port
                                    INADDR_LOOPBACK,                           // IP address
                                    1,                                         // encode ?
                                    0);                                        // map ?
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
      return;
    } // end IF
  } // end IF
  result = configuration_.address.addr_to_string (buffer,
                                                  sizeof (buffer),
                                                  1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  result =
      open (configuration_.address,     // local SAP
            0,                          // bytes to read
            1,                          // pass_addresses ?
            ACE_DEFAULT_ASYNCH_BACKLOG, // backlog
            1,                          // SO_REUSEADDR ?
            NULL,                       // proactor (use default)
            true,                       // validate new connections ?
            1,                          // reissue_accept ?
            -1);                        // number of initial accepts
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Server_AsynchListener_T::open(\"%s\"): \"%m\", returning\n"),
                buffer));
    return;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: started listening (TCP) (\"%s\")...\n"),
              inherited::get_handle (),
              buffer));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: started listening (TCP) (\"%s\")...\n"),
              inherited::get_handle (),
              buffer));
#endif

  isListening_ = true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::stop (bool waitForCompletion_in,
                                                 bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  if (!isListening_)
    return; // nothing to do

  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = inherited::cancel ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Acceptor::cancel(): \"%m\", continuing\n")));
  result = ACE_OS::closesocket (inherited::handle ());
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
                reinterpret_cast<size_t> (inherited::handle ())));
  inherited::handle (ACE_INVALID_HANDLE);
  if (false);
#else
  const ACE_Asynch_Accept& asynch_accept_r = inherited::asynch_accept ();
  ACE_POSIX_Asynch_Accept* posix_asynch_accept_p = NULL;
  try
  {
    posix_asynch_accept_p =
      dynamic_cast<ACE_POSIX_Asynch_Accept*> (asynch_accept_r.implementation ());
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<ACE_POSIX_Asynch_Accept*>(%@) failed, aborting\n"),
                asynch_accept_r.implementation ()));
    posix_asynch_accept_p = NULL;
  }
  if (!posix_asynch_accept_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<ACE_POSIX_Asynch_Accept*>(%@) failed, aborting\n"),
                asynch_accept_r.implementation ()));
    return;
  }
  result = posix_asynch_accept_p->close ();
  if (result == -1)
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_POSIX_Asynch_Accept::close(): \"%m\", continuing\n")));
#endif
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("stopped listening...\n")));

  isListening_ = false;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::isRunning"));

  return isListening_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::dump_state"));

  // *TODO*: do something meaningful here...
  ACE_ASSERT (false);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::handle_accept (const ACE_Asynch_Accept::Result &result)
{
  ACE_TRACE ("Net_Server_AsynchListener_T::handle_accept");

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Asynch_Acceptor.cpp

  // Variable for error tracking
  int error = 0;

  // If the asynchronous accept fails.
  if (!result.success () || result.accept_handle () == ACE_INVALID_HANDLE)
  {
    error = 1;
  }

  ACE_HANDLE listen_handle = this->handle ();
#if defined (ACE_WIN32)
  // In order to use accept handle with other Window Sockets 1.1
  // functions, we call the setsockopt function with the
  // SO_UPDATE_ACCEPT_CONTEXT option. This option initializes the
  // socket so that other Windows Sockets routines to access the
  // socket correctly.
  if (!error &&
      ACE_OS::setsockopt (result.accept_handle (),
      SOL_SOCKET,
      SO_UPDATE_ACCEPT_CONTEXT,
      (char *)&listen_handle,
      sizeof (listen_handle)) == -1)
  {
    error = 1;
  }
#endif /* ACE_WIN32 */

  // Parse address.
  ACE_INET_Addr local_address;
  ACE_INET_Addr remote_address;
  if (!error &&
      (this->validate_new_connection () || this->pass_addresses ()))
      // Parse the addresses.
      this->parse_address (result,
      remote_address,
      local_address);

  // Validate remote address
  if (!error &&
      this->validate_new_connection () &&
      (this->validate_connection (result, remote_address, local_address) == -1))
  {
    error = 1;
  }

  HandlerType *new_handler = 0;
  if (!error)
  {
    // The Template method
    new_handler = this->make_handler ();
    if (new_handler == 0)
    {
      error = 1;
    }
  }

  // If no errors
  if (!error)
  {
    // Update the Proactor unless make_handler() or constructed handler
    // set up its own.
    if (new_handler->proactor () == 0)
      new_handler->proactor (this->proactor ());

    // Pass the addresses
    if (this->pass_addresses ())
      new_handler->addresses (remote_address,
      local_address);

    // *EDIT*: set role
    new_handler->set (NET_ROLE_SERVER);

    // Pass the ACT
    if (result.act () != 0)
      new_handler->act (result.act ());

    // Set up the handler's new handle value
    new_handler->handle (result.accept_handle ());

    // Initiate the handler
    new_handler->open (result.accept_handle (),
                       result.message_block ());
  }

  // On failure, no choice but to close the socket
  if (error &&
      result.accept_handle () != ACE_INVALID_HANDLE)
      ACE_OS::closesocket (result.accept_handle ());

  // Delete the dynamically allocated message_block
  result.message_block ().release ();

  // Start off another asynchronous accept to keep the backlog going,
  // unless we closed the listen socket already (from the destructor),
  // or this callback is the result of a canceled/aborted accept.
  if (this->should_reissue_accept () &&
      listen_handle != ACE_INVALID_HANDLE
#if defined (ACE_WIN32)
      && result.error () != ERROR_OPERATION_ABORTED
#else
      && result.error () != ECANCELED
#endif
      )
      this->accept (this->bytes_to_read (), result.act ());
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
HandlerType*
Net_Server_AsynchListener_T<HandlerType,
                            AddressType,
                            ConfigurationType,
                            StateType,
                            StreamType,
                            HandlerConfigurationType,
                            UserDataType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::make_handler"));

  // initialize return value(s)
  HandlerType* connection_p = NULL;

  // default behavior
  // *TODO*: remove type inference
  ACE_NEW_NORETURN (connection_p,
                    HandlerType (configuration_.connectionManager,
                                 configuration_.statisticReportingInterval));
  if (!connection_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return connection_p;
}
