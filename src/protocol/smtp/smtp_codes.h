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

#ifndef SMTP_CODES_H
#define SMTP_CODES_H

#include "ace/Global_Macros.h"

class SMTP_Codes
{
 public:
  enum CommandType
  {
    SMTP_COMMAND_EHLO = 0,
    SMTP_COMMAND_MAIL,
    SMTP_COMMAND_RCPT,
    SMTP_COMMAND_DATA,
    SMTP_COMMAND_QUIT,
    /////////////////////////////////////
    SMTP_COMMAND_MAX,
    SMTP_COMMAND_INVALID
  };

  enum CodeType
  {
    SMTP_CODE_SYSTEM_STATUS_HELP_REPLY = 211,
    SMTP_CODE_HELP_MESSAGE = 214,
    SMTP_CODE_SERVICE_READY = 220,
    SMTP_CODE_SERVICE_CLOSING_CHANNEL = 221,
    SMTP_CODE_MAIL_COMPLETE = 250,
    SMTP_CODE_USER_NOT_LOCAL_WILL_FORWARD = 251,
    SMTP_CODE_CANNOT_VRFY_USER_WILL_CONTINUE = 252,
    //////////////////////////////////////
    SMTP_CODE_START_MAIL_INPUT = 354,
    //////////////////////////////////////
    SMTP_CODE_SERVICE_NOT_AVAILABLE_CLOSING_CHANNEL = 421,
    SMTP_CODE_MAILBOX_NOT_AVAILABLE = 450,
    SMTP_CODE_PROCESSING_ERROR_ABORTED = 451,
    SMTP_CODE_PROCESSING_ERROR_SYSTEM_STORAGE = 452,
    //////////////////////////////////////
    SMTP_CODE_ERROR_SYNTAX_COMMAND = 500,
    SMTP_CODE_ERROR_SYNTAX_ARGUMENT = 501,
    SMTP_CODE_ERROR_COMMAND_NOT_IMPLEMENTED = 502,
    SMTP_CODE_ERROR_COMMAND_BAD_SEQUENCE = 503,
    SMTP_CODE_ERROR_COMMAND_ARGUMENT_NOT_IMPLEMENTED = 504,
    SMTP_CODE_ERROR_MAILBOX_UNAVAILABLE = 550,
    SMTP_CODE_ERROR_USER_NOT_LOCAL_TRY_FORWARD = 551,
    SMTP_CODE_ERROR_EXCEEDED_SYSTEM_STORAGE = 552,
    SMTP_CODE_ERROR_MAILBOX_NAME_NOT_ALLOWED = 553,
    SMTP_CODE_ERROR_TRANSACTION_FAILED = 554,
    /////////////////////////////////////
    SMTP_CODE_MAX,
    SMTP_CODE_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (SMTP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (SMTP_Codes (const SMTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (SMTP_Codes& operator= (const SMTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~SMTP_Codes ())
};

#endif
