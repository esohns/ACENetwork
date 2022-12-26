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

#ifndef POP_STREAM_COMMON_H
#define POP_STREAM_COMMON_H

#include "ace/INET_Addr.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "stream_net_common.h"

#include "net_common.h"
#include "net_iconnection.h"

#include "pop_common.h"
#include "pop_configuration.h"
#include "pop_message.h"
#include "pop_sessionmessage.h"
#include "pop_stream.h"

// forward declarations
struct POP_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct POP_ConnectionState,
                          struct Net_StreamStatistic> POP_IConnection_t;

struct POP_Stream_SessionData
 : Stream_SessionData
{
  POP_Stream_SessionData ()
   : Stream_SessionData ()
   , connection (NULL)
   , connectionStates ()
   , statistic ()
  {}

  POP_IConnection_t*            connection;
  Stream_Net_ConnectionStates_t connectionStates;
  POP_Statistic_t               statistic;
};
typedef Stream_SessionData_T<struct POP_Stream_SessionData> POP_Stream_SessionData_t;

typedef POP_Message_T<enum Stream_MessageType> POP_Message_t;
typedef POP_SessionMessage_T<POP_Stream_SessionData_t,
                              struct Stream_UserData> POP_SessionMessage_t;
typedef Stream_ISessionDataNotify_T<struct POP_Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    POP_Message_t,
                                    POP_SessionMessage_t> POP_ISessionNotify_t;

struct POP_StreamState
 : Stream_State
{
  POP_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct POP_Stream_SessionData* sessionData;
};

typedef POP_Stream_T<struct POP_StreamState,
                      struct POP_StreamConfiguration,
                      POP_Statistic_t,
                      POP_StatisticHandler_t,
                      struct POP_ModuleHandlerConfiguration,
                      struct POP_Stream_SessionData,
                      POP_Stream_SessionData_t,
                      Stream_ControlMessage_t,
                      POP_Message_t,
                      POP_SessionMessage_t> POP_Stream_t;

#endif
