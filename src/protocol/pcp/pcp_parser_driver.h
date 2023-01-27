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

#ifndef PCP_PARSER_DRIVER_H
#define PCP_PARSER_DRIVER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "location.hh"

#include "common_parser_common.h"

#include "net_defines.h"

#include "pcp_defines.h"
//#include "pcp_exports.h"
//#include "pcp_scanner.h"

// forward declaration(s)
class ACE_Message_Queue_Base;
struct PCP_Record;
//class PCP_Scanner;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct YYLTYPE;

class PCP_ParserDriver
{
  friend class PCP_Scanner;

 public:
  PCP_ParserDriver (bool,  // debug scanning ?
                    bool); // debug parsing ?
  virtual ~PCP_ParserDriver ();

  // target data, needs to be set before invoking parse() !
  bool initialize (const struct Common_FlexBisonParserConfiguration&);

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
  ACE_Message_Block*                          fragment_;
  unsigned int                                offset_; // parsed entity bytes

  // target
  struct PCP_Record*                          record_;

  unsigned int                                padding_;

 private:
  ACE_UNIMPLEMENTED_FUNC (PCP_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (PCP_ParserDriver (const PCP_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (PCP_ParserDriver& operator= (const PCP_ParserDriver&))

  //// convenient typedefs
  //typedef PCP_Message_T<AllocatorConfigurationType> MESSAGE_T;

  // helper methods
  bool scan_begin ();
  void scan_end ();

  struct Common_FlexBisonParserConfiguration* configuration_;

  // context
  bool                                        trace_;

  //// parser
  //yy::PCP_Parser    parser_;

  // scanner
  yyscan_t                                    scannerState_;
  YY_BUFFER_STATE                             bufferState_;
  ACE_Message_Queue_Base*                     messageQueue_;
  bool                                        useYYScanBuffer_;

  bool                                        initialized_;
};

#endif
