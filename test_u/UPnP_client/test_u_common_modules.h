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
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#include "stream_module_xmlparser.h"
#include "stream_module_xpathquery.h"

//#include "stream_misc_dump.h"
#include "stream_misc_messagehandler.h"

//#include "stream_stat_statistic_report.h"

//#include "stream_net_io.h"

#include "net_connection_manager.h"

#include "http_module_parser.h"
#include "http_module_streamer.h"

#include "ssdp_module_streamer.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"

#include "test_u_upnp_client_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct UPnP_Client_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 UPnP_Client_SSDP_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> UPnP_Client_SSDP_ConnectionManager_t;

// declare module(s)
//typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
//                                     Stream_ControlMessage_t,
//                                     Test_U_Message,
//                                     Test_U_SessionMessage,
//                                     struct UPnP_Client_ModuleHandlerConfiguration,
//                                     enum Stream_ControlType,
//                                     enum Stream_SessionMessageType,
//                                     struct UPnP_Client_StreamState,
//                                     struct Stream_Statistic,
//                                     Test_U_SessionManager_t,
//                                     Common_Timer_Manager_t,
//                                     ACE_INET_Addr,
//                                     UPnP_Client_SSDP_ConnectionManager_t,
//                                     struct Stream_UserData> UPnP_Client_Module_SSDP_Net_Writer_t;
//typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
//                                     Stream_ControlMessage_t,
//                                     Test_U_Message,
//                                     Test_U_SessionMessage,
//                                     struct UPnP_Client_ModuleHandlerConfiguration,
//                                     enum Stream_ControlType,
//                                     enum Stream_SessionMessageType,
//                                     struct UPnP_Client_StreamState,
//                                     struct Stream_Statistic,
//                                     Test_U_SessionManager_t,
//                                     Common_Timer_Manager_t,
//                                     ACE_INET_Addr,
//                                     UPnP_Client_SSDP_ConnectionManager_t,
//                                     struct Stream_UserData> UPnP_Client_Module_SSDP_Net_Reader_t;
//
//typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
//                                     Stream_ControlMessage_t,
//                                     Test_U_Message,
//                                     Test_U_SessionMessage,
//                                     struct UPnP_Client_ModuleHandlerConfiguration,
//                                     enum Stream_ControlType,
//                                     enum Stream_SessionMessageType,
//                                     struct UPnP_Client_StreamState,
//                                     struct Stream_Statistic,
//                                     Test_U_SessionManager_t,
//                                     Common_Timer_Manager_t,
//                                     ACE_INET_Addr,
//                                     UPnP_Client_HTTP_ConnectionManager_t,
//                                     struct Stream_UserData> UPnP_Client_Module_HTTP_Net_Writer_t;
//typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
//                                     Stream_ControlMessage_t,
//                                     Test_U_Message,
//                                     Test_U_SessionMessage,
//                                     struct UPnP_Client_ModuleHandlerConfiguration,
//                                     enum Stream_ControlType,
//                                     enum Stream_SessionMessageType,
//                                     struct UPnP_Client_StreamState,
//                                     struct Stream_Statistic,
//                                     Test_U_SessionManager_t,
//                                     Common_Timer_Manager_t,
//                                     ACE_INET_Addr,
//                                     UPnP_Client_HTTP_ConnectionManager_t,
//                                     struct Stream_UserData> UPnP_Client_Module_HTTP_Net_Reader_t;

typedef SSDP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct UPnP_Client_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage> UPnP_Client_Module_SSDP_Streamer;

typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct UPnP_Client_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             Test_U_Message,
                             Test_U_SessionMessage> UPnP_Client_Module_Parser;
typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct UPnP_Client_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage> UPnP_Client_Module_HTTP_Streamer;

//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct UPnP_Client_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_Message,
//                                                      Test_U_SessionMessage,
//                                                      HTTP_Codes::MethodType,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      struct UPnP_Client_SessionData,
//                                                      UPnP_Client_SessionData_t> UPnP_Client_Module_StatisticReport_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct UPnP_Client_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_Message,
//                                                      Test_U_SessionMessage,
//                                                      HTTP_Codes::MethodType,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      struct UPnP_Client_SessionData,
//                                                      UPnP_Client_SessionData_t> UPnP_Client_Module_StatisticReport_WriterTask_t;

typedef Stream_Module_XMLParser_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct UPnP_Client_ModuleHandlerConfiguration,
                                  Stream_ControlMessage_t,
                                  Test_U_Message,
                                  Test_U_SessionMessage,
                                  UPnP_Client_SessionData_t,
                                  struct UPnP_Client_SessionData,
                                  struct Stream_Module_XMLParser_SAXParserContextBase> UPnP_Client_XMLParser;

typedef Stream_Module_XPathQuery_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct UPnP_Client_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_Message,
                                   Test_U_SessionMessage,
                                   UPnP_Client_SessionData_t,
                                   struct UPnP_Client_SessionData> UPnP_Client_XPathQuery;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct UPnP_Client_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       struct UPnP_Client_SessionData,
                                       struct Stream_UserData> UPnP_Client_MessageHandler;

