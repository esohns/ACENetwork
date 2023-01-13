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

#include <string>
#include <vector>

#include "ace/INET_Addr.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_data_base.h"

#include "net_common.h"

#include "ftp_codes.h"
#include "ftp_defines.h"

// forward declarations
struct FTP_Configuration;
struct FTP_Record;

/////////////////////////////////////////

// convenient type definitions
typedef FTP_Codes::CommandType FTP_Command_t;
typedef FTP_Codes::CodeType FTP_Code_t;

typedef std::vector<std::string> FTP_Parameters_t;
typedef FTP_Parameters_t::const_iterator FTP_ParametersConstIterator_t;

typedef std::vector<std::string> FTP_To_t;
typedef FTP_To_t::const_iterator FTP_ToConstIterator_t;

struct FTP_Request
{
  FTP_Request ()
   : command (FTP_Codes::FTP_COMMAND_INVALID)
   , domain (static_cast<u_short> (0),
             ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
             AF_INET)
   , from ()
   , parameters ()
   , to ()
   , data ()
  {}

  FTP_Command_t    command;
  ACE_INET_Addr     domain;
  std::string       from;
  FTP_Parameters_t parameters;
  FTP_To_t         to;
  std::string       data;
};

typedef std::vector<std::string> FTP_Text_t;
typedef FTP_Text_t::const_iterator FTP_TextConstIterator_t;

struct FTP_Record
{
  FTP_Record ()
   : request ()
   , code (FTP_Codes::FTP_CODE_INVALID)
   , text ()
  {}
  void operator+= (struct FTP_Record rhs_in)
  { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  // request
  struct FTP_Request       request;

  // response
  enum FTP_Codes::CodeType code;
  FTP_Text_t               text;
};

//////////////////////////////////////////

enum FTP_ProtocolState
{
  FTP_STATE_GREETING_RECEIVED = 0,
  FTP_STATE_EHLO_SENT,
  FTP_STATE_AUTH_SENT, // rfc4954
  FTP_STATE_AUTH_LOGIN_USER_SENT, // rfc4954
  FTP_STATE_AUTH_LOGIN_PASSWORD_SENT, // rfc4954
  FTP_STATE_AUTH_COMPLETE, // i.e. AUTH mechanism completed
  FTP_STATE_MAIL_SENT,
  FTP_STATE_RCPT_SENT,
  FTP_STATE_RCPTS_SENT,
  FTP_STATE_DATA_SENT,
  FTP_STATE_DATA_2_SENT, // i.e. message sent
  FTP_STATE_QUIT_SENT,
  /////////////////////////////////////
  FTP_STATE_MAX,
  FTP_STATE_INVALID
};

inline enum FTP_ProtocolState&
operator++ (enum FTP_ProtocolState& state_inout)
{ ACE_ASSERT (state_inout != FTP_STATE_MAX);
  if (state_inout == FTP_STATE_INVALID)
  {
    state_inout = FTP_STATE_GREETING_RECEIVED;
    return state_inout;
  } // end IF

  state_inout =
    static_cast<enum FTP_ProtocolState> ((static_cast<int> (state_inout) + 1) % FTP_STATE_MAX);
  return state_inout;
}
inline enum FTP_ProtocolState&
operator-- (enum FTP_ProtocolState& state_inout)
{ ACE_ASSERT (!((state_inout == FTP_STATE_GREETING_RECEIVED) ||
                (state_inout == FTP_STATE_MAX)               ||
                (state_inout == FTP_STATE_INVALID)));
  state_inout =
    static_cast<enum FTP_ProtocolState> ((static_cast<int> (state_inout) - 1));
  return state_inout;
}

//////////////////////////////////////////

typedef struct Stream_Statistic FTP_Statistic_t;
typedef Common_IStatistic_T<FTP_Statistic_t> FTP_IStatistic_t;
typedef Common_StatisticHandler_T<FTP_Statistic_t> FTP_StatisticHandler_t;

#endif
