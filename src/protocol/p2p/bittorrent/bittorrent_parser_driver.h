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

#ifndef BITTORRENT_PARSER_DRIVER_T_H
#define BITTORRENT_PARSER_DRIVER_T_H

#include <string>

#include <ace/Global_Macros.h>

#include "location.hh"

#include "bittorrent_defines.h"
#include "bittorrent_iparser.h"
#include "bittorrent_scanner.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Message_Queue_Base;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
//struct YYLTYPE;

template <typename MessageType,
          typename SessionMessageType>
class BitTorrent_ParserDriver_T
 : public BitTorrent_IParser_t
{
 public:
  BitTorrent_ParserDriver_T (bool,  // debug scanning ?
                             bool); // debug parsing ?
  virtual ~BitTorrent_ParserDriver_T ();

  // implement (part of) BitTorrent_IParser_T
  virtual void initialize (bool = NET_PROTOCOL_DEFAULT_LEX_TRACE,        // debug scanner ?
                           bool = NET_PROTOCOL_DEFAULT_YACC_TRACE,       // debug parser ?
                           ACE_Message_Queue_Base* = NULL,               // data buffer queue (yywrap)
//                           bool = BITTORRENT_DEFAULT_USE_YY_SCAN_BUFFER, // yy_scan_buffer() ? : yy_scan_bytes()
                           bool = false);                                // block in parse() ?
  inline virtual ACE_Message_Block* buffer () { return fragment_; };
  inline virtual bool debugScanner () const { return bittorrent_get_debug (scannerState_); };
  inline virtual bool isBlocking () const { return blockInParse_; };
  virtual void error (const yy::location&, // location
                      const std::string&); // message
//  virtual void error (const std::string&); // message
  inline virtual void offset (unsigned int offset_in) { offset_ += offset_in; }; // offset (increment)
  inline virtual unsigned int offset () const { return offset_; };
  virtual bool parse (ACE_Message_Block*); // data buffer handle
  virtual bool switchBuffer (bool = false); // unlink current fragment ?
  // *NOTE*: (waits for and) appends the next data chunk to fragment_;
  virtual void wait ();
  virtual struct BitTorrent_Record& current ();

  virtual void dump_state () const;

 protected:
  ACE_Message_Block*      fragment_;
  unsigned int            offset_; // parsed fragment bytes
  bool                    trace_;

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver_T (const BitTorrent_ParserDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_ParserDriver_T& operator= (const BitTorrent_ParserDriver_T&))

  // helper methods
  bool scan_begin ();
  void scan_end ();

  bool                    blockInParse_;
  bool                    isFirst_;

  // scanner
  yyscan_t                scannerState_;
  YY_BUFFER_STATE         bufferState_;
  ACE_Message_Queue_Base* messageQueue_;
  bool                    useYYScanBuffer_;

  bool                    isInitialized_;
};

// include template definition
#include "bittorrent_parser_driver.inl"

#endif
