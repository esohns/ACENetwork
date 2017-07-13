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
#include "ace/Svc_Handler.h"

#include "common_defines.h"

#include "net_macros.h"

template <typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           UserDataType>::Net_Client_SSL_Connector_T (ICONNECTION_MANAGER_T* connectionManager_in,
                                                                      const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited ()
 , configuration_ ()
 , connectionManager_ (connectionManager_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::Net_Client_SSL_Connector_T"));

  if (!connectionManager_)
    connectionManager_ = CONNECTION_MANAGER_T::SINGLETON_T::instance ();
}

template <typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           UserDataType>::~Net_Client_SSL_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::~Net_Client_SSL_Connector_T"));

}

template <typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_TransportLayerType
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           UserDataType>::transportLayer () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::transportLayer"));

  enum Net_TransportLayerType result = NET_TRANSPORTLAYER_INVALID;

  // *TODO*: find a better way to do this
  HandlerType* handler_p = NULL;
  int result_2 = const_cast<OWN_TYPE_T*> (this)->make_svc_handler (handler_p);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T<Net_UDPConnection_T>::make_svc_handler(): \"%m\", aborting\n")));
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
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::abort"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           UserDataType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::connect"));

  int result = -1;

  HandlerType* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_SSL_Connector_T::make_svc_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  result =
    inherited::connect (handler_p->peer (),              // return value: (connected) stream
                        address_in,                      // remote SAP
                        NULL,                            // timeout
                        ACE_sap_any_cast (AddressType&), // local address
                        1,                               // re-use address (SO_REUSEADDR) ?
                        O_RDWR,                          // flags
                        0);                              // perms
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
                ACE_TEXT ("failed to ACE_SSL_SOCK_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
  
    // clean up
    delete handler_p;

    return ACE_INVALID_HANDLE;
  } // end IF

  result = activate_svc_handler (handler_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_SSL_Connector_T::activate_svc_handler(): \"%m\", aborting\n")));

    // clean up
    delete handler_p;

    return ACE_INVALID_HANDLE;
  } // end IF

  return handler_p->get_handle ();
}

template <typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           UserDataType>::activate_svc_handler (HandlerType* handler_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::activate_svc_handler"));

  // pre-initialize the connection handler
  // *TODO*: remove type inference
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
  if (error || handler_in->open (iconnector_p) == -1)
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
template <typename HandlerType,
          typename ConnectorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (connectionManager_,
                                 statisticCollectionInterval_));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}
