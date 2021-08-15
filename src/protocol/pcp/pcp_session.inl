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

#include "net_common_tools.h"

#include "pcp_tools.h"

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType>
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::PCP_Session_T ()
 : configuration_ (NULL)
 , connection_ (NULL)
 , state_ ()
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::PCP_Session_T"));

}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType>
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::~PCP_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::~PCP_Session_T"));

  if (likely (connection_))
  {
    connection_->close ();
    connection_->decrease ();
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
          typename MessageType>
void
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::initialize (const ConnectionConfigurationType& configuration_in,
                                        PCP_IConnection_t* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::initialize"));

  configuration_ = &const_cast<ConnectionConfigurationType&> (configuration_in);
  connection_ = connection_in;
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType>
void
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::announce ()
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::announce"));

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
  struct PCP_Record PCP_record;
  PCP_record.version = PCP_Codes::PCP_VERSION_2;
  PCP_record.opcode = PCP_Codes::PCP_OPCODE_ANNOUNCE;
  //PCP_record.lifetime = 0;
  std::string device_identifier_string =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Net_Common_Tools::interfaceToString (configuration_->socketConfiguration.interfaceIdentifier);
#else
    configuration_->socketConfiguration.interfaceIdentifier;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INET_Addr gateway_address;
  if (!Net_Common_Tools::interfaceToIPAddress (device_identifier_string,
                                               PCP_record.client_address,
                                               gateway_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (device_identifier_string.c_str ())));
    message_p->release ();
    return;
  } // end IF
  //struct PCPOption option_s;
  //option_s.code = PCP_Codes::PCP_OPTION_THIRD_PARTY;
  //option_s.length = 16;
  //option_s.third_party.address =
  //  ACE_sap_any_cast (const ACE_INET_Addr&);
  //PCP_record.options.push_back (option_s);
  message_p->initialize (PCP_record,
                         message_p->sessionId (),
                         NULL);

  ACE_Message_Block* message_block_p = message_p;
  connection_->send (message_block_p);
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType>
void
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::map (const ACE_INET_Addr& externalAddress_in,
                                 const ACE_INET_Addr& internalAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::map"));

  // *TODO*: support third_party option
  ACE_UNUSED_ARG (internalAddress_in);

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
  struct PCP_Record PCP_record;
  PCP_record.version = PCP_Codes::PCP_VERSION_2;
  PCP_record.opcode = PCP_Codes::PCP_OPCODE_MAP;
  PCP_record.lifetime = PCP_DEFAULT_MAP_LIFETIME_S;
  std::string device_identifier_string =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Net_Common_Tools::interfaceToString (configuration_->socketConfiguration.interfaceIdentifier);
#else
    configuration_->socketConfiguration.interfaceIdentifier;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INET_Addr gateway_address;
  if (!Net_Common_Tools::interfaceToIPAddress (device_identifier_string,
                                               PCP_record.client_address,
                                               gateway_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (device_identifier_string.c_str ())));
    message_p->release ();
    return;
  } // end IF
  PCP_record.map.nonce = PCP_Tools::generateNonce ();
  PCP_record.map.protocol = IPPROTO_TCP;
  PCP_record.map.internal_port = internalAddress_in.get_port_number ();
  PCP_record.map.external_port = externalAddress_in.get_port_number ();
  PCP_record.map.external_address = externalAddress_in;
  //struct PCPOption filter_option_s;
  //filter_option_s.code = PCP_Codes::PCP_OPTION_FILTER;
  //filter_option_s.length = 20;
  //filter_option_s.filter.prefix_length = 0; // delete set filters
  //filter_option_s.filter.remote_peer_port = 0; // all ports
  //filter_option_s.filter.remote_peer_address =
  //  ACE_sap_any_cast (const ACE_INET_Addr&);
  //PCP_record.options.push_back (filter_option_s);
  message_p->initialize (PCP_record,
                         message_p->sessionId (),
                         NULL);

  state_.nonce = PCP_record.map.nonce;
  state_.timeStamp = COMMON_TIME_NOW;

  ACE_Message_Block* message_block_p = message_p;
  connection_->send (message_block_p);
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType>
void
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::peer (const ACE_INET_Addr& externalAddress_in,
                                  const ACE_INET_Addr& internalAddress_in,
                                  const ACE_INET_Addr& remotePeerAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::peer"));

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
  struct PCP_Record PCP_record;
  PCP_record.version = PCP_Codes::PCP_VERSION_2;
  PCP_record.opcode = PCP_Codes::PCP_OPCODE_PEER;
  PCP_record.lifetime = PCP_DEFAULT_MAP_LIFETIME_S;
  std::string device_identifier_string =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Net_Common_Tools::interfaceToString (configuration_->socketConfiguration.interfaceIdentifier);
#else
    configuration_->socketConfiguration.interfaceIdentifier;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INET_Addr gateway_address;
  if (!Net_Common_Tools::interfaceToIPAddress (device_identifier_string,
                                               PCP_record.client_address,
                                               gateway_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (device_identifier_string.c_str ())));
    message_p->release ();
    return;
  } // end IF
  PCP_record.peer.nonce = PCP_Tools::generateNonce ();
  PCP_record.peer.protocol = IPPROTO_TCP;
  PCP_record.peer.internal_port = internalAddress_in.get_port_number ();
  PCP_record.peer.external_port = externalAddress_in.get_port_number ();
  PCP_record.peer.external_address = externalAddress_in;
  PCP_record.peer.remote_peer_port = remotePeerAddress_in.get_port_number ();
  PCP_record.peer.remote_peer_address = remotePeerAddress_in;
  //struct PCPOption option_s;
  //option_s.code = PCP_Codes::PCP_OPTION_THIRD_PARTY;
  //option_s.length = 16;
  //option_s.third_party.address =
  //  ACE_sap_any_cast (const ACE_INET_Addr&);
  //PCP_record.options.push_back (option_s);
  message_p->initialize (PCP_record,
                         message_p->sessionId (),
                         NULL);

  ACE_Message_Block* message_block_p = message_p;
  connection_->send (message_block_p);
}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType>
void
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::authenticate ()
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::authenticate"));

}

template <typename StateType,
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename MessageType>
void
PCP_Session_T<StateType,
              ConnectionConfigurationType,
              ConnectionManagerType,
              MessageType>::notify (struct PCP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Session_T::notify"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  // clean up
  PCP_Tools::free (*record_inout);
  delete record_inout; record_inout = NULL;
}
