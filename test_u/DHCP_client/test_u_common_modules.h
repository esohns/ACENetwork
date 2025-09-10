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

#include "stream_misc_dump.h"

#include "stream_stat_statistic_report.h"

#include "stream_net_io.h"

#include "net_connection_manager.h"

#include "dhcp_common.h"
#include "dhcp_configuration.h"
//#include "dhcp_module_bisector.h"
#include "dhcp_module_discover.h"
#include "dhcp_module_parser.h"
#include "dhcp_module_streamer.h"
//#include "dhcp_stream_common.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"

#include "test_u_dhcp_client_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct DHCPClient_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 DHCPClient_ConnectionConfiguration,
                                 struct DHCP_ConnectionState,
                                 DHCP_Statistic_t,
                                 struct Net_UserData> DHCPClient_ConnectionManager_t;

// declare module(s)
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     struct DHCPClient_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct DHCPClient_StreamState,
                                     struct Stream_Statistic,
                                     Test_U_SessionManager_t,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     DHCPClient_ConnectionManager_t,
                                     struct Stream_UserData> DHCPClient_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     struct DHCPClient_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct DHCPClient_StreamState,
                                     struct Stream_Statistic,
                                     Test_U_SessionManager_t,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     DHCPClient_ConnectionManager_t,
                                     struct Stream_UserData> DHCPClient_Module_Net_Reader_t;

typedef DHCP_Module_Discover_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage,
                               struct DHCPClient_ModuleHandlerConfiguration,
                               DHCPClient_ConnectionManager_t,
                               DHCPClient_OutboundConnectorBcast_t,
                               DHCPClient_OutboundConnector_t> DHCPClient_Module_DHCPDiscover;
typedef DHCP_Module_DiscoverH_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_ControlMessage_t,
                                Test_U_Message,
                                Test_U_SessionMessage,
                                struct DHCPClient_ModuleHandlerConfiguration,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct DHCPClient_StreamState,
                                struct Stream_Statistic,
                                Test_U_SessionManager_t,
                                Common_Timer_Manager_t> DHCPClient_Module_DHCPDiscoverH;

typedef DHCP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct DHCPClient_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage> DHCPClient_Module_Streamer;

typedef DHCP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct DHCPClient_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             Test_U_Message,
                             Test_U_SessionMessage> DHCPClient_Module_Parser;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct DHCPClient_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      DHCP_MessageType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct DHCPClient_SessionData,
                                                      DHCPClient_SessionData_t> DHCPClient_Module_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct DHCPClient_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      DHCP_MessageType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct DHCPClient_SessionData,
                                                      DHCPClient_SessionData_t> DHCPClient_Module_StatisticReport_WriterTask_t;

typedef Stream_Module_Dump_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct DHCPClient_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             Test_U_Message,
                             Test_U_SessionMessage,
                             struct Stream_UserData> DHCPClient_Module_Dump;

// declare module(s)
DATASTREAM_MODULE_DUPLEX (struct DHCPClient_SessionData,                  // session data type
                          enum Stream_SessionMessageType,                 // session event type
                          struct DHCPClient_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                               // stream notification interface type
                          DHCPClient_Module_Net_Reader_t,                 // reader type
                          DHCPClient_Module_Net_Writer_t,                 // writer type
                          DHCPClient_Module_Net_IO);                      // name

DATASTREAM_MODULE_INPUT_ONLY (struct DHCPClient_SessionData,                // session data type
                              enum Stream_SessionMessageType,               // session event type
                              struct DHCPClient_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_protocol_default_dhcp_discover_module_name_string,
                              Stream_INotify_t,                             // stream notification interface type
                              DHCPClient_Module_DHCPDiscover);              // writer type
DATASTREAM_MODULE_INPUT_ONLY (struct DHCPClient_SessionData,                // session data type
                              enum Stream_SessionMessageType,               // session event type
                              struct DHCPClient_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_protocol_default_dhcp_discover_module_name_string,
                              Stream_INotify_t,                             // stream notification interface type
                              DHCPClient_Module_DHCPDiscoverH);             // writer type
DATASTREAM_MODULE_OUTPUT_ONLY (struct DHCPClient_SessionData,                // session data type
                               enum Stream_SessionMessageType,               // session event type
                               struct DHCPClient_ModuleHandlerConfiguration, // module handler configuration type
                               libacenetwork_protocol_default_dhcp_streamer_module_name_string,
                               Stream_INotify_t,                             // stream notification interface type
                               DHCPClient_Module_Streamer);                  // reader type
DATASTREAM_MODULE_DUPLEX (struct DHCPClient_SessionData,                // session data type
                          enum Stream_SessionMessageType,               // session event type
                          struct DHCPClient_ModuleHandlerConfiguration, // module handler configuration type
                          libacenetwork_protocol_default_dhcp_parser_module_name_string,
                          Stream_INotify_t,                             // stream notification interface type
                          DHCPClient_Module_Streamer,                   // reader type
                          DHCPClient_Module_Parser,                     // writer type
                          DHCPClient_Module_Marshal);                   // name

DATASTREAM_MODULE_DUPLEX (struct DHCPClient_SessionData,                  // session data type
                          enum Stream_SessionMessageType,                 // session event type
                          struct DHCPClient_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                               // stream notification interface type
                          DHCPClient_Module_StatisticReport_ReaderTask_t, // reader type
                          DHCPClient_Module_StatisticReport_WriterTask_t, // writer type
                          DHCPClient_Module_StatisticReport);             // name

DATASTREAM_MODULE_INPUT_ONLY (struct DHCPClient_SessionData,                // session data type
                              enum Stream_SessionMessageType,               // session event type
                              struct DHCPClient_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_dump_module_name_string,
                              Stream_INotify_t,                             // stream notification interface type
                              DHCPClient_Module_Dump);                      // writer type

#endif
