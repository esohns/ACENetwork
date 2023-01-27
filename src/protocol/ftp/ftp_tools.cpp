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

#include <regex>
#include <sstream>

#include "ace/Log_Msg.h"

#include "net_common_tools.h"
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
    case FTP_Codes::FTP_CODE_RESTART_MARKER:
      result = ACE_TEXT_ALWAYS_CHAR ("RESTART_MARKER"); break;
    case FTP_Codes::FTP_CODE_SERVICE_READY_N_MINUTES:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVICE_READY_N_MINUTES"); break;
    case FTP_Codes::FTP_CODE_DATA_OPEN_TRANSFER_STARTING:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA_OPEN_TRANSFER_STARTING"); break;
    case FTP_Codes::FTP_CODE_FILE_STATUS_OK_OPENING_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("FILE_STATUS_OK_OPENING_DATA"); break;
    case FTP_Codes::FTP_CODE_OK:
      result = ACE_TEXT_ALWAYS_CHAR ("OK"); break;
    case FTP_Codes::FTP_CODE_COMMAND_SUPERFLUOUS:
      result = ACE_TEXT_ALWAYS_CHAR ("COMMAND_SUPERFLUOUS"); break;
    case FTP_Codes::FTP_CODE_SYSTEM_STATUS_OR_HELP:
      result = ACE_TEXT_ALWAYS_CHAR ("SYSTEM_STATUS_OR_HELP"); break;
    case FTP_Codes::FTP_CODE_DIRECTORY_STATUS:
      result = ACE_TEXT_ALWAYS_CHAR ("DIRECTORY_STATUS"); break;
    case FTP_Codes::FTP_CODE_FILE_STATUS:
      result = ACE_TEXT_ALWAYS_CHAR ("FILE_STATUS"); break;
    case FTP_Codes::FTP_CODE_HELP_MESSAGE:
      result = ACE_TEXT_ALWAYS_CHAR ("HELP_MESSAGE"); break;
    case FTP_Codes::FTP_CODE_NAME_SYSTEM_TYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("NAME_SYSTEM_TYPE"); break;
    case FTP_Codes::FTP_CODE_SERVICE_READY_NEW_USER:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVICE_READY_NEW_USER"); break;
    case FTP_Codes::FTP_CODE_SERVICE_CLOSING_CTL:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVICE_CLOSING_CTL"); break;
    case FTP_Codes::FTP_CODE_DATA_OPEN_NO_TRANSFER:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA_OPEN_NO_TRANSFER"); break;
    case FTP_Codes::FTP_CODE_CLOSING_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("CLOSING_DATA"); break;
    case FTP_Codes::FTP_CODE_ENTERING_PASSIVE_MODE:
      result = ACE_TEXT_ALWAYS_CHAR ("ENTERING_PASSIVE_MODE"); break;
    case FTP_Codes::FTP_CODE_USER_LOGGED_IN:
      result = ACE_TEXT_ALWAYS_CHAR ("USER_LOGGED_IN"); break;
    case FTP_Codes::FTP_CODE_FILE_ACTION_OK:
      result = ACE_TEXT_ALWAYS_CHAR ("FILE_ACTION_OK"); break;
    case FTP_Codes::FTP_CODE_PATH_CREATED_OK:
      result = ACE_TEXT_ALWAYS_CHAR ("PATH_CREATED_OK"); break;
    case FTP_Codes::FTP_CODE_USER_OK_NEED_PASSWORD:
      result = ACE_TEXT_ALWAYS_CHAR ("USER_OK_NEED_PASSWORD"); break;
    case FTP_Codes::FTP_CODE_NEED_ACCOUNT_FOR_LOGIN:
      result = ACE_TEXT_ALWAYS_CHAR ("NEED_ACCOUNT_FOR_LOGIN"); break;
    case FTP_Codes::FTP_CODE_FILE_ACTION_PENDING_FURTHER_INFORMATION:
      result = ACE_TEXT_ALWAYS_CHAR ("FILE_ACTION_PENDING_FURTHER_INFORMATION"); break;
    case FTP_Codes::FTP_CODE_SERVICE_NOT_AVAILABLE_CLOSING_CTL:
      result = ACE_TEXT_ALWAYS_CHAR ("SERVICE_NOT_AVAILABLE_CLOSING_CTL"); break;
    case FTP_Codes::FTP_CODE_CANNOT_OPEN_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("CANNOT_OPEN_DATA"); break;
    case FTP_Codes::FTP_CODE_DATA_CLOSED_TRANSFER_ABORTED:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA_CLOSED_TRANSFER_ABORTED"); break;
    case FTP_Codes::FTP_CODE_FILE_ACTION_ABORTED_FILE_UNAVAILABLE:
      result = ACE_TEXT_ALWAYS_CHAR ("FILE_ACTION_ABORTED_FILE_UNAVAILABLE"); break;
    case FTP_Codes::FTP_CODE_ACTION_ABORTED:
      result = ACE_TEXT_ALWAYS_CHAR ("ACTION_ABORTED"); break;
    case FTP_Codes::FTP_CODE_FILE_ACTION_ABORTED_NO_SPACE:
      result = ACE_TEXT_ALWAYS_CHAR ("FILE_ACTION_ABORTED_NO_SPACE"); break;
    case FTP_Codes::FTP_CODE_ERROR_SYNTAX_COMMAND:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_SYNTAX_COMMAND"); break;
    case FTP_Codes::FTP_CODE_ERROR_SYNTAX_ARGUMENT:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_SYNTAX_ARGUMENT"); break;
    case FTP_Codes::FTP_CODE_ERROR_COMMAND_NOT_IMPLEMENTED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_COMMAND_NOT_IMPLEMENTED"); break;
    case FTP_Codes::FTP_CODE_ERROR_COMMAND_BAD_SEQUENCE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_COMMAND_BAD_SEQUENCE"); break;
    case FTP_Codes::FTP_CODE_ERROR_COMMAND_ARGUMENT_NOT_IMPLEMENTED:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_COMMAND_ARGUMENT_NOT_IMPLEMENTED"); break;
    case FTP_Codes::FTP_CODE_ERROR_USER_NOT_LOGGED_IN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_USER_NOT_LOGGED_IN"); break;
    case FTP_Codes::FTP_CODE_ERROR_NEED_ACCOUNT_FOR_STORING_FILES:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_NEED_ACCOUNT_FOR_STORING_FILES"); break;
    case FTP_Codes::FTP_CODE_ERROR_FILE_ACTION_ABORTED_FILE_UNAVAILABLE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_FILE_ACTION_ABORTED_FILE_UNAVAILABLE"); break;
    case FTP_Codes::FTP_CODE_ERROR_ACTION_ABORTED_PAGE_TYPE_UNKNOWN:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_ACTION_ABORTED_PAGE_TYPE_UNKNOWN"); break;
    case FTP_Codes::FTP_CODE_ERROR_FILE_ACTION_ABORTED_NO_SPACE:
      result = ACE_TEXT_ALWAYS_CHAR ("ERROR_FILE_ACTION_ABORTED_NO_SPACE"); break;
    case FTP_Codes::FTP_CODE_ERROR_FILE_ACTION_ABORTED_FILENAME_NOT_ALLOWED:
      result = ACE_TEXT_ALWAYS_CHAR ("FILE_ACTION_ABORTED_FILENAME_NOT_ALLOWED"); break;
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
    case FTP_Codes::FTP_COMMAND_USER:
      result = ACE_TEXT_ALWAYS_CHAR ("USER"); break;
    case FTP_Codes::FTP_COMMAND_PASS:
      result = ACE_TEXT_ALWAYS_CHAR ("PASS"); break;
    case FTP_Codes::FTP_COMMAND_ACCT:
      result = ACE_TEXT_ALWAYS_CHAR ("ACCT"); break;
    case FTP_Codes::FTP_COMMAND_CWD:
      result = ACE_TEXT_ALWAYS_CHAR ("CWD"); break;
    case FTP_Codes::FTP_COMMAND_CDUP:
      result = ACE_TEXT_ALWAYS_CHAR ("CDUP"); break;
    case FTP_Codes::FTP_COMMAND_SMNT:
      result = ACE_TEXT_ALWAYS_CHAR ("SMNT"); break;
    case FTP_Codes::FTP_COMMAND_REIN:
      result = ACE_TEXT_ALWAYS_CHAR ("REIN"); break;
    case FTP_Codes::FTP_COMMAND_QUIT:
      result = ACE_TEXT_ALWAYS_CHAR ("QUIT"); break;
    //////////////////////////////////////
    case FTP_Codes::FTP_COMMAND_PORT:
      result = ACE_TEXT_ALWAYS_CHAR ("PORT"); break;
    case FTP_Codes::FTP_COMMAND_PASV:
      result = ACE_TEXT_ALWAYS_CHAR ("PASV"); break;
    case FTP_Codes::FTP_COMMAND_TYPE:
      result = ACE_TEXT_ALWAYS_CHAR ("TYPE"); break;
    case FTP_Codes::FTP_COMMAND_STRU:
      result = ACE_TEXT_ALWAYS_CHAR ("STRU"); break;
    case FTP_Codes::FTP_COMMAND_MODE:
      result = ACE_TEXT_ALWAYS_CHAR ("MODE"); break;
    //////////////////////////////////////
    case FTP_Codes::FTP_COMMAND_RETR:
      result = ACE_TEXT_ALWAYS_CHAR ("RETR"); break;
    case FTP_Codes::FTP_COMMAND_STOR:
      result = ACE_TEXT_ALWAYS_CHAR ("STOR"); break;
    case FTP_Codes::FTP_COMMAND_STOU:
      result = ACE_TEXT_ALWAYS_CHAR ("STOU"); break;
    case FTP_Codes::FTP_COMMAND_APPE:
      result = ACE_TEXT_ALWAYS_CHAR ("APPE"); break;
    case FTP_Codes::FTP_COMMAND_ALLO:
      result = ACE_TEXT_ALWAYS_CHAR ("ALLO"); break;
    case FTP_Codes::FTP_COMMAND_REST:
      result = ACE_TEXT_ALWAYS_CHAR ("REST"); break;
    case FTP_Codes::FTP_COMMAND_RNFR:
      result = ACE_TEXT_ALWAYS_CHAR ("RNFR"); break;
    case FTP_Codes::FTP_COMMAND_RNTO:
      result = ACE_TEXT_ALWAYS_CHAR ("RNTO"); break;
    case FTP_Codes::FTP_COMMAND_ABOR:
      result = ACE_TEXT_ALWAYS_CHAR ("ABOR"); break;
    case FTP_Codes::FTP_COMMAND_DELE:
      result = ACE_TEXT_ALWAYS_CHAR ("DELE"); break;
    case FTP_Codes::FTP_COMMAND_RMD:
      result = ACE_TEXT_ALWAYS_CHAR ("RMD"); break;
    case FTP_Codes::FTP_COMMAND_MKD:
      result = ACE_TEXT_ALWAYS_CHAR ("MKD"); break;
    case FTP_Codes::FTP_COMMAND_PWD:
      result = ACE_TEXT_ALWAYS_CHAR ("PWD"); break;
    case FTP_Codes::FTP_COMMAND_LIST:
      result = ACE_TEXT_ALWAYS_CHAR ("LIST"); break;
    case FTP_Codes::FTP_COMMAND_NLST:
      result = ACE_TEXT_ALWAYS_CHAR ("NLST"); break;
    case FTP_Codes::FTP_COMMAND_SITE:
      result = ACE_TEXT_ALWAYS_CHAR ("SITE"); break;
    case FTP_Codes::FTP_COMMAND_SYST:
      result = ACE_TEXT_ALWAYS_CHAR ("SYST"); break;
    case FTP_Codes::FTP_COMMAND_STAT:
      result = ACE_TEXT_ALWAYS_CHAR ("STAT"); break;
    case FTP_Codes::FTP_COMMAND_HELP:
      result = ACE_TEXT_ALWAYS_CHAR ("HELP"); break;
    case FTP_Codes::FTP_COMMAND_NOOP:
      result = ACE_TEXT_ALWAYS_CHAR ("NOOP"); break;
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
    case FTP_STATE_INITIAL:
      result = ACE_TEXT_ALWAYS_CHAR ("INITIAL"); break;
    case FTP_STATE_USER_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("USER_SENT"); break;
    case FTP_STATE_PASS_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("PASS_SENT"); break;
    case FTP_STATE_READY:
      result = ACE_TEXT_ALWAYS_CHAR ("READY"); break;
    case FTP_STATE_REQUEST_SENT:
      result = ACE_TEXT_ALWAYS_CHAR ("REQUEST_SENT"); break;
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

