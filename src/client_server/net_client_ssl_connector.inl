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

#include "openssl/ssl.h"

#include "ace/Log_Msg.h"
#include "ace/Svc_Handler.h"

#include "common_defines.h"

#include "net_common_tools.h"
#include "net_macros.h"

template <typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           StreamType,
                           UserDataType>::Net_Client_SSL_Connector_T (bool managed_in)
 : inherited (ACE_Reactor::instance (), // default reactor
              // *IMPORTANT NOTE*: ACE_NONBLOCK is only set if timeout != NULL
              //                   (see: SOCK_Connector.cpp:94), set via the
              //                   "synch options" (see line 200 below)
              ACE_NONBLOCK)             // flags: non-blocking I/O
              //0)                       // flags
 , configuration_ (NULL)
 , managed_ (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::Net_Client_SSL_Connector_T"));

}

template <typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
ACE_HANDLE
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           StreamType,
                           UserDataType>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::connect"));

  int result = -1;
  HandlerType* handler_p = NULL;
  ACE_Synch_Options synch_options = ACE_Synch_Options::defaults;
  ACE_INET_Addr local_address = ACE_sap_any_cast (ACE_INET_Addr&);
  int reuse_addr_i = 1; // set SO_REUSEADDR ?
  int flags_i = O_RDWR;
  int permissions_i = 0;
  // *NOTE*: to enforce ACE_NONBLOCK, set ACE_Synch_Options::USE_REACTOR or
  //         ACE_Synch_Options::USE_TIMEOUT in the synch options (see:
  //         Connector.cpp:409 and net_sock_connector.cpp:219 and/or
  //         SOCK_Connector.cpp:94)
  result =
      inherited::connect (handler_p,      // service handler
                          address_in,     // remote SAP
                          synch_options,  // synch options
                          local_address,  // local SAP
                          reuse_addr_i,   // re-use address ?
                          flags_i,        // flags
                          permissions_i); // permissions
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::connect(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  return handler_p->get_handle ();
}

template <typename HandlerType,
          typename ConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
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
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_Client_SSL_Connector_T<HandlerType,
                           ConnectorType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           StreamType,
                           UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_SSL_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (managed_));
  if (unlikely (!handler_out))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return -1;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (configuration_->socketConfiguration.hostname.empty ())
  {
    configuration_->socketConfiguration.hostname =
      Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address,
                                           true,  // do not append port
                                           true); // resolve
    if (unlikely (configuration_->socketConfiguration.hostname.empty ()))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to resolve address (was: \"%s\"); cannot set TLS SNI hostname, continuing\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address, true, false).c_str ())));
      return 0;
    } // end IF
  } // end IF

  // support TLS SNI
  typename HandlerType::stream_type& stream_r = handler_out->peer ();
  SSL* context_p = stream_r.ssl ();
  ACE_ASSERT (context_p);
  int result =
      SSL_set_tlsext_host_name (context_p,
                                configuration_->socketConfiguration.hostname.c_str ());
  if (unlikely (result == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SSL_set_tlsext_host_name(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (configuration_->socketConfiguration.hostname.c_str ()),
                ACE_TEXT (Net_Common_Tools::SSLErrorToString ().c_str ())));
    delete handler_out; handler_out = NULL;
    return -1;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("set TLS SNI hostname: \"%s\"\n"),
              ACE_TEXT (configuration_->socketConfiguration.hostname.c_str ())));

  if (configuration_->socketConfiguration.version)
  {
    SSL_CTX* context_2 = SSL_get_SSL_CTX (context_p);
    ACE_ASSERT (context_2);
    SSL_CTX_set_min_proto_version (context_2,
                                   configuration_->socketConfiguration.version);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("set minimum protocol version: 0x%x\n"),
                configuration_->socketConfiguration.version));
  } // end IF

  return 0;
}
