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
#include "stdafx.h"

#include <ace/Synch.h>
#include "bittorrent_client_configuration.h"
#include "bittorrent_client_stream.h"

//BitTorrent_Client_PeerStream::BitTorrent_Client_PeerStream (const std::string& name_in)
// : inherited (name_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStream::BitTorrent_Client_PeerStream"));

//  for (Stream_ModuleListIterator_t iterator = inherited::modules_.begin ();
//       iterator != inherited::modules_.end ();
//       iterator++)
//    (*iterator)->next (NULL);
//}

//BitTorrent_Client_PeerStream::~BitTorrent_Client_PeerStream ()
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStream::~BitTorrent_Client_PeerStream"));

//  // *NOTE*: this implements an ordered shutdown on destruction
//  inherited::shutdown ();
//}

////bool
////BitTorrent_Client_PeerStream::load (Stream_ModuleList_t& list_out,
////                                    bool& deleteModules_out)
////{
////  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStream::load"));



////  return inherited::load (list_out,
////                          deleteModules_out);
////}

////////////////////////////////////////////

//BitTorrent_Client_TrackerStream::BitTorrent_Client_TrackerStream (const std::string& name_in)
// : inherited (name_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStream::BitTorrent_Client_TrackerStream"));

//  for (Stream_ModuleListIterator_t iterator = inherited::modules_.begin ();
//       iterator != inherited::modules_.end ();
//       iterator++)
//    (*iterator)->next (NULL);
//}

//BitTorrent_Client_TrackerStream::~BitTorrent_Client_TrackerStream ()
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStream::~BitTorrent_Client_TrackerStream"));

//  // *NOTE*: this implements an ordered shutdown on destruction
//  inherited::shutdown ();
//}

//bool
//BitTorrent_Client_TrackerStream::load (Stream_ModuleList_t& list_out,
//                                       bool& deleteModules_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStream::load"));



//  return inherited::load (list_out,
//                          deleteModules_out);
//}
