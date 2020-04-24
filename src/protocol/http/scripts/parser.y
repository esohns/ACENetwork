%defines                          "http_parser.h"
/* %file-prefix                      "" */
%language                         "C"
/*%language                         "C++"*/
%locations
%no-lines
%output                           "http_parser.cpp"
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
/* %define namespace                 {yy} */
/* %define api.namespace             {yy} */
/* *IMPORTANT NOTE*: do NOT mess with this (it's broken)
/*%name-prefix                         "http_"*/
/*%define api.prefix                   {http_}*/
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
/* %define parser_class_name         {HTTP_Parser} */
/*%define "parser_class_name"       "HTTP_Parser"*/
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               true */

%code requires {
// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef HTTP_PARSER_H
//#define HTTP_PARSER_H

#include <cstdio>
#include <string>

#include "common_iscanner.h"

/* enum yytokentype
{
  END = 0,
  METHOD = 258,
  URI = 259,
  VERSION = 260,
  HEADER = 261,
  DELIMITER = 262,
  STATUS = 263,
  REASON = 264,
  BODY = 265,
  CHUNK = 266
}; */
//#define YYTOKENTYPE
/*#undef YYTOKENTYPE*/
/* enum yytokentype; */
class HTTP_IParser;
struct HTTP_Record;
//class HTTP_Scanner;
//struct YYLTYPE;

/* #define YYSTYPE
typedef union YYSTYPE
{
  int          ival;
  std::string* sval;
} YYSTYPE; */
#undef YYSTYPE
//union YYSTYPE;

typedef void* yyscan_t;

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually, as there is no way to add the export symbol to
//         the declaration
#define YYDEBUG 1
extern int yydebug;
#define YYERROR_VERBOSE 1
#define YYLTYPE_IS_DECLARED 1
}

// calling conventions / parameter passing
%parse-param              { HTTP_IParser* iparser_p }
%parse-param              { yyscan_t yyscanner }
/*%lex-param                { YYSTYPE* yylval }
%lex-param                { YYLTYPE* yylloc } */
%lex-param                { HTTP_IParser* iparser_p }
%lex-param                { yyscan_t yyscanner }
/* %param                    { HTTP_IParser* iparser_p }
%param                    { yyscan_t yyscanner } */

%initial-action
{
  // initialize the location
  //@$.initialize (NULL);
  //@$.begin.filename = @$.end.filename = &iparser_p->file;
  //ACE_OS::memset (&@$, 0, sizeof (@$));

  // initialize the token value container
  $$.ival = 0;
  $$.sval = NULL;
}

// symbols
%union
{
  int          ival;
  std::string* sval;
}

/* %token <int>         INTEGER;
%token <std::string> STRING; */

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

#include "stream_dec_common.h"

#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_iparser.h"
#include "http_scanner.h"
#include "http_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex HTTP_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

//%token <std::string> METHOD      "method"
%token <sval> METHOD          "method"
%token <sval> URI             "uri"
%token <sval> _VERSION        "version"
%token <sval> HEADER          "header"
%token <ival> DELIMITER       "delimiter"
%token <sval> STATUS          "status"
%token <sval> REASON          "reason"
%token <ival> BODY            "body"
//%token <int>         BODY        "body"
%token <ival> CHUNK           "chunk"
//%token <std::string> REQUEST     "request_line"
//%token <std::string> RESPONSE    "status_line"
%token <ival> END_OF_FRAGMENT "end_of_fragment"
%token <ival> END 0           "end"

%type  <ival> message head body
%type  <ival> chunked_body chunks
%type  <ival> head_rest1 head_rest2 headers
%type  <ival> request_line_rest1 request_line_rest2
%type  <ival> status_line_rest1 status_line_rest2

%code provides {
/*void yysetdebug (int);*/
void yyerror (YYLTYPE*, HTTP_IParser*, yyscan_t, const char*);
//void yyerror (HTTP_IParser*, yyscan_t, const char*);
/*int yyparse (HTTP_IParser*, yyscan_t);*/
/*void yyprint (FILE*, enum yytokentype, YYSTYPE);*/

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // HTTP_PARSER_H
}

/*%printer                  { yyoutput << $$; } <*>;*/
/*%printer                  { debug_stream () << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival>*/
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT ("\"%s\""), (*$$).c_str ()); } <sval>
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT ("%d"), $$); } <ival>
%destructor               { delete $$; $$ = NULL; } <sval>
%destructor               { $$ = 0; } <ival>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start              message;

message:            head "delimiter" body            { $$ = $1 + $2 + $3; };
head:               "method" head_rest1              { $$ = $1->size () + $2 + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.method =
                                                         HTTP_Tools::MethodToType (*$1);
                                                     };
                    | "version" head_rest2           { $$ = $1->size () + $2 + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^");
                                                       regex_string +=
                                                         ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
                                                       regex_string +=
                                                         ACE_TEXT_ALWAYS_CHAR ("([[:digit:]]{1}\\.[[:digit:]]{1})$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*$1,
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP version (was: \"%s\"), aborting\n"),
                                                                     ACE_TEXT ($1->c_str ())));
                                                         YYABORT;
                                                       } // end IF
