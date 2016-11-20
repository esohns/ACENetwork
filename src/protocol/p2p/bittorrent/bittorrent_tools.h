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

#include "net_defines.h"

#include "bittorrent_common.h"
#include "bittorrent_exports.h"

class BitTorrent_Export BitTorrent_Tools
{
 public:
  inline virtual ~BitTorrent_Tools () {};

  // debug info
  static std::string Dictionary2String (const Bencoding_Dictionary_t&);
  static std::string List2String (const Bencoding_List_t&);
  inline static std::string MetaInfo2String (const Bencoding_Dictionary_t& metaInfo_in) { return BitTorrent_Tools::Dictionary2String (metaInfo_in); };

  static std::string Handshake2String (const struct BitTorrent_PeerHandshake&);
  static std::string Record2String (const struct BitTorrent_Record&);

  static std::string Type2String (enum BitTorrent_MessageType&);

//  static bool parseURL (const std::string&, // URL
//                        ACE_INET_Addr&,     // return value: host address
//                        std::string&);      // return value: URI
  // *IMPORTANT NOTE*: caller needs to free the return value (second argument)
  static bool parseMetaInfoFile (const std::string&,       // metainfo (aka .bittorrent) file
                                 Bencoding_Dictionary_t*&, // return value: metainfo
                                 bool = NET_PROTOCOL_DEFAULT_LEX_TRACE,
                                 bool = NET_PROTOCOL_DEFAULT_YACC_TRACE);

  static void free (Bencoding_Dictionary_t*&); // metainfo

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Tools ())
  //ACE_UNIMPLEMENTED_FUNC (~BitTorrent_Tools ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Tools (const BitTorrent_Tools&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Tools& operator= (const BitTorrent_Tools&))

  // helper methods
  static void free (Bencoding_List_t*&); // bencoded list
};

#endif
