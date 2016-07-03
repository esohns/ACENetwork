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

#ifndef DHCP_STREAM_H
#define DHCP_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "net_module_runtimestatistic.h"

#include "dhcp_common.h"
//#include "dhcp_module_bisector.h"
#include "dhcp_module_parser.h"
#include "dhcp_module_streamer.h"
#include "dhcp_stream_common.h"

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
class DHCP_Stream_T
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        //////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        //////////////////
                        int,
                        int,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        //////////////////
                        ConfigurationType,
                        //////////////////
                        StatisticContainerType,
                        //////////////////
                        Stream_ModuleConfiguration,
                        ModuleHandlerConfigurationType,
                        //////////////////
                        SessionDataType,
                        SessionDataContainerType,
                        SessionMessageType,
                        ProtocolMessageType>
{
 public:
  DHCP_Stream_T (const std::string&); // name
  virtual ~DHCP_Stream_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

  // *TODO*: remove this API
  void ping ();

 private:
  typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                        //////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        //////////////////
                        int,
                        int,
                        Stream_StateMachine_ControlState,
                        StreamStateType,
                        //////////////////
                        ConfigurationType,
                        //////////////////
                        StatisticContainerType,
                        //////////////////
                        Stream_ModuleConfiguration,
                        ModuleHandlerConfigurationType,
                        //////////////////
                        SessionDataType,
                        SessionDataContainerType,
                        SessionMessageType,
                        ProtocolMessageType> inherited;

  typedef DHCP_Module_Streamer_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 SessionMessageType,
                                 ProtocolMessageType> STREAMER_T;
  //typedef DHCP_Module_Bisector_T<ACE_SYNCH_MUTEX,
  //                               ///////////
  //                               ACE_MT_SYNCH,
  //                               Common_TimePolicy_t,
  //                               SessionMessageType,
  //                               ProtocolMessageType,
  //                               ///////////
  //                               ModuleHandlerConfigurationType,
  //                               ///////////
  //                               StreamStateType,
  //                               ///////////
  //                               SessionDataType,
  //                               SessionDataContainerType,
  //                               ///////////
  //                               StatisticContainerType> BISECTOR_T;
  typedef DHCP_Module_ParserH_T<ACE_SYNCH_MUTEX,
                                //////////
                                ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                SessionMessageType,
                                ProtocolMessageType,
                                //////////
                                ModuleHandlerConfigurationType,
                                //////////
                                int,
                                int,
                                StreamStateType,
                                //////////
                                SessionDataType,
                                SessionDataContainerType,
                                //////////
                                StatisticContainerType> PARSER_T;
  //typedef Stream_StreamModule_T<ACE_MT_SYNCH,
  //                              Common_TimePolicy_t,
  //                              Stream_ModuleConfiguration,
  //                              ModuleHandlerConfigurationType,
  //                              STREAMER_T,
  //                              BISECTOR_T> MODULE_MARSHAL_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;

  //typedef DHCP_Module_Parser_T<ACE_MT_SYNCH,
  //                             Common_TimePolicy_t,
  //                             SessionMessageType,
  //                             ProtocolMessageType> PARSER_T;
  //typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
  //                                       Common_TimePolicy_t,
  //                                       Stream_ModuleConfiguration,
  //                                       ModuleHandlerConfigurationType,
  //                                       PARSER_T> MODULE_PARSER_T;

  typedef Net_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            SessionMessageType,
                                            ProtocolMessageType,
                                            DHCP_MessageType_t,
                                            DHCP_RuntimeStatistic_t> STATISTIC_READER_T;
  typedef Net_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            SessionMessageType,
                                            ProtocolMessageType,
                                            DHCP_MessageType_t,
                                            DHCP_RuntimeStatistic_t> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (DHCP_Stream_T ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Stream_T (const DHCP_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_Stream_T& operator= (const DHCP_Stream_T&))

  // modules
  MODULE_MARSHAL_T   marshal_;
  //MODULE_PARSER_T    parser_;
  MODULE_STATISTIC_T runtimeStatistic_;
  // *NOTE*: the final module needs to be supplied to the stream from outside,
  //         otherwise data might be lost if event dispatch runs in (a) separate
  //         thread(s)
  //   DHCP_Module_Handler_Module handler_;
};

// include template implementation
#include "dhcp_stream.inl"

#endif
