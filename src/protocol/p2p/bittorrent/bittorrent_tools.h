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

#ifndef BITTORRENT_TOOLS_H
#define BITTORRENT_TOOLS_H

#include <string>

#include <ace/Global_Macros.h>

#include "common.h"

#include "bittorrent_common.h"
#include "bittorrent_exports.h"

class BitTorrent_Export BitTorrent_Tools
{
 public:
  inline virtual ~BitTorrent_Tools () {};

  // debug info
  static std::string Dictionary2String (const Bencoding_Dictionary_t&);
  static std::string List2String (const Bencoding_List_t&);
  static std::string MetaInfo2String (const Bencoding_Dictionary_t&);

  static std::string HandShake2String (const struct BitTorrent_PeerHandShake&);
  static std::string Record2String (const struct BitTorrent_PeerRecord&);

  static std::string Type2String (enum BitTorrent_MessageType&);

  // *NOTE*: this returns the 'info_hash' tracker HTTP request value
  static std::string MetaInfo2InfoHash (const Bencoding_Dictionary_t&); // metainfo
  static unsigned int MetaInfo2Length (const Bencoding_Dictionary_t&); // metainfo

  // *IMPORTANT NOTE*: caller needs to free the return value (second argument)
  static bool parseMetaInfoFile (const struct Common_ParserConfiguration&, // parser configuration
                                 const std::string&,                       // metainfo (aka .bittorrent) file
                                 Bencoding_Dictionary_t*&);                // return value: metainfo
  static void free (Bencoding_Dictionary_t*&); // metainfo

  // *NOTE*: this follows the Azureus style (encode client and version
  //         information)
  //         (see also: https://wiki.theory.org/BitTorrentSpecification#peer_id)
  static std::string generatePeerId ();
  // *NOTE*: the key length is not specified, so this generates 20 bytes, which
  //         makes it somewhat consistent with the other request values
  static std::string generateKey ();

  // *NOTE*: see also: https://wiki.theory.org/BitTorrentSpecification#Overview
  static bool torrentSupportsScrape (const std::string&); // announce URI
  static std::string AnnounceURL2ScrapeURL (const std::string&); // announce URI

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Tools ())
  //ACE_UNIMPLEMENTED_FUNC (~BitTorrent_Tools ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Tools (const BitTorrent_Tools&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Tools& operator= (const BitTorrent_Tools&))

  // helper methods
  static std::string bencode (const Bencoding_Dictionary_t&);
  static std::string bencode (const Bencoding_List_t&);
  static void free (Bencoding_List_t*&); // bencoded list
};

#endif
