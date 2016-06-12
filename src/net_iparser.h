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

#ifndef NET_IPARSER_H
#define NET_IPARSER_H

#include <string>

#include "common_idumpstate.h"

#include "net_defines.h"

// forward declarations
struct YYLTYPE;
class ACE_Message_Block;
class ACE_Message_Queue_Base;

template <typename RecordType>
class Net_IParser
 : public Common_IDumpState
{
 public:
  inline virtual ~Net_IParser () {};

  // target data, needs to be set before invoking parse() !
  virtual void initialize (RecordType&,                                    // target data record
                           bool = NET_PROTOCOL_DEFAULT_LEX_TRACE,          // debug scanner ?
                           bool = NET_PROTOCOL_DEFAULT_YACC_TRACE,         // debug parser ?
                           ACE_Message_Queue_Base* = NULL,                 // data buffer queue (yywrap)
                           bool = NET_PROTOCOL_DEFAULT_USE_YY_SCAN_BUFFER, // yy_scan_buffer() ? : yy_scan_bytes()
                           bool = false) = 0;                              // block in parse() ?

  virtual ACE_Message_Block* buffer () = 0;
  virtual RecordType* record () = 0;
  virtual bool debugScanner () const = 0;
  virtual bool isBlocking () const = 0;

  virtual void error (const struct YYLTYPE&,
                      const std::string&) = 0;
  virtual void finished () = 0;
  virtual bool hasFinished () const = 0;

  // *NOTE*: to be invoked by the scanner (ONLY !)
  virtual void offset (unsigned int) = 0; // offset (increment)
  virtual unsigned int offset () const = 0;

  virtual bool parse (ACE_Message_Block*) = 0; // data buffer handle
  virtual bool switchBuffer () = 0;
  virtual void wait () = 0;
};

#endif
