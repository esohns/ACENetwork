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

#ifndef SMTP_STREAM_COMMON_H
#define SMTP_STREAM_COMMON_H

#include "ace/INET_Addr.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_common.h"
#include "net_iconnection.h"

#include "smtp_common.h"
#include "smtp_configuration.h"
#include "smtp_message.h"
#include "smtp_sessionmessage.h"
#include "smtp_stream.h"

// forward declarations
struct SMTP_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct SMTP_ConnectionState,
                          struct Net_StreamStatistic> SMTP_IConnection_t;

struct SMTP_Stream_SessionData
 : Stream_SessionData
{
  SMTP_Stream_SessionData ()
   : Stream_SessionData ()
   , connection (NULL)
   , statistic ()
  {}

  SMTP_IConnection_t* connection;

  SMTP_Statistic_t    statistic;
};
typedef Stream_SessionData_T<struct SMTP_Stream_SessionData> SMTP_Stream_SessionData_t;

typedef SMTP_Message_T<enum Stream_MessageType> SMTP_Message_t;
typedef SMTP_SessionMessage_T<SMTP_Stream_SessionData_t,
                              struct Stream_UserData> SMTP_SessionMessage_t;
typedef Stream_ISessionDataNotify_T<struct SMTP_Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    SMTP_Message_t,
                                    SMTP_SessionMessage_t> SMTP_ISessionNotify_t;

struct SMTP_StreamState
 : Stream_State
{
  SMTP_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct SMTP_Stream_SessionData* sessionData;
};

typedef SMTP_Stream_T<struct SMTP_StreamState,
                      struct SMTP_StreamConfiguration,
                      SMTP_Statistic_t,
                      SMTP_StatisticHandler_t,
                      struct SMTP_ModuleHandlerConfiguration,
                      struct SMTP_Stream_SessionData,
                      SMTP_Stream_SessionData_t,
                      Stream_ControlMessage_t,
                      SMTP_Message_t,
                      SMTP_SessionMessage_t> SMTP_Stream_t;

#endif
