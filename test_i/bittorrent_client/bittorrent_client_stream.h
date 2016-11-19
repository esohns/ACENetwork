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

#ifndef BITTORRENT_CLIENT_STREAM_H
#define BITTORRENT_CLIENT_STREAM_H

#include <string>

#include <ace/Global_Macros.h>

#include "bittorrent_common.h"
#include "bittorrent_message.h"
#include "bittorrent_stream.h"
//#include "bittorrent_stream_common.h"

#include "bittorrent_client_common_modules.h"
#include "bittorrent_client_sessionmessage.h"
#include "bittorrent_client_stream_common.h"

#include "test_i_gtk_common.h"

//class BitTorrent_Client_PeerStream
// : public BitTorrent_PeerStream_T<struct BitTorrent_Client_StreamState,
//                                  struct BitTorrent_Client_StreamConfiguration,
//                                  BitTorrent_RuntimeStatistic_t,
//                                  struct BitTorrent_Client_ModuleHandlerConfiguration,
//                                  struct BitTorrent_Client_SessionData,
//                                  BitTorrent_Client_SessionData_t,
//                                  BitTorrent_Client_PeerControlMessage_t,
//                                  BitTorrent_Client_PeerMessage,
//                                  BitTorrent_Client_SessionMessage,
//                                  struct BitTorrent_Client_Configuration,
//                                  struct BitTorrent_Client_ConnectionState,
//                                  struct BitTorrent_Client_SocketHandlerConfiguration,
//                                  struct BitTorrent_Client_SessionState,
//                                  struct BitTorrent_Client_GTK_CBData>
//{
// public:
//  BitTorrent_Client_PeerStream (const std::string&); // name
//  virtual ~BitTorrent_Client_PeerStream ();

////  // implement (part of) Stream_IStreamControlBase
////  virtual bool load (Stream_ModuleList_t&, // return value: module list
////                     bool&);               // return value: delete modules ?

// private:
//  typedef BitTorrent_PeerStream_T<struct BitTorrent_Client_StreamState,
//                                  struct BitTorrent_Client_StreamConfiguration,
//                                  BitTorrent_RuntimeStatistic_t,
//                                  struct BitTorrent_Client_ModuleHandlerConfiguration,
//                                  struct BitTorrent_Client_SessionData,
//                                  BitTorrent_Client_SessionData_t,
//                                  BitTorrent_Client_PeerControlMessage_t,
//                                  BitTorrent_Client_PeerMessage,
//                                  BitTorrent_Client_SessionMessage,
//                                  struct BitTorrent_Client_Configuration,
//                                  struct BitTorrent_Client_ConnectionState,
//                                  struct BitTorrent_Client_SocketHandlerConfiguration,
//                                  struct BitTorrent_Client_SessionState,
//                                  struct BitTorrent_Client_GTK_CBData> inherited;

//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerStream ())
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerStream (const BitTorrent_Client_PeerStream&))
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerStream& operator= (const BitTorrent_Client_PeerStream&))
//};

////////////////////////////////////////////

//class BitTorrent_Client_TrackerStream
// : public BitTorrent_TrackerStream_T<struct BitTorrent_Client_StreamState,
//                                     struct BitTorrent_Client_StreamConfiguration,
//                                     BitTorrent_RuntimeStatistic_t,
//                                     struct BitTorrent_Client_ModuleHandlerConfiguration,
//                                     struct BitTorrent_Client_SessionData,
//                                     BitTorrent_Client_SessionData_t,
//                                     BitTorrent_Client_TrackerControlMessage_t,
//                                     BitTorrent_Client_TrackerMessage,
//                                     BitTorrent_Client_SessionMessage,
//                                     BitTorrent_Client_PeerStream,
//                                     struct BitTorrent_Client_Configuration,
//                                     struct BitTorrent_Client_ConnectionState,
//                                     struct BitTorrent_Client_SocketHandlerConfiguration,
//                                     struct BitTorrent_Client_SessionState,
//                                     struct BitTorrent_Client_GTK_CBData>
//{
// public:
//  BitTorrent_Client_TrackerStream (const std::string&); // name
//  virtual ~BitTorrent_Client_TrackerStream ();

//  // implement (part of) Stream_IStreamControlBase
//  virtual bool load (Stream_ModuleList_t&, // return value: module list
//                     bool&);               // return value: delete modules ?

// private:
//  typedef BitTorrent_TrackerStream_T<struct BitTorrent_Client_StreamState,
//                                     struct BitTorrent_Client_StreamConfiguration,
//                                     BitTorrent_RuntimeStatistic_t,
//                                     struct BitTorrent_Client_ModuleHandlerConfiguration,
//                                     struct BitTorrent_Client_SessionData,
//                                     BitTorrent_Client_SessionData_t,
//                                     BitTorrent_Client_TrackerControlMessage_t,
//                                     BitTorrent_Client_TrackerMessage,
//                                     BitTorrent_Client_SessionMessage,
//                                     BitTorrent_Client_PeerStream,
//                                     struct BitTorrent_Client_Configuration,
//                                     struct BitTorrent_Client_ConnectionState,
//                                     struct BitTorrent_Client_SocketHandlerConfiguration,
//                                     struct BitTorrent_Client_SessionState,
//                                     struct BitTorrent_Client_GTK_CBData> inherited;

//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerStream ())
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerStream (const BitTorrent_Client_TrackerStream&))
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerStream& operator= (const BitTorrent_Client_TrackerStream&))
//};

#endif
