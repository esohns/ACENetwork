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

#include <ace/INET_Addr.h>

#include "common_ui_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_imodule.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"
#include "stream_streammodule_base.h"

#include "net_common.h"
#include "net_defines.h"

#include "bittorrent_common.h"
#include "bittorrent_message.h"
//#include "bittorrent_module_parser.h"
#include "bittorrent_sessionmessage.h"
//#include "bittorrent_stream.h"

struct BitTorrent_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline BitTorrent_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    buffer = NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

typedef Stream_DataBase_T<struct BitTorrent_Record> BitTorrent_MessageData_t;

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
struct BitTorrent_StreamState;
struct BitTorrent_StreamConfiguration;
typedef Stream_Statistic BitTorrent_RuntimeStatistic_t;
struct BitTorrent_ModuleHandlerConfiguration;
struct BitTorrent_SessionData;
typedef Stream_SessionData_T<BitTorrent_SessionData> BitTorrent_SessionData_t;
typedef BitTorrent_Message_T<BitTorrent_SessionData> BitTorrent_Message_t;
typedef BitTorrent_SessionMessage_T<BitTorrent_SessionData> BitTorrent_SessionMessage_t;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct BitTorrent_AllocatorConfiguration,
                                BitTorrent_Message_t,
                                BitTorrent_SessionMessage_t> BitTorrent_ControlMessage_t;
struct BitTorrent_Configuration;
struct BitTorrent_ConnectionState;
struct Net_SocketHandlerConfiguration;
struct BitTorrent_SessionState;
struct Common_UI_State;
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

struct BitTorrent_ProtocolConfiguration;
template <typename AddressType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename StateType>
class BitTorrent_ISession_T;
struct Net_SocketConfiguration;
enum Stream_StateMachine_ControlState;
typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              struct BitTorrent_Configuration,
                              struct BitTorrent_ConnectionState,
                              BitTorrent_RuntimeStatistic_t,
                              struct Net_SocketConfiguration,
                              struct Net_SocketHandlerConfiguration,
                              BitTorrent_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_SessionState> BitTorrent_ISession_t;
struct BitTorrent_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline BitTorrent_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , CBData (NULL)
   , protocolConfiguration (NULL)
   , session (NULL)
   , streamConfiguration (NULL)
  {};

  struct Common_UI_GTKState*               CBData;
  struct BitTorrent_ProtocolConfiguration* protocolConfiguration;
  BitTorrent_ISession_t*                   session;
  struct BitTorrent_StreamConfiguration*   streamConfiguration;
};

struct BitTorrent_StreamConfiguration
 : Stream_Configuration
{
  inline BitTorrent_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
   , trackerMessageAllocator (NULL)
   , trackerModule (NULL)
  {
    bufferSize = BITTORRENT_BUFFER_SIZE;
  };

  struct BitTorrent_ModuleHandlerConfiguration* moduleHandlerConfiguration;

  Stream_IAllocator*                            trackerMessageAllocator;
  Stream_Module_t*                              trackerModule;
};

struct BitTorrent_ConnectionState;
struct BitTorrent_SessionData
 : Stream_SessionData
{
  inline BitTorrent_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
//   , currentStatistic ()
   , handshake (NULL)
  {};
  inline BitTorrent_SessionData& operator+= (const BitTorrent_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

//    currentStatistic =
//        ((currentStatistic.timeStamp > rhs_in.currentStatistic.timeStamp) ? currentStatistic
//                                                                          : rhs_in.currentStatistic);

    return *this;
  }

  struct BitTorrent_ConnectionState* connectionState;
//  BitTorrent_RuntimeStatistic_t      currentStatistic;
  struct BitTorrent_PeerHandshake*   handshake;
};

struct BitTorrent_StreamState
 : Stream_State
{
  inline BitTorrent_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
  {};

  BitTorrent_SessionData_t* currentSessionData;
};

typedef Stream_INotify_T<Stream_SessionMessageType> BitTorrent_INotify_t;

//////////////////////////////////////////

struct BitTorrent_ProtocolConfiguration
{
  inline BitTorrent_ProtocolConfiguration ()
  {};

};

//////////////////////////////////////////

template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename PeerStreamType,
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
                                   BitTorrent_PeerStream_t,
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

//////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
class BitTorrent_Module_PeerHandler_T;

typedef BitTorrent_Module_PeerHandler_T<ACE_INET_Addr,
                                        struct BitTorrent_ModuleHandlerConfiguration,
                                        BitTorrent_RuntimeStatistic_t,
                                        BitTorrent_ControlMessage_t,
                                        BitTorrent_Message_t,
                                        BitTorrent_SessionMessage_t,
                                        struct BitTorrent_SessionData,
                                        BitTorrent_PeerStream_t,
                                        enum Stream_StateMachine_ControlState,
                                        struct Net_SocketConfiguration,
                                        struct Net_SocketHandlerConfiguration,
                                        struct BitTorrent_Configuration,
                                        struct BitTorrent_ConnectionState,
                                        struct BitTorrent_SessionState,
                                        struct Common_GTKState> BitTorrent_PeerHandler_t;

DATASTREAM_MODULE_INPUT_ONLY_T (struct BitTorrent_SessionData,                // session data
                                enum Stream_SessionMessageType,               // session event type
                                struct BitTorrent_ModuleHandlerConfiguration, // module handler configuration type
                                BitTorrent_INotify_t,                         // stream notification interface type
                                BitTorrent_PeerHandler_t,                     // writer type
                                BitTorrent_PeerHandler);                      // name

//----------------------------------------

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename SessionStateType,
          typename CBDataType>
class BitTorrent_Module_TrackerHandler_T;

typedef BitTorrent_Module_TrackerHandler_T<ACE_INET_Addr,
                                           struct BitTorrent_ModuleHandlerConfiguration,
                                           BitTorrent_RuntimeStatistic_t,
                                           BitTorrent_ControlMessage_t,
                                           BitTorrent_Message_t,
                                           BitTorrent_SessionMessage_t,
                                           struct BitTorrent_SessionData,
                                           BitTorrent_PeerStream_t,
                                           enum Stream_StateMachine_ControlState,
                                           struct Net_SocketConfiguration,
                                           struct Net_SocketHandlerConfiguration,
                                           struct BitTorrent_Configuration,
                                           struct BitTorrent_ConnectionState,
                                           struct BitTorrent_SessionState,
                                           struct Common_GTKState> BitTorrent_TrackerHandler_t;

DATASTREAM_MODULE_INPUT_ONLY_T (struct BitTorrent_SessionData,                // session data
                                enum Stream_SessionMessageType,               // session event type
                                struct BitTorrent_ModuleHandlerConfiguration, // module handler configuration type
                                BitTorrent_INotify_t,                         // stream notification interface type
                                BitTorrent_TrackerHandler_t,                  // writer type
                                BitTorrent_TrackerHandler);                   // name

#endif