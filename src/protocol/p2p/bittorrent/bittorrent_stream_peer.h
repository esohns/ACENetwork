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

#ifndef BITTORRENT_STREAM_PEER_H
#define BITTORRENT_STREAM_PEER_H

#include <string>

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_inotify.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "net_module_runtimestatistic.h"

#include "bittorrent_common.h"
//#include "bittorrent_module_bisector.h"
//#include "bittorrent_module_handler.h"
#include "bittorrent_module_parser.h"
#include "bittorrent_module_streamer.h"

// forward declarations
typedef Stream_INotify_T<Stream_SessionMessageType> BitTorrent_INotify_t;
typedef Stream_Statistic BitTorrent_RuntimeStatistic_t;

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType, // socket-
          typename SessionStateType,
          typename CBDataType>
class BitTorrent_PeerStream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        struct Stream_ModuleConfiguration,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
{
 public:
  BitTorrent_PeerStream_T (const std::string&); // name
  virtual ~BitTorrent_PeerStream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // override Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&, // configuration
                           bool = true,              // setup pipeline ?
                           bool = true);             // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: delegates to the statistic report module
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  // just a dummy (set statisticReportingInterval instead)
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        struct Stream_ModuleConfiguration,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

  typedef BitTorrent_PeerStream_T<StreamStateType,
                                  ConfigurationType,
                                  StatisticContainerType,
                                  ModuleHandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConnectionConfigurationType,
                                  ConnectionStateType,
                                  HandlerConfigurationType, // socket-
                                  SessionStateType,
                                  CBDataType> OWN_TYPE_T;

  typedef BitTorrent_Module_Streamer_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType> STREAMER_T;
  //typedef BitTorrent_Module_Bisector_T<ACE_SYNCH_MUTEX,
  //                                     ACE_MT_SYNCH,
  //                                     Common_TimePolicy_t,
  //                                     SessionMessageType,
  //                                     ProtocolMessageType,
  //                                     ModuleHandlerConfigurationType,
  //                                     StreamStateType,
  //                                     SessionDataType,
  //                                     SessionDataContainerType,
  //                                     StatisticContainerType> BISECTOR_T;
  typedef BitTorrent_Module_ParserH_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ModuleHandlerConfigurationType,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType> PARSER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,             // session id type
                                SessionDataType,                // session data type
                                enum Stream_SessionMessageType, // session event type
                                Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                BitTorrent_INotify_t,           // stream notification interface type
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;
  //typedef BitTorrent_Module_Parser_T<ACE_MT_SYNCH,
  //                             Common_TimePolicy_t,
  //                             SessionMessageType,
  //                             ProtocolMessageType> PARSER_T;
  //typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
  //                                       Common_TimePolicy_t,
  //                                       Stream_ModuleConfiguration,
  //                                       ModuleHandlerConfigurationType,
  //                                       PARSER_T> MODULE_PARSER_T;

  typedef Net_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            ModuleHandlerConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            enum BitTorrent_MessageType,
                                            BitTorrent_RuntimeStatistic_t> STATISTIC_READER_T;
  typedef Net_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            ModuleHandlerConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            enum BitTorrent_MessageType,
                                            BitTorrent_RuntimeStatistic_t> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Stream_SessionId_t,             // session id type
                                SessionDataType,                // session data type
                                Stream_SessionMessageType,      // session event type
                                Stream_ModuleConfiguration,
                                ModuleHandlerConfigurationType,
                                BitTorrent_INotify_t,           // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

//  typedef BitTorrent_Module_PeerHandler_T<ACE_INET_Addr,
//                                          ModuleHandlerConfigurationType,
//                                          BitTorrent_RuntimeStatistic_t,
//                                          ControlMessageType,
//                                          DataMessageType,
//                                          SessionMessageType,
//                                          SessionDataType,
//                                          OWN_TYPE_T,
//                                          enum Stream_StateMachine_ControlState,
//                                          struct Net_SocketConfiguration,
//                                          HandlerConfigurationType,
//                                          ConnectionConfigurationType,
//                                          ConnectionStateType,
//                                          SessionStateType,
//                                          CBDataType> HANDLER_T;
//  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
//                                         Common_TimePolicy_t,
//                                         Stream_SessionId_t,             // session id type
//                                         SessionDataType,                // session data type
//                                         Stream_SessionMessageType,      // session event type
//                                         Stream_ModuleConfiguration,
//                                         ModuleHandlerConfigurationType,
//                                         BitTorrent_INotify_t,           // stream notification interface type
//                                         HANDLER_T> MODULE_HANDLER_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_PeerStream_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_PeerStream_T (const BitTorrent_PeerStream_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_PeerStream_T& operator= (const BitTorrent_PeerStream_T&))
};

// include template definition
#include "bittorrent_stream_peer.inl"

#endif
