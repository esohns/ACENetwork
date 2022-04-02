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

#ifndef TEST_I_AVSTREAM_PARSER_DRIVER_H
#define TEST_I_AVSTREAM_PARSER_DRIVER_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "test_i_av_stream_common.h"
#include "test_i_av_stream_parser.h"

// forward declaration(s)
// tell flex of the lexer's prototype ...
#define YY_DECL                                      \
enum yytokentype                                     \
AVStream_Scanner_lex (YYSTYPE* yylval_param,         \
                      YYLTYPE* yylloc_param,         \
                      AVStream_ParserDriver* driver, \
                      yyscan_t yyscanner)
// ... and declare it for the parser's sake
YY_DECL;
struct yy_buffer_state;
typedef void* yyscan_t;
int AVStream_Scanner_get_debug (yyscan_t);

class AVStream_ParserDriver
{
  // allow access to internals (i.e. the current message)
  //friend class yy::AVStream_Parser;
  // allow access to internals (i.e. error reporting)
//   friend class AVStream_Scanner;

 public:
  AVStream_ParserDriver ();
  virtual ~AVStream_ParserDriver ();

  bool initialize (const struct Common_FlexBisonParserConfiguration&);

  // *WARNING*: in order to use the faster yy_scan_buffer(), the argument needs
  // to have been prepared for usage by flex:
  // --> buffers need two trailing '\0's BEYOND their data
  //    (at positions length() + 1, length() + 2)
  bool parse (ACE_Message_Block*); // data

  // invoked by the scanner ONLY !!!
  bool switchBuffer ();
  inline bool getDebugScanner () const { return (AVStream_Scanner_get_debug (scannerState_) != 0); }

  inline virtual void message_cb () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // error-handling
  //void error (const yy::location&, // location
  //            const std::string&); // message
  void error (const YYLTYPE&, // location
              const std::string&); // message
  void error (const std::string&); // message

  ACE_Message_Block*                          buffer_;
  struct acestream_av_stream_header           header_;
  ACE_UINT32                                  missing_;

 private:
  //ACE_UNIMPLEMENTED_FUNC (AVStream_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (AVStream_ParserDriver (const AVStream_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (AVStream_ParserDriver& operator= (const AVStream_ParserDriver&))

  // helper methods
  void waitBuffer ();
  bool scan_begin (bool); // use yy_scan_buffer : yy_scan_bytes
  void scan_end ();

  struct Common_FlexBisonParserConfiguration* configuration_;

  // scanner
  yyscan_t                                    scannerState_;
  struct yy_buffer_state*                     bufferState_;
};

#endif
