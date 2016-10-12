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

#ifndef DHCP_PARSER_DRIVER_H
#define DHCP_PARSER_DRIVER_H

#include <string>

#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>

#include "location.hh"

#include "dhcp_defines.h"
#include "dhcp_exports.h"
//#include "dhcp_scanner.h"

// forward declaration(s)
class ACE_Message_Queue_Base;
struct DHCP_Record;
//class DHCP_Scanner;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct YYLTYPE;

class NET_PROTOCOL_DHCP_Export DHCP_ParserDriver
{
  friend class DHCP_Scanner;

 public:
  DHCP_ParserDriver (bool,  // debug scanning ?
                     bool); // debug parsing ?
  virtual ~DHCP_ParserDriver ();

  // target data, needs to be set before invoking parse() !
  void initialize (DHCP_Record&,                            // target data
                   bool = DHCP_DEFAULT_LEX_TRACE,           // debug scanner ?
                   bool = DHCP_DEFAULT_YACC_TRACE,          // debug parser ?
                   ACE_Message_Queue_Base* = NULL,          // data buffer queue (yywrap)
                   bool = DHCP_DEFAULT_USE_YY_SCAN_BUFFER); // yy_scan_buffer() ? : yy_scan_bytes()

  bool parse (ACE_Message_Block*); // data

  // error handling
  void error (const yy::location&, // location
              const std::string&); // message
  void error (const std::string&); // message
  void error (const YYLTYPE&,      // location
              const std::string&); // message

  // *NOTE*: to be invoked by the scanner (ONLY !)
  bool getDebugScanner () const;

  // *NOTE*: current (unscanned) data fragment
  ACE_Message_Block*      fragment_;
  unsigned int            offset_; // parsed entity bytes

  // target
  DHCP_Record*            record_;

 private:
  ACE_UNIMPLEMENTED_FUNC (DHCP_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_ParserDriver (const DHCP_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_ParserDriver& operator= (const DHCP_ParserDriver&))

  //// convenient typedefs
  //typedef DHCP_Message_T<AllocatorConfigurationType> MESSAGE_T;

  // helper methods
  bool scan_begin ();
  void scan_end ();

  // context
  bool                    trace_;

  //// parser
  //yy::DHCP_Parser    parser_;

  // scanner
  yyscan_t                scannerState_;
  YY_BUFFER_STATE         bufferState_;
  ACE_Message_Queue_Base* messageQueue_;
  bool                    useYYScanBuffer_;

  bool                    initialized_;
};

#endif
