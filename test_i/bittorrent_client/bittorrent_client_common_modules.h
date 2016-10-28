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

#ifndef BITTORRENT_CLIENT_COMMON_MODULES_H
#define BITTORRENT_CLIENT_COMMON_MODULES_H

#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_misc_statistic_report.h"

//#include "bittorrent_common_modules.h"
#include "bittorrent_module_bisector.h"
#include "bittorrent_stream_common.h"

#include "bittorrent_client_configuration.h"

// forward declarations
class BitTorrent_Message;
class BitTorrent_Client_SessionMessage;

//typedef BitTorrent_Module_Bisector_T<ACE_MT_SYNCH,
//                              ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              ACE_Message_Block,
//                              BitTorrent_Message,
//                              BitTorrent_Client_SessionMessage,
//                              BitTorrent_Client_ModuleHandlerConfiguration,
//                              int,
//                              int,
//                              BitTorrent_StreamState,
//                              BitTorrent_Client_SessionData,
//                              BitTorrent_Client_SessionData_t,
//                              BitTorrent_RuntimeStatistic_t> BitTorrent_Client_Module_Bisector_t;

typedef BitTorrent_Module_Parser_Module BitTorrent_Client_Module_Parser_Module;

typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   BitTorrent_Client_ModuleHandlerConfiguration,
                                                   ACE_Message_Block,
                                                   BitTorrent_Message,
                                                   BitTorrent_Client_SessionMessage,
                                                   enum BitTorrent_MessageType,
                                                   BitTorrent_RuntimeStatistic_t,
                                                   BitTorrent_Client_SessionData,
                                                   BitTorrent_Client_SessionData_t> BitTorrent_Client_StatisticReport_ReaderTask_t;
typedef Stream_Module_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   BitTorrent_Client_ModuleHandlerConfiguration,
                                                   ACE_Message_Block,
                                                   BitTorrent_Message,
                                                   BitTorrent_Client_SessionMessage,
                                                   enum BitTorrent_MessageType,
                                                   BitTorrent_RuntimeStatistic_t,
                                                   BitTorrent_Client_SessionData,
                                                   BitTorrent_Client_SessionData_t> BitTorrent_Client_StatisticReport_WriterTask_t;

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                 // task synch type
                          Common_TimePolicy_t,                          // time policy
                          Stream_ModuleConfiguration,                   // module configuration type
                          BitTorrent_Client_ModuleHandlerConfiguration, // module handler configuration type
                          BitTorrent_Module_Streamer_t,                 // reader type
                          BitTorrent_Client_Module_Bisector_t,          // writer type
                          BitTorrent_Client_Marshal);                   // name
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                   // task synch type
                          Common_TimePolicy_t,                            // time policy type
                          Stream_ModuleConfiguration,                     // module configuration type
                          BitTorrent_Client_ModuleHandlerConfiguration,   // module handler configuration type
                          BitTorrent_Client_StatisticReport_ReaderTask_t, // reader type
                          BitTorrent_Client_StatisticReport_WriterTask_t, // writer type
                          BitTorrent_Client_StatisticReport);             // name

#endif
