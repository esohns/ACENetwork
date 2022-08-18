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
 , connection_ (NULL)
 , state_ ()
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

  if (likely (connection_))
  {
    connection_->abort ();
    connection_->decrease (); connection_ = NULL;
  } // end IF

  ConnectionManagerType* connection_manager_p =
      ConnectionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->abort (false);
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
               UserData>::initialize (const ConnectionConfigurationType& configuration_in,
                                      const UserData& userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::initialize"));

  configuration_ = &const_cast<ConnectionConfigurationType&> (configuration_in);
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
               UserData>::getDeviceDescription (const std::string& URL_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::getDeviceDescription"));

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

  // step2: connect ?
  if (connection_)
    goto continue_;

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
  connection_ =
    connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle_h));
  ACE_ASSERT (connection_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%u: connected to %s\n"),
              connection_->id (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address, false, false).c_str ())));

continue_:
  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->allocatorConfiguration);
  ACE_ASSERT (configuration_->messageAllocator);
  ACE_ASSERT (connection_);

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
  connection_->send (message_block_p);
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
               UserData>::getServiceDescription (const std::string& URL_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::getServiceDescription"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->messageAllocator);
  ACE_ASSERT (connection_);

  // step1: allocate buffer
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

  // step2: configure message
  typename MessageType::DATA_T::DATA_T* record_p = NULL;
  typename MessageType::DATA_T* message_data_container_p = NULL;
  message_p->initialize (message_data_container_p,
                         1, //message_p->sessionId (),
                         NULL);

  ACE_Message_Block* message_block_p = message_p;
  connection_->send (message_block_p);
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
               UserData>::notify (const struct HTTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Session_T::notify"));

  HTTP_HeadersConstIterator_t iterator =
    record_in.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_LOCATION_STRING));
  ACE_ASSERT (iterator != record_in.headers.end ());
  HTTP_HeadersConstIterator_t iterator_2 =
    record_in.headers.find (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_SERVICE_TYPE_HEADER_STRING));
  ACE_ASSERT (iterator_2 != record_in.headers.end ());

  if (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_ST_ROOT_DEVICE_STRING),
                       (*iterator_2).second.c_str ()))
    getDeviceDescription ((*iterator).second);
}
