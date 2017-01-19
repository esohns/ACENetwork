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

#include <ace/Log_Msg.h>

#include "common_defines.h"

#include "net_common_tools.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::Net_Client_AsynchConnector_T (ICONNECTION_MANAGER_T* connectionManager_in,
                                                                          const ACE_Time_Value& statisticCollectionInterval_in)
 : configuration_ (NULL)
 , connectHandle_ (ACE_INVALID_HANDLE)
 , connectionManager_ (connectionManager_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::Net_Client_AsynchConnector_T"));

  int result = -1;

  if (!connectionManager_)
    connectionManager_ = CONNECTION_MANAGER_T::SINGLETON_T::instance ();

  // initialize base class
  result = inherited::open (true,  // pass addresses ?
                            NULL,  // default proactor
                            true); // validate new connections ?
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::open(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::~Net_Client_AsynchConnector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::~Net_Client_AsynchConnector_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  int result = -1;

  ICONNECTOR_T* iconnector_p = this;
  const void* act_p = iconnector_p;
  connectHandle_ = ACE_INVALID_HANDLE;
  result =
      connect (address_in,                            // remote address
               ACE_sap_any_cast (const AddressType&), // local address
               1,                                     // SO_REUSEADDR ?
               act_p);                                // asynchronous completion token
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = address_in.addr_to_string (buffer,
                                        sizeof (buffer),
                                        1);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (connectHandle_ != ACE_INVALID_HANDLE);

  //ACE_HANDLE return_value = ACE_INVALID_HANDLE;
  //ACE_Asynch_Connect& asynch_connect_r =
  //    inherited::asynch_connect ();
  //ACE_Asynch_Operation_Impl* asynch_operation_impl_p =
  //    asynch_connect_r.implementation ();
  //ACE_ASSERT (asynch_operation_impl_p);
  //ACE_UNUSED_ARG (return_value);

  return connectHandle_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::abort"));

  int result = -1;

  result = inherited::cancel ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::cancel(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const ACE_INET_Addr& remoteAddress_in,
                                                     const ACE_INET_Addr& localAddress_in,
                                                     int reuseAddr_in,
                                                     const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  int result = -1;
  ACE_Asynch_Connect& connect_r = inherited::asynch_connect ();
  //ACE_Asynch_Connect_Impl* connect_p =
  //  dynamic_cast<ACE_Asynch_Connect_Impl*> (connect_r.implementation ());
  //ACE_ASSERT (connect_p);

  // *NOTE*: some socket options need to be set before connect()ing
  //         --> set these here (this implements the shared_connect_start()
  //             method of ACE_SOCK_Connector (see: SOCK_Connector.h:301) for
  //             synchronous connects
  int protocol_family = remoteAddress_in.get_type ();
  connectHandle_ = ACE_OS::socket (protocol_family,
                                   SOCK_STREAM, // TCP
                                   0);
  if (connectHandle_ == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(%d,%d,0): \"%m\", aborting\n"),
                protocol_family, SOCK_STREAM));
    return -1;
  } // end IF

  // Reuse the address
  int one = 1;
  if (reuseAddr_in &&
      (protocol_family != PF_UNIX))
  {
    result = ACE_OS::setsockopt (connectHandle_,
                                 SOL_SOCKET,
                                 SO_REUSEADDR,
                                 reinterpret_cast<const char*> (&one),
                                 sizeof (one));
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                  connectHandle_));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_REUSEADDR): \"%m\", aborting\n"),
                  connectHandle_));
#endif
      goto close;
    } // end IF
  } // end IF

  ///////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // enable SIO_LOOPBACK_FAST_PATH on Win32
  if ((remoteAddress_in.get_type () == ACE_ADDRESS_FAMILY_INET) &&
      remoteAddress_in.is_loopback ()                           &&
      NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH)
    if (!Net_Common_Tools::setLoopBackFastPath (connectHandle_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLoopBackFastPath(0x%@): \"%m\", aborting\n"),
                  connectHandle_));
      goto close;
    } // end IF
#endif

  result = connect_r.connect (connectHandle_,
                              remoteAddress_in,
                              localAddress_in,
                              reuseAddr_in,
                              act_in,
                              0,
                              COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = remoteAddress_in.addr_to_string (buffer,
                                              sizeof (buffer),
                                              1);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connect_Impl::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
    goto close;
  } // end IF

  return 0;

close:
  result = ACE_OS::closesocket (connectHandle_);
  if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
                reinterpret_cast<size_t> (connectHandle_)));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                connectHandle_));
#endif

  return -1;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                                 const ACE_INET_Addr& remoteAddress_in,
                                                                 const ACE_INET_Addr& localAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::validate_connection"));

  ACE_UNUSED_ARG (localAddress_in);

  // *NOTE*: on error, the addresses are not passed through...

  int result = -1;
  unsigned long error = 0;

  // success ?
  result = result_in.success ();
  if (result != 1)
  {
    error = result_in.error ();

    // *TODO*: in case of errors, addresses are not supplied
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    int result_2 = remoteAddress_in.addr_to_string (buffer,
                                                    sizeof (buffer),
                                                    1);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    //if (error != ECONNREFUSED) // happens intermittently on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(\"%s\"): \"%s\", aborting\n"),
                buffer,
                ACE::sock_error (static_cast<int> (error))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(\"%s\"): \"%s\", aborting\n"),
                buffer,
                ACE_TEXT (ACE_OS::strerror (error))));
