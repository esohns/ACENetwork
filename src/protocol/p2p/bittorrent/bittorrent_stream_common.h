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

#include "stream_dec_common.h"

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

typedef Stream_DataBase_T<struct BitTorrent_PeerMessageData> BitTorrent_PeerMessageData_t;

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
struct BitTorrent_PeerSessionData;
typedef Stream_SessionData_T<struct BitTorrent_PeerSessionData> BitTorrent_PeerSessionData_t;
struct BitTorrent_TrackerSessionData;
typedef Stream_SessionData_T<struct BitTorrent_TrackerSessionData> BitTorrent_TrackerSessionData_t;

typedef BitTorrent_Message_T<BitTorrent_PeerSessionData_t,
                             struct Stream_UserData> BitTorrent_Message_t;
typedef BitTorrent_SessionMessage_T<struct BitTorrent_PeerSessionData,
                                    struct Stream_UserData> BitTorrent_PeerSessionMessage_t;
typedef BitTorrent_TrackerMessage_T<BitTorrent_TrackerSessionData_t,
                                    struct Stream_UserData> BitTorrent_TrackerMessage_t;
typedef BitTorrent_SessionMessage_T<struct BitTorrent_TrackerSessionData,
                                    struct Stream_UserData> BitTorrent_TrackerSessionMessage_t;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct BitTorrent_AllocatorConfiguration,
                                BitTorrent_Message_t,
                                BitTorrent_PeerSessionMessage_t> BitTorrent_PeerControlMessage_t;
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct BitTorrent_AllocatorConfiguration,
                                BitTorrent_TrackerMessage_t,
                                BitTorrent_TrackerSessionMessage_t> BitTorrent_TrackerControlMessage_t;

struct BitTorrent_Configuration;
struct BitTorrent_ConnectionState;
struct Net_SocketHandlerConfiguration;
struct BitTorrent_SessionState;
struct Common_UI_State;
typedef BitTorrent_PeerStream_T<struct BitTorrent_StreamState,
                                struct BitTorrent_StreamConfiguration,
                                BitTorrent_RuntimeStatistic_t,
                                struct BitTorrent_ModuleHandlerConfiguration,
                                struct BitTorrent_PeerSessionData,
                                BitTorrent_PeerSessionData_t,
                                BitTorrent_PeerControlMessage_t,
                                BitTorrent_Message_t,
                                BitTorrent_PeerSessionMessage_t,
                                struct BitTorrent_Configuration,
                                struct BitTorrent_ConnectionState,
                                struct Net_SocketHandlerConfiguration,
                                struct BitTorrent_SessionState,
                                struct Common_UI_State> BitTorrent_PeerStream_t;

struct BitTorrent_SessionConfiguration;
struct BitTorrent_ProtocolConfiguration;
template <typename AddressType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename ConfigurationType,
          typename StateType>
class BitTorrent_ISession_T;
struct Net_SocketConfiguration;
enum Stream_StateMachine_ControlState;
typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              struct BitTorrent_PeerConnectionConfiguration,
                              struct BitTorrent_TrackerConnectionConfiguration,
                              struct BitTorrent_PeerConnectionState,
                              BitTorrent_RuntimeStatistic_t,
                              struct Net_SocketConfiguration,
                              struct Net_SocketHandlerConfiguration,
                              struct Net_SocketHandlerConfiguration,
                              BitTorrent_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_SessionConfiguration,
                              struct BitTorrent_SessionState> BitTorrent_ISession_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct BitTorrent_PeerSessionData,
                                    enum Stream_SessionMessageType,
                                    BitTorrent_Message_t,
                                    BitTorrent_PeerSessionMessage_t> BitTorrent_IPeerNotify_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct BitTorrent_TrackerSessionData,
                                    enum Stream_SessionMessageType,
                                    BitTorrent_TrackerMessage_t,
                                    BitTorrent_TrackerSessionMessage_t> BitTorrent_ITrackerNotify_t;
