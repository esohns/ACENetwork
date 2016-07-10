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

#ifndef IRC_MODULE_PARSER_H
#define IRC_MODULE_PARSER_H

#include "ace/Global_Macros.h"

#include "stream_task_base_synch.h"

#include "irc_defines.h"
#include "irc_parser_driver.h"

 // tell flex of the lexer's prototype ...
extern yy::IRC_Parser::token_type
IRC_Scanner_lex (yy::IRC_Parser::semantic_type*, // token
                 yy::IRC_Parser::location_type*, // location
                 IRC_ParserDriver*,              // driver
                 unsigned int*,                  // return value: message count
                 yyscan_t);                      // scanner state
#define YY_DECL                                         \
yy::IRC_Parser::token_type                              \
IRC_Scanner_lex (yy::IRC_Parser::semantic_type* yylval, \
                 yy::IRC_Parser::location_type* yylloc, \
                 IRC_ParserDriver* driver,              \
                 unsigned int* messageCounter,          \
                 yyscan_t yyscanner)
// ... and declare it for the parser's sake
YY_DECL;

//void
//yy::IRC_Client_IRCParser::set(yyscan_t);

// forward declaration(s)
class Stream_IAllocator;

template <typename SynchStrategyType,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class IRC_Module_Parser_T
 : public Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 /////////
                                 ConfigurationType,
                                 /////////
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType>
{
 public:
  IRC_Module_Parser_T ();
  virtual ~IRC_Module_Parser_T ();

  // configuration / initialization
  bool initialize (Stream_IAllocator*,                 // message allocator
                   bool = IRC_DEFAULT_CRUNCH_MESSAGES, // crunch messages ?
                   bool = IRC_DEFAULT_LEX_TRACE,       // debug scanner ?
                   bool = IRC_DEFAULT_YACC_TRACE);     // debug parser ?

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

 private:
  typedef Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 /////////
                                 ConfigurationType,
                                 /////////
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Module_Parser_T (const IRC_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Module_Parser_T& operator= (const IRC_Module_Parser_T&))

  // helper methods
  DataMessageType* allocateMessage (unsigned int); // requested size

  // message allocator
  Stream_IAllocator* allocator_;

  // driver
  bool               debugScanner_;
  bool               debugParser_;
  IRC_ParserDriver   driver_;

  bool               crunchMessages_;
  bool               isInitialized_;
};

// include template implementation
#include "irc_module_parser.inl"

#endif
