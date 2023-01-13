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
#include "stdafx.h"

#include "ftp_tools.h"

#include <sstream>

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "ftp_codes.h"
#include "ftp_defines.h"

std::string
FTP_Tools::dump (const struct FTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Tools::dump"));

  std::ostringstream converter;
  std::string string_buffer;

  string_buffer = ACE_TEXT_ALWAYS_CHAR ("code: \t");
  string_buffer += FTP_Tools::CodeToString (record_in.code);
  string_buffer += ACE_TEXT_ALWAYS_CHAR (" (");
  converter << record_in.code;
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR (")\n");
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("text: (");
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << record_in.text.size ();
  string_buffer += converter.str ();
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");
  for (FTP_TextConstIterator_t iterator = record_in.text.begin ();
       iterator != record_in.text.end ();
       ++iterator)
  {
    string_buffer += *iterator;
    string_buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end FOR
  string_buffer += ACE_TEXT_ALWAYS_CHAR ("text /END");

  return string_buffer;
}

std::string
FTP_Tools::CodeToString (FTP_Code_t code_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Tools::CodeToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (code_in)
  {
    case FTP_Codes::FTP_CODE_SYSTEM_STATUS_HELP_REPLY:
      result = ACE_TEXT_ALWAYS_CHAR ("SYSTEM_STATUS_HELP_REPLY"); break;
    case FTP_Codes::FTP_CODE_HELP_MESSAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("HELP_MESSAGE"); break;
    case FTP_Codes::FTP_CODE_SERVICE_READY:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVICE_READY"); break;
    case FTP_Codes::FTP_CODE_SERVICE_CLOSING_CHANNEL:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVICE_CLOSING_CHANNEL"); break;
    case FTP_Codes::FTP_CODE_AUTHENTICATION_SUCCEEDED:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_SUCCEEDED"); break;
    case FTP_Codes::FTP_CODE_MAIL_COMPLETE:
      result = ACE_TEXT_ALWAYS_CHAR ("MAIL_COMPLETE"); break;
    case FTP_Codes::FTP_CODE_USER_NOT_LOCAL_WILL_FORWARD:
      result = ACE_TEXT_ALWAYS_CHAR ("USER_NOT_LOCAL_WILL_FORWARD"); break;
    case FTP_Codes::FTP_CODE_CANNOT_VRFY_USER_WILL_CONTINUE:
      result = ACE_TEXT_ALWAYS_CHAR ("CANNOT_VRFY_USER_WILL_CONTINUE"); break;
    case FTP_Codes::FTP_CODE_SERVER_CHALLENGE:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVER_CHALLENGE"); break;
    case FTP_Codes::FTP_CODE_START_MAIL_INPUT:
      result = ACE_TEXT_ALWAYS_CHAR ("START_MAIL_INPUT"); break;
    case FTP_Codes::FTP_CODE_SERVICE_NOT_AVAILABLE_CLOSING_CHANNEL:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVICE_NOT_AVAILABLE_CLOSING_CHANNEL"); break;
    case FTP_Codes::FTP_CODE_PASSWORD_TRANSITION_NEEDED:
      result = ACE_TEXT_ALWAYS_CHAR ("PASSWORD_TRANSITION_NEEDED"); break;
    case FTP_Codes::FTP_CODE_MAILBOX_NOT_AVAILABLE:
      result = ACE_TEXT_ALWAYS_CHAR ("MAILBOX_NOT_AVAILABLE"); break;
    case FTP_Codes::FTP_CODE_PROCESSING_ERROR_ABORTED:
      result = ACE_TEXT_ALWAYS_CHAR ("PROCESSING_ERROR_ABORTED"); break;
    case FTP_Codes::FTP_CODE_PROCESSING_ERROR_SYSTEM_STORAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("PROCESSING_ERROR_SYSTEM_STORAGE"); break;
    case FTP_Codes::FTP_CODE_TEMPORARY_AUTHENTICATION_FAILURE:
      result = ACE_TEXT_ALWAYS_CHAR ("TEMPORARY_AUTHENTICATION_FAILURE"); break;
    case FTP_Codes::FTP_CODE_ERROR_SYNTAX_COMMAND:
      result = ACE_TEXT_ALWAYS_CHAR ("SYNTAX_COMMAND"); break;
    case FTP_Codes::FTP_CODE_ERROR_SYNTAX_ARGUMENT:
      result = ACE_TEXT_ALWAYS_CHAR ("SYNTAX_ARGUMENT"); break;
    case FTP_Codes::FTP_CODE_ERROR_COMMAND_NOT_IMPLEMENTED:
      result = ACE_TEXT_ALWAYS_CHAR ("COMMAND_NOT_IMPLEMENTED"); break;
    case FTP_Codes::FTP_CODE_ERROR_COMMAND_BAD_SEQUENCE:
      result = ACE_TEXT_ALWAYS_CHAR ("COMMAND_BAD_SEQUENCE"); break;
    case FTP_Codes::FTP_CODE_ERROR_COMMAND_ARGUMENT_NOT_IMPLEMENTED:
      result = ACE_TEXT_ALWAYS_CHAR ("COMMAND_ARGUMENT_NOT_IMPLEMENTED"); break;
    case FTP_Codes::FTP_CODE_HOST_DOES_NOT_ACCEPT_MAIL:
      result = ACE_TEXT_ALWAYS_CHAR ("HOST_DOES_NOT_ACCEPT_MAIL"); break;
    case FTP_Codes::FTP_CODE_AUTHENTICATION_REQUIRED:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_REQUIRED"); break;
    case FTP_Codes::FTP_CODE_AUTHENTICATION_MECHANISM_TOO_WEAK:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_MECHANISM_TOO_WEAK"); break;
    case FTP_Codes::FTP_CODE_AUTHENTICATION_CREDENTIALS_INVALID:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTHENTICATION_CREDENTIALS_INVALID"); break;
    case FTP_Codes::FTP_CODE_ENCRYPTION_REQUIRED_FOR_AUTHENTICATION_MECHANISM:
      result = ACE_TEXT_ALWAYS_CHAR ("ENCRYPTION_REQUIRED_FOR_AUTHENTICATION_MECHANISM"); break;
    case FTP_Codes::FTP_CODE_ERROR_MAILBOX_UNAVAILABLE:
      result = ACE_TEXT_ALWAYS_CHAR ("MAILBOX_UNAVAILABLE"); break;
    case FTP_Codes::FTP_CODE_ERROR_USER_NOT_LOCAL_TRY_FORWARD:
      result = ACE_TEXT_ALWAYS_CHAR ("USER_NOT_LOCAL_TRY_FORWARD"); break;
    case FTP_Codes::FTP_CODE_ERROR_EXCEEDED_SYSTEM_STORAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("EXCEEDED_SYSTEM_STORAGE"); break;
    case FTP_Codes::FTP_CODE_ERROR_MAILBOX_NAME_NOT_ALLOWED:
      result = ACE_TEXT_ALWAYS_CHAR ("MAILBOX_NAME_NOT_ALLOWED"); break;
    case FTP_Codes::FTP_CODE_ERROR_TRANSACTION_FAILED:
      result = ACE_TEXT_ALWAYS_CHAR ("TRANSACTION_FAILED"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown code (was: %d), aborting\n"),
                  code_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
FTP_Tools::CommandToString (FTP_Command_t command_in)
{
  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (command_in)
  {
    case FTP_Codes::FTP_COMMAND_EHLO:
      result = ACE_TEXT_ALWAYS_CHAR ("EHLO"); break;
    case FTP_Codes::FTP_COMMAND_MAIL:
      result = ACE_TEXT_ALWAYS_CHAR ("MAIL"); break;
    case FTP_Codes::FTP_COMMAND_RCPT:
      result = ACE_TEXT_ALWAYS_CHAR ("RCPT"); break;
    case FTP_Codes::FTP_COMMAND_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA"); break;
    case FTP_Codes::FTP_COMMAND_QUIT:
      result = ACE_TEXT_ALWAYS_CHAR ("QUIT"); break;
    //////////////////////////////////////
    case FTP_Codes::FTP_COMMAND_AUTH:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTH"); break;
    //////////////////////////////////////
    case FTP_Codes::FTP_COMMAND_DATA_2:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA_2"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown command (was: %d), aborting\n"),
                  command_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
FTP_Tools::StateToString (enum FTP_ProtocolState state_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Tools::StateToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (state_in)
  {
    case FTP_STATE_GREETING_RECEIVED:
      result = ACE_TEXT_ALWAYS_CHAR ("GREETING_RECEIVED"); break;
    case FTP_STATE_EHLO_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("EHLO_SENT"); break;
    case FTP_STATE_AUTH_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTH_SENT"); break;
    case FTP_STATE_AUTH_LOGIN_USER_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTH_LOGIN_USER_SENT"); break;
    case FTP_STATE_AUTH_LOGIN_PASSWORD_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTH_LOGIN_PASSWORD_SENT"); break;
    case FTP_STATE_AUTH_COMPLETE:
      result = ACE_TEXT_ALWAYS_CHAR ("AUTH_COMPLETE"); break;
    case FTP_STATE_MAIL_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("MAIL_SENT"); break;
    case FTP_STATE_RCPT_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("RCPT_SENT"); break;
    case FTP_STATE_RCPTS_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("RCPTS_SENT"); break;
    case FTP_STATE_DATA_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA_SENT"); break;
    case FTP_STATE_DATA_2_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA_2_SENT"); break;
    case FTP_STATE_QUIT_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("QUIT_SENT"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown state (was: %d), aborting\n"),
                  state_in));
      break;
    }
  } // end SWITCH

  return result;
}
