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

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       HandlerConfigurationType,
                       StreamType,
                       UserDataType>::Net_Client_Connector_T (bool managed_in)
 : inherited (ACE_Reactor::instance (), // default reactor
              // *IMPORTANT NOTE*: ACE_NONBLOCK is only set if timeout != NULL
              //                   (see: SOCK_Connector.cpp:94), set via the
              //                   "synch options" (see line 200 below)
              ACE_NONBLOCK)             // flags: non-blocking I/O
              //0)                       // flags
 , configuration_ (NULL)
 , managed_ (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       HandlerConfigurationType,
                       StreamType,
                       UserDataType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

  int result = inherited::close ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::close(): \"%m\", continuing\n")));
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_TransportLayerType
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       HandlerConfigurationType,
                       StreamType,
                       UserDataType>::transportLayer () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::transportLayer"));

  enum Net_TransportLayerType result = NET_TRANSPORTLAYER_INVALID;

  // *TODO*: find a better way to do this
  HandlerType* handler_p = NULL;
  int result_2 = const_cast<OWN_TYPE_T*> (this)->make_svc_handler (handler_p);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T<Net_UDPConnection_T>::make_svc_handler(): \"%m\", aborting\n")));
    return NET_TRANSPORTLAYER_INVALID;
  } // end IF
  ACE_ASSERT (handler_p);

  result = handler_p->transportLayer ();

  delete handler_p; handler_p = NULL;

  return result;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       HandlerConfigurationType,
                       StreamType,
                       UserDataType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  HandlerType* handler_p = NULL;
//  synch_options.set (ACE_Synch_Options::USE_REACTOR, ACE_Time_Value::zero);
  // *NOTE*: to enforce ACE_NONBLOCK, set ACE_Synch_Options::USE_REACTOR or
  //         ACE_Synch_Options::USE_TIMEOUT in the synch options (see:
  //         Connector.cpp:409 and net_sock_connector.cpp:219 and/or
  //         SOCK_Connector.cpp:94)
  result =
      inherited::connect (handler_p,      // service handler
                          address_in,                      // remote SAP
                          ACE_Synch_Options::defaults,     // synch options
                          ACE_sap_any_cast (AddressType&), // local SAP
                          1,                               // set SO_REUSEADDR ?
                          O_RDWR,                          // flags
                          0);                              // permissions
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (error != ETIME) // 62: local close
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Connector::connect(%s): \"%m\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  return handler_p->get_handle ();
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       HandlerConfigurationType,
                       StreamType,
                       UserDataType>::activate_svc_handler (HandlerType* handler_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::activate_svc_handler"));

  // pre-initialize the connection handler
  handler_in->set (NET_ROLE_CLIENT);

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Connector.cpp:251

  // No errors initially
  int error = 0;

  // See if we should enable non-blocking I/O on the <svc_handler>'s
  // peer.
  //if (ACE_BIT_ENABLED (this->flags_, ACE_NONBLOCK) != 0)
  //{
    if (handler_in->peer ().enable (ACE_NONBLOCK) == -1)
      error = 1;
  //}
  //// Otherwise, make sure it's disabled by default.
  //else if (svc_handler->peer ().disable (ACE_NONBLOCK) == -1)
  //  error = 1;

  // We are connected now, so try to open things up.
  ICONNECTOR_T* iconnector_p = this;
  if (unlikely (error || handler_in->open (iconnector_p) == -1))
  {
    // Make sure to close down the <svc_handler> to avoid descriptor
    // leaks.
    // The connection was already made; so this close is a "normal"
    // close operation.
    handler_in->close (NORMAL_CLOSE_OPERATION);
    return -1;
  }
  else
    return 0;
}
template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       HandlerConfigurationType,
                       StreamType,
                       UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (managed_));
  if (unlikely (!handler_out))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}

