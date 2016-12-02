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

#ifndef BITTORRENT_CLIENT_COMMON_H
#define BITTORRENT_CLIENT_COMMON_H

#include <string>

#include "common.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_defines.h"

#include "bittorrent_common.h"
#include "bittorrent_stream_common.h"

struct BitTorrent_Client_CursesState;
struct BitTorrent_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline BitTorrent_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , cursesState (NULL)
  {};

  struct BitTorrent_Client_CursesState* cursesState;
};

struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_ConnectionState;
struct BitTorrent_Client_SocketHandlerConfiguration;
struct BitTorrent_Client_PeerStreamState;
struct BitTorrent_Client_StreamConfiguration;
struct BitTorrent_Client_ModuleHandlerConfiguration;
struct BitTorrent_Client_PeerSessionData;
template <typename DataType>
class Stream_SessionData_T;
typedef Stream_SessionData_T<struct BitTorrent_Client_PeerSessionData> BitTorrent_Client_PeerSessionData_t;
template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_ControlMessage_T;
template <typename SessionDataType,
          typename UserDataType>
class BitTorrent_Message_T;
typedef BitTorrent_Message_T<BitTorrent_Client_PeerSessionData_t,
                             struct BitTorrent_Client_UserData> BitTorrent_Client_PeerMessage_t;
template <typename SessionDataType,
          typename UserDataType>
class BitTorrent_SessionMessage_T;
typedef BitTorrent_SessionMessage_T<struct BitTorrent_Client_PeerSessionData,
                                    struct BitTorrent_Client_UserData> BitTorrent_Client_PeerSessionMessage_t;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct BitTorrent_AllocatorConfiguration,
                                BitTorrent_Client_PeerMessage_t,
                                BitTorrent_Client_PeerSessionMessage_t> BitTorrent_Client_PeerControlMessage_t;
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
typedef BitTorrent_PeerStream_T<struct BitTorrent_Client_PeerStreamState,
                                struct BitTorrent_Client_StreamConfiguration,
                                BitTorrent_RuntimeStatistic_t,
                                struct BitTorrent_Client_ModuleHandlerConfiguration,
                                struct BitTorrent_Client_PeerSessionData,
                                BitTorrent_Client_PeerSessionData_t,
                                BitTorrent_Client_PeerControlMessage_t,
                                BitTorrent_Client_PeerMessage_t,
                                BitTorrent_Client_PeerSessionMessage_t,
                                struct BitTorrent_Client_Configuration,
                                struct BitTorrent_Client_ConnectionState,
                                struct BitTorrent_Client_SocketHandlerConfiguration,
                                struct BitTorrent_Client_SessionState,
                                struct BitTorrent_Client_GTK_CBData> BitTorrent_Client_PeerStream_t;
struct BitTorrent_Client_SessionConfiguration;
struct BitTorrent_Client_SessionState;
template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename PeerStreamType,
          typename StreamStatusType,
          typename ConfigurationType,
          typename StateType>
class BitTorrent_ISession_T;
typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              struct BitTorrent_Client_Configuration,
                              struct BitTorrent_Client_ConnectionState,
                              BitTorrent_RuntimeStatistic_t,
                              struct Net_SocketConfiguration,
                              struct BitTorrent_Client_SocketHandlerConfiguration,
                              BitTorrent_Client_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_Client_SessionConfiguration,
                              struct BitTorrent_Client_SessionState> BitTorrent_Client_ISession_t;
template <typename SessionInterfaceType>
class BitTorrent_IControl_T;
typedef BitTorrent_IControl_T<BitTorrent_Client_ISession_t> BitTorrent_Client_IControl_t;
struct BitTorrent_Client_ModuleHandlerConfiguration;
struct BitTorrent_Client_ThreadData
{
  inline BitTorrent_Client_ThreadData ()
   : configuration (NULL)
   , controller (NULL)
   , cursesState (NULL)
   , filename ()
//   , groupID (-1)
//   , moduleHandlerConfiguration (NULL)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  struct BitTorrent_Client_Configuration*              configuration;
  BitTorrent_Client_IControl_t*                        controller;
  struct BitTorrent_Client_CursesState*                cursesState;
  std::string                                          filename; // metainfo (aka '.torrent') file URI
//  int                                                  groupID;
//  struct BitTorrent_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration;
  bool                                                 useReactor;
};

#endif