std::string
FTP_Tools::DataStateToString (enum FTP_ProtocolDataState state_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Tools::DataStateToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (state_in)
  {
    case FTP_STATE_DATA_INITIAL:
      result = ACE_TEXT_ALWAYS_CHAR ("INITIAL"); break;
    case FTP_STATE_DATA_LIST_DIRECTORY:
      result = ACE_TEXT_ALWAYS_CHAR ("LIST_DIRECTORY"); break;
    case FTP_STATE_DATA_LIST_FILE:
      result = ACE_TEXT_ALWAYS_CHAR ("LIST_FILE"); break;
    case FTP_STATE_DATA_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown data state (was: %d), aborting\n"),
                  state_in));
      break;
    }
  } // end SWITCH

  return result;
}

ACE_INET_Addr
FTP_Tools::parsePASVResponse (const std::string& PASVLine1_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Tools::parsePASVResponse"));

  ACE_INET_Addr result;

  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[^\\(]*\\()([[:digit:]]{1,3})(?:,)([[:digit:]]{1,3})(?:,)([[:digit:]]{1,3})(?:,)([[:digit:]]{1,3})(?:,)([[:digit:]]{1,3})(?:,)([[:digit:]]{1,3})(?:\\))(?:.*)$");
  std::regex::flag_type flags = std::regex_constants::ECMAScript;
  std::regex regex;
  std::smatch match_results;
  regex.assign (regex_string, flags);
  if (unlikely (!std::regex_match (PASVLine1_in,
                                   match_results,
                                   regex,
                                   std::regex_constants::match_default)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to parse PASV response line (was: \"%s\"), aborting\n"),
                ACE_TEXT (PASVLine1_in.c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  ACE_ASSERT (match_results[1].matched);
  ACE_ASSERT (match_results[2].matched);
  ACE_ASSERT (match_results[3].matched);
  ACE_ASSERT (match_results[4].matched);
  std::string address_string = match_results[1].str ();
  address_string += '.';
  address_string += match_results[2].str ();
  address_string += '.';
  address_string += match_results[3].str ();
  address_string += '.';
  address_string += match_results[4].str ();
  address_string += ':';

  ACE_ASSERT (match_results[5].matched);
  ACE_ASSERT (match_results[6].matched);
  std::stringstream converter (match_results[5].str ());
  ACE_UINT16 port_number_i = 0, temp_i = 0;
  converter >> port_number_i;
  port_number_i <<= 8;
  converter.clear ();
  converter.str (match_results[6].str ());
  converter >> temp_i;
  port_number_i |= temp_i;
  
  converter.clear ();
  converter << port_number_i;
  address_string += converter.str ();

  result = Net_Common_Tools::stringToIPAddress (address_string, 0);

  return result;
}
