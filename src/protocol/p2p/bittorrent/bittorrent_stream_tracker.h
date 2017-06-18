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

#ifndef BITTORRENT_STREAM_TRACKER_H
#define BITTORRENT_STREAM_TRACKER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_inotify.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "net_module_runtimestatistic.h"

#include "http_stream.h"

#include "bittorrent_common.h"
//#include "bittorrent_module_bisector.h"
//#include "bittorrent_module_handler.h"

// forward declarations
typedef Stream_INotify_T<enum Stream_SessionMessageType> BitTorrent_INotify_t;
typedef Stream_Statistic BitTorrent_RuntimeStatistic_t;

extern const char stream_name_string_[];

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
          typename PeerStreamType,
          ////////////////////////////////
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename HandlerConfigurationType, // socket-
          typename SessionStateType,
          typename CBDataType,
          ////////////////////////////////
          typename UserDataType>
class BitTorrent_TrackerStream_T
 : public HTTP_Stream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        UserDataType>
{
  typedef HTTP_Stream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        UserDataType> inherited;

 public:
  BitTorrent_TrackerStream_T ();
  virtual ~BitTorrent_TrackerStream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // override Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&); // configuration
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration
#endif

//  // implement Common_IStatistic_T
//  // *NOTE*: delegates to the statistic report module
//  virtual bool collect (StatisticContainerType&); // return value: statistic data
//  // just a dummy (set statisticReportingInterval instead)
//  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 private:
  typedef BitTorrent_TrackerStream_T<StreamStateType,
                                     ConfigurationType,
                                     StatisticContainerType,
                                     ModuleHandlerConfigurationType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     PeerStreamType,
                                     ConnectionConfigurationType,
                                     ConnectionStateType,
                                     HandlerConfigurationType, // socket-
                                     SessionStateType,
                                     CBDataType,
                                     UserDataType> OWN_TYPE_T;

//  typedef BitTorrent_Module_TrackerHandler_T<ACE_INET_Addr,
//                                             ModuleHandlerConfigurationType,
//                                             BitTorrent_RuntimeStatistic_t,
//                                             ControlMessageType,
//                                             DataMessageType,
//                                             SessionMessageType,
//                                             SessionDataType,
//                                             PeerStreamType,
//                                             enum Stream_StateMachine_ControlState,
//                                             struct Net_SocketConfiguration,
//                                             HandlerConfigurationType,
//                                             ConnectionConfigurationType,
//                                             ConnectionStateType,
//                                             SessionStateType,
//                                             CBDataType> HANDLER_T;
//  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
//                                         Common_TimePolicy_t,
//                                         Stream_SessionId_t,             // session id type
//                                         SessionDataType,                // session data type
//                                         Stream_SessionMessageType,      // session event type
//                                         Stream_ModuleConfiguration,
//                                         ModuleHandlerConfigurationType,
//                                         BitTorrent_INotify_t,           // stream notification interface type
//                                         HANDLER_T> MODULE_HANDLER_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerStream_T (const BitTorrent_TrackerStream_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerStream_T& operator= (const BitTorrent_TrackerStream_T&))
};

// include template definition
#include "bittorrent_stream_tracker.inl"

#endif
