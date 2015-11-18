%require                  "2.4.1"
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
 *         below). This interferes with ACE (version 6.2.3), when compiled with
 *         support for traditional iostreams */
/* %debug */
%language                 "c++"
%locations
%name-prefix              "yy"
%no-lines
%skeleton                 "lalr1.cc"
/* %skeleton         "glr.c" */
%token-table
%code top {
#include "stdafx.h"
}
%defines                  "irc_parser.h"
%output                   "irc_parser.cpp"
/* %define           api.pure */
/* %define           api.push_pull */
/* %define           parse.lac full */
%define api.namespace     {yy}
%error-verbose
%define parser_class_name {IRC_Parser}

%code requires {
class IRC_ParserDriver;
class IRC_Scanner;

typedef void* yyscan_t;
}

// calling conventions / parameter passing
%parse-param              { IRC_ParserDriver* driver }
%parse-param              { unsigned int* messageCount }
%parse-param              { yyscan_t yyscanner }
%lex-param                { IRC_ParserDriver* driver }
%lex-param                { unsigned int* messageCount }
%lex-param                { yyscan_t yyscanner }

%initial-action
{
  // Initialize the initial location
  //@$.begin.filename = @$.end.filename = &driver->file;

  // initialize the token value container
  // $$.ival = 0;
  $$.sval = NULL;
};

// symbols
%union
{
  int          ival;
  std::string* sval;
};

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: IRCparser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif
#include <string>

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "irc_message.h"
#include "irc_module_parser.h"
#include "irc_parser_driver.h"

// *TODO*: this shouldn't be necessary
#define yylex IRC_Scanner_lex
}

%token <ival> SPACE       "space"
%token <sval> ORIGIN      "origin"
%token <sval> USER        "user"
%token <sval> HOST        "host"
%token <sval> CMD_STRING  "cmd_string"
%token <ival> CMD_NUMERIC "cmd_numeric"
%token <sval> PARAM       "param"
%token        END 0       "end_of_message"
/* %type  <sval> message prefix ext_prefix command params trailing */

%printer                  { debug_stream () << *$$; } <sval>
%destructor               { delete $$; $$ = NULL; }   <sval>
%printer                  { debug_stream () << $$; }  <ival>
%destructor               { $$ = 0; }                 <ival>
/*%destructor { ACE_DEBUG ((LM_DEBUG,
                            ACE_TEXT ("discarding tagless symbol...\n"))); } <>*/

%%
%start                    message;
/* %nonassoc                 ':' '!' '@'; */

message:      prefix body                                     /* default */
              | body                                          /* default */
              | "end_of_message"                              /* default */
prefix:       ':' "origin" ext_prefix                         { driver->record_->prefix.origin = *$2;
/*                                                                ACE_DEBUG((LM_DEBUG,
                                                                           ACE_TEXT("set origin: \"%s\"\n"),
                                                                                                                                             driver->myCurrentMessage->prefix.origin.c_str())); */
                                                              };
ext_prefix:   '!' "user" ext_prefix                           { driver->record_->prefix.user = *$2;
/*                                                                ACE_DEBUG((LM_DEBUG,
                                                                           ACE_TEXT("set user: \"%s\"\n"),
                                                                                                                                                      driver->myCurrentMessage->prefix.user.c_str())); */
                                                              };
              | '@' "host" ext_prefix                         { driver->record_->prefix.host = *$2;
/*                                                                ACE_DEBUG((LM_DEBUG,
                                                                           ACE_TEXT("set host: \"%s\"\n"),
                                                                           driver.record_->prefix.host.c_str())); */
                                                              };
              | "space"                                       /* default */
body:         command params "end_of_message"                 /* default */
command:      "cmd_string"                                    { ACE_ASSERT (driver->record_->command.string == NULL);
                                                                ACE_NEW_NORETURN (driver->record_->command.string,
                                                                                  std::string (*$1));
                                                                ACE_ASSERT (driver->record_->command.string);
                                                                driver->record_->command.discriminator = IRC_Record::Command::STRING;
/*                                                                ACE_DEBUG((LM_DEBUG,
                                                                             ACE_TEXT("set command: \"%s\"\n"),
                                                                             driver->record_->command.string->c_str())); */
                                                              };
              | "cmd_numeric"                                 { driver->record_->command.numeric = static_cast<IRC_NumericCommand_t> ($1);
                                                                driver->record_->command.discriminator = IRC_Record::Command::NUMERIC;
/*                                                                ACE_DEBUG((LM_DEBUG,
                                                                             ACE_TEXT("set command (numeric): %d\n"),
                                                                             $1)); */
                                                              };
params:       "space" params                                  /* default */
              | ':' trailing                                  /* default */
              | "param" params                                { driver->record_->params.push_front (*$1);
/*                                                                ACE_DEBUG((LM_DEBUG,
                                                                             ACE_TEXT("set param: \"%s\"\n"),
                                                                             driver->record_->params.front().c_str())); */
                                                              };
              | %empty                                        /* empty */
trailing:     "param"                                         { driver->record_->params.push_front (*$1);
/*                                                                ACE_DEBUG((LM_DEBUG,
                                                                             ACE_TEXT("set final param: \"%s\"\n"),
                                                                             driver->record_->params.front().c_str())); */
                                                              };
              | %empty                                        /* empty */
%%

void
yy::IRC_Parser::error (const location_type& location_in,
                       const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::IRC_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Parser::set"));

  yyscanner = context_in;
}
