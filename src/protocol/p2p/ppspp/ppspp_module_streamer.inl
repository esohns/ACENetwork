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

#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include "net_macros.h"

#include "ppspp_common.h"
#include "ppspp_defines.h"
#include "ppspp_tools.h"

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
                       SessionMessageType>::HTTP_Module_Streamer_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Streamer_T::HTTP_Module_Streamer_T"));

}

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
                       SessionMessageType>::~HTTP_Module_Streamer_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Streamer_T::~HTTP_Module_Streamer_T"));

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
  ACE_ASSERT (message_inout->length () == 0);

  // serialize structured data
  // --> create the appropriate bytestream corresponding to its elements
  const typename DataMessageType::DATA_T& data_container_r =
      message_inout->get ();
  const typename DataMessageType::DATA_T::DATA_T& data_r =
        data_container_r.get ();
  ACE_ASSERT (data_r.HTTPRecord);
  std::string buffer;
  bool is_request = true;
  std::ostringstream converter;
  HTTP_HeadersIterator_t iterator;
  std::string content_buffer;
  // *TODO*: remove type inferences
  if (HTTP_Tools::isRequest (*data_r.HTTPRecord))
  {
    buffer = HTTP_Tools::Method2String (data_r.HTTPRecord->method);
    buffer += ACE_TEXT_ALWAYS_CHAR (" ");
    buffer += data_r.HTTPRecord->URI;
    buffer += ACE_TEXT_ALWAYS_CHAR (" ");
    buffer += ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
    buffer += HTTP_Tools::Version2String (data_r.HTTPRecord->version);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");

    // prepare content ?
    if (data_r.HTTPRecord->method == HTTP_Codes::HTTP_METHOD_POST)
    {
      for (HTTP_FormIterator_t iterator_2 = data_r.HTTPRecord->form.begin ();
           iterator_2 != data_r.HTTPRecord->form.end ();
           ++iterator_2)
      {
        content_buffer += (*iterator_2).first;
        content_buffer += ACE_TEXT_ALWAYS_CHAR ("=");
        content_buffer += (*iterator_2).second;
        content_buffer += ACE_TEXT_ALWAYS_CHAR ("&");
      } // end FOR
      content_buffer.erase (--content_buffer.end ());
      converter << content_buffer.size ();

      iterator =
        data_r.HTTPRecord->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
      if (iterator == data_r.HTTPRecord->headers.end ())
        data_r.HTTPRecord->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING),
                                                           converter.str ()));
      else
        (*iterator).second = converter.str ();
      iterator =
        data_r.HTTPRecord->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_TYPE_STRING));
      if (iterator == data_r.HTTPRecord->headers.end ())
        data_r.HTTPRecord->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_TYPE_STRING),
          ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_MIMETYPE_WWWURLENCODING_STRING)));
      else
        (*iterator).second = ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_MIMETYPE_WWWURLENCODING_STRING);
    } // end IF
  } // end IF
  else
  {
    is_request = false;

    buffer = ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
    buffer += HTTP_Tools::Version2String (data_r.HTTPRecord->version);
    buffer += ACE_TEXT_ALWAYS_CHAR (" ");
    converter << data_r.HTTPRecord->status;
    buffer += converter.str ();
    buffer += ACE_TEXT_ALWAYS_CHAR (" ");
    buffer += HTTP_Tools::Status2Reason (data_r.HTTPRecord->status);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");
  } // end ELSE

  // insert headers
  for (iterator = data_r.HTTPRecord->headers.begin ();
       iterator != data_r.HTTPRecord->headers.end ();
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

  for (iterator = data_r.HTTPRecord->headers.begin ();
       iterator != data_r.HTTPRecord->headers.end ();
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

  for (iterator = data_r.HTTPRecord->headers.begin ();
       iterator != data_r.HTTPRecord->headers.end ();
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
                ACE_TEXT ("[%u]: not enough buffer space (was: %d/%d), aborting\n"),
                message_inout->id (),
                message_inout->space (), buffer.size ()));

    // clean up
    passMessageDownstream_out = false;
    message_inout->release ();
    message_inout = NULL;

    return;
  } // end IF

  result = message_inout->copy (buffer.c_str (),
                                buffer.size ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));

    // clean up
    passMessageDownstream_out = false;
    message_inout->release ();
    message_inout = NULL;

    return;
  } // end IF

  // insert content ?
  if (content_buffer.empty ())
    goto continue_;

  // sanity check
  if (message_inout->space () < content_buffer.size ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("[%u]: not enough buffer space (was: %d/%d), aborting\n"),
                message_inout->id (),
                message_inout->space (), content_buffer.size ()));

    // clean up
    passMessageDownstream_out = false;
    message_inout->release ();
    message_inout = NULL;

    return;
  } // end IF

  result = message_inout->copy (content_buffer.c_str (),
                                content_buffer.size ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));

    // clean up
    passMessageDownstream_out = false;
    message_inout->release ();
    message_inout = NULL;

    return;
  } // end IF

continue_:
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: streamed [%u byte(s)]...\n"),
//               message_inout->id (),
//               message_inout->length ()));

  return;
}
