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

#ifndef HTTP_ANTLR_PARSER_DRIVER_T_H
#define HTTP_ANTLR_PARSER_DRIVER_T_H

#include <string>

#include "antlr4-runtime.h"

#include "ace/Global_Macros.h"

#include "common_parser_common.h"
#include "common_parser_defines.h"

#include "http_common.h"

#include "http_antlr_scanner.h"
#include "http_antlr_iparser.h"
#include "http_antlr_parser.h"
#include "http_antlr_parserBaseListener.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_ITask;

//////////////////////////////////////////

class HTTP_ANTLR_Streambuf
 : public std::stringbuf
{
  typedef std::stringbuf inherited;

 public:
  inline HTTP_ANTLR_Streambuf () : inherited () {}
  inline virtual ~HTTP_ANTLR_Streambuf () {}
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
class HTTP_ANTLRParserDriver_T
 : public http_antlr_parserBaseListener
 , public antlr4::ANTLRErrorListener
 , public HTTP_ANTLR_IParser
{
  typedef http_antlr_parserBaseListener inherited;

 public:
  HTTP_ANTLRParserDriver_T (Stream_ITask*); // (parent-) stream task
  virtual ~HTTP_ANTLRParserDriver_T ();

  // override (part of) http_antlr_parserBaseListener
  virtual void exitBody (http_antlr_parser::BodyContext*);
  virtual void exitChunks (http_antlr_parser::ChunksContext*);
  virtual void enterEveryRule (antlr4::ParserRuleContext*);

  // implement antlr4::ANTLRErrorListener
  virtual void syntaxError (antlr4::Recognizer*,
                            antlr4::Token*,
                            size_t,
                            size_t,
                            const std::string&,
                            std::exception_ptr);
  virtual void reportAmbiguity (antlr4::Parser*,
                                const antlr4::dfa::DFA&,
                                size_t,
                                size_t,
                                bool,
                                const antlrcpp::BitSet&,
                                antlr4::atn::ATNConfigSet*);
  virtual void reportAttemptingFullContext (antlr4::Parser*,
                                            const antlr4::dfa::DFA&,
                                            size_t,
                                            size_t,
                                            const antlrcpp::BitSet&,
                                            antlr4::atn::ATNConfigSet*);
  virtual void reportContextSensitivity (antlr4::Parser*,
                                         const antlr4::dfa::DFA&,
                                         size_t,
                                         size_t,
                                         size_t,
                                         antlr4::atn::ATNConfigSet*);

  // implement (part of) HTTP_ANTLR_IParser
  virtual bool initialize (const struct HTTP_ParserConfiguration&);
  inline virtual ACE_Message_Block* buffer () { return fragment_; }
  inline virtual bool isBlocking () const { ACE_ASSERT (configuration_); return configuration_->block; }
  inline virtual void offset (unsigned int offset_in) { offset_ += offset_in; } // offset (increment)
  inline virtual unsigned int offset () const { return static_cast<unsigned int> (offset_); }
  virtual bool begin (const char*, // buffer handle
                      size_t);     // buffer size
  virtual void end ();
  inline virtual const HTTP_ANTLR_IParser* const getP_2 () const { return this; }
  virtual bool parse (ACE_Message_Block*); // data buffer handle
  virtual bool switchBuffer (bool = false); // unlink current fragment ?
  // *NOTE*: (waits for and) appends the next data chunk to fragment_;
  virtual void waitBuffer ();
  inline virtual struct HTTP_Record& current () { return record_; }
  inline virtual void finished () { finished_ = true; };
  inline virtual bool hasFinished () const { return finished_; }

  inline virtual bool headerOnly () { ACE_ASSERT (configuration_); return configuration_->headerOnly; } // returns: parse HTTP header only ?

  virtual void dump_state () const;

 protected:
  struct HTTP_ParserConfiguration* configuration_;
  bool                             finished_; // processed the whole entity ?
  ACE_Message_Block*               fragment_;
  // antlr4::ANTLRInputStream         inputStream_;
  HTTP_ANTLR_Streambuf             inputBuffer_;
  std::wifstream                   inputStream_;
  antlr4::UnbufferedCharStream     input_;
  Stream_ITask*                    itask_;
  http_antlr_scanner               lexer_;
  antlr4::UnbufferedTokenStream    tokens_;
  size_t                           offset_; // parsed entity bytes
  http_antlr_parser                parser_;
  struct HTTP_Record               record_;

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_ANTLRParserDriver_T ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_ANTLRParserDriver_T (const HTTP_ANTLRParserDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_ANTLRParserDriver_T& operator= (const HTTP_ANTLRParserDriver_T&))

  inline virtual const HTTP_ParserConfiguration& getR () const { ACE_ASSERT (configuration_); return *configuration_; }
  inline virtual const Common_FlexScannerState& getR_2 () const { static Common_FlexScannerState dummy; ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) }
  inline virtual void setP (HTTP_ANTLR_IParser*) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  bool                             isFirst_;
  bool                             isInitialized_;
  ACE_Message_Queue_Base*          messageQueue_;
};

// include template definition
#include "http_antlr_parser_driver.inl"

#endif
