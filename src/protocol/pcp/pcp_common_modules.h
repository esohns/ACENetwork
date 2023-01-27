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

#ifndef PCP_COMMON_MODULES_H
#define PCP_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

//#include "net_module_runtimestatistic.h"

#include "pcp_codes.h"
#include "pcp_common.h"
#include "pcp_configuration.h"
//#include "pcp_module_bisector.h"
#include "pcp_module_parser.h"
#include "pcp_module_streamer.h"
//#include "pcp_stream_common.h"

// forward declarations
class PCP_SessionMessage;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType>
class PCP_Message_T;
typedef PCP_Message_T<Stream_AllocatorConfiguration,
                      PCP_ControlMessage_t,
                      PCP_SessionMessage> PCP_Message_t;
//struct PCP_StreamSessionData;
//struct PCP_StreamState;

typedef PCP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             PCP_ModuleHandlerConfiguration,
                             ACE_Message_Block,
                             PCP_Message_t,
                             PCP_SessionMessage> PCP_Module_Parser;
typedef PCP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               PCP_ModuleHandlerConfiguration,
                               ACE_Message_Block,
                               PCP_Message_t,
                               PCP_SessionMessage> PCP_Module_Streamer;

typedef Net_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          PCP_ModuleHandlerConfiguration,
                                          ACE_Message_Block,
                                          PCP_Message_t,
                                          PCP_SessionMessage,
                                          PCP_MessageType_t,
                                          PCP_RuntimeStatistic_t> PCP_Module_Statistic_ReaderTask_t;
typedef Net_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          PCP_ModuleHandlerConfiguration,
                                          ACE_Message_Block,
                                          PCP_Message_t,
                                          PCP_SessionMessage,
                                          PCP_MessageType_t,
                                          PCP_RuntimeStatistic_t> PCP_Module_Statistic_WriterTask_t;

//typedef PCP_Module_Streamer_T<ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              PCP_SessionMessage,
//                              PCP_Message> PCP_Module_Streamer_t;
//typedef PCP_Module_Bisector_T<ACE_SYNCH_MUTEX,
//                              ///////////
//                              ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              PCP_SessionMessage,
//                              PCP_Message_t,
//                              ///////////
//                              PCP_ModuleHandlerConfiguration,
//                              ///////////
//                              PCP_StreamState,
//                              ///////////
//                              PCP_Stream_SessionData,
//                              PCP_Stream_SessionData_t,
//                              ///////////
//                              PCP_RuntimeStatistic_t> PCP_Module_Bisector_t;

// declare module(s)
//DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                    // task synch type
//                              Common_TimePolicy_t,             // time policy
//                              Stream_ModuleConfiguration,      // module configuration type
//                              PCP_ModuleHandlerConfiguration, // module handler configuration type
//                              PCP_Module_Parser);             // writer type
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                    // task synch type
                          Common_TimePolicy_t,             // time policy
                          Stream_ModuleConfiguration,      // module configuration type
                          PCP_ModuleHandlerConfiguration, // module handler configuration type
                          PCP_Module_Streamer,            // reader type
                          PCP_Module_Parser,              // writer type
                          PCP_Module_Marshal);            // name

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                       // task synch type
                          Common_TimePolicy_t,                // time policy type
                          Stream_ModuleConfiguration,         // module configuration type
                          PCP_ModuleHandlerConfiguration,    // module handler configuration type
                          PCP_Module_Statistic_ReaderTask_t, // reader type
                          PCP_Module_Statistic_WriterTask_t, // writer type
                          PCP_Module_RuntimeStatistic);      // name

#endif
