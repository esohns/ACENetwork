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

#ifndef TEST_U_COMMON_MODULES_H
#define TEST_U_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_configuration.h"
#include "common_time_common.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_data.h"
#include "stream_streammodule_base.h"

#include "stream_file_sink.h"

#include "stream_stat_statistic_report.h"

#include "http_codes.h"
#include "http_common.h"
#include "http_configuration.h"
//#include "http_module_bisector.h"
#include "http_module_parser.h"
#include "http_module_streamer.h"

#include "test_u_common.h"
//#include "test_u_stream_common.h"

#include "test_u_message.h"
#include "test_u_session_message.h"
#include "test_u_HTTP_decoder_stream_common.h"

// forward declarations
struct Test_U_ModuleHandlerConfiguration;
struct Test_U_HTTPDecoder_SessionData;
typedef Stream_SessionData_T<struct Test_U_HTTPDecoder_SessionData> Test_U_HTTPDecoder_SessionData_t;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Common_FlexParserAllocatorConfiguration> Test_U_HTTPDecoder_ControlMessage_t;

// declare module(s)
typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct Test_U_ModuleHandlerConfiguration,
                             Test_U_HTTPDecoder_ControlMessage_t,
                             Test_U_Message,
                             Test_U_SessionMessage> Test_U_Module_Parser;
typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_U_ModuleHandlerConfiguration,
                               Test_U_HTTPDecoder_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage> Test_U_Module_Streamer;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_ModuleHandlerConfiguration,
                                                      Test_U_HTTPDecoder_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      HTTP_Method_t,
                                                      HTTP_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      struct Test_U_HTTPDecoder_SessionData,
                                                      Test_U_HTTPDecoder_SessionData_t> Test_U_Module_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_ModuleHandlerConfiguration,
                                                      Test_U_HTTPDecoder_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      HTTP_Method_t,
                                                      HTTP_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      struct Test_U_HTTPDecoder_SessionData,
                                                      Test_U_HTTPDecoder_SessionData_t> Test_U_Module_StatisticReport_WriterTask_t;

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_ModuleHandlerConfiguration,
                                   Test_U_HTTPDecoder_ControlMessage_t,
                                   Test_U_Message,
                                   Test_U_SessionMessage,
                                   struct Test_U_HTTPDecoder_SessionData> Test_U_Module_FileWriter;

// declare module(s)
DATASTREAM_MODULE_DUPLEX (struct Test_U_HTTPDecoder_SessionData,    // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                          libacenetwork_protocol_default_http_parser_module_name_string,
                          Stream_INotify_t,                         // stream notification interface type
                          Test_U_Module_Streamer,                   // reader type
                          Test_U_Module_Parser,                     // writer type
                          Test_U_Module_Marshal);                   // name

DATASTREAM_MODULE_DUPLEX (struct Test_U_HTTPDecoder_SessionData,      // session data type
                          enum Stream_SessionMessageType,             // session event type
                          struct Test_U_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                           // stream notification interface type
                          Test_U_Module_StatisticReport_ReaderTask_t, // reader type
                          Test_U_Module_StatisticReport_WriterTask_t, // writer type
                          Test_U_Module_StatisticReport);             // name

DATASTREAM_MODULE_INPUT_ONLY (struct Test_U_HTTPDecoder_SessionData,    // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_U_Module_FileWriter);                // writer type

#endif
