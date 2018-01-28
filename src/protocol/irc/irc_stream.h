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

#ifndef IRC_Stream_T_H
#define IRC_Stream_T_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_net_io_stream.h"

#include "stream_stat_common.h"
#include "stream_stat_statistic_report.h"

#include "irc_common.h"
#include "irc_exports.h"
#include "irc_module_bisector.h"
#include "irc_module_parser.h"
#include "irc_module_streamer.h"
#include "irc_record.h"
#include "irc_stream_common.h"

extern IRC_Export const char libacenetwork_default_irc_marshal_module_name_string[];
extern IRC_Export const char libacenetwork_default_irc_parser_module_name_string[];
extern IRC_Export const char libacenetwork_default_irc_stream_name_string[];

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ///////////////////////////////
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
          ////////////////////////////////
          typename UserDataType>
class IRC_Stream_T
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        libacenetwork_default_irc_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        struct IRC_AllocatorConfiguration,
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
                                        libacenetwork_default_irc_stream_name_string,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        StreamStateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        struct IRC_AllocatorConfiguration,
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
  IRC_Stream_T ();
  inline virtual ~IRC_Stream_T () { inherited::shutdown (); }

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE);

  // implement Common_IStatistic_T
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  // convenient types
  typedef IRC_Module_Streamer_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                ModuleHandlerConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                UserDataType> STREAMER_T;
  typedef IRC_Module_Bisector_T<ACE_MT_SYNCH,
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
                                UserDataType> BISECTOR_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,             // session id type
                                SessionDataType,                // session data type
                                enum Stream_SessionMessageType, // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacenetwork_default_irc_marshal_module_name_string,
                                Stream_INotify_t,               // stream notification interface type
                                STREAMER_T,
                                BISECTOR_T> MODULE_MARSHAL_T;

  typedef IRC_Module_Parser_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              ModuleHandlerConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType> PARSER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Stream_SessionId_t,             // session id type
                                         SessionDataType,                // session data type
                                         enum Stream_SessionMessageType, // session event type
                                         struct Stream_ModuleConfiguration,
                                         ModuleHandlerConfigurationType,
                                         libacenetwork_default_irc_parser_module_name_string,
                                         Stream_INotify_t,               // stream notification interface type
                                         PARSER_T> MODULE_PARSER_T;

  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        IRC_CommandType_t,
                                                        IRC_Statistic_t,
                                                        TimerManagerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        IRC_CommandType_t,
                                                        IRC_Statistic_t,
                                                        TimerManagerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,             // session id type
                                SessionDataType,                // session data type
                                enum Stream_SessionMessageType, // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,               // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (IRC_Stream_T (const IRC_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Stream_T& operator= (const IRC_Stream_T&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

// include template definition
#include "irc_stream.inl"

#endif
