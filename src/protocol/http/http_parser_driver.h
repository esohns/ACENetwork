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

#include "location.hh"

#include "http_exports.h"
//#include "http_parser.h"

// forward declaration(s)
class ACE_Message_Block;
class HTTP_Record;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct YYLTYPE;

class HTTP_Export HTTP_ParserDriver
{
 public:
  HTTP_ParserDriver (bool,  // debug scanning ?
                     bool); // debug parsing ?
  virtual ~HTTP_ParserDriver ();

  // target data, needs to be set PRIOR to invoking parse() !
  void initialize (HTTP_Record&,  // target data
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
  bool moreData ();
  bool getDebugScanner () const;

  // error handling
  void error (const yy::location&, // location
              const std::string&); // message
  void error (const std::string&); // message
  void error (const YYLTYPE&,      // location
              const std::string&); // message

  // target
  HTTP_Record*       record_;

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver (const HTTP_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver& operator= (const HTTP_ParserDriver&))

  //// convenient typedefs
  //typedef HTTP_Message_T<AllocatorConfigurationType> MESSAGE_T;

  // helper methods
  bool scan_begin (bool); // use yy_scan_buffer : yy_scan_bytes
  void scan_end ();

  // context
  bool               trace_;

  //// parser
  //yy::HTTP_Parser    parser_;

  // scanner
  yyscan_t           scannerState_;
  YY_BUFFER_STATE    bufferState_;

  // *NOTE*: stores unscanned data, enabling transitions between continuations
  ACE_Message_Block* fragment_;
  bool               fragmentIsResized_;

  bool               isInitialized_;
};

#endif
