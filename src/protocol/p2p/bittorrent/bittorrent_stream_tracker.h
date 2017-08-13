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
//#include "stream_inotify.h"

#include "http_stream.h"

#include "bittorrent_common.h"

// forward declarations
//typedef Stream_INotify_T<enum Stream_SessionMessageType> BitTorrent_INotify_t;

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
                        StatisticHandlerType,
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
                        StatisticHandlerType,
                        ModuleHandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        UserDataType> inherited;

 public:
  BitTorrent_TrackerStream_T ();
  inline virtual ~BitTorrent_TrackerStream_T () { inherited::shutdown (); };

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
                                     PeerStreamType,
                                     ConnectionConfigurationType,
                                     ConnectionStateType,
                                     HandlerConfigurationType, // socket-
                                     SessionStateType,
                                     CBDataType,
                                     UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerStream_T (const BitTorrent_TrackerStream_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_TrackerStream_T& operator= (const BitTorrent_TrackerStream_T&))
};

// include template definition
#include "bittorrent_stream_tracker.inl"

#endif
