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

#ifndef IRC_PARSER_DRIVER_H
#define IRC_PARSER_DRIVER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "location.hh"

#include "irc_exports.h"
#include "irc_parser.h"

// forward declaration(s)
struct yy_buffer_state;
typedef void* yyscan_t;
int IRC_Scanner_get_debug (yyscan_t);
class IRC_Record;

class IRC_Export IRC_ParserDriver
{
  // allow access to internals (i.e. the current message)
  friend class yy::IRC_Parser;
  // allow access to internals (i.e. error reporting)
//   friend class IRC_Scanner;

 public:
  IRC_ParserDriver (bool,  // debug scanning ?
                    bool); // debug parsing ?
  virtual ~IRC_ParserDriver ();

  // target data, needs to be set PRIOR to invoking parse() !
  void initialize (IRC_Record&,   // target data
                   bool = false,  // debug scanner ?
                   bool = false); // debug parser ?
  // *WARNING*: in order to use the faster yy_scan_buffer(), the argument needs
  // to have been prepared for usage by flex:
  // --> buffers need two trailing '\0's BEYOND their data
  //    (at positions length() + 1, length() + 2)
  // --> indicated by the second argument
  bool parse (ACE_Message_Block*, // data
              bool = false);      // is data prepared for yy_scan_buffer ?

  // invoked by the scanner ONLY !!!
  bool switchBuffer ();
  inline bool moreData () { return (fragment_->cont () != NULL); };
  inline bool getDebugScanner () const { return (IRC_Scanner_get_debug (scannerState_) != 0); };

  // error-handling
  void error (const yy::location&, // location
              const std::string&); // message
  void error (const std::string&); // message

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (IRC_ParserDriver (const IRC_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (IRC_ParserDriver& operator= (const IRC_ParserDriver&))

//   // clear current message
//   void reset();

  // helper methods
  bool scan_begin (bool); // use yy_scan_buffer : yy_scan_bytes
  void scan_end ();

  // context
  bool                    trace_;
  unsigned int            numberOfMessages_;

  // scanner
  yyscan_t                scannerState_;
  struct yy_buffer_state* bufferState_;

  // *NOTE*: stores unscanned data, enabling transitions between continuations
  ACE_Message_Block*      fragment_;
  bool                    fragmentIsResized_;

  // parser
  yy::IRC_Parser          parser_;

  // target
  IRC_Record*             record_;
  bool                    isInitialized_;
};

#endif
