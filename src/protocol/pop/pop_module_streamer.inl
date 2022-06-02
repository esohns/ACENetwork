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

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "pop_common.h"
#include "pop_defines.h"
#include "pop_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
POP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       SessionMessageType>::POP_Module_Streamer_T (ISTREAM_T* stream_in)
#else
                       SessionMessageType>::POP_Module_Streamer_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("POP_Module_Streamer_T::POP_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
POP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("POP_Module_Streamer_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  // *NOTE*: as this is an "upstream" module, the "wording" is wrong
  //         --> the logic remains the same, though
  passMessageDownstream_out = true;

  const typename DataMessageType::DATA_T& data_container_r =
      message_inout->getR ();
  const typename DataMessageType::DATA_T::DATA_T& data_r =
    data_container_r.getR ();

  int result = -1;
  std::string text_string;
  switch (data_r.request.command)
  {
    case POP_Codes::POP_COMMAND_AUTH_USER:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("USER ");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      result =
        message_inout->copy (data_r.request.parameters.front ().c_str (),
                             data_r.request.parameters.front ().size ());
      if (result == -1)
        goto error;
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                    2);
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_AUTH_PASS:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("PASS ");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      result =
        message_inout->copy (data_r.request.parameters.front ().c_str (),
                             data_r.request.parameters.front ().size ());
      if (result == -1)
        goto error;
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                    2);
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_AUTH_APOP:
    { ACE_ASSERT (false); // *TODO*
      break;
    }
    case POP_Codes::POP_COMMAND_AUTH_QUIT:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("QUIT\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_STAT:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("STAT\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_LIST:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("LIST");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      if (!data_r.request.parameters.empty ())
      {
        result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR (" "),
                                      1);
        if (result == -1)
          goto error;
        result =
          message_inout->copy (data_r.request.parameters.front ().c_str (),
                               data_r.request.parameters.front ().size ());
        if (result == -1)
          goto error;
      } // end IF
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                    2);
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_RETR:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("RETR ");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      result =
        message_inout->copy (data_r.request.parameters.front ().c_str (),
                             data_r.request.parameters.front ().size ());
      if (result == -1)
        goto error;
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                    2);
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_DELE:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("DELE ");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      result =
        message_inout->copy (data_r.request.parameters.front ().c_str (),
                             data_r.request.parameters.front ().size ());
      if (result == -1)
        goto error;
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                    2);
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_NOOP:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("NOOP\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_RSET:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("RSET\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_QUIT:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("QUIT\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_TOP:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("TOP ");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      POP_ParametersConstIterator_t iterator =
        data_r.request.parameters.begin ();
      result = message_inout->copy ((*iterator).c_str (),
                                    (*iterator).size ());
      if (result == -1)
        goto error;
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR (" "),
                                    1);
      if (result == -1)
        goto error;
      ++iterator;
      result = message_inout->copy ((*iterator).c_str (),
                                    (*iterator).size ());
      if (result == -1)
        goto error;
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                    2);
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_TRANS_UIDL:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("UIDL");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      if (!data_r.request.parameters.empty ())
      {
        result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR (" "),
                                      1);
        if (result == -1)
          goto error;
        result =
          message_inout->copy (data_r.request.parameters.front ().c_str (),
                               data_r.request.parameters.front ().size ());
        if (result == -1)
          goto error;
      } // end IF
      result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                    2);
      if (result == -1)
        goto error;
      break;
    }
    case POP_Codes::POP_COMMAND_UPDAT_QUIT:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("QUIT\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown POP command (was: \"%s\"), returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (POP_Tools::CommandToString (data_r.request.command).c_str ())));
      return;
    }
  } // end SWITCH

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("[%u]: streamed [%u byte(s)]...\n"),
  //            message_inout->getID (),
  //            message_inout->length ()));

  return;

error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
              inherited::mod_->name ()));
}
