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

#include <ace/Global_Macros.h>

#include "bittorrent_iparser.h"
#include "bittorrent_bencoding_parser.h"

#ifndef yyFlexLexer
#define yyFlexLexer BitTorrent_Bencoding_Scanner_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

#include "location.hh"

class BitTorrent_Bencoding_Scanner
 : public BitTorrent_Bencoding_Scanner_FlexLexer
 , public BitTorrent_Bencoding_IScanner_t
{
 public:
  BitTorrent_Bencoding_Scanner ()
   : BitTorrent_Bencoding_Scanner_FlexLexer (NULL, NULL)
   , location_ ()
   , parser_ (NULL)
  {};
  virtual ~BitTorrent_Bencoding_Scanner () {};

  // implement BitTorrent_Bencoding_IScanner
  inline virtual void set (BitTorrent_Bencoding_IParser* parser_in) { parser_ = parser_in; };

  // override (part of) yyFlexLexer
//    virtual int yylex ();
  virtual yy::BitTorrent_Bencoding_Parser::token_type yylex (yy::BitTorrent_Bencoding_Parser::semantic_type*,
                                                             yy::location*,
                                                             BitTorrent_Bencoding_IParser*);
//    virtual int yywrap ();

  yy::location                  location_;

 private:
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_Scanner ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_Scanner (const BitTorrent_Bencoding_Scanner&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Bencoding_Scanner& operator= (const BitTorrent_Bencoding_Scanner&))

  BitTorrent_Bencoding_IParser* parser_;
};

#endif
