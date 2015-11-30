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

#ifndef HTTP_PARSER_DRIVER_H
#define HTTP_PARSER_DRIVER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "location.hh"

#include "http_defines.h"
#include "http_exports.h"
//#include "http_scanner.h"

// forward declaration(s)
class ACE_Message_Queue_Base;
class HTTP_Record;
//class HTTP_Scanner;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct YYLTYPE;

class HTTP_Export HTTP_ParserDriver
{
  friend class HTTP_Scanner;

 public:
  HTTP_ParserDriver (bool,  // debug scanning ?
                     bool); // debug parsing ?
  virtual ~HTTP_ParserDriver ();

  // target data, needs to be set before invoking parse() !
  void initialize (HTTP_Record&,                            // target data
                   bool = HTTP_DEFAULT_LEX_TRACE,           // debug scanner ?
                   bool = HTTP_DEFAULT_YACC_TRACE,          // debug parser ?
                   ACE_Message_Queue_Base* = NULL,          // data buffer queue (yywrap)
                   bool = HTTP_DEFAULT_USE_YY_SCAN_BUFFER); // yy_scan_buffer() ? : yy_scan_bytes()

  bool parse (ACE_Message_Block*); // data

  // error handling
  void error (const yy::location&, // location
              const std::string&); // message
  void error (const std::string&); // message
  void error (const YYLTYPE&,      // location
              const std::string&); // message

  // *NOTE*: to be invoked by the scanner (ONLY !)
  bool switchBuffer ();
  bool getDebugScanner () const;
  void wait ();

  // *NOTE*: current (unscanned) data fragment
  bool                    finished_; // processed the whole entity ?
  ACE_Message_Block*      fragment_;
  unsigned int            offset_; // parsed entity bytes

  // target
  HTTP_Record*            record_;

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver (const HTTP_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver& operator= (const HTTP_ParserDriver&))

  //// convenient typedefs
  //typedef HTTP_Message_T<AllocatorConfigurationType> MESSAGE_T;

  // helper methods
  bool scan_begin ();
  void scan_end ();

  // context
  bool                    trace_;

  //// parser
  //yy::HTTP_Parser    parser_;

  // scanner
  yyscan_t                scannerState_;
  YY_BUFFER_STATE         bufferState_;
  ACE_Message_Queue_Base* messageQueue_;
  bool                    useYYScanBuffer_;

  bool                    initialized_;
};

#endif
