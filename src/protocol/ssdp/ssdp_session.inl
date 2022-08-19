/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
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

#include "ace/Assert.h"
#include "ace/Log_Msg.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "net_client_common_tools.h"

#include "http_codes.h"
#include "http_defines.h"
#include "http_tools.h"

#include "ssdp_defines.h"
#include "ssdp_tools.h"

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::SSDP_Session_T ()
 : configuration_ (NULL)
 , preferredServerPort_ (0)
 , state_ ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::SSDP_Session_T"));

}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::~SSDP_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::~SSDP_Session_T"));

  close ();
  wait ();
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::initialize (ACE_UINT16 preferredServerPort_in,
                                      const ConnectionConfigurationType& configuration_in,
                                      const UserData& userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::initialize"));

  configuration_ = &const_cast<ConnectionConfigurationType&> (configuration_in);
  preferredServerPort_ = preferredServerPort_in;
  userData_ = &const_cast<UserData&> (userData_in);
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::close"));

  ConnectionManagerType* connection_manager_p =
    ConnectionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->abort (false); // wait ?
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::wait ()
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::wait"));

  ConnectionManagerType* connection_manager_p =
    ConnectionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->wait ();
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::getURL (const std::string& URL_in) const
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::getURL"));

  // step1: parse URL, update configuration
  std::string hostname_string, URI_string;
  bool use_SSL_b = false;
  if (unlikely (!HTTP_Tools::parseURL (URL_in,
                                       configuration_->socketConfiguration.address,
                                       hostname_string,
                                       URI_string,
                                       use_SSL_b)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (URL_in.c_str ())));
    return;
  } // end IF

  ConnectorType connector (true);
  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  ConnectionManagerType* connection_manager_p = NULL;
  HTTP_IConnection_t* connection_p = NULL;

  // step2: connect
  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (userData_);

  handle_h =
    Net_Client_Common_Tools::connect<ConnectorType> (connector,
                                                     *configuration_,
                                                     *userData_,
                                                     configuration_->socketConfiguration.address,
                                                     true, true);
  if (unlikely (handle_h == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address, false, false).c_str ())));
    return;
  } // end IF

  connection_manager_p = ConnectionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_p =
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle_h));
  ACE_ASSERT (connection_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%u: connected to %s\n"),
              connection_p->id (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address, false, false).c_str ())));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->allocatorConfiguration);
  ACE_ASSERT (configuration_->messageAllocator);
  ACE_ASSERT (connection_p);

  // step3: allocate buffer
  size_t pdu_size_i =
    configuration_->allocatorConfiguration->defaultBufferSize +
    configuration_->allocatorConfiguration->paddingBytes;
  MessageType* message_p = NULL;
allocate:
  message_p =
    static_cast<MessageType*> (configuration_->messageAllocator->malloc (pdu_size_i));
  // keep retrying ?
  if (!message_p &&
      !configuration_->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate MessageType: \"%m\", returning\n")));
    return;
  } // end IF

  // step4: configure message
  typename MessageType::DATA_T::DATA_T* record_p = NULL;
  ACE_NEW_NORETURN (record_p,
                    typename MessageType::DATA_T::DATA_T ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate MessageType::DATA_T::DATA_T: \"%m\", returning\n")));
    message_p->release (); message_p = NULL;
    return;
  } // end IF
  record_p->method = HTTP_Codes::HTTP_METHOD_GET;
  record_p->URI = URI_string;
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                            hostname_string));

  typename MessageType::DATA_T* message_data_container_p = NULL;
  ACE_NEW_NORETURN (message_data_container_p,
                    typename MessageType::DATA_T (record_p,
                                                  true)); // delete ?
  if (!message_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate MessageType::DATA_T: \"%m\", returning\n")));
    delete record_p; record_p = NULL;
    message_p->release (); message_p = NULL;
    return;
  } // end IF
  message_p->initialize (message_data_container_p,
                         1, //message_p->sessionId (),
                         NULL);

  // step5: send message
  ACE_Message_Block* message_block_p = message_p;
  connection_p->send (message_block_p);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("retrieving URL \"%s\"\n"),
              ACE_TEXT (URL_in.c_str ())));

  // step6: release resources
  connection_p->decrease (); connection_p = NULL;
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::map (const ACE_INET_Addr& externalAddress_in,
                               const ACE_INET_Addr& internalAddress_in) const
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::map"));

  // sanity check(s)
  ACE_ASSERT (!state_.serviceDescriptionURL.empty ());

  // step1: parse URL, update configuration
  std::string hostname_string, URI_string;
  bool use_SSL_b = false;
  if (unlikely (!HTTP_Tools::parseURL (state_.serviceDescriptionURL,
                                       configuration_->socketConfiguration.address,
                                       hostname_string,
                                       URI_string,
                                       use_SSL_b)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (state_.serviceDescriptionURL.c_str ())));
    return;
  } // end IF

  ConnectorType connector (true);
  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
  ConnectionManagerType* connection_manager_p = NULL;
  HTTP_IConnection_t* connection_p = NULL;

  // step2: connect
  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (userData_);

  handle_h =
    Net_Client_Common_Tools::connect<ConnectorType> (connector,
                                                     *configuration_,
                                                     *userData_,
                                                     configuration_->socketConfiguration.address,
                                                     true, true);
  if (unlikely (handle_h == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address, false, false).c_str ())));
    return;
  } // end IF

  connection_manager_p = ConnectionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_p =
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle_h));
  ACE_ASSERT (connection_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%u: connected to %s\n"),
              connection_p->id (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address, false, false).c_str ())));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->allocatorConfiguration);
  ACE_ASSERT (configuration_->messageAllocator);
  ACE_ASSERT (connection_p);

  // step3: allocate buffer
  size_t pdu_size_i =
    configuration_->allocatorConfiguration->defaultBufferSize +
    configuration_->allocatorConfiguration->paddingBytes;
  MessageType* message_p = NULL, *message_2 = NULL;
