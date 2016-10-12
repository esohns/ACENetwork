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

#ifndef DHCP_COMMON_MODULES_H
#define DHCP_COMMON_MODULES_H

#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "net_module_runtimestatistic.h"

#include "dhcp_codes.h"
#include "dhcp_common.h"
#include "dhcp_configuration.h"
//#include "dhcp_module_bisector.h"
#include "dhcp_module_parser.h"
#include "dhcp_module_streamer.h"
//#include "dhcp_stream_common.h"

// forward declarations
class DHCP_SessionMessage;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType>
class DHCP_Message_T;
typedef DHCP_Message_T<Stream_AllocatorConfiguration,
                       DHCP_ControlMessage_t,
                       DHCP_SessionMessage> DHCP_Message_t;
//struct DHCP_StreamSessionData;
//struct DHCP_StreamState;

typedef DHCP_Module_Parser_T<ACE_MT_SYNCH,
                             Common_TimePolicy_t,
                             DHCP_ModuleHandlerConfiguration,
                             ACE_Message_Block,
                             DHCP_Message_t,
                             DHCP_SessionMessage> DHCP_Module_Parser;
typedef DHCP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               DHCP_ModuleHandlerConfiguration,
                               ACE_Message_Block,
                               DHCP_Message_t,
                               DHCP_SessionMessage> DHCP_Module_Streamer;

typedef Net_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          DHCP_ModuleHandlerConfiguration,
                                          ACE_Message_Block,
                                          DHCP_Message_t,
                                          DHCP_SessionMessage,
                                          DHCP_MessageType_t,
                                          DHCP_RuntimeStatistic_t> DHCP_Module_Statistic_ReaderTask_t;
typedef Net_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          DHCP_ModuleHandlerConfiguration,
                                          ACE_Message_Block,
                                          DHCP_Message_t,
                                          DHCP_SessionMessage,
                                          DHCP_MessageType_t,
                                          DHCP_RuntimeStatistic_t> DHCP_Module_Statistic_WriterTask_t;

//typedef DHCP_Module_Streamer_T<ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              DHCP_SessionMessage,
//                              DHCP_Message> DHCP_Module_Streamer_t;
//typedef DHCP_Module_Bisector_T<ACE_SYNCH_MUTEX,
//                              ///////////
//                              ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              DHCP_SessionMessage,
//                              DHCP_Message_t,
//                              ///////////
//                              DHCP_ModuleHandlerConfiguration,
//                              ///////////
//                              DHCP_StreamState,
//                              ///////////
//                              DHCP_Stream_SessionData,
//                              DHCP_Stream_SessionData_t,
//                              ///////////
//                              DHCP_RuntimeStatistic_t> DHCP_Module_Bisector_t;

// declare module(s)
//DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                    // task synch type
//                              Common_TimePolicy_t,             // time policy
//                              Stream_ModuleConfiguration,      // module configuration type
//                              DHCP_ModuleHandlerConfiguration, // module handler configuration type
//                              DHCP_Module_Parser);             // writer type
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                    // task synch type
                          Common_TimePolicy_t,             // time policy
                          Stream_ModuleConfiguration,      // module configuration type
                          DHCP_ModuleHandlerConfiguration, // module handler configuration type
                          DHCP_Module_Streamer,            // reader type
                          DHCP_Module_Parser,              // writer type
                          DHCP_Module_Marshal);            // name

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                       // task synch type
                          Common_TimePolicy_t,                // time policy type
                          Stream_ModuleConfiguration,         // module configuration type
                          DHCP_ModuleHandlerConfiguration,    // module handler configuration type
                          DHCP_Module_Statistic_ReaderTask_t, // reader type
                          DHCP_Module_Statistic_WriterTask_t, // writer type
                          DHCP_Module_RuntimeStatistic);      // name

#endif