//typedef Stream_Module_Dump_T<ACE_MT_SYNCH,
//                             Common_TimePolicy_t,
//                             struct UPnP_Client_ModuleHandlerConfiguration,
//                             Stream_ControlMessage_t,
//                             Test_U_Message,
//                             Test_U_SessionMessage,
//                             struct Stream_UserData> UPnP_Client_Module_Dump;

// declare module(s)
//DATASTREAM_MODULE_DUPLEX (struct UPnP_Client_SessionData,                  // session data type
//                          enum Stream_SessionMessageType,                 // session event type
//                          struct UPnP_Client_ModuleHandlerConfiguration,   // module handler configuration type
//                          libacestream_default_net_io_module_name_string,
//                          Stream_INotify_t,                               // stream notification interface type
//                          UPnP_Client_Module_SSDP_Net_Reader_t,           // reader type
//                          UPnP_Client_Module_SSDP_Net_Writer_t,           // writer type
//                          UPnP_Client_Module_SSDP_Net_IO);                // name
//DATASTREAM_MODULE_DUPLEX (struct UPnP_Client_SessionData,                  // session data type
//                          enum Stream_SessionMessageType,                 // session event type
//                          struct UPnP_Client_ModuleHandlerConfiguration,   // module handler configuration type
//                          libacestream_default_net_io_module_name_string,
//                          Stream_INotify_t,                               // stream notification interface type
//                          UPnP_Client_Module_HTTP_Net_Reader_t,           // reader type
//                          UPnP_Client_Module_HTTP_Net_Writer_t,           // writer type
//                          UPnP_Client_Module_HTTP_Net_IO);                // name

DATASTREAM_MODULE_OUTPUT_ONLY (struct UPnP_Client_SessionData,                // session data type
                               enum Stream_SessionMessageType,                // session event type
                               struct UPnP_Client_ModuleHandlerConfiguration, // module handler configuration type
                               libacenetwork_protocol_default_ssdp_streamer_module_name_string,
                               Stream_INotify_t,                              // stream notification interface type
                               UPnP_Client_Module_SSDP_Streamer);             // reader type
DATASTREAM_MODULE_DUPLEX (struct UPnP_Client_SessionData,                // session data type
                          enum Stream_SessionMessageType,                // session event type
                          struct UPnP_Client_ModuleHandlerConfiguration, // module handler configuration type
                          libacenetwork_protocol_default_http_parser_module_name_string,
                          Stream_INotify_t,                              // stream notification interface type
                          UPnP_Client_Module_SSDP_Streamer,              // reader type
                          UPnP_Client_Module_Parser,                     // writer type
                          UPnP_Client_Module_SSDP_Marshal);              // name
DATASTREAM_MODULE_DUPLEX (struct UPnP_Client_SessionData,                // session data type
                          enum Stream_SessionMessageType,                // session event type
                          struct UPnP_Client_ModuleHandlerConfiguration, // module handler configuration type
                          libacenetwork_protocol_default_http_parser_module_name_string,
                          Stream_INotify_t,                              // stream notification interface type
                          UPnP_Client_Module_HTTP_Streamer,              // reader type
                          UPnP_Client_Module_Parser,                     // writer type
                          UPnP_Client_Module_HTTP_Marshal);              // name

//DATASTREAM_MODULE_DUPLEX (struct UPnP_Client_SessionData,                  // session data type
//                          enum Stream_SessionMessageType,                 // session event type
//                          struct UPnP_Client_ModuleHandlerConfiguration,   // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                               // stream notification interface type
//                          UPnP_Client_Module_StatisticReport_ReaderTask_t, // reader type
//                          UPnP_Client_Module_StatisticReport_WriterTask_t, // writer type
//                          UPnP_Client_Module_StatisticReport);             // name

DATASTREAM_MODULE_INPUT_ONLY (struct UPnP_Client_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct UPnP_Client_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_xml_parser_module_name_string,
                              Stream_INotify_t,                                // stream notification interface type
                              UPnP_Client_XMLParser);                          // writer type

DATASTREAM_MODULE_INPUT_ONLY (struct UPnP_Client_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct UPnP_Client_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_xpath_query_module_name_string,
                              Stream_INotify_t,                                // stream notification interface type
                              UPnP_Client_XPathQuery);                         // writer type

DATASTREAM_MODULE_INPUT_ONLY (struct UPnP_Client_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct UPnP_Client_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                // stream notification interface type
                              UPnP_Client_MessageHandler);                     // writer type

//DATASTREAM_MODULE_INPUT_ONLY (struct UPnP_Client_SessionData,                // session data type
//                              enum Stream_SessionMessageType,               // session event type
//                              struct UPnP_Client_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_dump_module_name_string,
//                              Stream_INotify_t,                             // stream notification interface type
//                              UPnP_Client_Module_Dump);                      // writer type

#endif