allocate:
  message_p =
    static_cast<MessageType*> (configuration_->messageAllocator->malloc (pdu_size_i));
  // keep retrying ?
  if (!message_p &&
      !configuration_->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate MessageType: \"%m\", returning\n")));
    return;
  } // end IF

  // step4: configure message
  typename MessageType::DATA_T::DATA_T* record_p = NULL;
  ACE_NEW_NORETURN (record_p,
                    typename MessageType::DATA_T::DATA_T ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate MessageType::DATA_T::DATA_T: \"%m\", returning\n")));
    message_p->release (); message_p = NULL;
    return;
  } // end IF
  record_p->method = HTTP_Codes::HTTP_METHOD_POST;
  record_p->URI = state_.serviceControlURI;
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                            hostname_string));
  std::string header_string = ACE_TEXT_ALWAYS_CHAR ("text/xml; charset=\"utf-8\"");
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_TYPE_STRING),
                                            header_string));
  header_string = ACE_TEXT_ALWAYS_CHAR ("\"urn:schemas-upnp-org:service:");
  header_string += ACE_TEXT_ALWAYS_CHAR ("WANIPConnection:1");
  header_string += ACE_TEXT_ALWAYS_CHAR ("#");
  header_string += ACE_TEXT_ALWAYS_CHAR ("AddPortMapping");
  header_string += ACE_TEXT_ALWAYS_CHAR ("\"");
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("SOAPACTION"),
                                            header_string));

  std::string xml_body = ACE_TEXT_ALWAYS_CHAR ("<?xml version=\"1.0\"?>");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("<s:Envelope");
  xml_body += ACE_TEXT_ALWAYS_CHAR (" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"");
  xml_body += ACE_TEXT_ALWAYS_CHAR (" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("<s:Body>");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("<u:");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("AddPortMapping");
  xml_body += ACE_TEXT_ALWAYS_CHAR (" xmlns:u=\"urn:schemas-upnp-org:service:");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("WANIPConnection:1");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("\">");
  std::ostringstream converter;
  for (SSDP_ServiceActionArgumentsConstIterator_t iterator = state_.actionArguments.begin ();
       iterator != state_.actionArguments.end ();
       ++iterator)
  {
    xml_body += ACE_TEXT_ALWAYS_CHAR("<");
    xml_body += *iterator;
    xml_body += ACE_TEXT_ALWAYS_CHAR(">");
    if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewRemoteHost")))
      ;//xml_body += Net_Common_Tools::IPAddressToString (externalAddress_in, true, false);
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewRemotePort")))
    {
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << externalAddress_in.get_port_number ();
      xml_body += converter.str ();
    } // end ELSE IF
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewExternalPort")))
    {
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << externalAddress_in.get_port_number ();
      xml_body += converter.str ();
    } // end ELSE IF
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewProtocol")))
      xml_body += ACE_TEXT_ALWAYS_CHAR ("TCP");
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewInternalPort")))
    {
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << internalAddress_in.get_port_number ();
      xml_body += converter.str ();
    } // end ELSE IF
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewInternalClient")))
      xml_body += Net_Common_Tools::IPAddressToString (internalAddress_in, true, false);
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewEnabled")))
      xml_body += ACE_TEXT_ALWAYS_CHAR ("1");
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewPortMappingDescription")))
      ;//xml_body += ACE_TEXT_ALWAYS_CHAR ("testing UPnP client");
    else if (!ACE_OS::strcmp ((*iterator).c_str (), ACE_TEXT_ALWAYS_CHAR ("NewLeaseDuration")))
    {
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << 0;
      xml_body += converter.str ();
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown action argument (was: \"%s\"), returning\n"),
                  ACE_TEXT ((*iterator).c_str ())));
      delete record_p; record_p = NULL;
      message_p->release (); message_p = NULL;
      return;
    } // end ELSE
    xml_body += ACE_TEXT_ALWAYS_CHAR ("</");
    xml_body += *iterator;
    xml_body += ACE_TEXT_ALWAYS_CHAR (">");
  } // end FOR
  xml_body += ACE_TEXT_ALWAYS_CHAR ("</u:");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("AddPortMapping");
  xml_body += ACE_TEXT_ALWAYS_CHAR (">");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("</s:Body>");
  xml_body += ACE_TEXT_ALWAYS_CHAR ("</s:Envelope>");

  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << xml_body.size ();
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING),
                                            converter.str ()));

  typename MessageType::DATA_T* message_data_container_p = NULL;
  ACE_NEW_NORETURN (message_data_container_p,
                    typename MessageType::DATA_T (record_p,
                                                  true)); // delete ?
  if (!message_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate MessageType::DATA_T: \"%m\", returning\n")));
    delete record_p; record_p = NULL;
    message_p->release (); message_p = NULL;
    return;
  } // end IF
  message_p->initialize (message_data_container_p,
                         1, //message_p->sessionId (),
                         NULL);

