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

#include "net_connection_manager_common.h"
#include "net_macros.h"

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
Net_Client_AsynchConnector_T<AddressType,
                             ConfigurationType,
                             SessionDataType,
                             TransportLayerType,
                             ConnectionType>::Net_Client_AsynchConnector_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                            const ConfigurationType* configuration_in)
 : configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::Net_Client_AsynchConnector_T"));

  int result = -1;

  // init base class
  result = inherited::open (false, // pass addresses ?
                            NULL,  // default proactor
                            true); // validate new connections ?
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::open(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
Net_Client_AsynchConnector_T<AddressType,
                             ConfigurationType,
                             SessionDataType,
                             TransportLayerType,
                             ConnectionType>::~Net_Client_AsynchConnector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::~Net_Client_AsynchConnector_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
ConnectionType*
Net_Client_AsynchConnector_T<AddressType,
                             ConfigurationType,
                             SessionDataType,
                             TransportLayerType,
                             ConnectionType>::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::make_handler"));

  // init return value(s)
  ConnectionType* result = NULL;

  // default behavior
  ACE_NEW_NORETURN (result,
                    ConnectionType ());
                    //Net_AsynchTCPConnection (interfaceHandle_));
  if (!result)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
const ConfigurationType*
Net_Client_AsynchConnector_T<AddressType,
                             ConfigurationType,
                             SessionDataType,
                             TransportLayerType,
                             ConnectionType>::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::getConfiguration"));

  return configuration_;
}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
void
Net_Client_AsynchConnector_T<AddressType,
                             ConfigurationType,
                             SessionDataType,
                             TransportLayerType,
                             ConnectionType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::abort"));

  if (inherited::cancel () == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::cancel(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
bool
Net_Client_AsynchConnector_T<AddressType,
                             ConfigurationType,
                             SessionDataType,
                             TransportLayerType,
                             ConnectionType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::connect"));

  int result =
    inherited::connect (address_in,                            // remote SAP
                        ACE_sap_any_cast (const AddressType&), // local SAP
                        1,                                     // re-use address (SO_REUSEADDR) ?
                        NULL);                                 // ACT
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    if (address_in.addr_to_string (buffer,
                                   sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT("failed to ACE_Asynch_Connector::connect(\"%s\"): \"%m\", aborting\n"),
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
int
Net_Client_AsynchConnector_T<AddressType,
                             ConfigurationType,
                             SessionDataType,
                             TransportLayerType,
                             ConnectionType>::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                                   const ACE_INET_Addr& remoteSAP_in,
                                                                   const ACE_INET_Addr& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector_T::validate_connection"));

  // success ?
  if (result_in.success () == 0)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    if (remoteSAP_in.addr_to_string (buffer, sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector_T::connect(\"%s\"): \"%s\", aborting\n"),
                buffer,
                ACE_OS::strerror (result_in.error ())));
  } // end IF

  return ((result_in.success () == 1) ? 0 : -1);
}
