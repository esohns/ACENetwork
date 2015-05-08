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
#include "ace/Default_Constants.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/POSIX_Asynch_IO.h"

#include "net_macros.h"

#include "net_server_defines.h"

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::Net_Server_AsynchListener_T ()
 : inherited ()
 , addressFamily_ (ACE_ADDRESS_FAMILY_INET)
 , allocator_ (NULL)
 , configuration_ (NULL)
 , interfaceHandle_ (NULL)
 , isInitialized_ (false)
 , isListening_ (false)
 , listeningPort_ (NET_SERVER_DEFAULT_LISTENING_PORT)
 , useLoopback_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::Net_Server_AsynchListener_T"));

}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::~Net_Server_AsynchListener_T ()
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

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
int
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::validate_connection (const ACE_Asynch_Accept::Result& result_in,
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

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
int
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::accept (size_t bytesToRead_in,
                                                  const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::accept"));

  // *IMPORTANT NOTE*: with minor modifications, this code was copied from
  //                   Asynch_Acceptor.cpp

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
  if (allocator_)
    message_block_p =
      static_cast<ACE_Message_Block*> (allocator_->malloc (space_needed));
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

  // Initiate asynchronous accepts
  result =
    inherited::asynch_accept ().accept (*message_block_p,
                                        bytesToRead_in,
                                        ACE_INVALID_HANDLE,
                                        (act_in ? act_in
                                                : configuration_),
                                        0,
                                        COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL,
                                        //this->addr_family_,
                                        addressFamily_);
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

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
int
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::should_reissue_accept ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::should_reissue_accept"));

  // default behavior: delegate to baseclass
  return inherited::should_reissue_accept ();
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
bool
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::initialize (const Net_Server_ListenerConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::initialize"));

  addressFamily_ = configuration_in.addressFamily;
  allocator_ = configuration_in.allocator;
  configuration_ = configuration_in.socketHandlerConfiguration;
  interfaceHandle_ = configuration_in.connectionManager;
  listeningPort_ = configuration_in.portNumber;
  statisticCollectionInterval_ = configuration_in.statisticCollectionInterval;
  useLoopback_ = configuration_in.useLoopbackDevice;

  isInitialized_ = true;

  return true;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
bool
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::useReactor () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::useReactor"));

  return false;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
bool
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::isInitialized () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::isInitialized"));

  return isInitialized_;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
void
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::start"));

  // sanity check(s)
  if (isListening_)
    return; // nothing to do...

  // sanity check: configured ?
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, aborting\n")));
    return;
  } // end IF

  // not running --> start listening
  int result = -1;
  ACE_INET_Addr local_sap;
  if (useLoopback_)
    local_sap.set (listeningPort_, // port
                   // *PORTABILITY*: needed to disambiguate this under Windows :-(
                   // *TODO*: bind to specific interface/address ?
                   ACE_LOCALHOST,  // hostname
                   1,              // encode ?
                   AF_INET);       // address family
  else
    local_sap.set (listeningPort_,                       // port
                   // *PORTABILITY*: needed to disambiguate this under Windows :-(
                   // *TODO*: bind to specific interface/address ?
                   static_cast<ACE_UINT32> (INADDR_ANY), // hostname
                   1,                                    // encode ?
                   0);                                   // map ?
  result = inherited::open (local_sap,                  // local SAP
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
                listeningPort_));
    return;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("started listening (port: %u)...\n"),
              listeningPort_));

  // all is well...
  isListening_ = true;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
void
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::stop (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  if (!isListening_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("not listening --> nothing to do, returning\n")));
    return;
  } // end IF

  int result = -1;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  ACE_POSIX_Asynch_Accept* accept_p = NULL;
  try
  {
    accept_p =
        dynamic_cast<ACE_POSIX_Asynch_Accept*> (inherited::asynch_accept ().implementation ());
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<ACE_POSIX_Asynch_Accept*>(%@) failed, aborting\n"),
                inherited::asynch_accept ().implementation ()));

    accept_p = NULL;
  }
  if (!accept_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<ACE_POSIX_Asynch_Accept*>(%@) failed, aborting\n"),
                inherited::asynch_accept ().implementation ()));
    return;
  }
  result = accept_p->close ();
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
                ACE_TEXT ("stopped listening (port: %u)...\n"),
                listeningPort_));

  isListening_ = false;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
bool
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::isRunning"));

  return isListening_;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
void
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::dump_state"));

  // *TODO*: do something meaningful here...
  ACE_ASSERT (false);
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
HandlerType*
Net_Server_AsynchListener_T<ConfigurationType,
                            SocketHandlerConfigurationType,
                            UserDataType,
                            HandlerType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_AsynchListener_T::make_handler"));

  // initialize return value(s)
  HandlerType* connection_p = NULL;

  // default behavior
  ACE_NEW_NORETURN (connection_p,
                    HandlerType (interfaceHandle_,
                                 statisticCollectionInterval_));
  if (!connection_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return connection_p;
}
