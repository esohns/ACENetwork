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

#include "common_ui_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_imodule.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_common.h"

#include "bittorrent_common.h"
#include "bittorrent_message.h"
#include "bittorrent_sessionmessage.h"
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

  struct BitTorrent_Record* record;
};
typedef Stream_DataBase_T<struct BitTorrent_MessageData> BitTorrent_MessageData_t;

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
   , handshake ()
  {};

  struct BitTorrent_ConnectionState* connectionState;
  BitTorrent_RuntimeStatistic_t      currentStatistic;
  struct BitTorrent_PeerHandshake    handshake;
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

typedef BitTorrent_Message_T<BitTorrent_SessionData> BitTorrent_Message_t;
typedef BitTorrent_SessionMessage_T<BitTorrent_SessionData> BitTorrent_SessionMessage_t;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                struct Stream_AllocatorConfiguration,
                                BitTorrent_Message_t,
                                BitTorrent_SessionMessage_t> BitTorrent_ControlMessage_t;

//typedef Stream_INotify_T<Stream_SessionMessageType> BitTorrent_INotify_t;

struct BitTorrent_Configuration;
struct BitTorrent_ConnectionState;
struct Net_SocketHandlerConfiguration;
template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType,
          typename SessionStateType,
          typename CBDataType>
class BitTorrent_PeerStream_T;
typedef BitTorrent_PeerStream_T<struct BitTorrent_StreamState,
                                struct BitTorrent_StreamConfiguration,
                                BitTorrent_RuntimeStatistic_t,
                                struct BitTorrent_ModuleHandlerConfiguration,
                                struct BitTorrent_SessionData,
                                BitTorrent_SessionData_t,
                                BitTorrent_ControlMessage_t,
                                BitTorrent_Message_t,
                                BitTorrent_SessionMessage_t,
                                struct BitTorrent_Configuration,
                                struct BitTorrent_ConnectionState,
                                struct Net_SocketHandlerConfiguration,
                                struct BitTorrent_SessionState,
                                struct Common_UI_State> BitTorrent_PeerStream_t;
template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType,
          typename SessionStateType,
          typename CBDataType>
class BitTorrent_TrackerStream_T;
typedef BitTorrent_TrackerStream_T<struct BitTorrent_StreamState,
                                   struct BitTorrent_StreamConfiguration,
                                   BitTorrent_RuntimeStatistic_t,
                                   struct BitTorrent_ModuleHandlerConfiguration,
                                   struct BitTorrent_SessionData,
                                   BitTorrent_SessionData_t,
                                   BitTorrent_ControlMessage_t,
                                   BitTorrent_Message_t,
                                   BitTorrent_SessionMessage_t,
                                   struct BitTorrent_Configuration,
                                   struct BitTorrent_ConnectionState,
                                   struct Net_SocketHandlerConfiguration,
                                   struct BitTorrent_SessionState,
                                   struct Common_UI_State> BitTorrent_TrackerStream_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct BitTorrent_SessionData,
                                    enum Stream_SessionMessageType,
                                    BitTorrent_Message_t,
                                    BitTorrent_SessionMessage_t> BitTorrent_IStreamNotify_t;

#endif
