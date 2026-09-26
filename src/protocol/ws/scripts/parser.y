%defines                          "ws_parser.h"
/* %file-prefix                      "" */
%language                         "C"
/*%language                         "C++"*/
%locations
%no-lines
%output                           "ws_parser.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/*%skeleton                         "lalr1.cc"*/
%token-table
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
}

/* %define location_type */
/* %define api.location.type         {} */
/*%define namespace                    {zz}*/
/*%define api.namespace                {zz}*/
/* *IMPORTANT NOTE*: do NOT mess with this (it's broken)
/*%name-prefix                         "ws_"*/
/*%define api.prefix                   {ws_}*/
/*%pure-parser*/
%define api.pure                     true
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

/* %define parse.assert              {true} */
/*%error-verbose*/
%define parse.error                  verbose
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/* %define parser_class_name         {WebSocket_Parser} */
/*%define "parser_class_name"       "WebSocket_Parser"*/
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               true */

%code requires {
// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef WEBSOCKET_PARSER_H
//#define WEBSOCKET_PARSER_H

#include <cstdio>
#include <string>

#include "common_iparser.h"

/* enum yytokentype
{
  END = 0,
  FRAME = 265,
}; */
//#define YYTOKENTYPE
/*#undef YYTOKENTYPE*/
/* enum yytokentype; */
class WebSocket_IParser;
struct WebSocket_Record;
//class WebSocket_Scanner;
//struct YYLTYPE;

/* #define YYSTYPE
typedef union YYSTYPE
{
  ACE_INT64 ival;
} YYSTYPE; */
#undef YYSTYPE
//union YYSTYPE;

typedef void* yyscan_t;

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually, as there is no way to add the export symbol to
//         the declaration
#define ZZDEBUG 1
extern int zzdebug;
#define ZZERROR_VERBOSE 1
//#define YYLTYPE_IS_DECLARED 1

#undef YYTOKENTYPE
}

// calling conventions / parameter passing
%parse-param              { WebSocket_IParser* iparser_p }
%parse-param              { yyscan_t yyscanner }
/*%lex-param                { YYSTYPE* yylval }
%lex-param                { YYLTYPE* yylloc } */
%lex-param                { WebSocket_IParser* iparser_p }
%lex-param                { yyscan_t yyscanner }
/* %param                    { WebSocket_IParser* iparser_p }
%param                    { yyscan_t yyscanner } */

%initial-action
{
  // initialize the location
  //@$.initialize (NULL);
  //@$.begin.filename = @$.end.filename = &iparser_p->file;
  //ACE_OS::memset (&@$, 0, sizeof (@$));

  // initialize the token value container
  $$.ival = 0;
}

// symbols
%union
{
  ACE_INT64 ival;
}

/* %token <ACE_INT64> INTEGER; */

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif
#include <regex>
#include <sstream>
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
#include "ace/OS.h"

#include "common_string_tools.h"

#include "stream_dec_common.h"

#include "net_macros.h"

#include "ws_common.h"
#include "ws_defines.h"
#include "ws_iparser.h"
#include "ws_scanner.h"
#include "ws_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex WebSocket_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <ival> FRAME           "frame"
//%token <ival> END_OF_FRAGMENT "end_of_fragment"
%token <ival> END 0           "end"

%type <ival> frames

%code provides {
/*void yysetdebug (int);*/
void yyerror (YYLTYPE*, WebSocket_IParser*, yyscan_t, const char*);
//void yyerror (WebSocket_IParser*, yyscan_t, const char*);
/*int yyparse (WebSocket_IParser*, yyscan_t);*/
/*void yyprint (FILE*, enum yytokentype, YYSTYPE);*/

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // WebSocket_PARSER_H
}

/*%printer                  { yyoutput << $$; } <*>;*/
/*%printer                  { debug_stream () << $$; }  <ival>*/
/*%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT ("\"%s\""), (*$$).c_str ()); } <sval>*/
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT ("%q"), $$); } <ival>
/*%destructor               { delete $$; $$ = NULL; } <sval>*/
%destructor               { $$ = 0; } <ival>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start              frames;
frames:            frames "frame"                 { /* NOTE*: use right-recursion here to force early state reductions */
                                                    $$ = $1 + $2;
                                                    struct WebSocket_Record& record_r =
                                                      iparser_p->current ();
                                                    struct WebSocket_Record* record_p =
                                                      &record_r;
                                                    try {
                                                      iparser_p->record (record_p);
                                                    } catch (...) {
                                                      ACE_DEBUG ((LM_ERROR,
                                                                  ACE_TEXT ("caught exception in WebSocket_IParser::record(), continuing\n")));
                                                    }

                                                    YYACCEPT;
                                                  }
                   | /* empty */                  { $$ = 0; };
%%

/*
void
yy::WebSocket_Parser::error (const location_type& location_in,
                             const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Parser::error"));

//  ACE_UNUSED_ARG (location_in);

  iparser_p->error (location_in, message_in);
//  iparser_p->error (message_in);
}

void
yy::WebSocket_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Parser::set"));

  yyscanner = context_in;
}*/

/*void
yysetdebug (int debug_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yysetdebug"));

  yydebug = debug_in;
}*/

void
yyerror (YYLTYPE* location_in,
         WebSocket_IParser* iparser_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (location_in);
  ACE_ASSERT (iparser_in);

  try {
    iparser_in->error (*location_in,
                       std::string (message_in));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in WebSocket_IParser::error(), continuing\n")));
  }
}

/*void
yyprint (FILE* file_in,
         yytokentype type_in,
         YYSTYPE value_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyprint"));

  int result = -1;

  std::string format_string;
  switch (type_in)
  {
    case FRAME:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %q");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH

  result = ACE_OS::fprintf (file_in,
                            ACE_TEXT (format_string.c_str ()),
                            value_in.ival);
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
}*/
