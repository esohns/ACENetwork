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

//#include "bittorrent_module_bisector.h"
//#include "bittorrent_module_handler.h"
#include "bittorrent_module_parser.h"
#include "bittorrent_module_streamer.h"
#include "bittorrent_stream_common.h"

#include "bittorrent_client_gui_common.h"
#include "bittorrent_client_stream_common.h"

// forward declarations
//class BitTorrent_Client_Message;
//class BitTorrent_Client_SessionMessage;
struct BitTorrent_Client_GTK_CBData;

typedef BitTorrent_Module_Streamer_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct BitTorrent_Client_PeerModuleHandlerConfiguration,
                                     BitTorrent_Client_PeerControlMessage_t,
                                     BitTorrent_Client_PeerMessage_t,
                                     BitTorrent_Client_PeerSessionMessage_t> BitTorrent_Client_Streamer_t;

//typedef BitTorrent_Module_Bisector_T<ACE_MT_SYNCH,
//                              ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              BitTorrent_Client_PeerControlMessage_t,
//                              BitTorrent_Client_PeerMessage_t,
//                              BitTorrent_Client_PeerSessionMessage,
//                              struct BitTorrent_Client_PeerModuleHandlerConfiguration,
//                              enum Stream_ControlType,
//                              enum Stream_SessionMessageType,
//                              struct BitTorrent_StreamState,
//                              struct BitTorrent_Client_PeerSessionData,
//                              BitTorrent_Client_SessionData_t,
//                              BitTorrent_RuntimeStatistic_t> BitTorrent_Client_Module_Bisector_t;
typedef BitTorrent_Module_ParserH_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    BitTorrent_Client_PeerControlMessage_t,
                                    BitTorrent_Client_PeerMessage_t,
                                    BitTorrent_Client_PeerSessionMessage_t,
                                    struct BitTorrent_Client_PeerModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct BitTorrent_StreamState,
                                    struct BitTorrent_Client_PeerSessionData,
                                    BitTorrent_Client_PeerSessionData_t,
                                    BitTorrent_RuntimeStatistic_t,
                                    struct BitTorrent_Client_PeerUserData> BitTorrent_Client_Parser_t;

DATASTREAM_MODULE_DUPLEX (struct BitTorrent_Client_PeerSessionData,                // session data
                          enum Stream_SessionMessageType,                          // session event type
                          struct BitTorrent_Client_PeerModuleHandlerConfiguration, // module handler configuration type
                          BitTorrent_INotify_t,                                    // stream notification interface type
                          BitTorrent_Client_Streamer_t,                            // reader type
                          BitTorrent_Client_Parser_t,                              // writer type
                          BitTorrent_Client_Marshal);                              // name

typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct BitTorrent_Client_PeerModuleHandlerConfiguration,
                                                   BitTorrent_Client_PeerControlMessage_t,
                                                   BitTorrent_Client_PeerMessage_t,
                                                   BitTorrent_Client_PeerSessionMessage_t,
                                                   enum BitTorrent_MessageType,
                                                   BitTorrent_RuntimeStatistic_t,
                                                   struct BitTorrent_Client_PeerSessionData,
                                                   BitTorrent_Client_PeerSessionData_t> BitTorrent_Client_StatisticReport_ReaderTask_t;
typedef Stream_Module_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct BitTorrent_Client_PeerModuleHandlerConfiguration,
                                                   BitTorrent_Client_PeerControlMessage_t,
                                                   BitTorrent_Client_PeerMessage_t,
                                                   BitTorrent_Client_PeerSessionMessage_t,
                                                   enum BitTorrent_MessageType,
                                                   BitTorrent_RuntimeStatistic_t,
                                                   struct BitTorrent_Client_PeerSessionData,
                                                   BitTorrent_Client_PeerSessionData_t> BitTorrent_Client_StatisticReport_WriterTask_t;

DATASTREAM_MODULE_DUPLEX (struct BitTorrent_Client_PeerSessionData,                // session data
                          enum Stream_SessionMessageType,                          // session event type
                          struct BitTorrent_Client_PeerModuleHandlerConfiguration, // module handler configuration type
                          BitTorrent_INotify_t,                                    // stream notification interface type
                          BitTorrent_Client_StatisticReport_ReaderTask_t,          // reader type
                          BitTorrent_Client_StatisticReport_WriterTask_t,          // writer type
                          BitTorrent_Client_StatisticReport);                      // name

//////////////////////////////////////////

//typedef BitTorrent_Module_PeerHandler_T<ACE_INET_Addr,
//                                        struct BitTorrent_Client_ModuleHandlerConfiguration,
//                                        BitTorrent_RuntimeStatistic_t,
//                                        BitTorrent_Client_PeerControlMessage_t,
//                                        BitTorrent_Client_PeerMessage_t,
//                                        BitTorrent_Client_PeerSessionMessage_t,
//                                        struct BitTorrent_Client_PeerSessionData,
//                                        BitTorrent_Client_PeerStream_t,
//                                        enum Stream_StateMachine_ControlState,
//                                        struct Net_SocketConfiguration,
//                                        struct BitTorrent_Client_SocketHandlerConfiguration,
//                                        struct BitTorrent_Client_Configuration,
//                                        struct BitTorrent_Client_ConnectionState,
//                                        struct BitTorrent_Client_SessionState,
//                                        struct BitTorrent_Client_GTK_CBData> BitTorrent_Client_PeerHandler_t;

//DATASTREAM_MODULE_INPUT_ONLY_T (struct BitTorrent_Client_PeerSessionData,            // session data
//                                enum Stream_SessionMessageType,                      // session event type
//                                struct BitTorrent_Client_ModuleHandlerConfiguration, // module handler configuration type
//                                BitTorrent_INotify_t,                                // stream notification interface type
//                                BitTorrent_Client_PeerHandler_t,                     // writer type
//                                BitTorrent_Client_PeerHandler);                      // name

//----------------------------------------

//typedef BitTorrent_Module_TrackerHandler_T<ACE_INET_Addr,
//                                           struct BitTorrent_Client_ModuleHandlerConfiguration,
//                                           BitTorrent_RuntimeStatistic_t,
//                                           BitTorrent_Client_TrackerControlMessage_t,
//                                           BitTorrent_Client_TrackerMessage_t,
//                                           BitTorrent_Client_TrackerSessionMessage_t,
//                                           struct BitTorrent_Client_TrackerSessionData,
//                                           BitTorrent_Client_PeerStream_t,
//                                           enum Stream_StateMachine_ControlState,
//                                           struct Net_SocketConfiguration,
//                                           struct BitTorrent_Client_SocketHandlerConfiguration,
//                                           struct BitTorrent_Client_Configuration,
//                                           struct BitTorrent_Client_ConnectionState,
//                                           struct BitTorrent_Client_SessionState,
//                                           struct BitTorrent_Client_GTK_CBData> BitTorrent_Client_TrackerHandler_t;

//DATASTREAM_MODULE_INPUT_ONLY_T (struct BitTorrent_Client_TrackerSessionData,         // session data
//                                enum Stream_SessionMessageType,                      // session event type
//                                struct BitTorrent_Client_ModuleHandlerConfiguration, // module handler configuration type
//                                BitTorrent_INotify_t,                                // stream notification interface type
//                                BitTorrent_Client_TrackerHandler_t,                  // writer type
//                                BitTorrent_Client_TrackerHandler);                   // name

#endif
