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

#ifndef FTP_COMMON_MODULES_H
#define FTP_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_control_message.h"
#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_stat_statistic_report.h"

#include "ftp_codes.h"
#include "ftp_common.h"
#include "ftp_configuration.h"
#include "ftp_module_parser.h"
#include "ftp_module_send.h"
#include "ftp_module_streamer.h"
#include "ftp_stream_common.h"

// forward declarations
template <typename MessageType = enum Stream_MessageType>
class FTP_Message_T;
typedef FTP_Message_T<enum Stream_MessageType> FTP_Message_t;
template <//typename AllocatorType,
          ////////////////////////////////
          typename SessionDataType, // reference-counted
          ////////////////////////////////
          typename UserDataType>
class FTP_SessionMessage_T;
typedef FTP_SessionMessage_T<FTP_Stream_SessionData_t,
                             struct Stream_UserData> FTP_SessionMessage_t;

typedef FTP_Module_Streamer_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct FTP_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              FTP_Message_t,
                              FTP_SessionMessage_t> FTP_Module_Streamer_t;
typedef FTP_Module_Parser_T<ACE_MT_SYNCH,
                            Common_TimePolicy_t,
                            struct FTP_ModuleHandlerConfiguration,
                            Stream_ControlMessage_t,
                            FTP_Message_t,
                            FTP_SessionMessage_t> FTP_Module_Parser_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T <ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct FTP_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       FTP_Message_t,
                                                       FTP_SessionMessage_t,
                                                       FTP_Code_t,
                                                       FTP_Statistic_t,
                                                       Common_Timer_Manager_t,
                                                       struct Stream_FTP_SessionData,
                                                       FTP_Stream_SessionData_t> FTP_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T <ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct FTP_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       FTP_Message_t,
                                                       FTP_SessionMessage_t,
                                                       FTP_Code_t,
                                                       FTP_Statistic_t,
                                                       Common_Timer_Manager_t,
                                                       struct Stream_FTP_SessionData,
                                                       FTP_Stream_SessionData_t> FTP_Module_Statistic_WriterTask_t;

typedef FTP_Module_Send_T<ACE_MT_SYNCH,
                          Common_TimePolicy_t,
                          Stream_ControlMessage_t,
                          FTP_Message_t,
                          FTP_SessionMessage_t,
                          struct FTP_ModuleHandlerConfiguration,
                          struct FTP_ConnectionState,
                          typename ConnectionType> FTP_Module_Send;

// declare module(s)
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                           // task synch type
                          Common_TimePolicy_t,                    // time policy
                          struct Stream_ModuleConfiguration,      // module configuration type
                          struct FTP_ModuleHandlerConfiguration, // module handler configuration type
                          FTP_Module_Streamer_t,                 // reader type
                          FTP_Module_Parser_t,                   // writer type
                          FTP_Module_Marshal);                   // name

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                           // task synch type
                          Common_TimePolicy_t,                    // time policy type
                          struct Stream_ModuleConfiguration,      // module configuration type
                          struct FTP_ModuleHandlerConfiguration, // module handler configuration type
                          FTP_Module_Statistic_ReaderTask_t,     // reader type
                          FTP_Module_Statistic_WriterTask_t,     // writer type
                          FTP_Module_Statistic);                 // name

DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                           // task synch type
                              Common_TimePolicy_t,                    // time policy
                              Stream_ModuleConfiguration,             // module configuration type
                              struct FTP_ModuleHandlerConfiguration, // module handler configuration type
                              FTP_Module_Send);                      // writer type

#endif
