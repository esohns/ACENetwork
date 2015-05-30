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

#include "stream_streammodule_base.h"

#include "net_module_runtimestatistic.h"

#include "IRC_client_message.h"
#include "IRC_client_module_IRCsplitter.h"
#include "IRC_client_module_IRCstreamer.h"
#include "IRC_client_sessionmessage.h"
#include "IRC_client_stream_common.h"
#include "IRC_client_IRCmessage.h"

typedef Net_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          IRC_Client_SessionMessage,
                                          IRC_Client_Message,
                                          IRC_Client_CommandType_t,
                                          IRC_Client_RuntimeStatistic> IRC_Client_Module_Statistic_ReaderTask_t;
typedef Net_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          IRC_Client_SessionMessage,
                                          IRC_Client_Message,
                                          IRC_Client_CommandType_t,
                                          IRC_Client_RuntimeStatistic> IRC_Client_Module_Statistic_WriterTask_t;

// declare module(s)
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                             // task synch type
                          Common_TimePolicy_t,                      // time policy type
                          IRC_Client_Configuration,                 // configuration type
                          IRC_Client_Module_Statistic_ReaderTask_t, // reader type
                          IRC_Client_Module_Statistic_WriterTask_t, // writer type
                          IRC_Client_Module_RuntimeStatistic);      // name

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                  // task synch type
                          Common_TimePolicy_t,           // time policy
                          IRC_Client_Configuration,      // configuration type
                          IRC_Client_Module_IRCStreamer, // reader type
                          IRC_Client_Module_IRCSplitter, // writer type
                          IRC_Client_Module_IRCMarshal); // name

#endif
