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

#include "common_defines.h"

#include "net_common_tools.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::Net_Client_AsynchConnector_T (bool managed_in)
 : configuration_ (NULL)
 , handles_ ()
 , managed_ (managed_in)
 , SAP_ ()
 , condition_ (lock_)
 , lock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::Net_Client_AsynchConnector_T"));

  int result = -1;

  // initialize base class
  result =
    inherited::open (true,                      // pass addresses ?
                     ACE_Proactor::instance (), // default proactor
                     true);                     // validate new connections ?
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::open(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::~Net_Client_AsynchConnector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::~Net_Client_AsynchConnector_T"));

  //for (HANDLE_TO_ERROR_MAP_ITERATOR_T iterator = handles_.begin ();
  //     iterator != handles_.end ();
  //     ++iterator)
  //  abort ((*iterator).first);
  if (unlikely (!handles_.empty ()))
    abort ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_TransportLayerType
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::transportLayer () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::transportLayer"));

  enum Net_TransportLayerType result = NET_TRANSPORTLAYER_INVALID;

  // *TODO*: find a better way to do this
  HandlerType* handler_p = const_cast<OWN_TYPE_T*> (this)->make_handler ();
  if (unlikely (!handler_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::make_handler(): \"%m\", aborting\n")));
    return NET_TRANSPORTLAYER_INVALID;
  } // end IF
  ACE_ASSERT (handler_p);

  ITRANSPORTLAYER_T* itransportlayer_p = handler_p;
  result = itransportlayer_p->transportLayer ();

  // clean up
  delete handler_p;

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  int result = -1;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, ACE_INVALID_HANDLE);
    SAP_ = address_in;
  } // end lock scope

  result =
    connect (address_in,                         // remote address
             ACE_sap_any_cast (AddressType&),    // local address
             1,                                  // SO_REUSEADDR ?
             static_cast<ICONNECTOR_T*> (this)); // asynchronous completion token
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    return ACE_INVALID_HANDLE;
  } // end IF

  return reinterpret_cast<ACE_HANDLE> (result);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const AddressType& remoteAddress_in,
                                                     const AddressType& localAddress_in,
                                                     int reuseAddress_in,
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
  int protocol_family_i = remoteAddress_in.get_type ();
  ACE_HANDLE socket_h = ACE_OS::socket (protocol_family_i, // domain
                                        SOCK_STREAM,       // type
                                        0);                // protocol
  int one = 1;
  HANDLE_TO_ERROR_MAP_ITERATOR_T iterator;

  if (unlikely (socket_h == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(%d,%d,0): \"%m\", aborting\n"),
                remoteAddress_in.get_type (), SOCK_STREAM));
    return -1;
  } // end IF

  // set socket options
  if (likely (reuseAddress_in))
  { ACE_ASSERT (protocol_family_i != PF_UNIX);
    result = ACE_OS::setsockopt (socket_h,
                                 SOL_SOCKET,
                                 SO_REUSEADDR,
                                 reinterpret_cast<char*> (&one),
                                 sizeof (int));
    if (unlikely (result == -1))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(0x%@,SO_REUSEADDR): \"%m\", aborting\n"),
                  socket_h));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::setsockopt(%d,SO_REUSEADDR): \"%m\", aborting\n"),
                  socket_h));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
  // enable SIO_LOOPBACK_FAST_PATH on Win32 ?
  if ((protocol_family_i == ACE_ADDRESS_FAMILY_INET) &&
      remoteAddress_in.is_loopback ()                &&
      NET_INTERFACE_ENABLE_LOOPBACK_FASTPATH)
    if (unlikely (!Net_Common_Tools::setLoopBackFastPath (socket_h)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLoopBackFastPath(0x%@): \"%m\", aborting\n"),
                  socket_h));
      goto error;
    } // end IF
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
#endif // ACE_WIN32 || ACE_WIN64

  ///////////////////////////////////////

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, -1);
    handles_.insert (std::make_pair (socket_h, -1));
  } // end lock scope
  result = connect_r.connect (socket_h,
                              remoteAddress_in,
                              localAddress_in,
                              reuseAddress_in,
                              act_in,
                              0,
                              COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connect_Impl::connect(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (remoteAddress_in).c_str ())));
    goto error;
  } // end IF

  return reinterpret_cast<int> (socket_h);

