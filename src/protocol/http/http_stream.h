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

#ifndef HTTP_STREAM_H
#define HTTP_STREAM_H

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

#include "http_codes.h"
#include "http_common.h"
//#include "http_exports.h"
#include "http_module_parser.h"
#include "http_module_streamer.h"
#include "http_stream_common.h"

// forward declarations
//extern HTTP_Export const char libacenetwork_default_http_marshal_module_name_string[];
extern const char libacenetwork_default_http_marshal_module_name_string[];
//extern HTTP_Export const char libacenetwork_default_http_stream_name_string[];
extern const char libacenetwork_default_http_stream_name_string[];

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename AllocatorConfigurationType,
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConnectionManagerType,
          typename UserDataType>
class HTTP_Stream_T
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        libacenetwork_default_http_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        AllocatorConfigurationType,
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
                                        libacenetwork_default_http_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        AllocatorConfigurationType,
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
  HTTP_Stream_T ();
  inline virtual ~HTTP_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE);

  // override (part of) Common_IStatistic_T
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ModuleHandlerConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType> STREAMER_T;
  typedef HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ModuleHandlerConfigurationType,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                StreamStateType,
                                SessionDataType,
                                SessionDataContainerType,
                                StatisticContainerType,
                                TimerManagerType,
                                UserDataType> PARSER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,
                                SessionDataType,
                                enum Stream_SessionMessageType,
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacenetwork_default_http_marshal_module_name_string,
                                Stream_INotify_t,
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;

  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        HTTP_Method_t,
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
                                                        HTTP_Method_t,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,
                                SessionDataType,
                                enum Stream_SessionMessageType,
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (HTTP_Stream_T (const HTTP_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Stream_T& operator= (const HTTP_Stream_T&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

// include template definition
#include "http_stream.inl"

#endif
