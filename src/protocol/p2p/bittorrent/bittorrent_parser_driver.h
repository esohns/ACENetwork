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

#ifndef BITTORRENT_PARSER_DRIVER_H
#define BITTORRENT_PARSER_DRIVER_H

#include <stack>
#include <string>

#include "ace/Global_Macros.h"

#include "location.hh"

#include "common_configuration.h"
#include "common_parser_base.h"

#include "bittorrent_parser.h"
#include "bittorrent_scanner.h"

class BitTorrent_ParserDriver
 : public Common_ParserBase_T<struct Common_FlexBisonParserConfiguration,
                              yy::BitTorrent_Parser,
                              BitTorrent_IParser_t,
                              BitTorrent_IParser_t>
{
  typedef Common_ParserBase_T<struct Common_FlexBisonParserConfiguration,
                              yy::BitTorrent_Parser,
                              BitTorrent_IParser_t,
                              BitTorrent_IParser_t> inherited;

 public:
  BitTorrent_ParserDriver ();
  inline virtual ~BitTorrent_ParserDriver () {}

  // convenient types
  typedef Common_ParserBase_T<struct Common_FlexBisonParserConfiguration,
                              yy::BitTorrent_Parser,
                              BitTorrent_IParser_t,
                              BitTorrent_IParser_t> PARSER_BASE_T;

  // implement (part of) BitTorrent_IParser
  using PARSER_BASE_T::initialize;
  using PARSER_BASE_T::buffer;
//  using PARSER_BASE_T::debug;
  using PARSER_BASE_T::isBlocking;
  using PARSER_BASE_T::offset;
  using PARSER_BASE_T::parse;
  using PARSER_BASE_T::switchBuffer;
  using PARSER_BASE_T::waitBuffer;

  // implement (part of) BitTorrent_IParser_T
  virtual void error (const yy::location&, // location
                      const std::string&); // message
//  virtual void error (const std::string&); // message
  inline virtual struct BitTorrent_PeerRecord& current () { static struct BitTorrent_PeerRecord dummy; ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) }

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (struct BitTorrent_PeerRecord*&) = 0; // data record
  virtual void handshake (struct BitTorrent_PeerHandShake*&) = 0; // handshake

  inline virtual void dump_state () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver (const BitTorrent_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver& operator= (const BitTorrent_ParserDriver&))

  // implement Common_ILexScanner_T
  inline virtual const Common_FlexScannerState& getR () const { static Common_FlexScannerState dummy; ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) }
  inline virtual const BitTorrent_IParser_t* const getP () const { return this; }
  inline virtual void setP (BitTorrent_IParser_t*) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void debug (yyscan_t state_in, bool toggle_in) { BitTorrent_Scanner_set_debug ((toggle_in ? 1 : 0), state_in); }
  inline virtual void reset () { BitTorrent_Scanner_set_lineno (1, inherited::scannerState_.context); BitTorrent_Scanner_set_column (1, inherited::scannerState_.context); }
  virtual bool initialize (yyscan_t&, BitTorrent_IParser_t*);
  inline virtual void finalize (yyscan_t& state_inout) { int result = BitTorrent_Scanner_lex_destroy (state_inout); ACE_UNUSED_ARG (result); state_inout = NULL; }
  virtual struct yy_buffer_state* create (yyscan_t, // state handle
                                          char*,    // buffer handle
                                          size_t);  // buffer size
  inline virtual void destroy (yyscan_t state_in, struct yy_buffer_state*& buffer_inout) { BitTorrent_Scanner__delete_buffer (buffer_inout, state_in); buffer_inout = NULL; }
  inline virtual bool lex () { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
};

#endif