error:
  result = ACE_OS::closesocket (socket_h);
  if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                socket_h));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                socket_h));
#endif // ACE_WIN32 || ACE_WIN64

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, -1);
    iterator = handles_.find (socket_h);
    if (iterator != handles_.end ())
      handles_.erase (iterator);
  } // end lock scope

  return -1;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::abort"));

  int result = -1;

  result = inherited::cancel ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::cancel(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::wait (ACE_HANDLE handle_in,
                                                  const ACE_Time_Value& relativeTimeout_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::wait"));

  int result = -1;
  ACE_Time_Value absolute_timeout = ACE_Time_Value::zero;
  ACE_Time_Value* timeout_p =
    ((relativeTimeout_in != ACE_Time_Value::zero) ? &absolute_timeout : NULL);
  HANDLE_TO_ERROR_MAP_ITERATOR_T iterator;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, -1);
    iterator = handles_.find (handle_in);
    if (iterator == handles_.end ()) // --> nothing to do
      return 0; // *TODO*: race condition; result could be a false positive

    if (relativeTimeout_in != ACE_Time_Value::zero)
      absolute_timeout = COMMON_TIME_NOW + relativeTimeout_in;
    result = condition_.wait (timeout_p);
    if (unlikely (result == -1))
    {
      int error = ACE_OS::last_error ();
      if (error != ETIME)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition::wait(): \"%m\", returning\n")));
      else
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to ACE_Condition::wait(): \"%m\", returning\n")));
        result = ETIME;
      } // end ELSE
      goto continue_;
    } // end IF

    result = (*iterator).second;
  } // end lock scope

continue_:
  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::onConnect (ACE_HANDLE connectHandle_in,
                                                       int error_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::onConnect"));

  //if (error_in != ECONNREFUSED) // happens intermittently on Win32
  if (unlikely (error_in))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("0x%@: failed to Net_Client_AsynchConnector_T::connect(%s): \"%s\", aborting\n"),
                connectHandle_in,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (SAP_).c_str ()),
                ACE::sock_error (static_cast<int> (error_in))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%d: failed to Net_Client_AsynchConnector_T::connect(%s): \"%s\", aborting\n"),
                connectHandle_in,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (SAP_).c_str ()),
                ACE_TEXT (ACE_OS::strerror (error_in))));
#endif // ACE_WIN32 || ACE_WIN64
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                                 const AddressType& remoteSAP_in,
                                                                 const AddressType& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::validate_connection"));

  ACE_UNUSED_ARG (remoteSAP_in);
  ACE_UNUSED_ARG (localSAP_in);

  // *NOTE*: on error addresses are not passed in
  // *TODO*: on error addresses are not passed in

  int result = -1;
  ICONNECTOR_T* iconnector_p =
    const_cast<ICONNECTOR_T*> (reinterpret_cast<const ICONNECTOR_T*> (result_in.act ()));
  ACE_ASSERT (iconnector_p);
  HANDLE_TO_ERROR_MAP_ITERATOR_T iterator;

  // prevent race condition in wait()
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, -1);
    iterator = handles_.find (result_in.connect_handle ());
    ACE_ASSERT (iterator != handles_.end ());
    (*iterator).second =
      ((result_in.success () == 0) ? static_cast<int> (result_in.error ()) : 0);
  } // end lock scope
  // signal completion
  result = condition_.broadcast ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));

  result =
    ((result_in.success () == 0) ? static_cast<int> (result_in.error ()) : 0);
  try {
    iconnector_p->onConnect (result_in.connect_handle (),
                             result);
  } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IAsynchConnector_T::onConnect(0x%@,%d), continuing\n"),
                result_in.connect_handle (),
                result));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IAsynchConnector_T::onConnect(%d,%d), continuing\n"),
                result_in.connect_handle (),
                result));
