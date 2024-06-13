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

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "http_stream.h"

#include "bittorrent_common.h"
#include "bittorrent_module_parser.h"

template <typename StreamStateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename StatisticHandlerType,
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
          ////////////////////////////////
          typename ConnectionManagerType,
          typename UserDataType>
class BitTorrent_TrackerStream_T
 : public HTTP_Stream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        StatisticHandlerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConnectionManagerType,
                        UserDataType>
{
  typedef HTTP_Stream_T<StreamStateType,
                        ConfigurationType,
                        StatisticContainerType,
                        StatisticHandlerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConnectionManagerType,
                        UserDataType> inherited;

 public:
  BitTorrent_TrackerStream_T ();
  inline virtual ~BitTorrent_TrackerStream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);

 private:
  typedef BitTorrent_TrackerStream_T<StreamStateType,
                                     ConfigurationType,
                                     StatisticContainerType,
                                     StatisticHandlerType,
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
                                     ConnectionManagerType,
                                     UserDataType> OWN_TYPE_T;
  typedef BitTorrent_Module_TrackerParser_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            ModuleHandlerConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            UserDataType> PARSER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         SessionDataType,
                                         enum Stream_SessionMessageType,
                                         struct Stream_ModuleConfiguration,
                                         ModuleHandlerConfigurationType,
                                         libacenetwork_protocol_bittorrent_default_tracker_parser_module_name_string,
                                         Stream_INotify_t,
                                         PARSER_T> MODULE_PARSER_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerStream_T (const BitTorrent_TrackerStream_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerStream_T& operator= (const BitTorrent_TrackerStream_T&))
};

// include template definition
#include "bittorrent_stream_tracker.inl"

#endif
