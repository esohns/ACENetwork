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

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_dump.h"

#include "stream_stat_statistic_report.h"

#include "stream_net_output.h"

#include "net_connection_manager.h"

#include "pcp_common.h"
#include "pcp_configuration.h"
//#include "pcp_module_bisector.h"
#include "pcp_module_parser.h"
#include "pcp_module_streamer.h"
//#include "pcp_stream_common.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"

#include "test_u_pcp_client_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct PCPClient_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 PCPClient_ConnectionConfiguration,
                                 struct PCP_ConnectionState,
                                 PCP_Statistic_t,
                                 struct Net_UserData> PCPClient_ConnectionManager_t;

// declare module(s)
typedef Stream_Module_Net_OutputWriter_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct PCPClient_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         PCPClient_ConnectionManager_t,
                                         struct Stream_UserData> PCPClient_Module_Net_Writer_t;
typedef Stream_Module_Net_OutputReader_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct PCPClient_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         PCPClient_ConnectionManager_t,
                                         struct Stream_UserData> PCPClient_Module_Net_Reader_t;

typedef PCP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct PCPClient_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage> PCPClient_Module_Streamer;

typedef PCP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct PCPClient_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             Test_U_Message,
                             Test_U_SessionMessage> PCPClient_Module_Parser;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct PCPClient_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      PCP_Opcode_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> PCPClient_Module_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct PCPClient_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      PCP_Opcode_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> PCPClient_Module_StatisticReport_WriterTask_t;

typedef Stream_Module_Dump_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct PCPClient_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             Test_U_Message,
                             Test_U_SessionMessage,
                             struct Stream_UserData> PCPClient_Module_Dump;

// declare module(s)
DATASTREAM_MODULE_DUPLEX (struct PCPClient_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                     // session event type
                          struct PCPClient_ModuleHandlerConfiguration,        // module handler configuration type
                          libacestream_default_net_output_module_name_string,
                          Stream_INotify_t,                                   // stream notification interface type
                          PCPClient_Module_Net_Reader_t,                      // reader type
                          PCPClient_Module_Net_Writer_t,                      // writer type
                          PCPClient_Module_Net_Output);                       // name

DATASTREAM_MODULE_OUTPUT_ONLY (struct PCPClient_SessionData,                                   // session data type
                               enum Stream_SessionMessageType,                                 // session event type
                               struct PCPClient_ModuleHandlerConfiguration,                    // module handler configuration type
                               libacenetwork_protocol_default_pcp_streamer_module_name_string,
                               Stream_INotify_t,                                               // stream notification interface type
                               PCPClient_Module_Streamer);                                     // reader type
DATASTREAM_MODULE_DUPLEX (struct PCPClient_SessionData,                                 // session data type
                          enum Stream_SessionMessageType,                               // session event type
                          struct PCPClient_ModuleHandlerConfiguration,                  // module handler configuration type
                          libacenetwork_protocol_default_pcp_parser_module_name_string,
                          Stream_INotify_t,                                             // stream notification interface type
                          PCPClient_Module_Streamer,                                    // reader type
                          PCPClient_Module_Parser,                                      // writer type
                          PCPClient_Module_Marshal);                                    // name

DATASTREAM_MODULE_DUPLEX (struct PCPClient_SessionData,                        // session data type
                          enum Stream_SessionMessageType,                      // session event type
                          struct PCPClient_ModuleHandlerConfiguration,         // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                    // stream notification interface type
                          PCPClient_Module_StatisticReport_ReaderTask_t,       // reader type
                          PCPClient_Module_StatisticReport_WriterTask_t,       // writer type
                          PCPClient_Module_StatisticReport);                   // name

DATASTREAM_MODULE_INPUT_ONLY (struct PCPClient_SessionData,                      // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct PCPClient_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_misc_dump_module_name_string,
                              Stream_INotify_t,                                  // stream notification interface type
                              PCPClient_Module_Dump);                            // writer type

#endif
