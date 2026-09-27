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

#include <sstream>
#include <string>

#include "ace/Log_Msg.h"

#include "stream_dec_common.h"

#include "net_macros.h"

#include "ws_common.h"
#include "ws_defines.h"
#include "ws_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
WebSocket_Module_Streamer_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType>::WebSocket_Module_Streamer_T (typename inherited::TASK_BASE_T::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , handshakeComplete_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Streamer_T::WebSocket_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
WebSocket_Module_Streamer_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Streamer_T::handleDataMessage"));

  if (unlikely (!handshakeComplete_))
  { handshakeComplete_ = true;
    inherited::handleDataMessage (message_inout,
                                  passMessageDownstream_out);
    return;
  } // end IF

  // don't care (implies yes per default, if part of a stream)
  // *NOTE*: as this is an "upstream" module, the "wording" is wrong
  //         --> the logic remains the same, though
  passMessageDownstream_out = true;

  // sanity check(s)
  // *TODO*: in scenarios where message streamers are head-module siblings to
  //         the message parsers (standard marshalling functionality), any
  //         downstream statistic module will not be able to account for the
  //         outbound data byte-stream, as it has not been generated at that
  //         stage
  //         --> make the streamer sibling to any module task that generates
  //             outbound data to generate the byte stream early
  ACE_ASSERT (message_inout->length () == 0);

  // serialize structured data
  // --> create the appropriate bytestream corresponding to its elements
  int result;
  const typename DataMessageType::DATA_T& data_container_r =
    message_inout->getR ();
  typename DataMessageType::DATA_T::DATA_T& data_r =
    const_cast<typename DataMessageType::DATA_T::DATA_T&> (data_container_r.getR ());
  // *TODO*: remove type inferences
  struct WebSocket_Record& record_r = static_cast<struct WebSocket_Record&> (data_r);

  std::vector<uint8_t> buffer_a;
  uint8_t value_i = 0x80; // set "FIN" bit
  value_i |= record_r.opcode & 0x0F; // 4 bits
  buffer_a.push_back (value_i);
  
  value_i = 0x80; // set "masked" bit
  if (record_r.payloadSize <= 125)
  {
    value_i |= record_r.payloadSize & 0x7F; // 7 bits
    buffer_a.push_back (value_i);
  } // end IF
  else if (record_r.payloadSize <= Common_Tools::max<ACE_UINT64> (2, false))
  {
    value_i |= 0x7E; // 126
    buffer_a.push_back (value_i);
    uint16_t payload_size_i = record_r.payloadSize;
    buffer_a.push_back ((payload_size_i & 0xFF00) >> 8);
    buffer_a.push_back (payload_size_i & 0x00FF);
  } // end ELSE IF
  else
  {
    value_i |= 0x7F; // 127
    buffer_a.push_back (value_i);
    buffer_a.push_back ((record_r.payloadSize & 0xFF00000000000000) >> 56);
    buffer_a.push_back ((record_r.payloadSize & 0x00FF000000000000) >> 48);
    buffer_a.push_back ((record_r.payloadSize & 0x0000FF0000000000) >> 40);
    buffer_a.push_back ((record_r.payloadSize & 0x000000FF00000000) >> 32);
    buffer_a.push_back ((record_r.payloadSize & 0x00000000FF000000) >> 24);
    buffer_a.push_back ((record_r.payloadSize & 0x0000000000FF0000) >> 16);
    buffer_a.push_back ((record_r.payloadSize & 0x000000000000FF00) >> 8);
    buffer_a.push_back (record_r.payloadSize & 0x00000000000000FF);
  } // end ELSE

  static std::uniform_int_distribution<uint32_t> uniform_uint32_distribution;
  uint32_t masking_key_i =
    Common_Tools::getRandomNumber (uniform_uint32_distribution);
  buffer_a.push_back ((masking_key_i & 0xFF000000) >> 24);
  buffer_a.push_back ((masking_key_i & 0x00FF0000) >> 16);
  buffer_a.push_back ((masking_key_i & 0x0000FF00) >> 8);
  buffer_a.push_back (masking_key_i & 0x000000FF);

  // sanity check
  if (unlikely (message_inout->space () < buffer_a.size ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: [%u]: not enough buffer space (was: %B/%B), aborting\n"),
                inherited::mod_->name (),
                message_inout->id (),
                message_inout->space (), buffer_a.size ()));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF

  result = message_inout->copy (reinterpret_cast<char*> (buffer_a.data ()),
                                buffer_a.size ());
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF

  // insert content ?
  std::vector<uint8_t> payload_buffer_a;
  std::vector<uint8_t> masking_key_a;
  masking_key_a.push_back ((masking_key_i & 0xFF000000) >> 24);
  masking_key_a.push_back ((masking_key_i & 0x00FF0000) >> 16);
  masking_key_a.push_back ((masking_key_i & 0x0000FF00) >> 8);
  masking_key_a.push_back (masking_key_i & 0x000000FF);
  uint8_t* data_p =
    record_r.opcode == WebSocket_Codes::OPCODE_TEXT ? reinterpret_cast<uint8_t*> (record_r.payload.string)
                                                    : record_r.payload.blob;

  if (record_r.payloadSize == 0)
    goto continue_;

  // XOR the payload data with the masking key
  ACE_ASSERT (data_p);
  for (ACE_UINT64 i = 0;
       i < record_r.payloadSize;
       ++i)
  {
    payload_buffer_a.push_back (*data_p ^ masking_key_a[i % 4]);
    ++data_p;
  } // end FOR

  // sanity check
  if (message_inout->space () < payload_buffer_a.size ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: [%u]: not enough buffer space (was: %B/%B), aborting\n"),
                inherited::mod_->name (),
                message_inout->id (),
                message_inout->space (), payload_buffer_a.size ()));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF

  result = message_inout->copy (reinterpret_cast<char*> (payload_buffer_a.data ()),
                                payload_buffer_a.size ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF

continue_:
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: streamed [%u byte(s)]...\n"),
//               message_inout->id (),
//               message_inout->length ()));

  return;
}
