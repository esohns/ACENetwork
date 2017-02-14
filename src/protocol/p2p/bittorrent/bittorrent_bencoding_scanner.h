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

#ifndef BITTORRENT_BENCODING_SCANNER_H
#define BITTORRENT_BENCODING_SCANNER_H

// Flex expects the signature of yylex to be defined in the macro YY_DECL, and
// the C++ parser expects it to be declared. We can factor both as follows.
#ifndef YY_DECL
#define YY_DECL                                                                              \
yy::BitTorrent_Bencoding_Parser::token_type                                                  \
BitTorrent_Bencoding_Scanner::yylex (yy::BitTorrent_Bencoding_Parser::semantic_type* yylval, \
                                     yy::location* yylloc)
#endif

#include <ace/Global_Macros.h>

#ifndef yyFlexLexer
#define yyFlexLexer BitTorrent_Bencoding_Scanner_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

#include "location.hh"

#include "bittorrent_exports.h"
#include "bittorrent_iparser.h"
#include "bittorrent_bencoding_parser.h"

class BitTorrent_Export BitTorrent_Bencoding_Scanner
 : public BitTorrent_Bencoding_Scanner_FlexLexer
 , public BitTorrent_Bencoding_IScanner_t
{
 public:
  BitTorrent_Bencoding_Scanner ()
   : BitTorrent_Bencoding_Scanner_FlexLexer (NULL, NULL)
   , parser_ (NULL)
  {};
  virtual ~BitTorrent_Bencoding_Scanner () {};

  // implement Net_IScanner_T
  inline virtual void set (BitTorrent_Bencoding_IParser* parser_in) { parser_ = parser_in; };
  inline virtual ACE_Message_Block* buffer () { ACE_ASSERT (parser_); return parser_->buffer (); };
  inline virtual bool debugScanner () const { ACE_ASSERT (parser_); return parser_->debugScanner (); };
  inline virtual bool isBlocking () const { ACE_ASSERT (parser_); return parser_->isBlocking (); };

  inline virtual void error (const std::string& errorString_in) { ACE_ASSERT (parser_); parser_->error (yy::location (), errorString_in); };

  // *NOTE*: to be invoked by the scanner (ONLY !)
  inline virtual void offset (unsigned int offset_in) { ACE_ASSERT (parser_); parser_->offset (offset_in); };
  inline virtual unsigned int offset () const { ACE_ASSERT (parser_); return parser_->offset (); };

  // *IMPORTANT NOTE*: when the parser detects a frame end, it inserts a new
  //                   buffer to the continuation and passes 'true'
  //                   --> separate the current frame from the next
  inline virtual bool switchBuffer (bool unlink_in = false) { ACE_ASSERT (parser_); return parser_->switchBuffer (unlink_in); };
  inline virtual void waitBuffer () { ACE_ASSERT (parser_); return parser_->waitBuffer (); };

  // *NOTE*: this is the C interface (not needed by C++ scanners)
  inline virtual void debug (yyscan_t,
                             bool) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  inline virtual bool initialize (yyscan_t&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  inline virtual void finalize (yyscan_t&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  inline virtual struct yy_buffer_state* create (yyscan_t,
                                                 char*,
                                                 size_t) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) };
  inline virtual void destroy (yyscan_t,
                               struct yy_buffer_state*&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // override (part of) yyFlexLexer
  virtual yy::BitTorrent_Bencoding_Parser::token_type yylex (yy::BitTorrent_Bencoding_Parser::semantic_type*,
                                                             yy::location*);
//    virtual int yywrap ();

 private:
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_Scanner ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_Scanner (const BitTorrent_Bencoding_Scanner&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_Scanner& operator= (const BitTorrent_Bencoding_Scanner&))

  BitTorrent_Bencoding_IParser* parser_;
};

#endif
