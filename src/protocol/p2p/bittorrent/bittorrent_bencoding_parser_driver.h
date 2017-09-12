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

#include "net_parser_base.h"

#include "bittorrent_bencoding_parser.h"
#include "bittorrent_bencoding_scanner.h"
#include "bittorrent_iparser.h"

template <typename SessionMessageType>
class BitTorrent_Bencoding_ParserDriver_T
 : public Net_CppParserBase_T<struct Common_ParserConfiguration,
                              BitTorrent_Bencoding_Scanner,
                              yy::BitTorrent_Bencoding_Parser,
                              BitTorrent_Bencoding_IParser,
                              std::string,
                              SessionMessageType>
{
 public:
  BitTorrent_Bencoding_ParserDriver_T (bool,  // debug scanning ?
                                       bool); // debug parsing ?
  virtual ~BitTorrent_Bencoding_ParserDriver_T ();

  // convenient types
  typedef Net_CppParserBase_T<struct Common_ParserConfiguration,
                              BitTorrent_Bencoding_Scanner,
                              yy::BitTorrent_Bencoding_Parser,
                              BitTorrent_Bencoding_IParser,
                              std::string,
                              SessionMessageType> PARSER_BASE_T;

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
//  virtual void error (const YYLTYPE&,      // location
  virtual void error (const yy::location&, // location
                      const std::string&); // message
  inline virtual Bencoding_Dictionary_t& current () { ACE_ASSERT (bencoding_); return *bencoding_; };
  inline virtual bool hasFinished () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  virtual void record (Bencoding_Dictionary_t*&); // data record
  inline virtual Bencoding_Dictionary_t& getDictionary () { return *dictionaries_.top (); };
  inline virtual std::string& getKey () { return *keys_.top (); };
  inline virtual Bencoding_List_t& getList () { return *lists_.top (); };
  inline virtual void popDictionary () { dictionaries_.pop (); };
  inline virtual void popKey () { keys_.pop (); };
  inline virtual void popList () { lists_.pop (); };
  // *IMPORTANT NOTE*: this also sets metaInfo_ (on first invocation)
  virtual void pushDictionary (Bencoding_Dictionary_t*); // dictionary
  inline virtual void pushKey (std::string* key_in) { keys_.push (key_in); };
  inline virtual void pushList (Bencoding_List_t* list_in) { lists_.push (list_in); };
//  inline virtual const Bencoding_Dictionary_t& get () const { return *dictionaries_.top (); };
//  inline virtual const Bencoding_List_t& get () const { return *lists_.top (); };

  Bencoding_Dictionary_t*             bencoding_;

 private:
  typedef Net_CppParserBase_T<struct Common_ParserConfiguration,
                              BitTorrent_Bencoding_Scanner,
                              yy::BitTorrent_Bencoding_Parser,
                              BitTorrent_Bencoding_IParser,
                              std::string,
                              SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_ParserDriver_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_ParserDriver_T (const BitTorrent_Bencoding_ParserDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_ParserDriver_T& operator= (const BitTorrent_Bencoding_ParserDriver_T&))

  std::stack<Bencoding_Dictionary_t*> dictionaries_;
  std::stack<std::string*>            keys_;
  std::stack<Bencoding_List_t*>       lists_;
};

// include template definition
#include "bittorrent_bencoding_parser_driver.inl"

#endif