#endif
  } // end IF

  return ((result == 0) ? -1 : 0);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::handle_connect (const ACE_Asynch_Connect::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::handle_connect"));

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Asynch_Connector.cpp

  // Variable for error tracking
  int error = 0;

  // If the asynchronous connect fails.
  if (!result_in.success () ||
      result_in.connect_handle () == ACE_INVALID_HANDLE)
  {
    error = 1;
  }

  if (result_in.error () != 0)
  {
    error = 1;
  }

  // set blocking mode
  if (!error &&
      ACE::clr_flags
      (result_in.connect_handle (), ACE_NONBLOCK) != 0)
  {
    error = 1;
    ACELIB_ERROR ((LM_ERROR,
      ACE_TEXT ("%p\n"),
      ACE_TEXT ("ACE_Asynch_Connector::handle_connect : Set blocking mode")));
  }

  // Parse the addresses.
  ACE_INET_Addr local_address;
  ACE_INET_Addr remote_address;
  if (!error &&
      (this->validate_new_connection () || this->pass_addresses ()))
      this->parse_address (result_in,
      remote_address,
      local_address);

  // Call validate_connection even if there was an error - it's the only
  // way the application can learn the connect disposition.
  if (this->validate_new_connection () &&
      this->validate_connection (result_in, remote_address, local_address) == -1)
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
      ACELIB_ERROR ((LM_ERROR,
        ACE_TEXT ("%p\n"),
        ACE_TEXT ("ACE_Asynch_Connector::handle_connect : Making of new handler failed")));
    }
  }

  // If no errors
  if (!error)
  {
    // Update the Proactor.
    new_handler->proactor (this->proactor ());

    // Pass the addresses
    if (this->pass_addresses ())
      new_handler->addresses (remote_address,
      local_address);

    // *EDIT*: set role
    new_handler->set (NET_ROLE_CLIENT);

    // Pass the ACT
    if (result_in.act () != 0)
      new_handler->act (result_in.act ());

    // Set up the handler's new handle value
    new_handler->handle (result_in.connect_handle ());

    ACE_Message_Block  mb;

    // Initiate the handler with empty message block;
    new_handler->open (result_in.connect_handle (), mb);
  }

  // On failure, no choice but to close the socket
  if (error &&
      result_in.connect_handle () != ACE_INVALID_HANDLE)
      ACE_OS::closesocket (result_in.connect_handle ());
}
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
HandlerType*
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::make_handler"));

  // initialize return value(s)
  HandlerType* handler_p = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_p,
                    HandlerType (connectionManager_,
                                 statisticCollectionInterval_));
  if (!handler_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return handler_p;
}

