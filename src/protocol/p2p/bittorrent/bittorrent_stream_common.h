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

#ifndef BITTORRENT_STREAM_COMMON_H
#define BITTORRENT_STREAM_COMMON_H

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_imodule.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_common.h"

#include "bittorrent_common.h"
//#include "bittorrent_message.h"
//#include "bittorrent_sessionmessage.h"
//#include "bittorrent_stream.h"

struct BitTorrent_MessageData
{
  inline BitTorrent_MessageData ()
   : record (NULL)
  {};
  inline ~BitTorrent_MessageData ()
  {
    if (record)
      delete record;
  };

  BitTorrent_Record* record;
};
typedef Stream_DataBase_T<BitTorrent_MessageData> BitTorrent_MessageData_t;

struct BitTorrent_ConnectionState
 : Net_ConnectionState
{
  inline BitTorrent_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , handshake (NULL)
  {};

  struct BitTorrent_Configuration* configuration;
  struct BitTorrent_PeerHandshake* handshake;
};

struct BitTorrent_ModuleHandlerConfiguration
{};

struct BitTorrent_StreamConfiguration
{
  inline BitTorrent_StreamConfiguration ()
   : moduleHandlerConfiguration (NULL)
  {};

  struct BitTorrent_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

typedef Stream_Statistic BitTorrent_RuntimeStatistic_t;
struct BitTorrent_SessionData
 : Stream_SessionData
{
  inline BitTorrent_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , currentStatistic ()
   , handshake (NULL)
  {};

  struct BitTorrent_ConnectionState* connectionState;
  BitTorrent_RuntimeStatistic_t      currentStatistic;
  struct BitTorrent_PeerHandshake*   handshake;
};
typedef Stream_SessionData_T<BitTorrent_SessionData> BitTorrent_SessionData_t;

struct BitTorrent_StreamState
 : Stream_State
{
  inline BitTorrent_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
  {};

  BitTorrent_SessionData_t* currentSessionData;
};

//typedef Stream_ControlMessage_T<Stream_ControlType,
//                                Stream_AllocatorConfiguration,
//                                BitTorrent_Message_t,
//                                BitTorrent_SessionMessage_t> BitTorrent_ControlMessage_t;
//typedef BitTorrent_Message_T<Stream_AllocatorConfiguration,
//                             BitTorrent_ControlMessage_t,
//                             BitTorrent_SessionMessage_t> BitTorrent_Message_t;
//typedef BitTorrent_SessionMessage_T<BitTorrent_ControlMessage_t,
//                                    BitTorrent_Message_t> BitTorrent_SessionMessage_t;

//typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
//                                    BitTorrent_SessionData_t,
//                                    Stream_SessionMessageType,
//                                    struct BitTorrent_Record,
//                                    BitTorrent_SessionMessage_t> BitTorrent_ISessionNotify_t;

//typedef Common_INotify_T<unsigned int,
//                         BitTorrent_Stream_SessionData,
//                         BitTorrent_Record,
//                         BitTorrent_SessionMessage> BitTorrent_IStreamNotify_t;
typedef Stream_INotify_T<Stream_SessionMessageType> BitTorrent_INotify_t;

//typedef BitTorrent_Stream_T<BitTorrent_StreamState,
//                            BitTorrent_StreamConfiguration,
//                            BitTorrent_RuntimeStatistic_t,
//                            BitTorrent_ModuleHandlerConfiguration,
//                            BitTorrent_SessionData,
//                            BitTorrent_SessionData_t,
//                            BitTorrent_ControlMessage_t,
//                            BitTorrent_Message_t,
//                            BitTorrent_SessionMessage_t> BitTorrent_Stream_t;

#endif
