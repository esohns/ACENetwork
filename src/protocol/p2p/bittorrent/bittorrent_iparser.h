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

#include <string>

#include "common.h"
//#include "common_iget.h"

#include "net_iparser.h"

#include "bittorrent_common.h"

class BitTorrent_Bencoding_IParser
 : public Net_IRecordParser_T<struct Common_ParserConfiguration,
                              Bencoding_Dictionary_t>
 , public Net_IScanner_T<BitTorrent_Bencoding_IParser>
// , public Common_IGet_T<Bencoding_Dictionary_t>
// , public Common_IGet_T<Bencoding_List_t>
{
 public:
  virtual ~BitTorrent_Bencoding_IParser () {};

  using IPARSER_T::error;
  using Common_IScanner::error;

  virtual Bencoding_Dictionary_t& getDictionary () = 0;
  virtual std::string& getKey () = 0;
  virtual Bencoding_List_t& getList () = 0;
  virtual void popDictionary () = 0;
  virtual void popKey () = 0;
  virtual void popList () = 0;
  virtual void pushDictionary (Bencoding_Dictionary_t*) = 0; // dictionary handle
  virtual void pushKey (std::string*) = 0; // key handle
  virtual void pushList (Bencoding_List_t*) = 0; // list handle
};

template <typename RecordType>
class BitTorrent_IParser_T
 : public Net_IStreamParser_T<struct Common_ParserConfiguration,
                              RecordType>
 , public Net_IScanner_T<BitTorrent_IParser_T<RecordType> >
{
 public:
  // convenient types
  typedef Net_IStreamParser_T<struct Common_ParserConfiguration,
                              RecordType> IPARSER_T;
  typedef Net_IScanner_T<BitTorrent_IParser_T<RecordType> > ISCANNER_T;

  using IPARSER_T::error;
  using ISCANNER_T::error;

  virtual ~BitTorrent_IParser_T () {};

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void handshake (struct BitTorrent_PeerHandShake*&) = 0; // handshake
};

//////////////////////////////////////////

typedef Net_IScanner_T<BitTorrent_Bencoding_IParser> BitTorrent_Bencoding_IScanner_t;

typedef BitTorrent_IParser_T<struct BitTorrent_PeerRecord> BitTorrent_IParser_t;
typedef Net_IScanner_T<BitTorrent_IParser_t> BitTorrent_IScanner_t;

#endif
