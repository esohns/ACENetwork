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
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_streammodule_base.h"

#include "stream_stat_statistic_report.h"

//#include "net_module_sockethandler.h"
#include "net_remote_comm.h"

#include "test_u_common.h"
#include "test_u_configuration.h"

#include "net_client_common.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;

// declare module(s)
//typedef Net_Module_SocketHandler_T<ACE_MT_SYNCH,
//                                      Stream_ControlMessage_t,
//                                      Test_U_Message,
//                                      Test_U_SessionMessage,
//                                      struct ClientServer_ModuleHandlerConfiguration,
//                                      enum Stream_ControlType,
//                                      enum Stream_SessionMessageType,
//                                      struct Test_U_StreamState,
//                                      struct Test_U_StreamSessionData,
//                                      Test_U_StreamSessionData_t,
//                                      Net_Statistic_t,
//                                      Common_Timer_Manager_t,
//                                      Net_Remote_Comm::MessageHeader,
//                                      struct Test_U_UserData> Test_U_Module_SocketHandler;
//DATASTREAM_MODULE_INPUT_ONLY (struct Test_U_StreamSessionData,          // session data type
//                              enum Stream_SessionMessageType,                 // session event type
//                              struct ClientServer_ModuleHandlerConfiguration, // module handler configuration type
//                              libacenetwork_default_tcp_sockethandler_module_name_string,
//                              Stream_INotify_t,                               // stream notification interface type
//                              Test_U_Module_SocketHandler);             // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct ClientServer_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Net_MessageType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Test_U_StreamSessionData,
                                                      Test_U_StreamSessionData_t> Test_U_Module_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct ClientServer_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Net_MessageType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Test_U_StreamSessionData,
                                                      Test_U_StreamSessionData_t> Test_U_Module_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_U_StreamSessionData,            // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct ClientServer_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_U_Module_StatisticReport_ReaderTask_t, // reader type
                          Test_U_Module_StatisticReport_WriterTask_t, // writer type
                          Test_U_Module_StatisticReport);             // name

#endif
