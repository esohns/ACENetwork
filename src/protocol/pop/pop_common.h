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

#ifndef POP_COMMON_H
#define POP_COMMON_H

#include <string>
#include <vector>

#include "ace/INET_Addr.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_data_base.h"

#include "net_common.h"

#include "pop_codes.h"
#include "pop_defines.h"

// forward declarations
struct POP_Configuration;
struct POP_Record;

/////////////////////////////////////////

// convenient type definitions
typedef POP_Codes::CommandType POP_Command_t;

typedef std::vector<std::string> POP_Parameters_t;
typedef POP_Parameters_t::const_iterator POP_ParametersConstIterator_t;

struct POP_Request
{
  POP_Request ()
   : command (POP_Codes::POP_COMMAND_INVALID)
   , parameters ()
  {}

  POP_Command_t    command;
  POP_Parameters_t parameters;
};

typedef std::vector<std::string> POP_Text_t;
typedef POP_Text_t::const_iterator POP_TextConstIterator_t;

struct POP_Record
{
  POP_Record ()
   : request ()
   , status ()
   , text ()
  {}
  void operator+= (struct POP_Record rhs_in)
  { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  // request
  struct POP_Request       request;

  // response
  std::string              status;
  POP_Text_t               text;
};

//////////////////////////////////////////

enum POP_ProtocolState
{
  POP_STATE_GREETING_RECEIVED = 0,
  // AUTHENTICATION
  POP_STATE_AUTH_LOGIN_USER_SENT,
  POP_STATE_AUTH_LOGIN_PASSWORD_SENT,
  POP_STATE_AUTH_APOP_SENT,
  POP_STATE_AUTH_QUIT_SENT,
  // TRANSACTION
  POP_STATE_TRANS_STAT_SENT,
  POP_STATE_TRANS_LIST_SENT,
  POP_STATE_TRANS_RETR_SENT,
  POP_STATE_TRANS_DELE_SENT,
  POP_STATE_TRANS_NOOP_SENT,
  POP_STATE_TRANS_RSET_SENT,
  POP_STATE_TRANS_QUIT_SENT,
  POP_STATE_TRANS_TOP_SENT,
  POP_STATE_TRANS_UIDL_SENT,
  // UPDATE
  POP_STATE_UPDAT_QUIT_SENT,
  ////////////////////////////////////////
  POP_STATE_MAX,
  POP_STATE_INVALID
};

inline enum POP_ProtocolState&
operator++ (enum POP_ProtocolState& state_inout)
{ ACE_ASSERT (state_inout != POP_STATE_MAX);
  if (state_inout == POP_STATE_INVALID)
  {
    state_inout = POP_STATE_GREETING_RECEIVED;
    return state_inout;
  } // end IF

  state_inout =
    static_cast<enum POP_ProtocolState> ((static_cast<int> (state_inout) + 1) % POP_STATE_MAX);
  return state_inout;
}
inline enum POP_ProtocolState&
operator-- (enum POP_ProtocolState& state_inout)
{ ACE_ASSERT (!((state_inout == POP_STATE_GREETING_RECEIVED) ||
                (state_inout == POP_STATE_MAX)               ||
                (state_inout == POP_STATE_INVALID)));
  state_inout =
    static_cast<enum POP_ProtocolState> ((static_cast<int> (state_inout) - 1));
  return state_inout;
}

//////////////////////////////////////////

typedef struct Stream_Statistic POP_Statistic_t;
typedef Common_IStatistic_T<POP_Statistic_t> POP_IStatistic_t;
typedef Common_StatisticHandler_T<POP_Statistic_t> POP_StatisticHandler_t;

#endif
