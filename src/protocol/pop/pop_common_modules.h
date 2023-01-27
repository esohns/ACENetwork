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

#ifndef POP_COMMON_MODULES_H
#define POP_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_control_message.h"
#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_stat_statistic_report.h"

#include "pop_codes.h"
#include "pop_common.h"
#include "pop_configuration.h"
#include "pop_module_parser.h"
#include "pop_module_send.h"
#include "pop_module_streamer.h"
#include "pop_stream_common.h"

// forward declarations
template <typename MessageType = enum Stream_MessageType>
class POP_Message_T;
typedef POP_Message_T<enum Stream_MessageType> POP_Message_t;
template <//typename AllocatorType,
          ////////////////////////////////
          typename SessionDataType, // reference-counted
          ////////////////////////////////
          typename UserDataType>
class POP_SessionMessage_T;
typedef POP_SessionMessage_T<POP_Stream_SessionData_t,
                              struct Stream_UserData> POP_SessionMessage_t;

typedef POP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct POP_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               POP_Message_t,
                               POP_SessionMessage_t> POP_Module_Streamer_t;
typedef POP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             struct POP_ModuleHandlerConfiguration,
                             Stream_ControlMessage_t,
                             POP_Message_t,
                             POP_SessionMessage_t> POP_Module_Parser_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T <ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct POP_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       POP_Message_t,
                                                       POP_SessionMessage_t,
                                                       Stream_CommandType_t,
                                                       POP_Statistic_t,
                                                       Common_Timer_Manager_t,
                                                       struct Stream_POP_SessionData,
                                                       POP_Stream_SessionData_t> POP_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T <ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct POP_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       POP_Message_t,
                                                       POP_SessionMessage_t,
                                                       Stream_CommandType_t,
                                                       POP_Statistic_t,
                                                       Common_Timer_Manager_t,
                                                       struct Stream_POP_SessionData,
                                                       POP_Stream_SessionData_t> POP_Module_Statistic_WriterTask_t;

typedef POP_Module_Send_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           Stream_ControlMessage_t,
                           POP_Message_t,
                           POP_SessionMessage_t,
                           struct POP_ModuleHandlerConfiguration,
                           struct POP_ConnectionState,
                           typename ConnectionType> POP_Module_Send;

// declare module(s)
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                           // task synch type
                          Common_TimePolicy_t,                    // time policy
                          struct Stream_ModuleConfiguration,      // module configuration type
                          struct POP_ModuleHandlerConfiguration, // module handler configuration type
                          POP_Module_Streamer_t,                 // reader type
                          POP_Module_Parser_t,                   // writer type
                          POP_Module_Marshal);                   // name

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                           // task synch type
                          Common_TimePolicy_t,                    // time policy type
                          struct Stream_ModuleConfiguration,      // module configuration type
                          struct POP_ModuleHandlerConfiguration, // module handler configuration type
                          POP_Module_Statistic_ReaderTask_t,     // reader type
                          POP_Module_Statistic_WriterTask_t,     // writer type
                          POP_Module_Statistic);                 // name

DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                           // task synch type
                              Common_TimePolicy_t,                    // time policy
                              Stream_ModuleConfiguration,             // module configuration type
                              struct POP_ModuleHandlerConfiguration, // module handler configuration type
                              POP_Module_Send);                      // writer type

#endif
