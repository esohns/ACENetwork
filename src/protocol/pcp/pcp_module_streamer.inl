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

#include <string>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_macros.h"

#include "pcp_common.h"
#include "pcp_defines.h"
#include "pcp_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
PCP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       SessionMessageType>::PCP_Module_Streamer_T (ISTREAM_T* stream_in)
#else
                       SessionMessageType>::PCP_Module_Streamer_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_Streamer_T::PCP_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
PCP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_Streamer_T::handleDataMessage"));

  int result = -1;
  ACE_UINT16 short_i = 0;
  ACE_UINT32 int_i = 0;
  ACE_UINT8 address_a[NET_ADDRESS_IPV6_ADDRESS_BYTES];
  ACE_UINT8 nonce_a[PCP_NONCE_BYTES];

  // don't care (implies yes per default, if part of a stream)
  // *NOTE*: as this is an "upstream" module, the "wording" is wrong
  //         --> the logic remains the same, though
  passMessageDownstream_out = true;

  // sanity check(s)
  ACE_ASSERT (message_inout->length () == 0);
  ACE_ASSERT (message_inout->capacity () >= PCP_MESSAGE_SIZE);

  // serialize structured data
  // --> create the appropriate bytestream corresponding to its elements
  const typename DataMessageType::DATA_T& data_r =
      message_inout->getR ();
  result =
    message_inout->copy (reinterpret_cast<const char*> (&data_r.version),
                         sizeof (ACE_UINT8));
  if (result == -1)
    goto error;
  result =
    message_inout->copy (reinterpret_cast<const char*> (&data_r.opcode),
                         sizeof (ACE_UINT8));
  if (result == -1)
    goto error;
  switch (data_r.opcode)
  {
    case PCP_Codes::PCP_OPCODE_ANNOUNCE:
    case PCP_Codes::PCP_OPCODE_MAP:
    case PCP_Codes::PCP_OPCODE_PEER:
    {
      result =
        message_inout->copy (reinterpret_cast<const char*> (&short_i), // 0
                             sizeof (ACE_UINT16));
      if (result == -1)
        goto error;
      break;
    }
    case PCP_Codes::PCP_OPCODE_AUTHENTICATION:
    {
      result =
        message_inout->copy (reinterpret_cast<const char*> (&short_i), // 0
                             sizeof (ACE_UINT8));
      if (result == -1)
        goto error;
      result =
        message_inout->copy (reinterpret_cast<const char*> (&data_r.result_code),
                             sizeof (ACE_UINT8));
      if (result == -1)
        goto error;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown opcode (was: %d), aborting\n"),
                  data_r.opcode));
      goto error;
    }
  } // end SWITCH
  int_i = data_r.lifetime;
  int_i =
    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (int_i) : int_i);
  result =
    message_inout->copy (reinterpret_cast<const char*> (&int_i),
                         sizeof (ACE_UINT32));
  if (result == -1)
    goto error;
  PCP_Tools::mapAddress (data_r.client_address,
                         address_a);
  result =
    message_inout->copy (reinterpret_cast<const char*> (&address_a[0]),
                         sizeof (ACE_UINT8[NET_ADDRESS_IPV6_ADDRESS_BYTES]));
  if (result == -1)
    goto error;
  switch (data_r.opcode)
  {
    case PCP_Codes::PCP_OPCODE_ANNOUNCE:
      break;
    case PCP_Codes::PCP_OPCODE_MAP:
    {
      ACE_OS::memset (nonce_a, 0, sizeof (ACE_UINT8[PCP_NONCE_BYTES]));
      unsigned int offset_i =
        sizeof (ACE_UINT8[PCP_NONCE_BYTES]) - sizeof (ACE_UINT64);
      ACE_UINT64 int64_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (data_r.map.nonce)
                                               : data_r.map.nonce);
      ACE_OS::memcpy (nonce_a + offset_i, &int64_i, sizeof (ACE_UINT64));
      result =
        message_inout->copy (reinterpret_cast<const char*> (&nonce_a[0]),
                             sizeof (ACE_UINT8[PCP_NONCE_BYTES]));
      if (result == -1)
        goto error;
      result =
        message_inout->copy (reinterpret_cast<const char*> (&data_r.map.protocol),
                             sizeof (ACE_UINT8));
      if (result == -1)
        goto error;
      result =
        message_inout->copy (reinterpret_cast<const char*> (&data_r.map.reserved + 1),
                             sizeof (ACE_UINT8) * 3);
      if (result == -1)
        goto error;
      short_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (data_r.map.internal_port)
                                               : data_r.map.internal_port);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&short_i),
                             sizeof (ACE_UINT16));
      if (result == -1)
        goto error;
      short_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (data_r.map.external_port)
                                               : data_r.map.external_port);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&short_i),
                             sizeof (ACE_UINT16));
      if (result == -1)
        goto error;
      PCP_Tools::mapAddress (*data_r.map.external_address,
                             address_a);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&address_a[0]),
                             sizeof (ACE_UINT8[NET_ADDRESS_IPV6_ADDRESS_BYTES]));
      if (result == -1)
        goto error;
      break;
    }
    case PCP_Codes::PCP_OPCODE_PEER:
    {
      ACE_OS::memset (nonce_a, 0, sizeof (ACE_UINT8[PCP_NONCE_BYTES]));
      unsigned int offset_i =
        sizeof (ACE_UINT8[PCP_NONCE_BYTES]) - sizeof (ACE_UINT64);
      ACE_UINT64 int64_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (data_r.map.nonce)
                                               : data_r.map.nonce);
      ACE_OS::memcpy (nonce_a + offset_i, &int64_i, sizeof (ACE_UINT64));
      result =
        message_inout->copy (reinterpret_cast<const char*> (&nonce_a[0]),
                             sizeof (ACE_UINT8[PCP_NONCE_BYTES]));
      if (result == -1)
        goto error;
      result =
        message_inout->copy (reinterpret_cast<const char*> (&data_r.peer.protocol),
                             sizeof (ACE_UINT8));
      if (result == -1)
        goto error;
      result =
        message_inout->copy (reinterpret_cast<const char*> (&data_r.peer.reserved + 1),
                             sizeof (ACE_UINT8) * 3);
      if (result == -1)
        goto error;
      short_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (data_r.peer.internal_port)
                                               : data_r.peer.internal_port);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&short_i),
                             sizeof (ACE_UINT16));
      if (result == -1)
        goto error;
      short_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (data_r.peer.external_port)
                                               : data_r.peer.external_port);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&short_i),
                             sizeof (ACE_UINT16));
      if (result == -1)
        goto error;
      PCP_Tools::mapAddress (*data_r.peer.external_address,
                             address_a);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&address_a[0]),
                             sizeof (ACE_UINT8[NET_ADDRESS_IPV6_ADDRESS_BYTES]));
      if (result == -1)
        goto error;
      short_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (data_r.peer.remote_peer_port)
                                               : data_r.peer.remote_peer_port);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&short_i),
                             sizeof (ACE_UINT16));
      if (result == -1)
        goto error;
      result =
        message_inout->copy (reinterpret_cast<const char*> (&data_r.peer.reserved_2),
                             sizeof (ACE_UINT16));
      if (result == -1)
        goto error;
      PCP_Tools::mapAddress (*data_r.peer.remote_peer_address,
                             address_a);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&address_a[0]),
                             sizeof (ACE_UINT8[NET_ADDRESS_IPV6_ADDRESS_BYTES]));
      if (result == -1)
        goto error;
      break;
    }
    case PCP_Codes::PCP_OPCODE_AUTHENTICATION:
    {
      int_i = data_r.authentication.session_id;
      int_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (int_i) : int_i);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&int_i),
                             sizeof (ACE_UINT32));
      if (result == -1)
        goto error;
      int_i = data_r.authentication.sequence_number;
      int_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (int_i) : int_i);
      result =
        message_inout->copy (reinterpret_cast<const char*> (&int_i),
                             sizeof (ACE_UINT32));
      if (result == -1)
        goto error;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown opcode (was: %d), aborting\n"),
                  data_r.opcode));
      goto error;
    }
  } // end SWITCH
  for (PCP_OptionsConstIterator_t iterator = data_r.options.begin ();
       iterator != data_r.options.end ();
       ++iterator)
  {
    result =
      message_inout->copy (reinterpret_cast<const char*> (&(*iterator).code),
                           sizeof (ACE_UINT8));
    if (result == -1)
      goto error;
    result =
      message_inout->copy (reinterpret_cast<const char*> (&(*iterator).reserved),
                           sizeof (ACE_UINT8));
    if (result == -1)
      goto error;
    short_i =
      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD ((*iterator).length)
                                             : (*iterator).length);
    result =
      message_inout->copy (reinterpret_cast<const char*> (&short_i),
                            sizeof (ACE_UINT16));
    if (result == -1)
      goto error;
    switch ((*iterator).code)
    {
      case PCP_Codes::PCP_OPTION_THIRD_PARTY:
      {
        PCP_Tools::mapAddress (*(*iterator).third_party.address,
                               address_a);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&address_a[0]),
                               sizeof (ACE_UINT8[NET_ADDRESS_IPV6_ADDRESS_BYTES]));
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_PREFER_FAILURE:
        break;
      case PCP_Codes::PCP_OPTION_FILTER:
      {
        result =
          message_inout->copy (reinterpret_cast<const char*> (&(*iterator).filter.reserved),
                               sizeof (ACE_UINT8));
        if (result == -1)
          goto error;
        result =
          message_inout->copy (reinterpret_cast<const char*> (&(*iterator).filter.prefix_length),
                               sizeof (ACE_UINT8));
        if (result == -1)
          goto error;
        short_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD ((*iterator).filter.remote_peer_port)
                                                 : (*iterator).filter.remote_peer_port);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&short_i),
                                sizeof (ACE_UINT16));
        if (result == -1)
          goto error;
        PCP_Tools::mapAddress (*(*iterator).filter.remote_peer_address,
                               address_a);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&address_a[0]),
                               sizeof (ACE_UINT8[NET_ADDRESS_IPV6_ADDRESS_BYTES]));
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_NONCE:
      {
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).nonce.nonce)
                                                 : (*iterator).nonce.nonce);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_AUTHENTICATION_TAG:
      {
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).authentication_tag.session_id)
                                                 : (*iterator).authentication_tag.session_id);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).authentication_tag.sequence_number)
                                                 : (*iterator).authentication_tag.sequence_number);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).authentication_tag.key_id)
                                                 : (*iterator).authentication_tag.key_id);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        result =
          message_inout->copy (reinterpret_cast<const char*> ((*iterator).authentication_tag.data),
                               (*iterator).length - 12);
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_PA_AUTHENTICATION_TAG:
      {
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).pa_authentication_tag.key_id)
                                                 : (*iterator).pa_authentication_tag.key_id);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        result =
          message_inout->copy (reinterpret_cast<const char*> ((*iterator).pa_authentication_tag.data),
                               (*iterator).length - 4);
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_EAP_PAYLOAD:
      {
        result =
          message_inout->copy (reinterpret_cast<const char*> ((*iterator).eap_payload.data),
                               (*iterator).length);
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_PSEUDO_RANDOM_FUNCTION:
      {
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).pseudo_random_function.id)
                                                 : (*iterator).pseudo_random_function.id);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_MAC_ALGORITHM:
      {
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).mac_algorithm.id)
                                                 : (*iterator).mac_algorithm.id);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_SESSION_LIFETIME:
      {
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).session_lifetime.lifetime)
                                                 : (*iterator).session_lifetime.lifetime);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_RECEIVED_PAK:
      {
        int_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ((*iterator).received_pak.sequence_number)
                                                 : (*iterator).received_pak.sequence_number);
        result =
          message_inout->copy (reinterpret_cast<const char*> (&int_i),
                               sizeof (ACE_UINT32));
        if (result == -1)
          goto error;
        break;
      }
      case PCP_Codes::PCP_OPTION_ID_INDICATOR:
      {
        result =
          message_inout->copy (reinterpret_cast<const char*> ((*iterator).id_indicator.data),
                               (*iterator).length);
        if (result == -1)
          goto error;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown option code (was: %d), aborting\n"),
                    (*iterator).code));
        goto error;
      }
    } // end SWITCH
  } // end FOR

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("[%u]: streamed [%u byte(s)]...\n"),
  //            message_inout->getID (),
  //            message_inout->length ()));

  return;

error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", continuing\n")));
}
