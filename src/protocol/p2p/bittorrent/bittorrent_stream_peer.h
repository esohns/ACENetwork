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

#ifndef BITTORRENT_STREAM_PEER_H
#define BITTORRENT_STREAM_PEER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_inotify.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_net_io_stream.h"

#include "stream_stat_common.h"
#include "stream_stat_statistic_report.h"

#include "bittorrent_common.h"
//#include "bittorrent_module_bisector.h"
//#include "bittorrent_module_handler.h"
#include "bittorrent_module_parser.h"
#include "bittorrent_module_streamer.h"

// forward declarations
typedef Stream_INotify_T<enum Stream_SessionMessageType> BitTorrent_INotify_t;

extern const char libacenetwork_default_bittorrent_marshal_module_name_string[];
extern const char libacenetwork_default_bittorrent_handler_module_name_string[];
extern const char libacenetwork_default_bittorrent_stream_name_string[];

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType, // socket-
          typename SessionStateType,
          ////////////////////////////////
          typename ConnectionManagerType,
          typename UserDataType>
class BitTorrent_PeerStream_T
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        libacenetwork_default_bittorrent_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        struct Common_Parser_FlexAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        ModuleHandlerConfigurationType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ACE_INET_Addr,
                                        ConnectionManagerType,
                                        UserDataType>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        libacenetwork_default_bittorrent_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        struct Common_Parser_FlexAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        ModuleHandlerConfigurationType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ACE_INET_Addr,
                                        ConnectionManagerType,
                                        UserDataType> inherited;

 public:
  BitTorrent_PeerStream_T ();
  inline virtual ~BitTorrent_PeerStream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
                           ACE_HANDLE);
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);
#endif

  // implement Common_IStatistic_T
  // *NOTE*: delegates to the statistic report module
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  // just a dummy (set statisticReportingInterval instead)
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 private:
  typedef BitTorrent_PeerStream_T<StreamStateType,
                                  ConfigurationType,
                                  StatisticContainerType,
                                  TimerManagerType,
                                  ModuleHandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConnectionConfigurationType,
                                  ConnectionStateType,
                                  HandlerConfigurationType, // socket-
                                  SessionStateType,
                                  ConnectionManagerType,
                                  UserDataType> OWN_TYPE_T;
  typedef BitTorrent_Module_Streamer_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       ModuleHandlerConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType> STREAMER_T;
  //typedef BitTorrent_Module_Bisector_T<ACE_SYNCH_MUTEX,
  //                                     ACE_MT_SYNCH,
  //                                     Common_TimePolicy_t,
  //                                     SessionMessageType,
  //                                     ProtocolMessageType,
  //                                     ModuleHandlerConfigurationType,
  //                                     StreamStateType,
  //                                     SessionDataType,
  //                                     SessionDataContainerType,
  //                                     StatisticContainerType> BISECTOR_T;
  typedef BitTorrent_Module_PeerParser_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         ModuleHandlerConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         UserDataType> PARSER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,             // session id type
                                SessionDataType,                // session data type
                                enum Stream_SessionMessageType, // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacenetwork_default_bittorrent_marshal_module_name_string,
                                BitTorrent_INotify_t,           // stream notification interface type
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;
  //typedef BitTorrent_Module_Parser_T<ACE_MT_SYNCH,
  //                             Common_TimePolicy_t,
  //                             SessionMessageType,
  //                             ProtocolMessageType> PARSER_T;
  //typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
  //                                       Common_TimePolicy_t,
  //                                       struct Stream_ModuleConfiguration,
  //                                       ModuleHandlerConfigurationType,
  //                                       PARSER_T> MODULE_PARSER_T;

  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        enum BitTorrent_MessageType,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        enum BitTorrent_MessageType,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,                // session id type
                                SessionDataType,                   // session data type
                                enum Stream_SessionMessageType,    // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacestream_default_stat_report_module_name_string,
                                BitTorrent_INotify_t,              // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_PeerStream_T (const BitTorrent_PeerStream_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_PeerStream_T& operator= (const BitTorrent_PeerStream_T&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

// include template definition
#include "bittorrent_stream_peer.inl"

#endif
