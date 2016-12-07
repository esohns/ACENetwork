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

#ifndef BITTORRENT_PARSER_DRIVER_T_H
#define BITTORRENT_PARSER_DRIVER_T_H

#include <stack>
#include <string>

#include <ace/Global_Macros.h>

#include "location.hh"

#include "net_parser_base.h"

#include "bittorrent_parser.h"
//#include "bittorrent_scanner.h"

template <typename MessageType,
          typename SessionMessageType>
class BitTorrent_ParserDriver_T
 : public Net_ParserBase_T<BitTorrent_Scanner,
                           yy::BitTorrent_Parser,
                           BitTorrent_IParser_t,
                           std::string,
                           SessionMessageType>
{
 public:
  BitTorrent_ParserDriver_T (bool,  // debug scanning ?
                             bool); // debug parsing ?
  virtual ~BitTorrent_ParserDriver_T ();

  // convenient types
  typedef Net_ParserBase_T<BitTorrent_Scanner,
                           yy::BitTorrent_Parser,
                           BitTorrent_IParser_t,
                           std::string,
                           SessionMessageType> PARSER_BASE_T;

  // implement (part of) BitTorrent_IParser
  using PARSER_BASE_T::initialize;
  using PARSER_BASE_T::buffer;
  using PARSER_BASE_T::debugScanner;
  using PARSER_BASE_T::isBlocking;
  using PARSER_BASE_T::offset;
  using PARSER_BASE_T::parse;
  using PARSER_BASE_T::switchBuffer;
  using PARSER_BASE_T::wait;

  // implement (part of) BitTorrent_IParser_T
  virtual void error (const yy::location&, // location
                      const std::string&); // message
//  virtual void error (const std::string&); // message
  inline virtual struct BitTorrent_Record& current () { ACE_ASSERT (record_); return *record_; };

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (struct BitTorrent_Record*&); // data record
  virtual void handshake (struct BitTorrent_PeerHandshake*&); // handshake

  virtual void dump_state () const;

  struct BitTorrent_PeerHandshake* handShake_;
  struct BitTorrent_Record*        record_;

 private:
  typedef Net_ParserBase_T<BitTorrent_Scanner,
                           yy::BitTorrent_Parser,
                           BitTorrent_IParser_t,
                           std::string,
                           SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver_T (const BitTorrent_ParserDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver_T& operator= (const BitTorrent_ParserDriver_T&))
};

// include template definition
#include "bittorrent_parser_driver.inl"

#endif