/////////////////////////////////////////
// specialization (for UDP)
template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_UDPSocketConfiguration_t,
                       StreamType,
                       UserDataType>::Net_Client_Connector_T (bool managed_in)
 : configuration_ (NULL)
 , managed_ (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_UDPSocketConfiguration_t,
                       StreamType,
                       UserDataType>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  CONNECTION_T* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T<Net_UDPConnection_T>::make_svc_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  // pre-initialize the connection handler
  Net_ILinkLayer_T<Net_UDPSocketConfiguration_t>* ilinklayer_p = handler_p;
  ilinklayer_p->set (Net_Common_Tools::isLocal (address_in) ? NET_ROLE_SERVER
                                                            : NET_ROLE_CLIENT);

  result = activate_svc_handler (handler_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T::activate_svc_handler(), (address was: %s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    return ACE_INVALID_HANDLE;
  } // end IF

  return handler_p->get_handle ();
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_UDPSocketConfiguration_t,
                       StreamType,
                       UserDataType>::initialize_svc_handler (ACE_HANDLE handle_in,
                                                              HandlerType* handler_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::initialize_svc_handler"));

  // Transfer ownership of the ACE_HANDLE to the SVC_HANDLER
  handler_in->set_handle (handle_in);

  this->activate_svc_handler (handler_in);
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_UDPSocketConfiguration_t,
                       StreamType,
                       UserDataType>::activate_svc_handler (HandlerType* handler_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::activate_svc_handler"));

  int result = -1;
  ICONNECTOR_T* iconnector_p = this;

  result = handler_in->open (iconnector_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Svc_Handler::open(0x%@): \"%m\", aborting\n"),
                iconnector_p));
    int result_2 = handler_in->close (CLOSE_DURING_NEW_CONNECTION);
    ACE_UNUSED_ARG (result_2);
    return -1;
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_UDPSocketConfiguration_t,
                       StreamType,
                       UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    CONNECTION_T (managed_));
  if (unlikely (!handler_out))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_UDPSocketConfiguration_t,
                       StreamType,
                       UserDataType>::connect_svc_handler (CONNECTION_T*& handler_inout,
                                                           const ACE_SOCK_Connector::PEER_ADDR& remoteAddress_in,
                                                           ACE_Time_Value* timeout_in,
                                                           const ACE_SOCK_Connector::PEER_ADDR& localAddress_in,
                                                           int reuseAddress_in,
                                                           int flags_in,
                                                           int permissions_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect_svc_handler"));

  ACE_UNUSED_ARG (remoteAddress_in);
  ACE_UNUSED_ARG (timeout_in);
  ACE_UNUSED_ARG (localAddress_in);
  ACE_UNUSED_ARG (reuseAddress_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (permissions_in);

  // sanity check(s)
  ACE_ASSERT (handler_inout);

  ICONNECTOR_T* iconnector_p = this;
  int result = handler_inout->open (iconnector_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Svc_Handler::open(0x%@): \"%m\", aborting\n"),
                iconnector_p));
    return -1;
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_UDPSocketConfiguration_t,
                       StreamType,
                       UserDataType>::connect_svc_handler (CONNECTION_T*& handler_inout,
                                                           CONNECTION_T*& handlerCopy_inout,
                                                           const ACE_SOCK_Connector::PEER_ADDR& remoteAddress_in,
                                                           ACE_Time_Value* timeout_in,
                                                           const ACE_SOCK_Connector::PEER_ADDR& localAddress_in,
                                                           int reuseAddress_in,
                                                           int flags_in,
                                                           int permissions_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect_svc_handler"));

  ACE_UNUSED_ARG (handlerCopy_inout);
  ACE_UNUSED_ARG (remoteAddress_in);
  ACE_UNUSED_ARG (timeout_in);
  ACE_UNUSED_ARG (localAddress_in);
  ACE_UNUSED_ARG (reuseAddress_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (permissions_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1);
}

#if defined (NETLINK_SUPPORT)
/////////////////////////////////////////
// specialization (for Netlink)
template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       Net_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_NetlinkSocketConfiguration_t,
                       StreamType,
                       UserDataType>::Net_Client_Connector_T (bool managed_in)
 : configuration_ (NULL)
 , managed_ (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       Net_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_NetlinkSocketConfiguration_t,
                       StreamType,
                       UserDataType>::connect (const Net_Netlink_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  // *TODO*: implement TCP version
  int result = -1;

  HandlerType* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T::make_svc_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  ICONNECTOR_T* iconnector_p = this;
  result = handler_p->open (iconnector_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::open(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::NetlinkAddressToString (address_in).c_str ())));
    return ACE_INVALID_HANDLE;
  } // end IF

  return handler_p->get_handle ();
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_Connector_T<ACE_SYNCH_USE,
                       HandlerType,
                       ConnectorType,
                       Net_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StatisticContainerType,
                       Net_NetlinkSocketConfiguration_t,
                       StreamType,
                       UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (managed_));
  if (unlikely (!handler_out))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}
#endif // NETLINK_SUPPORT
