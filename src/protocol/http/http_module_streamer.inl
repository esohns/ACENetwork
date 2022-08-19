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

#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       SessionMessageType>::HTTP_Module_Streamer_T (ISTREAM_T* stream_in)
#else
                       SessionMessageType>::HTTP_Module_Streamer_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Streamer_T::HTTP_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
HTTP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Streamer_T::handleDataMessage"));

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
  bool is_request = true;
  std::ostringstream converter;
  HTTP_HeadersIterator_t iterator;
  std::string content_buffer;
  // *TODO*: remove type inferences
  struct HTTP_Record& record_r = static_cast<struct HTTP_Record&> (data_r);
  if (HTTP_Tools::isRequest (record_r))
  {
    buffer = HTTP_Tools::MethodToString (record_r.method);
    buffer += ACE_TEXT_ALWAYS_CHAR (" ");
    buffer += record_r.URI;

    // process form data, if any
    switch (record_r.method)
    {
      case HTTP_Codes::HTTP_METHOD_GET:
      {
        if (!record_r.form.empty ())
          buffer += ACE_TEXT_ALWAYS_CHAR ("?");
        for (HTTP_FormIterator_t iterator_2 = record_r.form.begin ();
             iterator_2 != record_r.form.end ();
             ++iterator_2)
        {
          buffer += (*iterator_2).first;
          buffer += ACE_TEXT_ALWAYS_CHAR ("=");
          buffer += (*iterator_2).second;
          buffer += ACE_TEXT_ALWAYS_CHAR ("&");
        } // end FOR
        if (!record_r.form.empty ())
          buffer.erase (--buffer.end ());

        buffer += ACE_TEXT_ALWAYS_CHAR (" ");
        buffer += ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
        buffer += HTTP_Tools::VersionToString (record_r.version);

        break;
      }
      case HTTP_Codes::HTTP_METHOD_POST:
      {
        buffer += ACE_TEXT_ALWAYS_CHAR (" ");
        buffer += ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
        buffer += HTTP_Tools::VersionToString (record_r.version);

        if (record_r.form.empty ())
          break;

        for (HTTP_FormIterator_t iterator_2 = record_r.form.begin ();
             iterator_2 != record_r.form.end ();
             ++iterator_2)
        {
          content_buffer += (*iterator_2).first;
          content_buffer += ACE_TEXT_ALWAYS_CHAR ("=");
          content_buffer += (*iterator_2).second;
          content_buffer += ACE_TEXT_ALWAYS_CHAR ("&");
        } // end FOR
        content_buffer.erase (--content_buffer.end ());

        // add content-length header ?
        converter << content_buffer.size ();
        iterator =
            record_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
        if (iterator == record_r.headers.end ())
          record_r.headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING),
                                                   converter.str ()));
        else
        { ACE_ASSERT ((*iterator).second.empty ()); // *TODO*: allow '0' ?
          (*iterator).second = converter.str ();
        } // end ELSE
        iterator =
            record_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_TYPE_STRING));
        if (iterator == record_r.headers.end ())
          record_r.headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_TYPE_STRING),
                                                   ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_MIMETYPE_WWWURLENCODING_STRING)));
        else
        { ACE_ASSERT ((*iterator).second.empty ());
          (*iterator).second =
            ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_MIMETYPE_WWWURLENCODING_STRING);
        } // end ELSE

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
  } // end IF
  else
  {
    is_request = false;

    buffer = ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
    buffer += HTTP_Tools::VersionToString (record_r.version);
    buffer += ACE_TEXT_ALWAYS_CHAR (" ");
    converter << record_r.status;
    buffer += converter.str ();
    buffer += ACE_TEXT_ALWAYS_CHAR (" ");
    buffer += HTTP_Tools::StatusToReason (record_r.status);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");
  } // end ELSE

  // insert headers
  for (iterator = record_r.headers.begin ();
       iterator != record_r.headers.end ();
       ++iterator)
  {
    if (!HTTP_Tools::isHeaderType ((*iterator).first,
                                   HTTP_Codes::HTTP_HEADER_GENERAL))
      continue;

    buffer += (*iterator).first;
    buffer += ACE_TEXT_ALWAYS_CHAR (": ");
    buffer += (*iterator).second;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");
  } // end FOR

  for (iterator = record_r.headers.begin ();
       iterator != record_r.headers.end ();
       ++iterator)
  {
    if (!HTTP_Tools::isHeaderType ((*iterator).first,
                                   (is_request ? HTTP_Codes::HTTP_HEADER_REQUEST
                                               : HTTP_Codes::HTTP_HEADER_RESPONSE)))
      continue;

    buffer += (*iterator).first;
    buffer += ACE_TEXT_ALWAYS_CHAR (": ");
    buffer += (*iterator).second;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");
  } // end FOR

  for (iterator = record_r.headers.begin ();
       iterator != record_r.headers.end ();
       ++iterator)
  {
    if (!HTTP_Tools::isHeaderType ((*iterator).first,
                                   HTTP_Codes::HTTP_HEADER_ENTITY))
      continue;

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

  // insert content ?
  if (content_buffer.empty ())
    goto continue_;

  // sanity check
  if (message_inout->space () < content_buffer.size ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: [%u]: not enough buffer space (was: %d/%d), aborting\n"),
                inherited::mod_->name (),
                message_inout->id (),
                message_inout->space (), content_buffer.size ()));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF

  result = message_inout->copy (content_buffer.c_str (),
                                content_buffer.size ());
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