//////////////////////////////////////////

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<Net_AsynchUDPConnectionBase_T<HandlerType,
                                                           ConfigurationType,
                                                           StateType,
                                                           StatisticContainerType,
                                                           HandlerConfigurationType,
                                                           StreamType,
                                                           UserDataType>,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::Net_Client_AsynchConnector_T (ICONNECTION_MANAGER_T* connectionManager_in,
                                                                          const ACE_Time_Value& statisticCollectionInterval_in)
 : configuration_ (NULL)
 , connectionManager_ (connectionManager_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::Net_Client_AsynchConnector_T"));

  int result = -1;

  if (!connectionManager_)
    connectionManager_ = CONNECTION_MANAGER_T::SINGLETON_T::instance ();

  // initialize base class
  result = inherited::open (true,  // pass addresses ?
                            NULL,  // default proactor
                            true); // validate new connections ?
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::open(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<Net_AsynchUDPConnectionBase_T<HandlerType,
                                                           ConfigurationType,
                                                           StateType,
                                                           StatisticContainerType,
                                                           HandlerConfigurationType,
                                                           StreamType,
                                                           UserDataType>,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::~Net_Client_AsynchConnector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::~Net_Client_AsynchConnector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_AsynchConnector_T<Net_AsynchUDPConnectionBase_T<HandlerType,
                                                           ConfigurationType,
                                                           StateType,
                                                           StatisticContainerType,
                                                           HandlerConfigurationType,
                                                           StreamType,
                                                           UserDataType>,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::abort"));

  int result = -1;

  result = inherited::cancel ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::cancel(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_AsynchConnector_T<Net_AsynchUDPConnectionBase_T<HandlerType,
                                                           ConfigurationType,
                                                           StateType,
                                                           StatisticContainerType,
                                                           HandlerConfigurationType,
                                                           StreamType,
                                                           UserDataType>,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                                 const ACE_INET_Addr& remoteSAP_in,
                                                                 const ACE_INET_Addr& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::validate_connection"));

  // *NOTE*: on error, the addresses are not passed through...

  ACE_UNUSED_ARG (localSAP_in);

  int result = -1;
  unsigned long error = 0;

  // success ?
  result = result_in.success ();
  if (result != 1)
  {
    error = result_in.error ();

    ACE_INET_Addr peer_address = remoteSAP_in;
    // *NOTE*: in case of errors, addresses are not supplied
    if (error)
    { // sanity check(s)
      ACE_ASSERT (remoteSAP_in.is_any ());
      ACE_ASSERT (configuration_);
      ACE_ASSERT (configuration_->socketConfiguration);
      peer_address = configuration_->socketConfiguration->address;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ()),
                ACE::sock_error (static_cast<int> (error))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ()),
                ACE_TEXT (ACE_OS::strerror (error))));
#endif
  } // end IF

  return ((result == 1) ? 0 : -1);
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_AsynchConnector_T<Net_AsynchUDPConnectionBase_T<HandlerType,
                                                           ConfigurationType,
                                                           StateType,
                                                           StatisticContainerType,
                                                           HandlerConfigurationType,
                                                           StreamType,
                                                           UserDataType>,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  ACE_UNUSED_ARG (address_in);

  CONNECTION_T* handler_p = NULL;
  handler_p = make_handler ();
  if (!handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T<Net_AsynchUDPConnection_T>::make_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  // pre-initialize the connection handler
  // *TODO*: remove type inference
  handler_p->set (NET_ROLE_CLIENT);

  ICONNECTOR_T* iconnector_p = this;
  const void* act_p = iconnector_p;
  handler_p->act (act_p);

  // *NOTE*: the handler registers with the reactor and will be freed (or
  //         recycled) automatically
  ACE_Message_Block message_block;
  handler_p->open (ACE_INVALID_HANDLE,
                   message_block);

  return 0;
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              HandlerConfigurationType,
                              StreamType,
                              UserDataType>*
Net_Client_AsynchConnector_T<Net_AsynchUDPConnectionBase_T<HandlerType,
                                                           ConfigurationType,
                                                           StateType,
                                                           StatisticContainerType,
                                                           HandlerConfigurationType,
                                                           StreamType,
                                                           UserDataType>,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::make_handler"));

  // initialize return value(s)
  CONNECTION_T* handler_p = NULL;

  // default behavior
  // *TODO*: remove type inference
  ACE_NEW_NORETURN (handler_p,
                    CONNECTION_T (connectionManager_,
                                  statisticCollectionInterval_));
  if (!handler_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return handler_p;
}

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<HandlerType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::Net_Client_AsynchConnector_T (ICONNECTION_MANAGER_T* connectionManager_in,
                                                                          const ACE_Time_Value& statisticCollectionInterval_in)
 : configuration_ (NULL)
 , connectionManager_ (connectionManager_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::Net_Client_AsynchConnector_T"));

  int result = -1;

  if (!connectionManager_)
    connectionManager_ = CONNECTION_MANAGER_T::SINGLETON_T::instance ();

  // initialize base class
  result = inherited::open (true,  // pass addresses ?
                            NULL,  // default proactor
                            true); // validate new connections ?
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::open(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<HandlerType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::~Net_Client_AsynchConnector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::~Net_Client_AsynchConnector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_AsynchConnector_T<HandlerType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::abort"));

  int result = -1;

  result = inherited::cancel ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::cancel(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_AsynchConnector_T<HandlerType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                                 const Net_Netlink_Addr& remoteSAP_in,
                                                                 const Net_Netlink_Addr& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::validate_connection"));

  // *NOTE*: on error, the addresses are not passed through...

  int result = -1;
  unsigned long error = 0;

  // success ?
  result = result_in.success ();
  if (result != 1)
  {
    error = result_in.error ();

    // *TODO*: in case of errors, addresses are not supplied
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    // *TODO*: find a replacement for ACE_INET_Addr::addr_to_string
    //  int result_2 = remoteSAP_in.addr_to_string (buffer, sizeof (buffer));
    //    if (result_2 == -1)
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(\"%s\"): \"%s\", aborting\n"),
                buffer,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                ACE::sock_error (error)));
#else
                ACE_TEXT (ACE_OS::strerror (error))));
#endif
  } // end IF

  return ((result == 0) ? -1 : 0);
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_AsynchConnector_T<HandlerType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const Net_Netlink_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  HandlerType* handler_p = NULL;
  handler_p = make_handler ();
  if (!handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::make_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  // pre-initialize the connection handler
  // *TODO*: remove type inference
  handler_p->set (NET_ROLE_CLIENT);

  ICONNECTOR_T* iconnector_p = this;
  const void* act_p = iconnector_p;
  handler_p->act (act_p);

  // *NOTE*: the handler registers with the reactor and will be freed (or
  //         recycled) automatically
  ACE_Message_Block message_block;
  handler_p->open (ACE_INVALID_HANDLE,
                   message_block);

  return 0;
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
HandlerType*
Net_Client_AsynchConnector_T<HandlerType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::make_handler"));

  // initialize return value(s)
  HandlerType* handler_p = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_p,
                    HandlerType (connectionManager_,
                                 statisticCollectionInterval_));
  if (!handler_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return handler_p;
}

#endif
