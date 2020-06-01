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

#ifndef TEST_I_M3U_PARSER_DRIVER_H
#define TEST_I_M3U_PARSER_DRIVER_H

#include <stack>
#include <string>

#include "ace/Global_Macros.h"

#include "location.hh"

#include "common_parser_base.h"
#include "common_parser_common.h"

#include "m3u_iparser.h"
#include "m3u_parser.h"
#include "m3u_scanner.h"

// forward declarations
typedef void* yyscan_t;
struct yy_buffer_state;

class M3U_ParserDriver
 : public Common_ParserBase_T<struct Common_ParserConfiguration,
                              yy::parser,
                              M3U_IParser_t,
                              std::string>
{
  typedef Common_ParserBase_T<struct Common_ParserConfiguration,
                              yy::parser,
                              M3U_IParser_t,
                              std::string> inherited;

 public:
  M3U_ParserDriver ();
  inline virtual ~M3U_ParserDriver () {}

  // convenient types
  typedef Common_ParserBase_T<struct Common_ParserConfiguration,
                              yy::parser,
                              M3U_IParser_t,
                              std::string> PARSER_BASE_T;

  // implement (part of) Bencoding_IParser
  using PARSER_BASE_T::initialize;
  using PARSER_BASE_T::buffer;
//  using PARSER_BASE_T::debug;
  using PARSER_BASE_T::isBlocking;
  using PARSER_BASE_T::offset;
  using PARSER_BASE_T::parse;
  using PARSER_BASE_T::switchBuffer;
  using PARSER_BASE_T::waitBuffer;

  // implement (part of) M3U_IParser_t
  inline virtual M3U_Playlist_t& current () { ACE_ASSERT (playlist_); return *playlist_; }
  inline virtual struct M3U_Element& current_2 () { ACE_ASSERT (element_); return *element_; }
  inline virtual bool hasFinished () const { return inherited::finished_; }

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (M3U_Playlist_t*&); // data record

  inline virtual void dump_state () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (M3U_ParserDriver (const M3U_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (M3U_ParserDriver& operator= (const M3U_ParserDriver&))

  inline virtual void setP (M3U_Playlist_t* playlist_in) { ACE_ASSERT (!playlist_); playlist_ = playlist_in; }
  inline virtual void setP_2 (struct M3U_Element* element_in) { ACE_ASSERT (!element_); element_ = element_in; }

  // implement Common_ILexScanner_T
  inline virtual const M3U_IParser_t* const getP_2 () const { return this; }
  inline virtual void debug (yyscan_t state_in, bool toggle_in) { M3U_set_debug ((toggle_in ? 1 : 0), state_in); }
  inline virtual bool initialize (yyscan_t& state_inout, struct Common_ScannerState* scannerState_in) { ACE_UNUSED_ARG (scannerState_in); return (M3U_lex_init_extra (this, &state_inout) == 0); }
  inline virtual void finalize (yyscan_t& state_inout) { int result = M3U_lex_destroy (state_inout); ACE_UNUSED_ARG (result); state_inout = NULL; }
  virtual struct yy_buffer_state* create (yyscan_t, // state handle
                                          char*,    // buffer handle
                                          size_t);  // buffer size
  inline virtual void destroy (yyscan_t state_in, struct yy_buffer_state*& buffer_inout) { M3U__delete_buffer (buffer_inout, state_in); buffer_inout = NULL; }
//  inline virtual bool lex (yyscan_t state_in, yy::location* location_in) { ACE_ASSERT (false); return Bencoding_lex (NULL, location_in, this, state_in); }

  M3U_Playlist_t*     playlist_;
  struct M3U_Element* element_;
};

#endif
