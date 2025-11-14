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

#ifndef FTP_STREAM_H
#define FTP_STREAM_H

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

#include "ftp_common.h"
//#include "ftp_module_bisector.h"
//#include "ftp_exports.h"
#include "ftp_module_parser.h"
#include "ftp_module_streamer.h"
//#include "ftp_stream_common.h"

//extern NET_PROTOCOL_FTP_Export const char libacenetwork_default_ftp_marshal_module_name_string[];
extern const char libacenetwork_default_ftp_marshal_module_name_string[];
//extern NET_PROTOCOL_FTP_Export const char libacenetwork_default_ftp_stream_name_string[];
extern const char libacenetwork_default_ftp_stream_name_string[];

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename StatisticHandlerType,
          ////////////////////////////////
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionManagerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class FTP_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        libacenetwork_default_ftp_stream_name_string,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleHandlerConfigurationType,
                        SessionManagerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        struct Stream_UserData>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        libacenetwork_default_ftp_stream_name_string,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleHandlerConfigurationType,
                        SessionManagerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        struct Stream_UserData> inherited;

 public:
  FTP_Stream_T ();
  inline virtual ~FTP_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

 private:
  typedef FTP_Module_Streamer_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType> STREAMER_T;
  //typedef FTP_Module_Parser_T<ACE_MT_SYNCH,
  //                            Common_TimePolicy_t,
  //                            SessionMessageType,
  //                            ProtocolMessageType> PARSER_T;
  typedef FTP_Module_ParserH_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ModuleHandlerConfigurationType,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType> PARSER_T;
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
                                typename SessionMessageType::DATA_T::DATA_T, // session data type
                                enum Stream_SessionMessageType,    // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacenetwork_default_ftp_marshal_module_name_string,
                                Stream_INotify_t,                  // stream notification interface type
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;

  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        FTP_Code_t,
                                                        FTP_Statistic_t,
                                                        Common_Timer_Manager_t,
                                                        struct Stream_UserData> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        ModuleHandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        FTP_Code_t,
                                                        FTP_Statistic_t,
                                                        Common_Timer_Manager_t,
                                                        struct Stream_UserData> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                typename SessionMessageType::DATA_T::DATA_T, // session data type
                                enum Stream_SessionMessageType,    // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,                  // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (FTP_Stream_T (const FTP_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Stream_T& operator= (const FTP_Stream_T&))

  // modules
  //MODULE_PARSER_T    parser_;
  MODULE_MARSHAL_T   marshal_;
  MODULE_STATISTIC_T statistic_;
};

// include template definition
#include "ftp_stream.inl"

#endif
