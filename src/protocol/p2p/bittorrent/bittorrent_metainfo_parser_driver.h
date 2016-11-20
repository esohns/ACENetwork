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

#ifndef BITTORRENT_METAINFO_PARSER_DRIVER_T_H
#define BITTORRENT_METAINFO_PARSER_DRIVER_T_H

#include <string>

#include <ace/Global_Macros.h>

#include "location.hh"

#include "net_parser_base.h"

#include "bittorrent_metainfo_parser.h"
#include "bittorrent_metainfo_scanner.h"

template <typename SessionMessageType>
class BitTorrent_MetaInfo_ParserDriver_T
 : public Net_ParserBase_T<BitTorrent_MetaInfoScanner,
                           yy::BitTorrent_MetaInfo_Parser,
                           BitTorrent_MetaInfo_IParser,
                           std::string,
                           SessionMessageType>
{
 public:
  BitTorrent_MetaInfo_ParserDriver_T (bool,  // debug scanning ?
                                      bool); // debug parsing ?
  virtual ~BitTorrent_MetaInfo_ParserDriver_T ();

  // convenient types
  typedef Net_ParserBase_T<BitTorrent_MetaInfoScanner,
                           yy::BitTorrent_MetaInfo_Parser,
                           BitTorrent_MetaInfo_IParser,
                           std::string,
                           SessionMessageType> PARSER_BASE_T;

  // implement (part of) BitTorrent_MetaInfo_IParser_T
  using PARSER_BASE_T::initialize;
  using PARSER_BASE_T::buffer;
  using PARSER_BASE_T::debugScanner;
  using PARSER_BASE_T::isBlocking;
  using PARSER_BASE_T::offset;
  using PARSER_BASE_T::parse;
  using PARSER_BASE_T::switchBuffer;
  using PARSER_BASE_T::wait;
//  virtual void error (const YYLTYPE&,      // location
  virtual void error (const yy::location&, // location
                      const std::string&); // message
//  virtual void error (const std::string&); // message
  inline virtual Bencoding_Dictionary_t& current () { return *currentDictionary_; };
  inline virtual bool hasFinished () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  virtual void record (Bencoding_Dictionary_t*&); // data record
  inline virtual void set (Bencoding_Dictionary_t* dictionary_in) { currentDictionary_ = dictionary_in; };
  inline virtual void set (Bencoding_List_t* list_in) { currentList_ = list_in; };
  inline virtual const Bencoding_List_t* const get () const { return currentList_; };

  virtual void dump_state () const;

  Bencoding_Dictionary_t* metaInfo_;

 private:
  typedef Net_ParserBase_T<BitTorrent_MetaInfoScanner,
                           yy::BitTorrent_MetaInfo_Parser,
                           BitTorrent_MetaInfo_IParser,
                           std::string,
                           SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_MetaInfo_ParserDriver_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_MetaInfo_ParserDriver_T (const BitTorrent_MetaInfo_ParserDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_MetaInfo_ParserDriver_T& operator= (const BitTorrent_MetaInfo_ParserDriver_T&))

  Bencoding_Dictionary_t* currentDictionary_;
  Bencoding_List_t*       currentList_;
};

// include template definition
#include "bittorrent_metainfo_parser_driver.inl"

#endif
