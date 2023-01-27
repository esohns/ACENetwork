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

//#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

#include "ace/config-lite.h"
#include "ace/Assert.h"
#include "ace/Basic_Types.h"
#include "ace/Message_Block.h"
#include "ace/OS.h"

//#include "common_statistic_handler.h"

#include "bittorrent_defines.h"

//struct string_p_hash
// : public std::hash<std::string>
//{
//  size_t operator () (const std::string* string_in) const
//  { ACE_ASSERT (string_in);
//    return std::hash<std::string>::operator() (*string_in);
//  }
//};
//struct string_p_equal
//{
//  bool operator () (const std::string* lhs, const std::string* rhs) const
//  { ACE_ASSERT (lhs); ACE_ASSERT (rhs);
//    return (*lhs == *rhs);
//  }
//};

//////////////////////////////////////////

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

struct BitTorrent_Piece_Chunk
{
  BitTorrent_Piece_Chunk ()
   : data (NULL)
   , offset (0)
  {};

  ACE_Message_Block* data;
  unsigned int       offset;
};
typedef std::vector<struct BitTorrent_Piece_Chunk> BitTorrent_PieceChunks_t;
typedef BitTorrent_PieceChunks_t::const_iterator BitTorrent_PieceChunksConstIterator_t;
typedef BitTorrent_PieceChunks_t::iterator BitTorrent_PieceChunksIterator_t;
struct bittorrent_piece_chunks_less
{
  bool operator() (const struct BitTorrent_Piece_Chunk& lhs_in, const struct BitTorrent_Piece_Chunk& rhs_in) const
  { // *NOTE*: sort so that given equal offsets, the chunks with the most data
    //         are first in line
    if (lhs_in.offset != rhs_in.offset)
      return (lhs_in.offset < rhs_in.offset);
    return (lhs_in.data->total_length () > rhs_in.data->total_length ());
  };
};

struct BitTorrent_Piece
{
  BitTorrent_Piece ()
   : chunks ()
   , filename ()
   , hash ()
   , length (0)
  {};

  BitTorrent_PieceChunks_t chunks;
  std::string              filename;
  ACE_UINT8                hash[BITTORRENT_PRT_INFO_PIECE_HASH_SIZE]; // SHA1
  unsigned int             length;
};
typedef std::vector<struct BitTorrent_Piece> BitTorrent_Pieces_t;
typedef BitTorrent_Pieces_t::const_iterator BitTorrent_PiecesConstIterator_t;
typedef BitTorrent_Pieces_t::iterator BitTorrent_PiecesIterator_t;

struct BitTorrent_PeerHandShake
{
  BitTorrent_PeerHandShake ()
   : pstr (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_PEER_HANDSHAKE_PSTR_STRING))
   , reserved ()
   , info_hash ()
   , peer_id ()
  {
    ACE_OS::memset (reserved, 0, sizeof (ACE_UINT8[BITTORRENT_PEER_HANDSHAKE_RESERVED_SIZE]));
  }

  std::string pstr;
  ACE_UINT8   reserved[BITTORRENT_PEER_HANDSHAKE_RESERVED_SIZE];
  std::string info_hash;
  std::string peer_id;
};

typedef std::vector<ACE_UINT8> BitTorrent_MessagePayload_Bitfield;
typedef BitTorrent_MessagePayload_Bitfield::const_iterator BitTorrent_MessagePayload_BitfieldIterator;
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

struct BitTorrent_PeerRecord
{
  BitTorrent_PeerRecord ()
   : length (0)
   , type (BITTORRENT_MESSAGETYPE_INVALID)
   , bitfield ()
  {}
  inline void operator+= (struct BitTorrent_PeerRecord rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }

  unsigned int                               length;
  enum BitTorrent_MessageType                type;

  // *TODO*: this belongs in the union; however, traditional C doesn't support
  //         class-type union members
  BitTorrent_MessagePayload_Bitfield         bitfield;
  union
  {
    struct BitTorrent_MessagePayload_Cancel  cancel;
    BitTorrent_MessagePayload_Have           have;
    struct BitTorrent_MessagePayload_Piece   piece;
    BitTorrent_MessagePayload_Port           port;
    struct BitTorrent_MessagePayload_Request request;
  };
};

struct net_bittorrent_piece_index_remove_predicate
 : public std::binary_function<unsigned int,
                               std::vector<unsigned int>,
                               bool>
{
  inline bool operator() (const unsigned int index_in, const std::vector<unsigned int>& array_in) const { return std::find (array_in.begin (), array_in.end (), index_in) != array_in.end (); }
};

//////////////////////////////////////////

// *NOTE*: session --> controller notification
enum BitTorrent_Event
{
  BITTORRENT_EVENT_CANCELLED = ACE_Message_Block::MB_USER,
  BITTORRENT_EVENT_COMPLETE,
  BITTORRENT_EVENT_NO_MORE_PEERS,
  BITTORRENT_EVENT_TRACKER_REDIRECTED,
  ////////////////////////////////////////
  BITTORRENT_EVENT_MAX,
  BITTORRENT_EVENT_INVALID,
};

//////////////////////////////////////////

//typedef struct Net_StreamStatistic BitTorrent_Statistic_t;
//typedef Common_StatisticHandler_T<BitTorrent_Statistic_t> BitTorrent_StatisticHandler_t;

#endif