//                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                                       ACE_ASSERT (!match_results.empty ());
                                                       ACE_ASSERT (match_results[1].matched);

                                                       record_r.version =
                                                           HTTP_Tools::VersionToType (match_results[1].str ());
                                                     };
head_rest1:         request_line_rest1 headers       { $$ = $1 + $2; };
request_line_rest1: "uri" request_line_rest2         { $$ = $1->size () + $2 + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.URI = *$1;
                                                     };
request_line_rest2: "version"                        { $$ = $1->size () + 2;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.version =
                                                         HTTP_Tools::VersionToType (*$1);
                                                     };
head_rest2:         status_line_rest1 headers        { $$ = $1 + $2; };
status_line_rest1:  "status" status_line_rest2       { $$ = $1->size () + $2 + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       std::istringstream converter;
                                                       converter.str (*$1);
                                                       int status = -1;
                                                       converter >> status;
                                                       record_r.status =
                                                           static_cast<HTTP_Status_t> (status);
                                                     };
status_line_rest2:  "reason"                         { $$ = $1->size () + 2;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.reason = *$1;
                                                     };
headers:            headers "header"                 { /* NOTE*: use right-recursion here to force early state reductions
                                                                 (i.e. parse headers). This is required so the scanner can
                                                                 act on any set transfer encoding. */
                                                       $$ = $1 + $2->size ();
                                                       /* *TODO*: modify the scanner so it emits the proper fields itself */
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^([^:]+):\\s(.+)$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*$2,
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP header (was: \"%s\"), returning\n"),
                                                                     ACE_TEXT ($2->c_str ())));
                                                         break;
                                                       } // end IF
//                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                                       ACE_ASSERT (!match_results.empty ());
                                                       ACE_ASSERT (match_results[1].matched);
                                                       ACE_ASSERT (match_results[2].matched);
                                                       ACE_ASSERT (!match_results[2].str ().empty ());

                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.headers[match_results[1]] =
                                                         match_results[2];

                                                       // upcall ?
                                                       if (match_results[1] == ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING))
                                                       {
                                                         try {
                                                           iparser_p->encoding (match_results[1]);
                                                         } catch (...) {
                                                           ACE_DEBUG ((LM_ERROR,
                                                                       ACE_TEXT ("caught exception in HTTP_IParser::encoding(), continuing\n")));
                                                         }
                                                       } };
                    |                                { $$ = 0; };
/*                    | %empty                         { $$ = 0; }; */
body:               "body"                           { $$ = $1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       HTTP_HeadersIterator_t iterator =
                                                         record_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
                                                       ACE_ASSERT (iterator != record_r.headers.end ());
                                                       std::istringstream converter;
                                                       converter.str ((*iterator).second);
                                                       unsigned int content_length = 0;
                                                       converter >> content_length;
                                                       if ($1 == static_cast<int> (content_length))
                                                       {
                                                         struct HTTP_Record* record_p =
                                                           &record_r;
                                                         try {
                                                           iparser_p->record (record_p);
                                                         } catch (...) {
                                                           ACE_DEBUG ((LM_ERROR,
                                                                       ACE_TEXT ("caught exception in HTTP_IParser::record(), continuing\n")));
                                                         }
                                                         YYACCEPT;
                                                       } };
                    | "chunk" chunked_body           { $$ = $1 + $2; };
chunked_body:       chunks headers "delimiter"     { $$ = $1 + $2 + $3; // *TODO*: potential conflict here (i.e. incomplete chunk may be accepted)
                                                     struct HTTP_Record& record_r =
                                                       iparser_p->current ();
                                                     struct HTTP_Record* record_p =
                                                       &record_r;
                                                     try {
                                                       iparser_p->record (record_p);
                                                     } catch (...) {
                                                       ACE_DEBUG ((LM_ERROR,
                                                                   ACE_TEXT ("caught exception in HTTP_IParser::record(), continuing\n")));
                                                     }
                                                     YYACCEPT; };
chunks:             "chunk" chunks                   { $$ = $1 + $2; };
                    |                                { $$ = 0; };
/*                    | %empty                         { $$ = 0; }; */
%%

/*
void
yy::HTTP_Parser::error (const location_type& location_in,
                        const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Parser::error"));

//  ACE_UNUSED_ARG (location_in);

  iparser_p->error (location_in, message_in);
//  iparser_p->error (message_in);
}

void
yy::HTTP_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Parser::set"));

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
         HTTP_IParser* iparser_in,
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
                ACE_TEXT ("caught exception in HTTP_IParser::error(), continuing\n")));
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
    case METHOD:
    case URI:
    case _VERSION:
    case HEADER:
    case STATUS:
    case REASON:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case DELIMITER:
    case BODY:
    case END:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %d");
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
                            value_in);
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
}*/
