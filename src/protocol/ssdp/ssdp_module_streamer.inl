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

#include "net_macros.h"

#include "http_defines.h"
#include "http_tools.h"

#include "ssdp_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
SSDP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       SessionMessageType>::SSDP_Module_Streamer_T (ISTREAM_T* stream_in)
#else
                       SessionMessageType>::SSDP_Module_Streamer_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Module_Streamer_T::SSDP_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
SSDP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Module_Streamer_T::handleDataMessage"));

  int result = -1;

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
  const typename DataMessageType::DATA_T& data_container_r =
      message_inout->getR ();
  typename DataMessageType::DATA_T::DATA_T& data_r =
        const_cast<typename DataMessageType::DATA_T::DATA_T&> (data_container_r.getR ());
  std::string buffer;
  std::ostringstream converter;
  HTTP_HeadersIterator_t iterator;
  std::string content_buffer;
  // *TODO*: remove type inferences
  struct HTTP_Record& record_r = static_cast<struct HTTP_Record&> (data_r);
  ACE_ASSERT (HTTP_Tools::isRequest (record_r));

  buffer =
    SSDP_Tools::MethodToString (static_cast<SSDP_Method_t> (record_r.method));
  buffer += ACE_TEXT_ALWAYS_CHAR (" ");
  buffer += record_r.URI;

  switch (record_r.method)
  {
    case SSDP_Codes::SSDP_METHOD_M_SEARCH:
    {
      buffer += ACE_TEXT_ALWAYS_CHAR (" ");
      buffer += ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
      buffer += HTTP_Tools::VersionToString (record_r.version);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown HTTP method (was: \"%s\"), continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (HTTP_Tools::MethodToString (record_r.method).c_str ())));
      break;
    }
  } // end SWITCH

  buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");

  // insert headers
  for (iterator = record_r.headers.begin ();
       iterator != record_r.headers.end ();
       ++iterator)
  {
    buffer += (*iterator).first;
    buffer += ACE_TEXT_ALWAYS_CHAR (": ");
    buffer += (*iterator).second;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");
  } // end FOR

  buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");

  // sanity check
  if (message_inout->space () < buffer.size ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: [%u]: not enough buffer space (was: %d/%d), aborting\n"),
                inherited::mod_->name (),
                message_inout->id (),
                message_inout->space (), buffer.size ()));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF

  result = message_inout->copy (buffer.c_str (),
                                buffer.size ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF

//continue_:
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: streamed [%u byte(s)]...\n"),
//               message_inout->id (),
//               message_inout->length ()));

  return;
}
