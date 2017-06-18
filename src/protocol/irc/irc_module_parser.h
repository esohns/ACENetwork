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

#include "common_time_common.h"

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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class IRC_Module_Parser_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IRC_Module_Parser_T (ISTREAM_T*);                     // stream handle
#else
  IRC_Module_Parser_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~IRC_Module_Parser_T ();

  // configuration / initialization
  virtual bool initialize (const ConfigurationType&,   // configuration handle
                           Stream_IAllocator* = NULL); // allocator handle

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Module_Parser_T ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Module_Parser_T (const IRC_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Module_Parser_T& operator= (const IRC_Module_Parser_T&))

  bool             crunchMessages_;

  // driver
  bool             debugScanner_;
  bool             debugParser_;
  IRC_ParserDriver driver_;
};

// include template definition
#include "irc_module_parser.inl"

#endif
