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

#ifndef WEBSOCKET_STREAM_H
#define WEBSOCKET_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
//#include "stream_inotify.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_net_io_stream.h"

#include "stream_stat_common.h"
#include "stream_stat_statistic_report.h"

#include "ws_codes.h"
#include "ws_common.h"
//#include "ws_exports.h"
#include "ws_module_parser.h"
#include "ws_module_streamer.h"
#include "ws_stream_common.h"

// forward declarations
//extern WebSocket_Export const char libacenetwork_default_WebSocket_marshal_module_name_string[];
extern const char libacenetwork_default_ws_marshal_module_name_string[];
//extern WebSocket_Export const char libacenetwork_default_WebSocket_stream_name_string[];
extern const char libacenetwork_default_ws_stream_name_string[];

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionManagerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConnectionManagerType,
          typename UserDataType,
          typename ParserDriverType>
class WebSocket_Stream_T
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        libacenetwork_default_ws_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        ModuleHandlerConfigurationType,
                                        SessionManagerType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ACE_INET_Addr,
                                        ConnectionManagerType,
                                        UserDataType>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        libacenetwork_default_ws_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        ModuleHandlerConfigurationType,
                                        SessionManagerType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ACE_INET_Addr,
                                        ConnectionManagerType,
                                        UserDataType> inherited;

 public:
  WebSocket_Stream_T ();
  inline virtual ~WebSocket_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);

  // override (part of) Common_IStatistic_T
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  typedef WebSocket_Module_Streamer_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ModuleHandlerConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType> STREAMER_T;
  typedef WebSocket_Module_Parser_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    ModuleHandlerConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ParserDriverType> PARSER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                typename SessionMessageType::DATA_T::DATA_T,
                                enum Stream_SessionMessageType,
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacenetwork_default_ws_marshal_module_name_string,
                                Stream_INotify_t,
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;

  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        WebSocket_OpCode_t,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        UserDataType> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        WebSocket_OpCode_t,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        UserDataType> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                typename SessionMessageType::DATA_T::DATA_T,
                                enum Stream_SessionMessageType,
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (WebSocket_Stream_T (const WebSocket_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Stream_T& operator= (const WebSocket_Stream_T&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

// include template definition
#include "ws_stream.inl"

#endif
