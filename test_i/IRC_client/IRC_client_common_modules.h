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

#ifndef IRC_CLIENT_COMMON_MODULES_H
#define IRC_CLIENT_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"
#include "stream_stat_statistic_report.h"

#include "irc_common.h"
#include "irc_message.h"
#include "irc_module_bisector.h"
#include "irc_module_parser.h"
#include "irc_module_streamer.h"
#include "irc_record.h"

#include "IRC_client_stream_common.h"

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct IRC_Client_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> IRC_Client_SessionManager_t;

typedef IRC_Module_Streamer_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct IRC_Client_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              IRC_Message,
                              IRC_Client_SessionMessage,
                              struct Stream_UserData> IRC_Client_Module_Streamer_t;
typedef IRC_Module_Bisector_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Stream_ControlMessage_t,
                              IRC_Message,
                              IRC_Client_SessionMessage,
                              struct IRC_Client_ModuleHandlerConfiguration,
                              enum Stream_ControlType,
                              enum Stream_SessionMessageType,
                              struct IRC_Client_StreamState,
                              struct Stream_Statistic,
                              IRC_Client_SessionManager_t,
                              Common_Timer_Manager_t,
                              struct Stream_UserData> IRC_Client_Module_Bisector_t;

typedef IRC_Module_Parser_T<ACE_MT_SYNCH,
                            Common_TimePolicy_t,
                            struct IRC_Client_ModuleHandlerConfiguration,
                            Stream_ControlMessage_t,
                            IRC_Message,
                            IRC_Client_SessionMessage> IRC_Client_Module_Parser;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct IRC_Client_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      IRC_Message,
                                                      IRC_Client_SessionMessage,
                                                      IRC_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> IRC_Client_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct IRC_Client_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      IRC_Message,
                                                      IRC_Client_SessionMessage,
                                                      IRC_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> IRC_Client_Module_Statistic_WriterTask_t;

DATASTREAM_MODULE_DUPLEX (struct IRC_Client_SessionData,                // session data type
                          enum Stream_SessionMessageType,               // session event type
                          struct IRC_Client_ModuleHandlerConfiguration, // module handler configuration type
                          libacenetwork_protocol_default_irc_bisector_module_name_string,
                          Stream_INotify_t,                             // stream notification interface type
                          IRC_Client_Module_Streamer_t,                 // reader type
                          IRC_Client_Module_Bisector_t,                 // writer type
                          IRC_Client_Module_Marshal);                   // name
DATASTREAM_MODULE_INPUT_ONLY (struct IRC_Client_SessionData,                // session data type
                              enum Stream_SessionMessageType,               // session event type
                              struct IRC_Client_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_protocol_default_irc_parser_module_name_string,
                              Stream_INotify_t,                             // stream notification interface type
                              IRC_Client_Module_Parser);                    // writer type
DATASTREAM_MODULE_DUPLEX (struct IRC_Client_SessionData,                // session data type
                          enum Stream_SessionMessageType,               // session event type
                          struct IRC_Client_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                             // stream notification interface type
                          IRC_Client_Module_Statistic_ReaderTask_t,     // reader type
                          IRC_Client_Module_Statistic_WriterTask_t,     // writer type
                          IRC_Client_Module_StatisticReport);           // name

#endif
