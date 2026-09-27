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

#ifndef WEBSOCKET_COMMON_H
#define WEBSOCKET_COMMON_H

#include <string>
#include <map>

#include "ace/OS.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"

#include "net_common.h"

#include "http_common.h"

#include "ws_codes.h"

// forward declarations
struct WebSocket_Record;
class WebSocket_SessionMessage;
struct WebSocket_SessionData;

typedef Stream_ISessionDataNotify_T<struct WebSocket_SessionData,
                                    enum Stream_SessionMessageType,
                                    struct WebSocket_Record,
                                    WebSocket_SessionMessage> WebSocket_ISessionNotify_t;

typedef Net_StreamStatistic_t WebSocket_Statistic_t;
typedef Common_IStatistic_T<WebSocket_Statistic_t> WebSocket_IStatisticReportingHandler_t;
typedef Common_StatisticHandler_T<WebSocket_Statistic_t> WebSocket_StatisticReportingHandler_t;

struct WebSocket_Record
 : HTTP_Record
{
  WebSocket_Record ()
   : HTTP_Record ()
   , final (true)
   , opcode (WebSocket_Codes::OPCODE_INVALID)
   , payload ()
   , payloadSize (0)
   , reason ()
   , status (WebSocket_Codes::STATUS_INVALID)
  {
    payload.blob = NULL;
  }
  ~WebSocket_Record ()
  {
    if (opcode == WebSocket_Codes::OPCODE_TEXT && payload.string)
      delete [] payload.string;
    else if (opcode == WebSocket_Codes::OPCODE_BINARY && payload.blob)
      delete [] payload.blob;
    payload.blob = NULL;
  }

  inline struct WebSocket_Record& operator= (struct HTTP_Record& rhs_in) { HTTP_Record::operator= (rhs_in); return *this; }
  inline void operator+= (struct WebSocket_Record rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  void reset ()
  {
    HTTP_Record::reset ();

    final = true;
    if (opcode == WebSocket_Codes::OPCODE_TEXT && payload.string)
      delete [] payload.string;
    else if (opcode == WebSocket_Codes::OPCODE_BINARY && payload.blob)
      delete [] payload.blob;
    opcode = WebSocket_Codes::OPCODE_INVALID;
    payload.blob = NULL;
    payloadSize = 0;
    reason.clear ();
    status = WebSocket_Codes::STATUS_INVALID;
  }

  bool                        final; // i.e. FIN-bit was set
  WebSocket_Codes::OpCodeType opcode;
  union
  {
    uint8_t* blob;
    char*    string;
  }                           payload;
  ACE_UINT64                  payloadSize;
  std::string                 reason;
  WebSocket_Codes::StatusType status;
};
typedef Stream_DataBase_T<struct WebSocket_Record> WebSocket_MessageData_t;

#endif
