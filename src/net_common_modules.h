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

#ifndef NET_COMMON_MODULES_H
#define NET_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "net_common.h"
#include "net_message.h"
#include "net_module_runtimestatistic.h"
#include "net_module_sockethandler.h"
#include "net_remote_comm.h"
#include "net_sessionmessage.h"
#include "net_stream_common.h"

// declare module(s)
typedef Net_Module_SocketHandler_T<Stream_State,
                                   Net_UserData,
                                   Net_StreamSessionData_t,
                                   Net_SessionMessage,
                                   Net_Message,
                                   Net_Remote_Comm::MessageHeader> Net_Module_SocketHandler;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,               // task synch type
                              Common_TimePolicy_t,        // time policy type
                              Stream_ModuleConfiguration, // configuration type
                              Net_Module_SocketHandler);  // writer type

typedef Net_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Net_SessionMessage,
                                          Net_Message,
                                          Net_MessageType_t,
                                          Stream_Statistic> Net_Module_Statistic_ReaderTask_t;
typedef Net_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Net_SessionMessage,
                                          Net_Message,
                                          Net_MessageType_t,
                                          Stream_Statistic> Net_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                      // task synch type
                          Common_TimePolicy_t,               // time policy type
                          Stream_ModuleConfiguration,        // configuration type
                          Net_Module_Statistic_ReaderTask_t, // reader type
                          Net_Module_Statistic_WriterTask_t, // writer type
                          Net_Module_RuntimeStatistic);      // name

#endif
