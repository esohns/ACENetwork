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

#ifndef IRC_COMMON_MODULES_H
#define IRC_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_statistic_report.h"

#include "irc_common.h"
#include "irc_configuration.h"
//#include "irc_module_bisector.h"
#include "irc_module_parser.h"
#include "irc_module_streamer.h"
//#include "irc_stream_common.h"

// forward declarations
class IRC_Message;
template <typename SessionDataType,
          typename UserDataType>
class IRC_SessionMessage_T;
typedef IRC_SessionMessage_T<IRC_Stream_SessionData_t,
                             struct Stream_UserData> IRC_SessionMessage_t;
//struct IRC_Stream_SessionData;
//struct IRC_StreamState;

typedef IRC_Module_Parser_T<ACE_MT_SYNCH,
                            Common_TimePolicy_t,
                            struct IRC_ModuleHandlerConfiguration,
                            ACE_Message_Block,
                            IRC_Message,
                            IRC_SessionMessage_t> IRC_Module_Parser;

typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct IRC_ModuleHandlerConfiguration,
                                                   ACE_Message_Block,
                                                   IRC_Message,
                                                   IRC_SessionMessage_t,
                                                   IRC_CommandType_t,
                                                   IRC_RuntimeStatistic_t,
                                                   struct IRC_Stream_SessionData,
                                                   IRC_Stream_SessionData_t> IRC_Module_StatisticReport_ReaderTask_t;
typedef Stream_Module_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct IRC_ModuleHandlerConfiguration,
                                                   ACE_Message_Block,
                                                   IRC_Message,
                                                   IRC_SessionMessage_t,
                                                   IRC_CommandType_t,
                                                   IRC_RuntimeStatistic_t,
                                                   struct IRC_Stream_SessionData,
                                                   IRC_Stream_SessionData_t> IRC_Module_StatisticReport_WriterTask_t;

//typedef IRC_Module_Streamer_T<ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              struct IRC_ModuleHandlerConfiguration,
//                              ACE_Message_Block,
//                              IRC_Message,
//                              IRC_SessionMessage_t> IRC_Module_Streamer_t;
//typedef IRC_Module_Bisector_T<ACE_SYNCH_MUTEX,
//                              ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              IRC_SessionMessage_t,
//                              IRC_Message,
//                              struct IRC_ModuleHandlerConfiguration,
//                              IRC_StreamState,
//                              IRC_Stream_SessionData,
//                              IRC_Stream_SessionData_t,
//                              IRC_RuntimeStatistic_t> IRC_Module_Bisector_t;

// declare module(s)
DATASTREAM_MODULE_INPUT_ONLY (struct IRC_Stream_SessionData,         // session data type
                              enum Stream_SessionMessageType,        // session event type
                              struct IRC_ModuleHandlerConfiguration, // module handler configuration type
                              IRC_Stream_INotify_t,                  // stream notification interface type
                              IRC_Module_Parser);                    // writer type

DATASTREAM_MODULE_DUPLEX (struct IRC_Stream_SessionData,           // session data type
                          enum Stream_SessionMessageType,          // session event type
                          struct IRC_ModuleHandlerConfiguration,   // module handler configuration type
                          IRC_Stream_INotify_t,                    // stream notification interface type
                          IRC_Module_StatisticReport_ReaderTask_t, // reader type
                          IRC_Module_StatisticReport_WriterTask_t, // writer type
                          IRC_Module_StatisticReport);             // name

//DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                          // task synch type
//                          Common_TimePolicy_t,                   // time policy
//                          struct Stream_ModuleConfiguration,     // module configuration type
//                          struct IRC_ModuleHandlerConfiguration, // module handler configuration type
//                          IRC_Module_Streamer,                   // reader type
//                          IRC_Module_Bisector_t,                 // writer type
//                          IRC_Module_Marshal);                   // name

#endif