allocate_2:
  message_2 =
    static_cast<MessageType*> (configuration_->messageAllocator->malloc (pdu_size_i));
  // keep retrying ?
  if (!message_2 &&
      !configuration_->messageAllocator->block ())
    goto allocate_2;
  if (!message_2)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate MessageType: \"%m\", returning\n")));
    message_p->release (); message_p = NULL;
    return;
  } // end IF

  int result = message_2->copy (xml_body.c_str (),
                                xml_body.size ());
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
    message_p->release (); message_p = NULL;
    message_2->release (); message_2 = NULL;
    return;
  } // end IF
  message_p->cont (message_2);

  // step5: send message
  ACE_Message_Block* message_block_p = message_p;
  connection_p->send (message_block_p);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("mapping \"%s\" --> \"%s\"\n"),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (externalAddress_in, false, false).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (internalAddress_in, false, false).c_str ())));

  // step6: release resources
  connection_p->decrease (); connection_p = NULL;
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::notifySSDPResponse (const struct HTTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::notifySSDPResponse"));

  HTTP_HeadersConstIterator_t iterator =
    record_in.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_LOCATION_STRING));
  ACE_ASSERT (iterator != record_in.headers.end ());
  HTTP_HeadersConstIterator_t iterator_2 =
    record_in.headers.find (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_SERVICE_TYPE_HEADER_STRING));
  ACE_ASSERT (iterator_2 != record_in.headers.end ());

  if (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_ST_ROOT_DEVICE_STRING),
                       (*iterator_2).second.c_str ()))
  {
    // step1: parse URL
    ACE_INET_Addr address;
    std::string hostname_string, URI_string;
    bool use_SSL_b = false;
    if (unlikely (!HTTP_Tools::parseURL ((*iterator).second,
                                         address,
                                         hostname_string,
                                         URI_string,
                                         use_SSL_b)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                  ACE_TEXT ((*iterator).second.c_str ())));
      return;
    } // end IF
    // step2: filter discovery replies
    if (address.get_port_number () != preferredServerPort_)
      return;

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("found \"%s\", notifying location: \"%s\"\n"),
                ACE_TEXT ((*iterator_2).second.c_str ()),
                ACE_TEXT ((*iterator).second.c_str ())));

    // remember device description URL
    state_.deviceDescriptionURL = (*iterator).second;

    getURL ((*iterator).second);
  } // end IF
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::notifyServiceDescriptionControlURIs (const std::string& serviceDescriptionURI_in,
                                                               const std::string& serviceControlURI_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::notifyServiceDescriptionControlURIs"));

  // sanity check(s)
  ACE_ASSERT (!state_.deviceDescriptionURL.empty ());

  // step1: parse URL, update configuration
  ACE_INET_Addr address;
  std::string hostname_string, URI_string;
  bool use_SSL_b = false;
  if (unlikely (!HTTP_Tools::parseURL (state_.deviceDescriptionURL,
                                       configuration_->socketConfiguration.address,
                                       hostname_string,
                                       URI_string,
                                       use_SSL_b)))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
               ACE_TEXT (state_.deviceDescriptionURL.c_str ())));
    return;
  } // end IF

  state_.serviceDescriptionURL = ACE_TEXT_ALWAYS_CHAR ("http");
  if (use_SSL_b)
    state_.serviceDescriptionURL += ACE_TEXT_ALWAYS_CHAR ("s");
  state_.serviceDescriptionURL += ACE_TEXT_ALWAYS_CHAR ("://");
  state_.serviceDescriptionURL += hostname_string;
  state_.serviceDescriptionURL += serviceDescriptionURI_in;

  state_.serviceControlURI = serviceControlURI_in;

  getURL (state_.serviceDescriptionURL);
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType,
          typename ConnectorType,
          typename UserData>
void
SSDP_Session_T<StateType,
               ConnectionConfigurationType,
               ConnectionManagerType,
               MessageType,
               ConnectorType,
               UserData>::notifyServiceActionArguments (const SSDP_ServiceActionArguments_t& arguments_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::notifyServiceDescriptionControlURIs"));

  // sanity check(s)
  ACE_ASSERT (state_.actionArguments.empty ());

  state_.actionArguments = arguments_in;
}
