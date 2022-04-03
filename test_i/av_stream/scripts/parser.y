%defines                          "test_i_av_stream_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "c"
%locations
%no-lines
%output                           "test_i_av_stream_parser.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/*%skeleton                         "lalr1.cc"*/
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
#undef YYTOKENTYPE
}

/*%define location_type*/
/*%define api.location.type         {location}*/
/*%define namespace                 {avstream}*/
/*%define api.namespace             {avstream}*/
/*%name-prefix                      "avstream"*/
/* %define api.prefix                {yy} */
/*%pure-parser*/
/*%define api.pure                  true*/
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/* %define api.token.constructor */
/* %define api.token.prefix          {} */
%token-table
/* %define api.value.type            variant */
%define api.value.union.name      YYSTYPE
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/*%define parse.assert              true*/
/*%error-verbose*/
%define parse.error               verbose
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/*%define parser_class_name         {AVStream_Parser}*/
/*%define api.parser.class          {AVStream_Parser}*/
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
/*%debug*/
%define parse.trace               true

%code requires {
class AVStream_ParserDriver;
class AVStream_Scanner;

typedef void* yyscan_t;

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually; apparently, there is no easy way to add the export
//         symbol to the declaration
/*#define YYDEBUG 1*/
/*extern int IRC_Export irc_debug;*/
/*#define YYERROR_VERBOSE 1*/
}

// calling conventions / parameter passing
%parse-param              { AVStream_ParserDriver* driver }
%parse-param              { yyscan_t yyscanner }
%lex-param                { YYSTYPE yylval }
%lex-param                { YYLTYPE yylloc }
%lex-param                { AVStream_ParserDriver* driver }
%lex-param                { yyscan_t yyscanner }

%initial-action
{
  // Initialize the initial location
  //@$.begin.filename = @$.end.filename = &driver->file;

  // initialize the token value container
  $$.ival = 0;
};

// symbols
%union
{
  int ival;
};

%code {
#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "test_i_av_stream_server_message.h"
#include "test_i_module_parser.h"
#include "test_i_av_stream_scanner.h"
#include "test_i_av_stream_parser_driver.h"

// *TODO*: this shouldn't be necessary
#define yylex AVStream_Scanner_lex

void yyerror (AVStream_ParserDriver*, yyscan_t, const char*);
}

%token <ival> TYPE        "type"
%token <ival> LENGTH      "length"
%token <ival> DATA        "data"
%token <ival> BODY        "body"
%token        END 0       "end_of_message"
%type  <ival> message header msg_body ext_msg_body

%code provides {
/*void yysetdebug (int);
void yyerror (YYLTYPE*, AVStream_IParser*, yyscan_t, const char*);
int yyparse (AVStream_IParser*, yyscan_t);
void yyprint (FILE*, yytokentype, YYSTYPE);*/
}

/*%printer                  { debug_stream () << $$; }  <ival>*/
%printer                  { fprintf (yyo, "%d", $$); }  <ival>
%destructor               { $$ = 0; }                 <ival>
/*%destructor { ACE_DEBUG ((LM_DEBUG,
                            ACE_TEXT ("discarding tagless symbol...\n"))); } <>*/

%%
%start        message;
message:      header msg_body                                 { $$ = $1 + $2; };
              | %empty                                        { $$ = 0; };
header:       "type"                                          { driver->header_.type = $1;
                                                              }
              "length"                                        { $$ = $1 + $3;
                                                                driver->header_.length = $3;
                                                                driver->missing_ = $3;
                                                              };
msg_body:     "data"                                          { driver->missing_ -= $1;
                                                              }
              ext_msg_body                                    { $$ = $1 + $3; };
              | "body"                                        { $$ = $1;
                                                                driver->missing_ -= $1;
                                                                ACE_ASSERT (!driver->missing_);
                                                                driver->message_cb ();
                                                                YYACCEPT;
                                                              };
ext_msg_body: "data"                                          { driver->missing_ -= $1;
                                                              }
              ext_msg_body                                    { $$ = $1 + $3; };
              | "body"                                        { $$ = $1;
                                                                driver->missing_ -= $1;
                                                                ACE_ASSERT (!driver->missing_);
                                                                driver->message_cb ();
                                                                YYACCEPT;
                                                              };
%%

/*void
yy::AVStream_Parser::error (const location_type& location_in,
                            const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::AVStream_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_Parser::set"));

  yyscanner = context_in;
}*/

void
yyerror (AVStream_ParserDriver* driver_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("AVStream_Parser::error"));

  ACE_UNUSED_ARG (context_in);

  driver_in->error (message_in);
}