typedef std::list<BitTorrent_IPeerNotify_t*> BitTorrent_IPeerSubscribers_t;
typedef BitTorrent_IPeerSubscribers_t::const_iterator BitTorrent_IPeerSubscribersIterator_t;
typedef std::list<BitTorrent_ITrackerNotify_t*> BitTorrent_ITrackerSubscribers_t;
typedef BitTorrent_ITrackerSubscribers_t::const_iterator BitTorrent_ITrackerSubscribersIterator_t;
struct BitTorrent_PeerModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline BitTorrent_PeerModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
//   , CBData (NULL)
   , destination ()
   , protocolConfiguration (NULL)
   , session (NULL)
//   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {};

//  struct Common_UI_GTKState*               CBData;
  std::string                              destination; // folder
  struct BitTorrent_ProtocolConfiguration* protocolConfiguration;
  BitTorrent_ISession_t*                   session;
//  struct BitTorrent_StreamConfiguration*   streamConfiguration;
  BitTorrent_IPeerNotify_t*                subscriber;  // (initial) subscriber
  BitTorrent_IPeerSubscribers_t*           subscribers;
};
struct BitTorrent_TrackerModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline BitTorrent_TrackerModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
//   , CBData (NULL)
   , destination ()
   , protocolConfiguration (NULL)
   , session (NULL)
//   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {};

//  struct Common_UI_GTKState*               CBData;
  std::string                              destination; // folder
  struct BitTorrent_ProtocolConfiguration* protocolConfiguration;
  BitTorrent_ISession_t*                   session;
//  struct BitTorrent_StreamConfiguration*   streamConfiguration;
  BitTorrent_ITrackerNotify_t*             subscriber;  // (initial) subscriber
  BitTorrent_ITrackerSubscribers_t*        subscribers;
};

