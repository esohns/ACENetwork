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
    FTP_COMMAND_USER = 0,
    FTP_COMMAND_PASS,
    FTP_COMMAND_ACCT,
    FTP_COMMAND_CWD,
    FTP_COMMAND_CDUP,
    FTP_COMMAND_SMNT,
    FTP_COMMAND_REIN,
    FTP_COMMAND_QUIT,
    //////////////////////////////////////
    FTP_COMMAND_PORT,
    FTP_COMMAND_PASV,
    FTP_COMMAND_TYPE,
    FTP_COMMAND_STRU,
    FTP_COMMAND_MODE,
    //////////////////////////////////////
    FTP_COMMAND_RETR,
    FTP_COMMAND_STOR,
    FTP_COMMAND_STOU,
    FTP_COMMAND_APPE,
    FTP_COMMAND_ALLO,
    FTP_COMMAND_REST,
    FTP_COMMAND_RNFR,
    FTP_COMMAND_RNTO,
    FTP_COMMAND_ABOR,
    FTP_COMMAND_DELE,
    FTP_COMMAND_RMD,
    FTP_COMMAND_MKD,
    FTP_COMMAND_PWD,
    FTP_COMMAND_LIST,
    FTP_COMMAND_NLST,
    FTP_COMMAND_SITE,
    FTP_COMMAND_SYST,
    FTP_COMMAND_STAT,
    FTP_COMMAND_HELP,
    FTP_COMMAND_NOOP,
    //////////////////////////////////////
    FTP_COMMAND_MAX,
    FTP_COMMAND_INVALID
  };

  enum CodeType
  {
    FTP_CODE_RESTART_MARKER = 110,
    FTP_CODE_SERVICE_READY_N_MINUTES = 120,
    FTP_CODE_DATA_OPEN_TRANSFER_STARTING = 125,
    FTP_CODE_FILE_STATUS_OK_OPENING_DATA = 150,
    //////////////////////////////////////
    FTP_CODE_OK = 200,
    FTP_CODE_COMMAND_SUPERFLUOUS = 202,
    FTP_CODE_SYSTEM_STATUS_OR_HELP = 211,
    FTP_CODE_DIRECTORY_STATUS = 212,
    FTP_CODE_FILE_STATUS = 213,
    FTP_CODE_HELP_MESSAGE = 214,
    FTP_CODE_NAME_SYSTEM_TYPE = 215,
    FTP_CODE_SERVICE_READY_NEW_USER = 220,
    FTP_CODE_SERVICE_CLOSING_CTL = 221,
    FTP_CODE_DATA_OPEN_NO_TRANSFER = 225,
    FTP_CODE_CLOSING_DATA = 226,
    FTP_CODE_ENTERING_PASSIVE_MODE = 227,
    FTP_CODE_USER_LOGGED_IN = 230,
    FTP_CODE_FILE_ACTION_OK = 250,
    FTP_CODE_PATH_CREATED_OK = 257,
    //////////////////////////////////////
    FTP_CODE_USER_OK_NEED_PASSWORD = 331,
    FTP_CODE_NEED_ACCOUNT_FOR_LOGIN = 332,
    FTP_CODE_FILE_ACTION_PENDING_FURTHER_INFORMATION = 350,
    //////////////////////////////////////
    FTP_CODE_SERVICE_NOT_AVAILABLE_CLOSING_CTL = 421,
    FTP_CODE_CANNOT_OPEN_DATA = 425,
    FTP_CODE_DATA_CLOSED_TRANSFER_ABORTED = 426,
    FTP_CODE_FILE_ACTION_ABORTED_FILE_UNAVAILABLE = 450,
    FTP_CODE_ACTION_ABORTED = 451,
    FTP_CODE_FILE_ACTION_ABORTED_NO_SPACE = 452,
    //////////////////////////////////////
    FTP_CODE_ERROR_SYNTAX_COMMAND = 500,
    FTP_CODE_ERROR_SYNTAX_ARGUMENT = 501,
    FTP_CODE_ERROR_COMMAND_NOT_IMPLEMENTED = 502,
    FTP_CODE_ERROR_COMMAND_BAD_SEQUENCE = 503,
    FTP_CODE_ERROR_COMMAND_ARGUMENT_NOT_IMPLEMENTED = 504,
    FTP_CODE_ERROR_USER_NOT_LOGGED_IN = 530,
    FTP_CODE_ERROR_NEED_ACCOUNT_FOR_STORING_FILES = 532,
    FTP_CODE_ERROR_FILE_ACTION_ABORTED_FILE_UNAVAILABLE = 550,
    FTP_CODE_ERROR_ACTION_ABORTED_PAGE_TYPE_UNKNOWN = 551,
    FTP_CODE_ERROR_FILE_ACTION_ABORTED_NO_SPACE = 552,
    FTP_CODE_ERROR_FILE_ACTION_ABORTED_FILENAME_NOT_ALLOWED = 553,
    //////////////////////////////////////
    FTP_CODE_MAX,
    FTP_CODE_INVALID
  };

  enum RecordType
  {
    FTP_RECORD_DIRECTORY = 0,
    FTP_RECORD_FILE,
    FTP_RECORD_DATA,
    //////////////////////////////////////
    FTP_RECORD_MAX,
    FTP_RECORD_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Codes (const FTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Codes& operator= (const FTP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~FTP_Codes ())
};

#endif
