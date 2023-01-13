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

#ifndef FTP_CODES_H
#define FTP_CODES_H

#include "ace/Global_Macros.h"

class FTP_Codes
{
 public:
  enum CommandType
  {
    FTP_COMMAND_EHLO = 0,
    FTP_COMMAND_MAIL,
    FTP_COMMAND_RCPT,
    FTP_COMMAND_DATA,
    FTP_COMMAND_QUIT,
    /////////////////////////////////////
    FTP_COMMAND_AUTH, // rfc4954
    /////////////////////////////////////
    FTP_COMMAND_DATA_2, // pseudo-command: actual message
    /////////////////////////////////////
    FTP_COMMAND_MAX,
    FTP_COMMAND_INVALID
  };

  enum CodeType
  {
    FTP_CODE_SYSTEM_STATUS_HELP_REPLY = 211,
    FTP_CODE_HELP_MESSAGE = 214,
    FTP_CODE_SERVICE_READY = 220,
    FTP_CODE_SERVICE_CLOSING_CHANNEL = 221,
    FTP_CODE_AUTHENTICATION_SUCCEEDED = 235, // rfc4954
    FTP_CODE_MAIL_COMPLETE = 250,
    FTP_CODE_USER_NOT_LOCAL_WILL_FORWARD = 251,
    FTP_CODE_CANNOT_VRFY_USER_WILL_CONTINUE = 252,
    //////////////////////////////////////
    FTP_CODE_SERVER_CHALLENGE = 334, // rfc4954
    FTP_CODE_START_MAIL_INPUT = 354,
    //////////////////////////////////////
    FTP_CODE_SERVICE_NOT_AVAILABLE_CLOSING_CHANNEL = 421,
    FTP_CODE_PASSWORD_TRANSITION_NEEDED = 432, // rfc4954
    FTP_CODE_MAILBOX_NOT_AVAILABLE = 450,
    FTP_CODE_PROCESSING_ERROR_ABORTED = 451,
    FTP_CODE_PROCESSING_ERROR_SYSTEM_STORAGE = 452,
    FTP_CODE_TEMPORARY_AUTHENTICATION_FAILURE = 454, // rfc4954
    //////////////////////////////////////
    FTP_CODE_ERROR_SYNTAX_COMMAND = 500,
    FTP_CODE_ERROR_SYNTAX_ARGUMENT = 501,
    FTP_CODE_ERROR_COMMAND_NOT_IMPLEMENTED = 502,
    FTP_CODE_ERROR_COMMAND_BAD_SEQUENCE = 503,
    FTP_CODE_ERROR_COMMAND_ARGUMENT_NOT_IMPLEMENTED = 504,
    FTP_CODE_HOST_DOES_NOT_ACCEPT_MAIL = 521, // rfc1846
    FTP_CODE_AUTHENTICATION_REQUIRED = 530, // rfc4954
    FTP_CODE_AUTHENTICATION_MECHANISM_TOO_WEAK = 534, // rfc4954
    FTP_CODE_AUTHENTICATION_CREDENTIALS_INVALID = 535, // rfc4954
    FTP_CODE_ENCRYPTION_REQUIRED_FOR_AUTHENTICATION_MECHANISM = 538, // rfc4954
    FTP_CODE_ERROR_MAILBOX_UNAVAILABLE = 550,
    FTP_CODE_ERROR_USER_NOT_LOCAL_TRY_FORWARD = 551,
    FTP_CODE_ERROR_EXCEEDED_SYSTEM_STORAGE = 552,
    FTP_CODE_ERROR_MAILBOX_NAME_NOT_ALLOWED = 553,
    FTP_CODE_ERROR_TRANSACTION_FAILED = 554,
    /////////////////////////////////////
    FTP_CODE_MAX,
    FTP_CODE_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Codes (const FTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Codes& operator= (const FTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~FTP_Codes ())
};

#endif
