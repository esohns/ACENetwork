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

#include "net_common.h"
#include "net_macros.h"

template <typename AddressType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename ConnectionType>
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       ConnectionType>::Net_Client_Connector_T (const ConfigurationType* configuration_in,
                                                                ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                unsigned int statisticCollectionInterval_in)
 : inherited (ACE_Reactor::instance (), // default reactor
              ACE_NONBLOCK)             // flags: non-blocking I/O
              //0)                       // flags
 , configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename ConnectionType>
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       ConnectionType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename ConnectionType>
int
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       ConnectionType>::make_svc_handler (ConnectionType*& handler_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // init return value(s)
  handler_inout = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_inout,
                    ConnectionType (interfaceHandle_,
                                    statisticCollectionInterval_));
  if (!handler_inout)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return ((handler_inout == NULL) ? -1 : 0);
}

template <typename AddressType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename ConnectionType>
void
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       ConnectionType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

  if (inherited::close () == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::close(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename ConnectionType>
bool
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       ConnectionType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  ConnectionType* handler_p = NULL;
  int result = -1;

  result =
      inherited::connect (handler_p,                       // service handler
                          address_in,                      // remote SAP
                          ACE_Synch_Options::defaults,     // synch options
                          ACE_sap_any_cast (AddressType&), // local SAP
                          1,                               // re-use address (SO_REUSEADDR) ?
                          O_RDWR,                          // flags
                          0);                              // perms
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    // *TODO*: find a replacement for ACE_Netlink_Addr::addr_to_string
//    result = address_in.addr_to_string (buffer, sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
    return false;
  } // end IF

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename ConnectionType>
const SocketHandlerConfigurationType*
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       ConnectionType>::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
//  return configuration_;
}

/////////////////////////////////////////

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType>
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::Net_Client_Connector_T (const ConfigurationType* configuration_in,
                                                                                   ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                   unsigned int statisticCollectionInterval_in)
 : configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType>
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType>
int
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::make_svc_handler (CONNECTION_T*& handler_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // init return value(s)
  handler_inout = NULL;

  ACE_NEW_NORETURN (handler_inout,
                    CONNECTION_T (interfaceHandle_,
                                  statisticCollectionInterval_));
  if (!handler_inout)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return ((handler_inout == NULL) ? -1 : 0);
}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType>
void
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType>
bool
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  CONNECTION_T* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T<Net_UDPConnection>::make_svc_handler(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (handler_p);

  // *NOTE*: the handler registers with the reactor and will be freed (or
  //         recycled) automatically
  result =
      handler_p->open (const_cast<ConfigurationType*> (configuration_));
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = address_in.addr_to_string (buffer, sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
    return false;
  } // end IF

  return true;
}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType>
const SocketHandlerConfigurationType*
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       ITransportLayerType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
//  return configuration_;
}

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType>
Net_Client_Connector_T<ACE_Netlink_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       Net_INetlinkTransportLayer_t,
                       HandlerType>::Net_Client_Connector_T (const ConfigurationType* configuration_in,
                                                             ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                             unsigned int statisticCollectionInterval_in)
 : configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType>
Net_Client_Connector_T<ACE_Netlink_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       Net_INetlinkTransportLayer_t,
                       HandlerType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType>
int
Net_Client_Connector_T<ACE_Netlink_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       Net_INetlinkTransportLayer_t,
                       HandlerType>::make_svc_handler (HandlerType*& handler_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // init return value(s)
  handler_inout = NULL;

  ACE_NEW_NORETURN (handler_inout,
                    HandlerType (interfaceHandle_,
                                 statisticCollectionInterval_));
  if (!handler_inout)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return ((handler_inout == NULL) ? -1 : 0);
}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType>
void
Net_Client_Connector_T<ACE_Netlink_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       Net_INetlinkTransportLayer_t,
                       HandlerType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType>
bool
Net_Client_Connector_T<ACE_Netlink_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       Net_INetlinkTransportLayer_t,
                       HandlerType>::connect (const ACE_Netlink_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  HandlerType* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T::make_svc_handler(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (handler_p);

  // *NOTE*: the handler registers with the reactor and will be freed (or
  //         recycled) automatically
  result =
      handler_p->open (const_cast<ConfigurationType*> (configuration_));
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    // *TODO*: find a replacement for ACE_INET_Addr::addr_to_string
//    result = address_in.addr_to_string (buffer, sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::open(\"%s\"): \"%m\", aborting\n"),
                buffer));
    return false;
  } // end IF

  return true;
}

template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType>
const SocketHandlerConfigurationType*
Net_Client_Connector_T<ACE_Netlink_Addr,
                       ConfigurationType,
                       SocketHandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       Net_INetlinkTransportLayer_t,
                       HandlerType>::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
//  return configuration_;
}
#endif
