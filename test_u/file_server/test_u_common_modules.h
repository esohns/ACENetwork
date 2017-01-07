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

#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_streammodule_base.h"

#include "stream_file_source.h"

#include "stream_misc_statistic_report.h"

#include "stream_module_io.h"

#include "test_u_common.h"
#include "test_u_configuration.h"

#include "file_server_common.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;

// declare module(s)
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_U_ControlMessage_t,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     struct Test_U_ModuleHandlerConfiguration,
                                     int,
                                     enum Stream_SessionMessageType,
                                     struct Test_U_StreamState,
                                     struct Test_U_StreamSessionData,
                                     Test_U_StreamSessionData_t,
                                     Net_RuntimeStatistic_t,
                                     ACE_INET_Addr,
                                     Test_U_InetConnectionManager_t,
                                     struct Test_U_UserData> Test_U_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_U_ModuleHandlerConfiguration,
                                     Test_U_ControlMessage_t,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     struct Test_U_StreamSessionData,
                                     Test_U_StreamSessionData_t,
                                     ACE_INET_Addr,
                                     Test_U_InetConnectionManager_t,
                                     struct Test_U_UserData> Test_U_Module_Net_Reader_t;
DATASTREAM_MODULE_DUPLEX (struct Test_U_StreamSessionData,          // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                          Test_U_IStreamNotify_t,                   // stream notification interface type
                          Test_U_Module_Net_Reader_t,               // reader type
                          Test_U_Module_Net_Writer_t,               // writer type
                          Test_U_Net_IO);                           // name

typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Test_U_ModuleHandlerConfiguration,
                                                   Test_U_ControlMessage_t,
                                                   Test_U_Message,
                                                   Test_U_SessionMessage,
                                                   int,
                                                   Net_RuntimeStatistic_t,
                                                   struct Test_U_StreamSessionData,
                                                   Test_U_StreamSessionData_t> Test_U_Module_StatisticReport_ReaderTask_t;
typedef Stream_Module_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Test_U_ModuleHandlerConfiguration,
                                                   Test_U_ControlMessage_t,
                                                   Test_U_Message,
                                                   Test_U_SessionMessage,
                                                   int,
                                                   Net_RuntimeStatistic_t,
                                                   struct Test_U_StreamSessionData,
                                                   Test_U_StreamSessionData_t> Test_U_Module_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_U_StreamSessionData,            // session data type
                          enum Stream_SessionMessageType,             // session event type
                          struct Test_U_ModuleHandlerConfiguration,   // module handler configuration type
                          Test_U_IStreamNotify_t,                     // stream notification interface type
                          Test_U_Module_StatisticReport_ReaderTask_t, // reader type
                          Test_U_Module_StatisticReport_WriterTask_t, // writer type
                          Test_U_StatisticReport);                    // name

typedef Stream_Module_FileReader_Writer_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          struct Test_U_ModuleHandlerConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage,
                                          struct Test_U_StreamSessionData> Test_U_FileReader;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_U_StreamSessionData,          // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              Test_U_IStreamNotify_t,                   // stream notification interface type
                              Test_U_FileReader);                       // writer type

#endif
