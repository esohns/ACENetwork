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

#ifndef TEST_I_COMMON_MODULES_H
#define TEST_I_COMMON_MODULES_H

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_dump.h"

#include "stream_stat_statistic_report.h"

#include "net_connection_manager.h"

#include "http_common.h"
#include "http_module_parser.h"
#include "http_network.h"

#include "test_i_common.h"

#include "test_i_url_stream_load_stream_common.h"

// forward declarations
class Test_I_SessionMessage;
class Test_I_Message;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct Test_I_URLStreamLoad_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct HTTP_Stream_UserData> Test_I_ConnectionManager_t;

// declare module(s)
typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                               Test_I_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage> Test_I_HTTPStreamer;
typedef HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Test_I_ControlMessage_t,
                              Test_I_Message,
                              Test_I_SessionMessage,
                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                              enum Stream_ControlType,
                              enum Stream_SessionMessageType,
                              struct Test_I_URLStreamLoad_StreamState,
                              struct Test_I_URLStreamLoad_SessionData,
                              Test_I_URLStreamLoad_SessionData_t,
                              HTTP_Statistic_t,
                              Common_Timer_Manager_t,
                              struct HTTP_Stream_UserData> Test_I_HTTPParser;
//typedef HTTP_Module_ParserH_T<ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              Test_I_ControlMessage_t,
//                              Test_I_Message,
//                              Test_I_SessionMessage,
//                              struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
//                              enum Stream_ControlType,
//                              enum Stream_SessionMessageType,
//                              struct Test_I_URLStreamLoad_StreamState,
//                              struct Test_I_URLStreamLoad_SessionData,
//                              Test_I_URLStreamLoad_SessionData_t,
//                              HTTP_Statistic_t,
//                              HTTP_StatisticHandler_Proactor_t,
//                              struct HTTP_Stream_UserData> Test_I_AsynchHTTPParser;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                                   Test_I_ControlMessage_t,
                                                   Test_I_Message,
                                                   Test_I_SessionMessage,
                                                   HTTP_Method_t,
                                                   HTTP_Statistic_t,
                                                   Common_Timer_Manager_t,
                                                   struct Test_I_URLStreamLoad_SessionData,
                                                   Test_I_URLStreamLoad_SessionData_t> Test_I_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                                   Test_I_ControlMessage_t,
                                                   Test_I_Message,
                                                   Test_I_SessionMessage,
                                                   HTTP_Method_t,
                                                   HTTP_Statistic_t,
                                                   Common_Timer_Manager_t,
                                                   struct Test_I_URLStreamLoad_SessionData,
                                                   Test_I_URLStreamLoad_SessionData_t> Test_I_StatisticReport_WriterTask_t;
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                   Common_TimePolicy_t,
//                                                   struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
//                                                   Test_I_ControlMessage_t,
//                                                   Test_I_Message,
//                                                   Test_I_SessionMessage,
//                                                   HTTP_Method_t,
//                                                   HTTP_Statistic_t,
//                                                   HTTP_StatisticHandler_Proactor_t,
//                                                   struct Test_I_URLStreamLoad_SessionData,
//                                                   Test_I_URLStreamLoad_SessionData_t> Test_I_StatisticReport_AsynchReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                   Common_TimePolicy_t,
//                                                   struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
//                                                   Test_I_ControlMessage_t,
//                                                   Test_I_Message,
//                                                   Test_I_SessionMessage,
//                                                   HTTP_Method_t,
//                                                   HTTP_Statistic_t,
//                                                   HTTP_StatisticHandler_Proactor_t,
//                                                   struct Test_I_URLStreamLoad_SessionData,
//                                                   Test_I_URLStreamLoad_SessionData_t> Test_I_StatisticReport_AsynchWriterTask_t;

DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                          Test_I_IStreamNotify_t,                                 // stream notification interface type
                          Test_I_HTTPStreamer,                                    // reader type
                          Test_I_HTTPParser,                                      // writer type
                          Test_I_HTTPMarshal);                                    // name
//DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                         // session event type
//                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
//                          Test_I_IStreamNotify_t,                                 // stream notification interface type
//                          Test_I_HTTPStreamer,                                    // reader type
//                          Test_I_AsynchHTTPParser,                                // writer type
//                          Test_I_AsynchHTTPMarshal);                              // name

DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
                          Test_I_IStreamNotify_t,                                 // stream notification interface type
                          Test_I_StatisticReport_ReaderTask_t,                    // reader type
                          Test_I_StatisticReport_WriterTask_t,                    // writer type
                          Test_I_StatisticReport);                                // name
//DATASTREAM_MODULE_DUPLEX (struct Test_I_URLStreamLoad_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                         // session event type
//                          struct Test_I_URLStreamLoad_ModuleHandlerConfiguration, // module handler configuration type
//                          Test_I_IStreamNotify_t,                                 // stream notification interface type
//                          Test_I_StatisticReport_AsynchReaderTask_t,              // reader type
//                          Test_I_StatisticReport_AsynchWriterTask_t,              // writer type
//                          Test_I_AsynchStatisticReport);                          // name

#endif
