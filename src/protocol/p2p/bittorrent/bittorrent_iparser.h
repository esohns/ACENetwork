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

#ifndef BITTORRENT_IPARSER_T_H
#define BITTORRENT_IPARSER_T_H

#include "net_iparser.h"

#include "bittorrent_common.h"
#include "bittorrent_stream_common.h"

template <typename MessageType>
class BitTorrent_IParser_T
 : public Net_IStreamParser_T<MessageType>
{
 public:
  inline virtual ~BitTorrent_IParser_T () {};

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void handshake (struct BitTorrent_PeerHandshake*&) = 0; // handshake
};

//////////////////////////////////////////

typedef Net_IRecordParser_T<struct BitTorrent_MetaInfo> BitTorrent_MetaInfo_IParser_t;
typedef Net_IScanner_T<BitTorrent_MetaInfo_IParser_t> BitTorrent_MetaInfo_IScanner_t;

typedef BitTorrent_IParser_T<struct BitTorrent_Record> BitTorrent_IParser_t;

#endif
