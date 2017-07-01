%defines                          "irc_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "c++"
%locations
%no-lines
%output                           "irc_parser.cpp"
%require                          "2.4.1"
/*%skeleton                         "glr.c"*/
%skeleton                         "lalr1.cc"
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
}

/*%define location_type*/
/*%define api.location.type         {location}*/
/*%define namespace                 {irc}*/
/*%define api.namespace             {irc}*/
/*%name-prefix                      "irc"*/
/* %define api.prefix                {yy} */
/*%pure-parser*/
/*%define api.pure                  true*/
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/* %define api.token.constructor */
/* %define api.token.prefix          {} */
%token-table
/* %define api.value.type            variant */
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/*%define parse.assert              true*/
/*%error-verbose*/
%define parse.error               verbose
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
%define parser_class_name         {IRC_Parser}
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
/*%debug*/
%define parse.trace               true

%code requires {
#include "irc_exports.h"

/*class std::string;*/
class IRC_ParserDriver;
class IRC_Scanner;

typedef void* yyscan_t;

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually; apparently, there is no easy way to add the export
//         symbol to the declaration
/*#define YYDEBUG 1*/
/*extern int IRC_Export irc_debug;*/
/*#define YYERROR_VERBOSE 1*/
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
// *NOTE*: necessary only if %debug is set in the definition file (see above)
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

#include "ace/Synch.h"
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

%code provides {
/*void IRC_Export yysetdebug (int);
void IRC_Export yyerror (YYLTYPE*, IRC_IParser*, yyscan_t, const char*);
int IRC_Export yyparse (IRC_IParser*, yyscan_t);
void IRC_Export yyprint (FILE*, yytokentype, YYSTYPE);*/
}

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
prefix:       ':' "origin" ext_prefix                         { driver->record_->prefix_.origin = *$2;
/*                                                              ACE_DEBUG ((LM_DEBUG,
                                                                            ACE_TEXT ("set origin: \"%s\"\n"),
                                                                driver->myCurrentMessage->prefix_.origin.c_str ())); */
                                                              };
ext_prefix:   '!' "user" ext_prefix                           { driver->record_->prefix_.user = *$2;
/*                                                              ACE_DEBUG ((LM_DEBUG,
                                                                            ACE_TEXT ("set user: \"%s\"\n"),
                                                                driver->myCurrentMessage->prefix_.user.c_str ())); */
                                                              };
              | '@' "host" ext_prefix                         { driver->record_->prefix_.host = *$2;
/*                                                              ACE_DEBUG ((LM_DEBUG,
                                                                            ACE_TEXT ("set host: \"%s\"\n"),
                                                                driver.record_->prefix_.host.c_str ())); */
                                                              };
              | "space"                                       /* default */
body:         command params "end_of_message"                 /* default */
command:      "cmd_string"                                    { ACE_ASSERT (driver->record_->command_.string == NULL);
                                                                ACE_NEW_NORETURN (driver->record_->command_.string,
                                                                                  std::string (*$1));
                                                                ACE_ASSERT (driver->record_->command_.string);
                                                                driver->record_->command_.discriminator = IRC_Record::Command::STRING;
/*                                                              ACE_DEBUG ((LM_DEBUG,
                                                                            ACE_TEXT ("set command: \"%s\"\n"),
                                                                            driver->record_->command_.string->c_str ())); */
                                                              };
              | "cmd_numeric"                                 { driver->record_->command_.numeric = static_cast<IRC_NumericCommand_t> ($1);
                                                                driver->record_->command_.discriminator = IRC_Record::Command::NUMERIC;
/*                                                              ACE_DEBUG ((LM_DEBUG,
                                                                            ACE_TEXT ("set command (numeric): %d\n"),
                                                                            $1)); */
                                                              };
params:       "space" params                                  /* default */
              | ':' trailing                                  /* default */
              | "param" params                                { driver->record_->parameters_.push_front (*$1);
/*                                                              ACE_DEBUG ((LM_DEBUG,
                                                                            ACE_TEXT ("set param: \"%s\"\n"),
                                                                            driver->record_->parameters_.front ().c_str ())); */
                                                              };
/*              | %empty                                      empty */
              |                                               /* empty */
trailing:     "param"                                         { driver->record_->parameters_.push_front (*$1);
/*                                                              ACE_DEBUG ((LM_DEBUG,
                                                                            ACE_TEXT ("set final param: \"%s\"\n"),
                                                                            driver->record_->parameters_.front ().c_str ())); */
                                                              };
/*              | %empty                                      empty */
              |                                               /* empty */
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
