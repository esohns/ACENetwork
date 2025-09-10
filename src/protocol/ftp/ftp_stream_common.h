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

#ifndef FTP_STREAM_COMMON_H
#define FTP_STREAM_COMMON_H

#include "ace/INET_Addr.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"
#include "stream_session_manager.h"

#include "stream_net_common.h"

#include "net_common.h"
#include "net_iconnection.h"

#include "ftp_common.h"
#include "ftp_configuration.h"
#include "ftp_message.h"
#include "ftp_sessionmessage.h"
#include "ftp_stream.h"

// forward declarations
struct FTP_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct FTP_ConnectionState,
                          struct Net_StreamStatistic> FTP_IConnection_t;

typedef Stream_DataBase_T<struct FTP_Record> FTP_MessageData_t;

struct FTP_Stream_SessionData
 : Stream_SessionData
{
  FTP_Stream_SessionData ()
   : Stream_SessionData ()
   , connection (NULL)
   , connectionStates ()
   , statistic ()
  {}

  FTP_IConnection_t*            connection;
  Stream_Net_ConnectionStates_t connectionStates;
  FTP_Statistic_t               statistic;
};
typedef Stream_SessionData_T<struct FTP_Stream_SessionData> FTP_Stream_SessionData_t;

typedef FTP_Message_T<FTP_MessageData_t,
                      enum Stream_MessageType> FTP_Message_t;
typedef FTP_SessionMessage_T<FTP_MessageData_t,
                             FTP_Stream_SessionData_t,
                             struct Stream_UserData> FTP_SessionMessage_t;
typedef Stream_ISessionDataNotify_T<struct FTP_Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    FTP_Message_t,
                                    FTP_SessionMessage_t> FTP_ISessionNotify_t;

struct FTP_StreamState
 : Stream_State
{
  FTP_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct FTP_Stream_SessionData* sessionData;
};

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct FTP_Stream_SessionData,
                                 FTP_Statistic_t,
                                 struct Stream_UserData> FTP_SessionManager_t;

typedef FTP_Stream_T<struct FTP_StreamState,
                     struct FTP_StreamConfiguration,
                     FTP_Statistic_t,
                     FTP_StatisticHandler_t,
                     struct FTP_ModuleHandlerConfiguration,
                     FTP_SessionManager_t,
                     Stream_ControlMessage_t,
                     FTP_Message_t,
                     FTP_SessionMessage_t> FTP_Stream_t;

#endif
