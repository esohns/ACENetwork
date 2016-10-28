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

#ifndef BITTORRENT_COMMON_H
#define BITTORRENT_COMMON_H

#include <string>
#include <vector>

#include <ace/Basic_Types.h>
#include <ace/OS.h>

enum BitTorrent_MessageType
{
  BITTORRENT_MESSAGETYPE_CHOKE = 0,
  BITTORRENT_MESSAGETYPE_UNCHOKE,
  BITTORRENT_MESSAGETYPE_INTERESTED,
  BITTORRENT_MESSAGETYPE_NOT_INTERESTED,
  BITTORRENT_MESSAGETYPE_HAVE,
  BITTORRENT_MESSAGETYPE_BITFIELD,
  BITTORRENT_MESSAGETYPE_REQUEST,
  BITTORRENT_MESSAGETYPE_PIECE,
  BITTORRENT_MESSAGETYPE_CANCEL,
  ////////////////////////////////////////
  BITTORRENT_MESSAGETYPE_MAX,
  BITTORRENT_MESSAGETYPE_INVALID,
};

struct BitTorrent_PeerHandshake
{
  inline BitTorrent_PeerHandshake ()
   : version ()
   , reserved ()
   , hash ()
   , peer_id ()
  {
    ACE_OS::memset (reserved, 0, sizeof (reserved));
  };

  std::string version;
  ACE_UINT8   reserved[8];
  std::string hash;
  std::string peer_id;
};

typedef std::vector<ACE_UINT8> BitTorrent_MessagePayload_Bitfield;
struct BitTorrent_MessagePayload_Cancel
{
  unsigned int index;
  unsigned int begin;
  unsigned int length;
};
typedef ACE_UINT32 BitTorrent_MessagePayload_Have; // piece index
struct BitTorrent_MessagePayload_Piece
{
  unsigned int index;
  unsigned int begin;
};
typedef ACE_UINT16 BitTorrent_MessagePayload_Port; // port number
struct BitTorrent_MessagePayload_Request
{
  unsigned int index;
  unsigned int begin;
  unsigned int length;
};

struct BitTorrent_Record
{
  inline BitTorrent_Record ()
   : length (0)
   , type (BITTORRENT_MESSAGETYPE_INVALID)
  {};

  unsigned int                               length;
  BitTorrent_MessageType                     type;

  // *TODO*: this belongs in the union; however, traditional C doesn't support
  //         class-type union members
  BitTorrent_MessagePayload_Bitfield         bitfield;
  union {
    struct BitTorrent_MessagePayload_Cancel  cancel;
    BitTorrent_MessagePayload_Have           have;
    struct BitTorrent_MessagePayload_Piece   piece;
    BitTorrent_MessagePayload_Port           port;
    struct BitTorrent_MessagePayload_Request request;
  };
};

#endif
