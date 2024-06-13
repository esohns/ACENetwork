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

#include "ftp_common.h"
#include "ftp_defines.h"
#include "ftp_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
FTP_Module_Streamer_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType>::FTP_Module_Streamer_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Streamer_T::FTP_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
FTP_Module_Streamer_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Streamer_T::handleDataMessage"));

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
    case FTP_Codes::FTP_COMMAND_USER:
    { ACE_ASSERT (!data_r.request.parameters.empty ());
      text_string = ACE_TEXT_ALWAYS_CHAR ("USER ");
      text_string += data_r.request.parameters.front ();
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_PASS:
    { ACE_ASSERT (!data_r.request.parameters.empty ());
      text_string = ACE_TEXT_ALWAYS_CHAR ("PASS ");
      text_string += data_r.request.parameters.front ();
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_CWD:
    { ACE_ASSERT (!data_r.request.parameters.empty ());
      text_string = ACE_TEXT_ALWAYS_CHAR ("CWD ");
      text_string += data_r.request.parameters.front ();
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_PORT:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("PORT");
      ACE_ASSERT (false); // *TODO*
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_PASV:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("PASV");
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_RETR:
    { ACE_ASSERT (!data_r.request.parameters.empty ());
      text_string = ACE_TEXT_ALWAYS_CHAR ("RETR ");
      text_string += data_r.request.parameters.front ();
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_LIST:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("LIST");
      if (!data_r.request.parameters.empty ())
      {
        text_string = ACE_TEXT_ALWAYS_CHAR (" ");
        text_string += data_r.request.parameters.front ();
      } // end IF
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_NLST:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("NLST");
      if (!data_r.request.parameters.empty ())
      {
        text_string = ACE_TEXT_ALWAYS_CHAR (" ");
        text_string += data_r.request.parameters.front ();
      } // end IF
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    case FTP_Codes::FTP_COMMAND_QUIT:
    {
      text_string = ACE_TEXT_ALWAYS_CHAR ("QUIT");
      text_string += ACE_TEXT_ALWAYS_CHAR ("\r\n");
      result = message_inout->copy (text_string.c_str (),
                                    text_string.size ());
      if (result == -1)
        goto error;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown FTP command (was: \"%s\"), returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (FTP_Tools::CommandToString (data_r.request.command).c_str ())));
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
