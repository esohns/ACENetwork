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

#ifndef BITTORRENT_BENCODING_PARSER_DRIVER_T_H
#define BITTORRENT_BENCODING_PARSER_DRIVER_T_H

#include <stack>
#include <string>

#include "ace/Global_Macros.h"

#include "location.hh"

#include "common_parser_common.h"
#include "common_parser_cpp_base.h"

#include "bittorrent_bencoding_parser.h"
#include "bittorrent_bencoding_scanner.h"
#include "bittorrent_iparser.h"

class BitTorrent_Bencoding_ParserDriver
 : public Common_CppParserBase_T<struct Common_FlexBisonParserConfiguration,
                                 BitTorrent_Bencoding_Scanner,
                                 struct Common_FlexScannerState,
                                 yy::BitTorrent_Bencoding_Parser,
                                 BitTorrent_Bencoding_IParser,
                                 BitTorrent_Bencoding_IParser>
{
  typedef Common_CppParserBase_T<struct Common_FlexBisonParserConfiguration,
                                 BitTorrent_Bencoding_Scanner,
                                 struct Common_FlexScannerState,
                                 yy::BitTorrent_Bencoding_Parser,
                                 BitTorrent_Bencoding_IParser,
                                 BitTorrent_Bencoding_IParser> inherited;

 public:
  BitTorrent_Bencoding_ParserDriver (bool,  // debug scanning ?
                                     bool); // debug parsing ?
  inline virtual ~BitTorrent_Bencoding_ParserDriver () {}

  // convenient types
  typedef Common_CppParserBase_T<struct Common_FlexBisonParserConfiguration,
                                 BitTorrent_Bencoding_Scanner,
                                 struct Common_FlexScannerState,
                                 yy::BitTorrent_Bencoding_Parser,
                                 BitTorrent_Bencoding_IParser,
                                 BitTorrent_Bencoding_IParser> PARSER_BASE_T;

  // implement (part of) BitTorrent_Bencoding_IParser
  using PARSER_BASE_T::buffer;
//  using PARSER_BASE_T::debug;
  using PARSER_BASE_T::isBlocking;
  using PARSER_BASE_T::offset;
  using PARSER_BASE_T::switchBuffer;
  using PARSER_BASE_T::waitBuffer;
//  virtual void error (const std::string&); // message

  using PARSER_BASE_T::initialize;
  virtual void dump_state () const;
  using PARSER_BASE_T::parse;
  virtual void error (const struct YYLTYPE&, // location
                      const std::string&);   // message
  virtual void error (const yy::location&, // location
                      const std::string&); // message
  inline virtual struct Bencoding_Element& current () { ACE_ASSERT (bencoding_); return *bencoding_; }
  inline virtual bool hasFinished () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  virtual void record (struct Bencoding_Element*&); // data record
  virtual void pushInteger (ACE_INT64);
  virtual void pushString (std::string*);
  virtual void pushList (Bencoding_List_t*); // list
  inline virtual void pushKey (std::string* key_in) { ACE_ASSERT (!key_); key_ = key_in; }
  virtual void pushDictionary (Bencoding_Dictionary_t*); // dictionary
  inline virtual void pop () { current_.pop (); }

  struct Bencoding_Element*             bencoding_;

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_ParserDriver (const BitTorrent_Bencoding_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_ParserDriver& operator= (const BitTorrent_Bencoding_ParserDriver&))

  virtual void push (struct Bencoding_Element*); // element

  bool                                  isFirst_; // first element ?
  std::stack<struct Bencoding_Element*> current_;
  std::string*                          key_;
};

#endif
