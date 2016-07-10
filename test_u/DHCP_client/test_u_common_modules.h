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

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_dump.h"
#include "stream_misc_runtimestatistic.h"

#include "stream_module_io.h"

//#include "net_connection_manager.h"

#include "dhcp_common.h"
#include "dhcp_configuration.h"
//#include "dhcp_module_bisector.h"
#include "dhcp_module_discover.h"
#include "dhcp_module_parser.h"
#include "dhcp_module_streamer.h"
//#include "dhcp_stream_common.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;
//typedef Net_Connection_Manager_T<ACE_INET_Addr,
//                                 Test_U_Configuration,
//                                 Test_U_ConnectionState,
//                                 Test_U_RuntimeStatistic_t,
//                                 /////////
//                                 Test_U_UserData> Test_U_ConnectionManager_t;

// declare module(s)
typedef Stream_Module_Net_IOWriter_T<ACE_SYNCH_MUTEX,
                                     /////
                                     ACE_Message_Block,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     /////
                                     Test_U_StreamModuleHandlerConfiguration,
                                     /////
                                     int,
                                     int,
                                     Test_U_StreamState,
                                     /////
                                     Test_U_StreamSessionData,
                                     Test_U_StreamSessionData_t,
                                     /////
                                     Test_U_RuntimeStatistic_t,
                                     /////
                                     ACE_INET_Addr,
                                     Test_U_ConnectionManager_t> Test_U_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     /////
                                     Test_U_StreamModuleHandlerConfiguration,
                                     /////
                                     ACE_Message_Block,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     /////
                                     Test_U_StreamSessionData,
                                     Test_U_StreamSessionData_t,
                                     /////
                                     ACE_INET_Addr,
                                     Test_U_ConnectionManager_t> Test_U_Module_Net_Reader_t;

typedef DHCP_Module_Discover_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               ///////////
                               ACE_Message_Block,
                               Test_U_Message,
                               Test_U_SessionMessage,
                               ///////////
                               Test_U_StreamModuleHandlerConfiguration,
                               ///////////
                               Test_U_ConnectionManager_t,
                               Test_U_OutboundConnectorBcast_t,
                               Test_U_OutboundConnector_t> Test_U_Module_DHCPDiscover;
typedef DHCP_Module_DiscoverH_T<ACE_SYNCH_MUTEX,
                                //////////
                                ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                //////////
                                Test_U_StreamModuleHandlerConfiguration,
                                //////////
                                ACE_Message_Block,
                                Test_U_Message,
                                Test_U_SessionMessage,
                                //////////
                                int,
                                int,
                                Test_U_StreamState,
                                //////////
                                Test_U_StreamSessionData,
                                Test_U_StreamSessionData_t,
                                //////////
                                Test_U_RuntimeStatistic_t> Test_U_Module_DHCPDiscoverH;

//typedef DHCP_Module_Bisector_T<ACE_SYNCH_MUTEX,
//                               ACE_MT_SYNCH,
//                               Common_TimePolicy_t,
//                               Test_U_SessionMessage,
//                               Test_U_Message,
//                               Test_U_StreamModuleHandlerConfiguration,
//                               Test_U_StreamState,
//                               Test_U_StreamSessionData,
//                               Test_U_StreamSessionData_t,
//                               Test_U_RuntimeStatistic_t> Test_U_Module_Bisector;

typedef DHCP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               Test_U_StreamModuleHandlerConfiguration,
                               ACE_Message_Block,
                               Test_U_Message,
                               Test_U_SessionMessage> Test_U_Module_Streamer;

typedef DHCP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             Test_U_StreamModuleHandlerConfiguration,
                             ACE_Message_Block,
                             Test_U_Message,
                             Test_U_SessionMessage> Test_U_Module_Parser;
