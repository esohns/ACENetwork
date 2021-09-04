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

#include "smtp_common.h"
#include "smtp_defines.h"
#include "smtp_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
SMTP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       SessionMessageType>::SMTP_Module_Streamer_T (ISTREAM_T* stream_in)
#else
                       SessionMessageType>::SMTP_Module_Streamer_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Streamer_T::SMTP_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
SMTP_Module_Streamer_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("SMTP_Module_Streamer_T::handleDataMessage"));

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
    case SMTP_Codes::SMTP_COMMAND_EHLO:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("EHLO [");
      text_string +=
        Net_Common_Tools::IPAddressToString (data_r.request.domain, true, false);
      text_string += ACE_TEXT_ALWAYS_CHAR ("]\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case SMTP_Codes::SMTP_COMMAND_MAIL:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("MAIL FROM:<");
      text_string += data_r.request.from;
      text_string += ACE_TEXT_ALWAYS_CHAR (">\r\n");
      // *TODO*: support 'Mail-parameters'
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case SMTP_Codes::SMTP_COMMAND_RCPT:
    { ACE_ASSERT (!data_r.request.to.empty ());
      text_string = ACE_TEXT_ALWAYS_CHAR ("RCPT TO:<");
      text_string += data_r.request.to.front ();
      text_string += ACE_TEXT_ALWAYS_CHAR (">\r\n");
      // *TODO*: support 'Rcpt-parameters'
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case SMTP_Codes::SMTP_COMMAND_DATA:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("DATA\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case SMTP_Codes::SMTP_COMMAND_QUIT:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("QUIT\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case SMTP_Codes::SMTP_COMMAND_AUTH:
    {
      if (data_r.request.parameters.empty ())
      { // --> send initial AUTH
        text_string = ACE_TEXT_ALWAYS_CHAR ("AUTH LOGIN\r\n");
        result = message_inout->copy (text_string.c_str (),
                                      text_string.size ());
        if (result == -1)
          goto error;
      } // end IF
      else
      { // --> send username/password
        result =
          message_inout->copy (data_r.request.parameters.front ().c_str (),
                               data_r.request.parameters.front ().size ());
        if (result == -1)
          goto error;
        result = message_inout->copy (ACE_TEXT_ALWAYS_CHAR ("\r\n"),
                                      2);
        if (result == -1)
          goto error;
      } // end ELSE
      break;
    }
    case SMTP_Codes::SMTP_COMMAND_DATA_2:
    {
      std::string end_sequence = ACE_TEXT_ALWAYS_CHAR ("\r\n.\r\n");
      // sanity check(s)
      std::string::size_type position =
        data_r.request.data.find (end_sequence, std::string::npos);
      if (unlikely (position != std::string::npos))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: illegal message data (contained end-of-message sequence at: %d), cropping and continuing\n"),
                    inherited::mod_->name (),
                    position));
        text_string = data_r.request.data;
        text_string.erase (position, std::string::npos);
      } // end IF
      else
        text_string = data_r.request.data;
      text_string += end_sequence;
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown SMTP command (was: \"%s\"), returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (SMTP_Tools::CommandToString (data_r.request.command).c_str ())));
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
