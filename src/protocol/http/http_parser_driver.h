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

#include "net_iparser.h"

#include "location.hh"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Message_Queue_Base;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct YYLTYPE;

template <typename RecordType,
          typename SessionMessageType>
class HTTP_ParserDriver
 : public Net_IParser<RecordType>
{
 public:
  HTTP_ParserDriver (bool,  // debug scanning ?
                     bool); // debug parsing ?
  virtual ~HTTP_ParserDriver ();

  // implement Net_IParser
  virtual void initialize (RecordType&,                                    // target data record
                           bool = NET_PROTOCOL_DEFAULT_LEX_TRACE,          // debug scanner ?
                           bool = NET_PROTOCOL_DEFAULT_YACC_TRACE,         // debug parser ?
                           ACE_Message_Queue_Base* = NULL,                 // data buffer queue (yywrap)
                           bool = NET_PROTOCOL_DEFAULT_USE_YY_SCAN_BUFFER, // yy_scan_buffer() ? : yy_scan_bytes()
                           bool = false);                                  // block in parse() ?

  inline virtual ACE_Message_Block* buffer () { return fragment_; };
  inline virtual RecordType* record () { return record_; };
  virtual bool debugScanner () const;
  inline virtual bool isBlocking () const { return blockInParse_; };

  virtual void error (const YYLTYPE&,      // location
                      const std::string&); // message
  inline virtual void finished () { finished_ = true; };
  inline virtual bool hasFinished () const { return finished_; };

  inline virtual void offset (unsigned int offset_in) { offset_ += offset_in; }; // offset (increment)
  inline virtual unsigned int offset () const { return offset_; };

  virtual bool parse (ACE_Message_Block*); // data buffer handle
  virtual bool switchBuffer ();
  virtual void wait ();

  virtual void dump_state () const;

 protected:
  bool                    finished_; // processed the whole entity ?
  ACE_Message_Block*      fragment_;
  unsigned int            offset_; // parsed entity bytes
  RecordType*             record_;

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver (const HTTP_ParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver& operator= (const HTTP_ParserDriver&))

  // helper methods
  bool scan_begin ();
  void scan_end ();

  // error handling
  void error (const yy::location&, // location
              const std::string&); // message
  void error (const std::string&); // message

  bool                    blockInParse_;
  bool                    isFirst_;
  bool                    trace_;

  //// parser
  //yy::HTTP_Parser    parser_;

  // scanner
  yyscan_t                scannerState_;
  YY_BUFFER_STATE         bufferState_;
  ACE_Message_Queue_Base* messageQueue_;
  bool                    useYYScanBuffer_;

  bool                    isInitialized_;
};

// include template definition
#include "http_parser_driver.inl"

#endif
