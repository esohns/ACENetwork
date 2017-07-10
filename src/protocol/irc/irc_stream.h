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
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_misc_statistic_report.h"

#include "irc_common.h"
#include "irc_exports.h"
#include "irc_module_bisector.h"
#include "irc_module_parser.h"
#include "irc_module_streamer.h"
#include "irc_record.h"
#include "irc_stream_common.h"

extern IRC_Export const char stream_irc_stream_name_string_[];

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
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
          typename UserDataType>
class IRC_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_irc_stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        struct IRC_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_irc_stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        struct IRC_AllocatorConfiguration,
                        struct Stream_ModuleConfiguration,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

 public:
  IRC_Stream_T ();
  virtual ~IRC_Stream_T ();

  //// implement (part of) Stream_IStreamControlBase
  //virtual bool load (Stream_ModuleList_t&, // return value: module list
  //                   bool&);               // return value: delete modules ?

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
                                UserDataType> BISECTOR_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,             // session id type
                                SessionDataType,                // session data type
                                enum Stream_SessionMessageType, // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                IRC_Stream_INotify_t,           // stream notification interface type
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
                                         IRC_Stream_INotify_t,           // stream notification interface type
                                         PARSER_T> MODULE_PARSER_T;

  typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                     Common_TimePolicy_t,
                                                     ModuleHandlerConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType,
                                                     IRC_CommandType_t,
                                                     IRC_RuntimeStatistic_t,
                                                     SessionDataType,
                                                     SessionDataContainerType> STATISTIC_READER_T;
  typedef Stream_Module_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                     Common_TimePolicy_t,
                                                     ModuleHandlerConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType,
                                                     IRC_CommandType_t,
                                                     IRC_RuntimeStatistic_t,
                                                     SessionDataType,
                                                     SessionDataContainerType> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,             // session id type
                                SessionDataType,                // session data type
                                enum Stream_SessionMessageType, // session event type
                                struct Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                IRC_Stream_INotify_t,           // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (IRC_Stream_T (const IRC_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Stream_T& operator= (const IRC_Stream_T&))

  //// *TODO*: remove this API
  //void ping ();
};

// include template definition
#include "irc_stream.inl"

#endif
