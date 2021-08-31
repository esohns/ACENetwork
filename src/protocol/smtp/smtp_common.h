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

#ifndef SMTP_COMMON_H
#define SMTP_COMMON_H

#include <string>
#include <vector>

#include "ace/INET_Addr.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_data_base.h"

#include "net_common.h"

#include "smtp_codes.h"
#include "smtp_defines.h"

// forward declarations
struct SMTP_Configuration;
struct SMTP_Record;

/////////////////////////////////////////

// convenient type definitions
typedef SMTP_Codes::CommandType SMTP_Command_t;
typedef SMTP_Codes::CodeType SMTP_Code_t;

typedef std::vector<std::string> SMTP_Parameters_t;
typedef SMTP_Parameters_t::const_iterator SMTP_ParametersConstIterator_t;

typedef std::vector<std::string> SMTP_To_t;
typedef SMTP_To_t::const_iterator SMTP_ToConstIterator_t;

struct SMTP_Request
{
  SMTP_Request ()
    : address (static_cast<u_short> (0),
               ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
               AF_INET)
    , command (SMTP_Codes::SMTP_COMMAND_INVALID)
    , from ()
    , parameters ()
    , to ()
    , data ()
  {}

  ACE_INET_Addr     address;
  SMTP_Command_t    command;
  std::string       from;
  SMTP_Parameters_t parameters;
  SMTP_To_t         to;
  std::string       data;
};

typedef std::vector<std::string> SMTP_Text_t;
typedef SMTP_Text_t::const_iterator SMTP_TextConstIterator_t;

struct SMTP_Record
{
  SMTP_Record ()
   : request ()
   , code (SMTP_Codes::SMTP_CODE_INVALID)
   , text ()
  {}
  void operator+= (struct SMTP_Record rhs_in)
  { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  // request
  struct SMTP_Request       request;

  // response
  enum SMTP_Codes::CodeType code;
  SMTP_Text_t               text;
};

//////////////////////////////////////////

enum SMTP_ProtocolState
{
  SMTP_STATE_GREETING_RECEIVED = 0,
  SMTP_STATE_EHLO_SENT,
  SMTP_STATE_MAIL_SENT,
  SMTP_STATE_RCPT_SENT,
  SMTP_STATE_RCPTS_SENT,
  SMTP_STATE_DATA_SENT,
  SMTP_STATE_QUIT_SENT,
  /////////////////////////////////////
  SMTP_STATE_MAX,
  SMTP_STATE_INVALID
};

typedef struct Stream_Statistic SMTP_Statistic_t;
typedef Common_IStatistic_T<SMTP_Statistic_t> SMTP_IStatistic_t;
typedef Common_StatisticHandler_T<SMTP_Statistic_t> SMTP_StatisticHandler_t;

#endif