#endif // ACE_WIN32 || ACE_WIN64
  }

  return ((result_in.success () == 0) ? -1 : 0);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::handle_connect (const ACE_Asynch_Connect::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::handle_connect"));

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Asynch_Connector.cpp

  // Variable for error tracking
  int error = 0;

  // If the asynchronous connect fails.
  if (unlikely (!result_in.success () ||
                result_in.connect_handle () == ACE_INVALID_HANDLE))
  {
    error = 1;
  }

  if (unlikely (result_in.error () != 0))
  {
    error = 1;
  }

  // set blocking mode
  if (unlikely (!error &&
                (ACE::clr_flags (result_in.connect_handle (),
                                 ACE_NONBLOCK) != 0)))
  {
    error = 1;
    ACELIB_ERROR ((LM_ERROR,
                   ACE_TEXT ("%p\n"),
                   ACE_TEXT ("ACE_Asynch_Connector::handle_connect : Set blocking mode")));
  }

  // Parse the addresses.
  AddressType local_address;
  AddressType remote_address;
  if (likely (!error &&
              (this->validate_new_connection () ||
               this->pass_addresses ())))
    this->parse_address (result_in,
                         remote_address,
                         local_address);

  // Call validate_connection even if there was an error - it's the only
  // way the application can learn the connect disposition.
  if (unlikely (this->validate_new_connection () &&
                this->validate_connection (result_in, remote_address, local_address) == -1))
  {
    error = 1;
  }

  HandlerType *new_handler = 0;
  if (likely (!error))
  {
    // The Template method
    new_handler = this->make_handler ();
    if (unlikely (new_handler == 0))
    {
      error = 1;
      ACELIB_ERROR ((LM_ERROR,
        ACE_TEXT ("%p\n"),
        ACE_TEXT ("ACE_Asynch_Connector::handle_connect : Making of new handler failed")));
    }
  }

  // If no errors
  if (likely (!error))
  {
    // Update the Proactor.
    new_handler->proactor (this->proactor ());

    // Pass the addresses
    if (likely (this->pass_addresses ()))
      new_handler->addresses (remote_address,
                              local_address);

    // *EDIT*: set role
    new_handler->set (NET_ROLE_CLIENT);

    // Pass the ACT
    if (likely (result_in.act () != 0))
      new_handler->act (result_in.act ());

    // Set up the handler's new handle value
    new_handler->handle (result_in.connect_handle ());

    ACE_Message_Block  mb;

    // Initiate the handler with empty message block;
    new_handler->open (result_in.connect_handle (), mb);

    new_handler->decrease ();
  }

  // On failure, no choice but to close the socket
  if (unlikely (error &&
                (result_in.connect_handle () != ACE_INVALID_HANDLE)))
    ACE_OS::closesocket (result_in.connect_handle ());
}
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
HandlerType*
Net_Client_AsynchConnector_T<HandlerType,
                             AddressType,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             SocketConfigurationType,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::make_handler"));

  // initialize return value(s)
  HandlerType* handler_p = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_p,
                    HandlerType (managed_));
  if (unlikely (!handler_p))
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
Net_Client_AsynchConnector_T<HandlerType,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             struct Net_UDPSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::Net_Client_AsynchConnector_T (bool managed_in)
 : configuration_ (NULL)
 , managed_ (managed_in)
 , SAP_ ()
 , condition_ (lock_)
 , lock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::Net_Client_AsynchConnector_T"));

  int result = -1;

  // initialize base class
  result =
    inherited::open (true,                      // pass addresses ?
                     ACE_Proactor::instance (), // default proactor
                     true);                     // validate new connections ?
  if (unlikely (result == -1))
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
int
Net_Client_AsynchConnector_T<HandlerType,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             struct Net_UDPSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                                 const ACE_INET_Addr& remoteSAP_in,
                                                                 const ACE_INET_Addr& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::validate_connection"));

  // *NOTE*: on error, the addresses are not passed through
  // *TODO*: in case of errors, addresses are not supplied

  ACE_UNUSED_ARG (localSAP_in);

  int result = -1;
  unsigned long error = 0;

  // success ?
  result = result_in.success ();
  if (unlikely (result != 1))
  {
    error = result_in.error ();

    ACE_INET_Addr peer_address = remoteSAP_in;
    if (error)
    { // sanity check(s)
      ACE_ASSERT (remoteSAP_in.is_any ());
      peer_address = SAP_;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ()),
                ACE::sock_error (static_cast<int> (error))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ()),
                ACE_TEXT (ACE_OS::strerror (error))));
