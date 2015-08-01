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
#include "ace/POSIX_Asynch_IO.h"

#include "common.h"

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
                                                                const ACE_INET_Addr& remoteSAP_in,
                                                                const ACE_INET_Addr& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::validate_connection"));

  int result = -1;

  // success ?
  result = result_in.success ();
  if (result == 0)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset(buffer, 0, sizeof (buffer));
    int result_2 = remoteSAP_in.addr_to_string (buffer, sizeof (buffer));
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Acceptor::accept(\"%s\"): \"%s\", aborting\n"),
                buffer,
                ACE_TEXT (ACE_OS::strerror (result_in.error ()))));
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
    message_block_p =
      static_cast<ACE_Message_Block*> (configuration_.messageAllocator->malloc (space_needed));
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

  // initiate asynchronous accepts
  ACE_Asynch_Accept& asynch_accept_r = inherited::asynch_accept ();
  ILISTENER_T* listener_p = this;
  const void* act_p = (act_in ? act_in : listener_p);
  result = asynch_accept_r.accept (*message_block_p,
                                   bytesToRead_in,
                                   ACE_INVALID_HANDLE,
                                   act_p,
                                   0,
                                   COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL,
                                   //this->addr_family_,
                                   configuration_.addressFamily);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Accept::accept(): \"%m\", aborting\n")));

    // Cleanup on error
    message_block_p->release ();

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
  int result = -1;
  ACE_INET_Addr local_address;
  // *TODO*: remove type inferences
  if (configuration_.useLoopbackDevice)
    result = local_address.set (configuration_.portNumber, // port
                                // *PORTABILITY*: needed to disambiguate this under Windows :-(
                                // *TODO*: bind to specific interface/address ?
                                ACE_LOCALHOST,             // hostname
                                1,                         // encode ?
                                AF_INET);                  // address family
  else
    result = local_address.set (configuration_.portNumber,            // port
                                // *PORTABILITY*: needed to disambiguate this under Windows :-(
                                // *TODO*: bind to specific interface/address ?
                                static_cast<ACE_UINT32> (INADDR_ANY), // hostname
                                1,                                    // encode ?
                                0);                                   // map ?
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
    return;
  } // end IF
  result = inherited::open (local_address,              // local SAP
                            0,                          // bytes_to_read
                            1,                          // pass_addresses ?
                            ACE_DEFAULT_ASYNCH_BACKLOG, // backlog
                            1,                          // reuse address ?
                            NULL,                       // proactor (use default)
                            true,                       // validate new connections ?
                            1,                          // reissue_accept ?
                            -1);                        // number of initial accepts
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Acceptor::open(%u): \"%m\", returning\n"),
                configuration_.portNumber));
    return;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("started listening (port: %u)...\n"),
              configuration_.portNumber));

  // all is well...
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
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("not listening --> nothing to do, returning\n")));
    return;
  } // end IF

  int result = -1;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
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
#else
  result = inherited::cancel ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Acceptor::cancel(): \"%m\", continuing\n")));
  result = ACE_OS::closesocket (inherited::handle ());
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(): \"%m\", continuing\n")));
  inherited::handle (ACE_INVALID_HANDLE);
  if (false);
#endif
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("stopped listening (port was: %u)...\n"),
                configuration_.portNumber));

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