//typedef DHCP_Module_ParserH_T<ACE_SYNCH_MUTEX,
//                              ////////////
//                              ACE_MT_SYNCH,
//                              ////////////
//                              Common_TimePolicy_t,
//                              Test_U_SessionMessage,
//                              Test_U_Message,
//                              Test_U_StreamModuleHandlerConfiguration,
//                              Test_U_StreamState,
//                              Test_U_StreamSessionData,
//                              Test_U_StreamSessionData_t,
//                              Test_U_RuntimeStatistic_t> Test_U_Module_Parser;

typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,

                                             Test_U_StreamModuleHandlerConfiguration,

                                             ACE_Message_Block,
                                             Test_U_Message,
                                             Test_U_SessionMessage,

                                             DHCP_MessageType_t,
                                             Test_U_RuntimeStatistic_t,
                                             Test_U_StreamSessionData,
                                             Test_U_StreamSessionData_t> Test_U_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,

                                             Test_U_StreamModuleHandlerConfiguration,

                                             ACE_Message_Block,
                                             Test_U_Message,
                                             Test_U_SessionMessage,

                                             DHCP_MessageType_t,
                                             Test_U_RuntimeStatistic_t,
                                             Test_U_StreamSessionData,
                                             Test_U_StreamSessionData_t> Test_U_Module_Statistic_WriterTask_t;

typedef Stream_Module_Dump_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,

                             Test_U_StreamModuleHandlerConfiguration,

                             ACE_Message_Block,
                             Test_U_Message,
                             Test_U_SessionMessage,

                             Test_U_StreamSessionData_t> Test_U_Module_Dump;

// declare module(s)
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                             // task synch type
                          Common_TimePolicy_t,                      // time policy
                          Stream_ModuleConfiguration,               // module configuration type
                          Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                          Test_U_Module_Net_Reader_t,              // reader type
                          Test_U_Module_Net_Writer_t,              // writer type
                          Test_U_Module_Net_IO);                   // name

DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                            // task synch type
                              Common_TimePolicy_t,                     // time policy
                              Stream_ModuleConfiguration,              // module configuration type
                              Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                              Test_U_Module_DHCPDiscover);             // writer type
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                            // task synch type
                              Common_TimePolicy_t,                     // time policy
                              Stream_ModuleConfiguration,              // module configuration type
                              Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                              Test_U_Module_DHCPDiscoverH);            // writer type

typedef ACE_Thru_Task<ACE_MT_SYNCH,
                      Common_TimePolicy_t> Test_U_ThruTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                            // task synch type
                          Common_TimePolicy_t,                     // time policy
                          Stream_ModuleConfiguration,              // module configuration type
                          Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                          Test_U_Module_Streamer,                  // reader type
                          Test_U_ThruTask_t,                       // writer type
                          Test_U_Module_Streamer);                 // name
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                            // task synch type
                          Common_TimePolicy_t,                     // time policy
                          Stream_ModuleConfiguration,              // module configuration type
                          Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                          Test_U_Module_Streamer,                  // reader type
                          Test_U_Module_Parser,                    // writer type
                          Test_U_Module_Marshal);                  // name
//DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                            // task synch type
//                              Common_TimePolicy_t,                     // time policy
//                              Stream_ModuleConfiguration,              // module configuration type
//                              Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
//                              Test_U_Module_Parser);                   // writer type

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                            // task synch type
                          Common_TimePolicy_t,                     // time policy type
                          Stream_ModuleConfiguration,              // module configuration type
                          Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                          Test_U_Module_Statistic_ReaderTask_t,    // reader type
                          Test_U_Module_Statistic_WriterTask_t,    // writer type
                          Test_U_Module_RuntimeStatistic);         // name

DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                            // task synch type
                              Common_TimePolicy_t,                     // time policy
                              Stream_ModuleConfiguration,              // module configuration type
                              Test_U_StreamModuleHandlerConfiguration, // module handler configuration type
                              Test_U_Module_Dump);                     // writer type

#endif
