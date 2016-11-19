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
#include <string>
#include <vector>

#include <ace/Assert.h>
#include <ace/Basic_Types.h>
#include <ace/config-macros.h>
#include <ace/OS.h>

typedef std::vector<std::string> BitTorrent_MetaInfo_List_t;
typedef BitTorrent_MetaInfo_List_t::const_iterator BitTorrent_MetaInfo_ListIterator_t;

struct BitTorrent_MetaInfo_InfoDictionary_Common
{
  inline BitTorrent_MetaInfo_InfoDictionary_Common ()
   : pieceLength (0)
   , pieces ()
   , _private (-1)
  {};

  unsigned int pieceLength;
  std::string  pieces;
  int          _private;
};
struct BitTorrent_MetaInfo_InfoDictionary_SingleFile
 : BitTorrent_MetaInfo_InfoDictionary_Common
{
//  inline BitTorrent_MetaInfo_InfoDictionary_SingleFile ()
//   : BitTorrent_MetaInfo_InfoDictionary_Common ()
//   , name ()
//   , length (0)
//   , md5sum ()
//  {};

  std::string  name;
  unsigned int length;
  std::string  md5sum;
};
struct BitTorrent_MetaInfo_InfoDictionary_MultipleFile_File
{
//  inline BitTorrent_MetaInfo_InfoDictionary_MultipleFile_File ()
//   : length (0)
//   , md5sum ()
//   , path ()
//  {};

  unsigned int length;
  std::string  md5sum;
  std::string  path;
};
typedef std::vector<struct BitTorrent_MetaInfo_InfoDictionary_MultipleFile_File> BitTorrent_MetaInfo_InfoDictionary_MultipleFile_Files_t;
typedef BitTorrent_MetaInfo_InfoDictionary_MultipleFile_Files_t::const_iterator BitTorrent_MetaInfo_InfoDictionary_MultipleFile_FilesIterator_t;
struct BitTorrent_MetaInfo_InfoDictionary_MultipleFile
 : BitTorrent_MetaInfo_InfoDictionary_Common
{
//  inline BitTorrent_MetaInfo_InfoDictionary_MultipleFile ()
//   : BitTorrent_MetaInfo_InfoDictionary_Common ()
//   , name ()
//   , files ()
//  {};

  std::string                                             name;
  BitTorrent_MetaInfo_InfoDictionary_MultipleFile_Files_t files;
};
union BitTorrent_MetaInfo_InfoDictionary
{
  struct BitTorrent_MetaInfo_InfoDictionary_SingleFile*   single_file;
  struct BitTorrent_MetaInfo_InfoDictionary_MultipleFile* multiple_file;
};
typedef std::list<std::string> BitTorrent_MetaInfo_AnnounceList_t;
typedef BitTorrent_MetaInfo_AnnounceList_t::const_iterator BitTorrent_MetaInfo_AnnounceListIterator_t;
typedef std::list<BitTorrent_MetaInfo_AnnounceList_t> BitTorrent_MetaInfo_AnnounceLists_t;
typedef BitTorrent_MetaInfo_AnnounceLists_t::const_iterator BitTorrent_MetaInfo_AnnounceListsIterator_t;
struct BitTorrent_MetaInfo
{
  inline BitTorrent_MetaInfo ()
   : singleFileMode (true)
   , info ()
   , announce ()
   , announceList ()
   , creationDate (0)
   , comment ()
   , createdBy ()
   , encoding ()
  {};

  bool                                     singleFileMode;

  union BitTorrent_MetaInfo_InfoDictionary info;
  std::string                              announce;
  BitTorrent_MetaInfo_AnnounceLists_t      announceList;
  time_t                                   creationDate;
  std::string                              comment;
  std::string                              createdBy;
  std::string                              encoding;
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
 inline void operator+= (struct BitTorrent_Record rhs_in)
 { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); };

  unsigned int                               length;
  enum BitTorrent_MessageType                type;

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
