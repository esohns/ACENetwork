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

#include <list>
//#include <unordered_map>
#include <string>
#include <vector>

#include "ace/Assert.h"
#include "ace/Basic_Types.h"
#include "ace/config-macros.h"
#include "ace/OS.h"

#include "stream_common.h"

#include "stream_stat_statistic_handler.h"

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

// *NOTE*: the bencoding format is not really type-safe, so 'strict' languages
//         like C/C++ need to jump through a few hoops here
struct Bencoding_Element;
typedef std::vector<Bencoding_Element*> Bencoding_List_t;
typedef Bencoding_List_t::const_iterator Bencoding_ListIterator_t;
//typedef std::unordered_map<std::string*,
//                           Bencoding_Element*,
//                           string_p_hash,
//                           string_p_equal> Bencoding_Dictionary_t;
typedef std::vector<std::pair<std::string*,
                              Bencoding_Element*> > Bencoding_Dictionary_t;
typedef Bencoding_Dictionary_t::const_iterator Bencoding_DictionaryIterator_t;

struct Bencoding_Element
{
  inline Bencoding_Element ()
   : type (BENCODING_TYPE_INVALID)
  {};

  enum Bencoding_ElementType
  {
    BENCODING_TYPE_INTEGER = 0,
    BENCODING_TYPE_STRING,
    BENCODING_TYPE_LIST,
    BENCODING_TYPE_DICTIONARY,
    //////////////////////////////////////
    BENCODING_TYPE_MAX,
    BENCODING_TYPE_INVALID
  };

  enum Bencoding_ElementType type;
  union
  {
    ACE_INT64               integer;
    std::string*            string;
    Bencoding_List_t*       list;
    Bencoding_Dictionary_t* dictionary;
  };
};

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

struct BitTorrent_PeerHandShake
{
  inline BitTorrent_PeerHandShake ()
   : pstr (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_PEER_HANDSHAKE_PSTR_STRING))
   , reserved ()
   , info_hash ()
   , peer_id ()
  {
    ACE_OS::memset (reserved, 0, sizeof (reserved));
  };

  std::string pstr;
  ACE_UINT8   reserved[BITTORRENT_PEER_HANDSHAKE_RESERVED_SIZE];
  std::string info_hash;
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

struct BitTorrent_PeerRecord
{
  inline BitTorrent_PeerRecord ()
   : length (0)
   , type (BITTORRENT_MESSAGETYPE_INVALID)
  {};
 inline void operator+= (struct BitTorrent_PeerRecord rhs_in)
 { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); };

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

//////////////////////////////////////////

enum BitTorrent_Event
{
  BITTORRENT_EVENT_CANCELLED = 0,
  BITTORRENT_EVENT_COMPLETE,
  ////////////////////////////////////////
  BITTORRENT_EVENT_MAX,
  BITTORRENT_EVENT_INVALID,
};

//////////////////////////////////////////

typedef struct Stream_Statistic BitTorrent_Statistic_t;

typedef Stream_StatisticHandler_Reactor_T<BitTorrent_Statistic_t> BitTorrent_StatisticHandler_Reactor_t;
typedef Stream_StatisticHandler_Proactor_T<BitTorrent_Statistic_t> BitTorrent_StatisticHandler_Proactor_t;

#endif