struct BitTorrent_PeerStreamConfiguration
 : Stream_Configuration
{
  inline BitTorrent_PeerStreamConfiguration ()
   : Stream_Configuration ()
   , filename ()
   , metaInfo (NULL)
   , moduleHandlerConfiguration (NULL)
  {
    bufferSize = BITTORRENT_BUFFER_SIZE;
  };

  std::string                                       filename; // .torrent file
  Bencoding_Dictionary_t*                           metaInfo;

  struct BitTorrent_PeerModuleHandlerConfiguration* moduleHandlerConfiguration;
};
struct BitTorrent_TrackerStreamConfiguration
 : Stream_Configuration
{
  inline BitTorrent_TrackerStreamConfiguration ()
   : Stream_Configuration ()
   , filename ()
   , metaInfo (NULL)
   , moduleHandlerConfiguration (NULL)
  {
    bufferSize = BITTORRENT_BUFFER_SIZE;
  };

  std::string                                          filename; // .torrent file
  Bencoding_Dictionary_t*                              metaInfo;

  struct BitTorrent_TrackerModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct BitTorrent_ConnectionState;
struct BitTorrent_PeerSessionData
 : Stream_SessionData
{
  inline BitTorrent_PeerSessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
//   , currentStatistic ()
   , filename ()
   , handShake (NULL)
  {};
  inline BitTorrent_PeerSessionData& operator+= (const BitTorrent_PeerSessionData& rhs_in)
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
  std::string                        filename; // .torrent file
  struct BitTorrent_PeerHandShake*   handShake;
};

struct BitTorrent_TrackerSessionData
 : Stream_SessionData
{
  inline BitTorrent_TrackerSessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
//   , userData (NULL)
  {};
  inline BitTorrent_TrackerSessionData& operator+= (const BitTorrent_TrackerSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

//    currentStatistic =
//        ((currentStatistic.timeStamp > rhs_in.currentStatistic.timeStamp) ? currentStatistic
//                                                                          : rhs_in.currentStatistic);

    return *this;
  }

  struct BitTorrent_ConnectionState*        connectionState;
  enum Stream_Decoder_CompressionFormatType format;
//  struct BitTorrent_UserData*               userData;
};

struct BitTorrent_PeerStreamState
 : Stream_State
{
  inline BitTorrent_PeerStreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {};

  struct BitTorrent_PeerSessionData* sessionData;
  struct BitTorrent_UserData*        userData;
};
struct BitTorrent_TrackerStreamState
 : Stream_State
{
  inline BitTorrent_TrackerStreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {};

  struct BitTorrent_TrackerSessionData* sessionData;
  struct BitTorrent_UserData*           userData;
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
typedef BitTorrent_TrackerStream_T<struct BitTorrent_TrackerStreamState,
                                   struct BitTorrent_TrackerStreamConfiguration,
                                   BitTorrent_RuntimeStatistic_t,
                                   struct BitTorrent_TrackerModuleHandlerConfiguration,
                                   struct BitTorrent_TrackerSessionData,
                                   BitTorrent_TrackerSessionData_t,
                                   BitTorrent_TrackerControlMessage_t,
                                   BitTorrent_TrackerMessage_t,
                                   BitTorrent_TrackerSessionMessage_t,
                                   BitTorrent_PeerStream_t,
                                   struct BitTorrent_Configuration,
                                   struct BitTorrent_ConnectionState,
                                   struct Net_SocketHandlerConfiguration,
                                   struct BitTorrent_SessionState,
                                   struct Common_UI_State> BitTorrent_TrackerStream_t;

//////////////////////////////////////////

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
class BitTorrent_PeerStreamHandler_T;
typedef BitTorrent_PeerStreamHandler_T<struct BitTorrent_PeerSessionData,
                                       struct Stream_UserData,
                                       BitTorrent_ISession_t,
                                       struct Common_UI_State> BitTorrent_PeerStreamHandler_t;
template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
class BitTorrent_TrackerStreamHandler_T;
typedef BitTorrent_TrackerStreamHandler_T<struct BitTorrent_TrackerSessionData,
                                          struct Stream_UserData,
                                          BitTorrent_ISession_t,
                                          struct Common_UI_State> BitTorrent_TrackerStreamHandler_t;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
class Stream_Module_MessageHandler_T;
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct BitTorrent_PeerModuleHandlerConfiguration,
                                       BitTorrent_PeerControlMessage_t,
                                       BitTorrent_Message_t,
                                       BitTorrent_PeerSessionMessage_t,
                                       Stream_SessionId_t,
                                       struct BitTorrent_PeerSessionData> BitTorrent_PeerHandler_t;

DATASTREAM_MODULE_INPUT_ONLY_T (struct BitTorrent_PeerSessionData,                // session data
                                enum Stream_SessionMessageType,                   // session event type
                                struct BitTorrent_PeerModuleHandlerConfiguration, // module handler configuration type
                                BitTorrent_INotify_t,                             // stream notification interface type
                                BitTorrent_PeerHandler_t,                         // writer type
                                BitTorrent_PeerHandler);                          // name

//----------------------------------------

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct BitTorrent_TrackerModuleHandlerConfiguration,
                                       BitTorrent_TrackerControlMessage_t,
                                       BitTorrent_TrackerMessage_t,
                                       BitTorrent_TrackerSessionMessage_t,
                                       Stream_SessionId_t,
                                       struct BitTorrent_TrackerSessionData> BitTorrent_TrackerHandler_t;

DATASTREAM_MODULE_INPUT_ONLY_T (struct BitTorrent_TrackerSessionData,                // session data
                                enum Stream_SessionMessageType,                      // session event type
                                struct BitTorrent_TrackerModuleHandlerConfiguration, // module handler configuration type
                                BitTorrent_INotify_t,                                // stream notification interface type
                                BitTorrent_TrackerHandler_t,                         // writer type
                                BitTorrent_TrackerHandler);                          // name

#endif