#endif // ACE_WIN32 || ACE_WIN64
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
Net_Client_AsynchConnector_T<HandlerType,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             struct Net_UDPSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, ACE_INVALID_HANDLE);
    SAP_ = address_in;
  } // end lock scope

  CONNECTION_T* handler_p = NULL;
  handler_p = make_handler ();
  if (unlikely (!handler_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T<Net_AsynchUDPConnection_T>::make_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  // pre-initialize the connection handler
  handler_p->set (Net_Common_Tools::isLocal (address_in) ? NET_ROLE_CLIENT
                                                         : NET_ROLE_SERVER);

  ICONNECTOR_T* iconnector_p = this;
  const void* act_p = iconnector_p;
  handler_p->act (act_p);

  // *NOTE*: the handler registers with the proactor and will be freed (or
  //         recycled) automatically
  ACE_Message_Block message_block;
  handler_p->open (ACE_INVALID_HANDLE,
                   message_block);

  return handler_p->handle ();
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
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             struct Net_UDPSocketConfiguration,
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
                    CONNECTION_T (managed_));
  if (unlikely (!handler_p))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return handler_p;
}

/////////////////////////////////////////

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
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
                             struct Net_NetlinkSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::Net_Client_AsynchConnector_T (bool managed_in)
 : configuration_ (NULL)
 , managed_ (managed_in)
 , SAP_ ()
 , condition_ (lock_)
 , lock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::Net_Client_AsynchConnector_T"));

  int result = -1;

  // initialize base class
  result =
    inherited::open (true,                      // pass addresses ?
                     ACE_Proactor::instance (), // default proactor
                     true);                     // validate new connections ?
  if (unlikely (result == -1))
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
int
Net_Client_AsynchConnector_T<HandlerType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             struct Net_NetlinkSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                                 const Net_Netlink_Addr& remoteSAP_in,
                                                                 const Net_Netlink_Addr& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::validate_connection"));

  // *NOTE*: on error addresses are not passed through

  int result = -1;
  unsigned long error = 0;

  // success ?
  result = result_in.success ();
  if (unlikely (result != 1))
  {
    error = result_in.error ();

    Net_Netlink_Addr peer_address = remoteSAP_in;
    if (error)
    { // sanity check(s)
      ACE_ASSERT (remoteSAP_in.is_any ());
      peer_address = SAP_;
    } // end IF

    // *TODO*: in case of errors, addresses are not supplied
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(%s): \"%s\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::NetlinkAddressToString (peer_address).c_str ()),
                ACE_TEXT (ACE_OS::strerror (error))));
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
                             struct Net_NetlinkSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::connect (const Net_Netlink_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, ACE_INVALID_HANDLE);
    SAP_ = address_in;
  } // end lock scope

  HandlerType* handler_p = make_handler ();
  if (unlikely (!handler_p))
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
                             struct Net_NetlinkSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::make_handler"));

  // initialize return value(s)
  HandlerType* handler_p = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_p,
                    HandlerType (managed_));
  if (unlikely (!handler_p))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return handler_p;
}
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
