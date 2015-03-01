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
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       ConnectionType>::Net_Client_Connector_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                const ConfigurationType* configuration_in)
 : inherited (ACE_Reactor::instance (), // default reactor
              ACE_NONBLOCK)             // flags: non-blocking I/O
              //0)                       // flags
 , configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       ConnectionType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
int
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       ConnectionType>::make_svc_handler (ConnectionType*& handler_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // init return value(s)
  handler_inout = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_inout,
                    ConnectionType ());
                    //Net_TCPConnection (interfaceHandle_));
  if (!handler_inout)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return ((handler_inout == NULL) ? -1 : 0);
}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
void
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       ConnectionType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

  if (inherited::close () == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::close(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
bool
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       ConnectionType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  ConnectionType* handler = NULL;
  int result = -1;

  result = inherited::connect (handler,                         // service handler
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
    result = address_in.addr_to_string (buffer, sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));

    return false;
  } // end IF

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
const ConfigurationType*
Net_Client_Connector_T<AddressType,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       ConnectionType>::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  return configuration_;
}

/////////////////////////////////////////

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType>
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       Net_UDPConnection>::Net_Client_Connector_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                   const ConfigurationType* configuration_in)
 : configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType>
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       Net_UDPConnection>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType>
int
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       Net_UDPConnection>::make_svc_handler (Net_UDPConnection*& handler_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // init return value(s)
  handler_inout = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_inout,
                    Net_UDPConnection ());
                    //Net_TCPConnection (interfaceHandle_));
  if (!handler_inout)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return ((handler_inout == NULL) ? -1 : 0);
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType>
void
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       Net_UDPConnection>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return);
}

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType>
bool
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       Net_UDPConnection>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  Net_UDPConnection* handler = NULL;
  result = make_svc_handler (handler);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T<Net_UDPConnection>::make_svc_handler(): \"%m\", aborting\n")));

    return false;
  } // end IF
  ACE_ASSERT (handler);

  result =
      handler->open (const_cast<ConfigurationType*> (this->getConfiguration ()));
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

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType>
const ConfigurationType*
Net_Client_Connector_T<ACE_INET_Addr,
                       ConfigurationType,
                       SessionDataType,
                       TransportLayerType,
                       Net_UDPConnection>::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  return configuration_;
}
