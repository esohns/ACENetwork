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

#ifndef FTP_COMMON_H
#define FTP_COMMON_H

#include <list>
#include <string>
#include <vector>

#include "ace/INET_Addr.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "common_file_common.h"

#include "stream_common.h"
#include "stream_data_base.h"

#include "net_common.h"

#include "ftp_codes.h"
#include "ftp_defines.h"

/////////////////////////////////////////

struct FTP_LoginOptions
{
  FTP_LoginOptions ()
   : cwd ()
   , password (ACE_TEXT_ALWAYS_CHAR (FTP_DEFAULT_ANONYMOUS_PASS))
   , server ((u_short)0, (ACE_UINT32)0)
   , user (ACE_TEXT_ALWAYS_CHAR (FTP_DEFAULT_ANONYMOUS_USER))
  {};

  std::string   cwd; // initial-
  std::string   password;
  ACE_INET_Addr server;
  std::string   user;
};

// convenient type definitions
typedef FTP_Codes::CommandType FTP_Command_t;
typedef FTP_Codes::CodeType FTP_Code_t;

typedef std::vector<std::string> FTP_Parameters_t;
typedef FTP_Parameters_t::const_iterator FTP_ParametersConstIterator_t;

struct FTP_Request
{
  FTP_Request ()
   : address ((u_short)0, (ACE_UINT32)0)
   , command (FTP_Codes::FTP_COMMAND_INVALID)
   , is_directory_list (true)
   , parameters ()
  {}

  ACE_INET_Addr    address;
  FTP_Command_t    command;
  bool             is_directory_list; // ? : file-
  FTP_Parameters_t parameters;
};

typedef std::vector<std::string> FTP_Text_t;
typedef FTP_Text_t::const_iterator FTP_TextConstIterator_t;

struct FTP_Record
{
  FTP_Record ()
   : request ()
   , code (FTP_Codes::FTP_CODE_INVALID)
   , text ()
   , type (FTP_Codes::FTP_RECORD_INVALID)
   , entries ()
  {}
  void operator+= (struct FTP_Record rhs_in)
  { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  // request
  struct FTP_Request         request;

  // response
  enum FTP_Codes::CodeType   code;
  FTP_Text_t                 text;

  // data
  enum FTP_Codes::RecordType type;
  Common_File_Entries_t      entries;
};

typedef std::list<struct FTP_Record> FTP_Records_t;
typedef FTP_Records_t::const_iterator FTP_RecordsConstIterator_t;

//////////////////////////////////////////

enum FTP_ProtocolState
{
  FTP_STATE_INITIAL = 0,
  FTP_STATE_USER_SENT,
  FTP_STATE_PASS_SENT,
  FTP_STATE_READY,
  FTP_STATE_REQUEST_SENT,
  FTP_STATE_QUIT_SENT,
  /////////////////////////////////////
  FTP_STATE_MAX,
  FTP_STATE_INVALID
};

enum FTP_ProtocolDataState
{
  FTP_STATE_DATA_INITIAL = 0,
  FTP_STATE_DATA_LIST_DIRECTORY,
  FTP_STATE_DATA_LIST_FILE,
  FTP_STATE_DATA_DATA,
  /////////////////////////////////////
  FTP_STATE_DATA_MAX,
  FTP_STATE_DATA_INVALID
};

inline enum FTP_ProtocolState&
operator++ (enum FTP_ProtocolState& state_inout)
{ ACE_ASSERT (state_inout != FTP_STATE_MAX);
  if (state_inout == FTP_STATE_INVALID)
  {
    state_inout = FTP_STATE_INITIAL;
    return state_inout;
  } // end IF

  state_inout =
    static_cast<enum FTP_ProtocolState> ((static_cast<int> (state_inout) + 1) % FTP_STATE_MAX);
  return state_inout;
}
inline enum FTP_ProtocolState&
operator-- (enum FTP_ProtocolState& state_inout)
{ ACE_ASSERT (!((state_inout == FTP_STATE_INITIAL)   ||
                (state_inout == FTP_STATE_MAX)       ||
                (state_inout == FTP_STATE_INVALID)));
  state_inout =
    static_cast<enum FTP_ProtocolState> ((static_cast<int> (state_inout) - 1));
  return state_inout;
}

//////////////////////////////////////////

typedef struct Net_StreamStatistic FTP_Statistic_t;
typedef Common_IStatistic_T<FTP_Statistic_t> FTP_IStatistic_t;
typedef Common_StatisticHandler_T<FTP_Statistic_t> FTP_StatisticHandler_t;

#endif
