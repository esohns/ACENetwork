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

#include "ace/Message_Block.h"
#include "ace/Stream_Modules.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_streammodule_base.h"

#include "stream_dec_mpeg_ts_decoder.h"

#include "stream_file_source.h"

#include "stream_net_io.h"
#include "stream_net_target.h"

#include "stream_stat_statistic_report.h"

#include "test_u_common.h"
#include "test_u_configuration.h"
#include "test_u_connection_manager_common.h"

#include "file_server_connection_common.h"
#include "file_server_common.h"
#include "file_server_stream_common.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;

// declare module(s)
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_U_ControlMessage_t,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     struct Test_U_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct FileServer_StreamState,
                                     struct FileServer_SessionData,
                                     FileServer_SessionData_t,
                                     Net_Statistic_t,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     FileServer_InetConnectionManager_t,
                                     struct FileServer_UserData> Test_U_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Test_U_ControlMessage_t,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     struct Test_U_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct FileServer_StreamState,
                                     struct FileServer_SessionData,
                                     FileServer_SessionData_t,
                                     Net_Statistic_t,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     FileServer_InetConnectionManager_t,
                                     struct FileServer_UserData> Test_U_Module_Net_Reader_t;
DATASTREAM_MODULE_DUPLEX (struct FileServer_SessionData,            // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                   // stream notification interface type
                          Test_U_Module_Net_Reader_t,               // reader type
                          Test_U_Module_Net_Writer_t,               // writer type
                          Test_U_Net_IO);                           // name

typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_ModuleHandlerConfiguration,
                                   Test_U_ControlMessage_t,
                                   Test_U_Message,
                                   Test_U_SessionMessage,
                                   FileServer_SessionData_t,
                                   FileServer_ConnectionConfigurationIterator_t,
                                   FileServer_InetConnectionManager_t,
                                   Test_U_UDPConnector_t> Test_U_Module_Net_UDPTarget;
typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_ModuleHandlerConfiguration,
                                   Test_U_ControlMessage_t,
                                   Test_U_Message,
                                   Test_U_SessionMessage,
                                   FileServer_SessionData_t,
                                   FileServer_ConnectionConfigurationIterator_t,
                                   FileServer_InetConnectionManager_t,
                                   Test_U_UDPAsynchConnector_t> Test_U_Module_Net_AsynchUDPTarget;
DATASTREAM_MODULE_INPUT_ONLY (struct FileServer_SessionData,            // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_net_target_module_name_string,
                              Stream_INotify_t,                   // stream notification interface type
                              Test_U_Module_Net_UDPTarget);             // writer type
DATASTREAM_MODULE_INPUT_ONLY (struct FileServer_SessionData,            // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_net_target_module_name_string,
                              Stream_INotify_t,                   // stream notification interface type
                              Test_U_Module_Net_AsynchUDPTarget);       // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      int,
                                                      Net_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      struct FileServer_SessionData,
                                                      FileServer_SessionData_t> Test_U_Module_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      int,
                                                      Net_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      struct FileServer_SessionData,
                                                      FileServer_SessionData_t> Test_U_Module_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct FileServer_SessionData,              // session data type
                          enum Stream_SessionMessageType,             // session event type
                          struct Test_U_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                     // stream notification interface type
                          Test_U_Module_StatisticReport_ReaderTask_t, // reader type
                          Test_U_Module_StatisticReport_WriterTask_t, // writer type
                          Test_U_StatisticReport);                    // name

typedef Stream_Decoder_MPEG_TS_Decoder_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_U_ModuleHandlerConfiguration,
                                         Test_U_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         FileServer_SessionData_t> Test_U_MPEG_TS_Decoder;
DATASTREAM_MODULE_OUTPUT_ONLY (struct FileServer_SessionData,            // session data type
                               enum Stream_SessionMessageType,           // session event type
                               struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                               libacestream_default_dec_mpeg_ts_module_name_string,
                               Stream_INotify_t,                   // stream notification interface type
                               Test_U_MPEG_TS_Decoder);                  // reader type

typedef Stream_Module_FileReader_Writer_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          struct Test_U_ModuleHandlerConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage,
                                          struct FileServer_SessionData,
                                          struct FileServer_UserData> Test_U_FileReader;
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Test_U_ControlMessage_t,
                                    Test_U_Message,
                                    Test_U_SessionMessage,
                                    struct Test_U_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct FileServer_StreamState,
                                    struct FileServer_SessionData,
                                    FileServer_SessionData_t,
                                    Net_Statistic_t,
                                    Common_Timer_Manager_t,
                                    struct FileServer_UserData> Test_U_FileReaderH;
DATASTREAM_MODULE_INPUT_ONLY (struct FileServer_SessionData,            // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                   // stream notification interface type
                              Test_U_FileReader);                       // writer type
DATASTREAM_MODULE_INPUT_ONLY (struct FileServer_SessionData,            // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                   // stream notification interface type
                              Test_U_FileReaderH);                      // writer type

#endif
