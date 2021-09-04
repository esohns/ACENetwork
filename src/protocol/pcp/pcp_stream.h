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

#ifndef PCP_STREAM_H
#define PCP_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_stat_common.h"
#include "stream_stat_statistic_report.h"

#include "pcp_common.h"
//#include "pcp_module_bisector.h"
//#include "pcp_exports.h"
#include "pcp_module_parser.h"
#include "pcp_module_streamer.h"
#include "pcp_stream_common.h"

//extern NET_PROTOCOL_PCP_Export const char libacenetwork_default_pcp_marshal_module_name_string[];
extern const char libacenetwork_default_pcp_marshal_module_name_string[];
//extern NET_PROTOCOL_PCP_Export const char libacenetwork_default_pcp_stream_name_string[];
extern const char libacenetwork_default_pcp_stream_name_string[];

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename StatisticHandlerType,
          ////////////////////////////////
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class PCP_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        libacenetwork_default_pcp_stream_name_string,
                        int,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        libacenetwork_default_pcp_stream_name_string,
                        int,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

 public:
  PCP_Stream_T ();
  inline virtual ~PCP_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&); // configuration
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration
#endif

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

 private:
  typedef PCP_Module_Streamer_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType> STREAMER_T;
  //typedef PCP_Module_Bisector_T<ACE_SYNCH_MUTEX,
  //                               ACE_MT_SYNCH,
  //                               Common_TimePolicy_t,
  //                               SessionMessageType,
  //                               ProtocolMessageType,
  //                               ModuleHandlerConfigurationType,
  //                               StreamStateType,
  //                               SessionDataType,
  //                               SessionDataContainerType,
  //                               StatisticContainerType> BISECTOR_T;
  //typedef PCP_Module_Parser_T<ACE_MT_SYNCH,
  //                             Common_TimePolicy_t,
  //                             SessionMessageType,
  //                             ProtocolMessageType> PARSER_T;
  typedef PCP_Module_ParserH_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ModuleHandlerConfigurationType,
                                int,
                                enum Stream_SessionMessageType,
                                StreamStateType,
                                SessionDataType,
                                SessionDataContainerType,
                                StatisticContainerType,
                                StatisticHandlerType> PARSER_T;
  //typedef Stream_StreamModule_T<ACE_MT_SYNCH,
  //                              Common_TimePolicy_t,
  //                              struct Stream_ModuleConfiguration,
  //                              ModuleHandlerConfigurationType,
  //                              STREAMER_T,
  //                              BISECTOR_T> MODULE_MARSHAL_T;
  //typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
  //                                       Common_TimePolicy_t,
  //                                       Stream_ModuleConfiguration,
  //                                       ModuleHandlerConfigurationType,
  //                                       PARSER_T> MODULE_PARSER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                SessionDataType,                   // session data type
                                enum Stream_SessionMessageType,    // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacenetwork_default_pcp_marshal_module_name_string,
                                Stream_INotify_t,                  // stream notification interface type
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;

  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        PCP_Opcode_t,
                                                        PCP_Statistic_t,
                                                        StatisticHandlerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        PCP_Opcode_t,
                                                        PCP_Statistic_t,
                                                        StatisticHandlerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                SessionDataType,                   // session data type
                                enum Stream_SessionMessageType,    // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,                  // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (PCP_Stream_T (const PCP_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (PCP_Stream_T& operator= (const PCP_Stream_T&))

  // modules
  //MODULE_PARSER_T    parser_;
  MODULE_MARSHAL_T   marshal_;
  MODULE_STATISTIC_T statistic_;
};

// include template definition
#include "pcp_stream.inl"

#endif
