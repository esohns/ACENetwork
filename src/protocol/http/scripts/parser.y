%defines                          "http_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "http_parser.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/* %skeleton                         "lalr1.cc" */
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
%name-prefix                      "yy"
/* %define api.prefix                {yy} */
%pure-parser
/* %define api.pure                  true */
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/* %define api.token.constructor */
/* %define api.token.prefix          {} */
/* %define api.value.type            variant */
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/* %define parse.assert              {true} */
%error-verbose
/* %define parse.error               verbose */
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/* %define parser_class_name         {HTTP_Parser} */
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               {true} */

%code requires {
#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <cstdio>
#include <string>

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
#undef YYTOKENTYPE
/* enum yytokentype; */
template <typename RecordType>
class Net_IParser;
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

//#define YYERROR_VERBOSE
}

// calling conventions / parameter passing
%parse-param              { Net_IParser<HTTP_Record>* driver }
%parse-param              { yyscan_t yyscanner }
/*%lex-param                { YYSTYPE* yylval }
%lex-param                { YYLTYPE* yylloc } */
%lex-param                { Net_IParser<HTTP_Record>* driver }
%lex-param                { yyscan_t yyscanner }
/* %param                    { Net_IParser* driver }
%param                    { yyscan_t yyscanner } */

%initial-action
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  //@$.begin.filename = @$.end.filename = &driver->file;
  ACE_OS::memset (&@$, 0, sizeof (YYLTYPE));

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

#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_parser_driver.h"
#include "http_scanner.h"
#include "http_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex HTTP_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <sval> METHOD      "method"
%token <sval> URI         "uri"
%token <sval> VERSION     "version"
%token <sval> HEADER      "header"
%token <ival> DELIMITER   "delimiter"
%token <sval> STATUS      "status"
%token <sval> REASON      "reason"
%token <ival> BODY        "body"
%token <ival> CHUNK       "chunk"
/* %token <std::string> METHOD      "method"
%token <std::string> VERSION     "version"
%token <std::string> REQUEST     "request_line"
%token <std::string> RESPONSE    "status_line"
%token <std::string> HEADER      "header"
%token <std::string> DELIMITER   "delimiter"
%token <int>         BODY        "body" */
%token <ival> END 0       "end_of_fragment"

%type  <ival> message head body
%type  <ival> head_rest1 head_rest2 headers chunks
%type  <ival> request_line_rest1 request_line_rest2
%type  <ival> status_line_rest1 status_line_rest2

%code provides {
void yyerror (YYLTYPE*, Net_IParser<HTTP_Record>*, yyscan_t, const char*);
int yyparse (Net_IParser<HTTP_Record>* driver, yyscan_t yyscanner);
void yyprint (FILE*, yytokentype, YYSTYPE);

#endif // HTTP_PARSER_H
}

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival> */
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (*$$).c_str ()); } <sval>
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %d"), $$); }             <ival>
%destructor               { delete $$; $$ = NULL; } <sval>
%destructor               { $$ = 0; }               <ival>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start        message;

message:            head "delimiter" body            { $$ = $1 + $2 + $3; };
head:               "method" head_rest1              { $$ = (*$1).size () + $2 + 1;
                                                       driver->record ()->method =
                                                         HTTP_Tools::Method2Type (*$1);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set method: \"%s\"\n"),
//                                                                   ACE_TEXT ((*$1).c_str ())));
                                                     };
                    | "version" head_rest2           { $$ = (*$1).size () + $2 + 1;
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
                                                                     ACE_TEXT ("invalid HTTP version (was: \"%s\"), continuing\n"),
                                                                     ACE_TEXT ((*$1).c_str ())));
                                                       } // end IF
                                                       else
                                                       {
                                                         ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                                         ACE_ASSERT (match_results[1].matched);

                                                         driver->record ()->version =
                                                             HTTP_Tools::Version2Type (match_results[1].str ());
//                                                         ACE_DEBUG ((LM_DEBUG,
//                                                                     ACE_TEXT ("set version: \"%s\"\n"),
//                                                                     ACE_TEXT (match_results[1].str ().c_str ())));
                                                       } // end ELSE
                                                     };
                    | "end_of_fragment"              { $$ = $1;
                                                       yyclearin; };
head_rest1:         request_line_rest1 headers       { $$ = $1 + $2; };
request_line_rest1: "uri" request_line_rest2         { $$ = (*$1).size () + $2 + 1;
                                                       driver->record ()->URI = *$1;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set URI: \"%s\"\n"),
//                                                                   ACE_TEXT ((*$1).c_str ())));
                                                     };
                    | "end_of_fragment"              { $$ = $1;
                                                       yyclearin; };
request_line_rest2: "version"                        { $$ = (*$1).size () + 2;
                                                       driver->record ()->version =
                                                         HTTP_Tools::Version2Type (*$1);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set version: \"%s\"\n"),
//                                                                   ACE_TEXT ((*$1).c_str ())));
                                                     };
                    | "end_of_fragment"              { $$ = $1;
                                                       yyclearin; };
head_rest2:         status_line_rest1 headers        { $$ = $1 + $2; };
status_line_rest1:  "status" status_line_rest2       { $$ = (*$1).size () + $2 + 1;
                                                       std::stringstream converter;
                                                       converter.str (*$1);
                                                       int status;
                                                       converter >> status;
                                                       driver->record ()->status =
                                                           static_cast<HTTP_Status_t> (status);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set status: %d\n"),
//                                                                   status));
                                                     };
                    | "end_of_fragment"              { $$ = $1;
                                                       yyclearin; };
status_line_rest2:  "reason"                         { $$ = (*$1).size () + 2;
                                                       driver->record ()->reason = *$1;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set reason: \"%s\"\n"),
//                                                                   ACE_TEXT ((*$1).c_str ())));
                                                     };
                    | "end_of_fragment"              { $$ = $1;
                                                       yyclearin; };
headers:            headers "header"                 { /* NOTE*: use right-recursion here to force early state reductions
                                                                 (i.e. parse headers). This is required so the scanner can
                                                                 act on any set transfer encoding. */
                                                       $$ = $1 + (*$2).size ();
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
                                                                     ACE_TEXT ("invalid HTTP header (was: \"%s\"), continuing\n"),
                                                                     ACE_TEXT ((*$2).c_str ())));
                                                       } // end IF
                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());

                                                       ACE_ASSERT (match_results[1].matched);
                                                       HTTP_Record* record_p = driver->record ();
                                                       ACE_ASSERT (record_p);
                                                       HTTP_HeadersIterator_t iterator =
                                                         record_p->headers.find (match_results[1]);
                                                       if (iterator != record_p->headers.end ())
                                                       {
                                                         ACE_DEBUG ((LM_WARNING,
                                                                     ACE_TEXT ("duplicate HTTP header (was: \"%s\"), continuing\n"),
                                                                     ACE_TEXT (match_results[1].str ().c_str ())));
                                                       } // end IF
                                                       ACE_ASSERT (match_results[2].matched);
                                                       ACE_ASSERT (!match_results[2].str ().empty ());
                                                       record_p->headers[match_results[1]] =
                                                         match_results[2];
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set header: \"%s\" to \"%s\"\n"),
//                                                                   ACE_TEXT (match_results[1].str ().c_str ()),
//                                                                   ACE_TEXT (match_results[2].str ().c_str ())));
                                                     };
                    |                                { $$ = 0; };
//                    | %empty                         { $$ = 0; };
body:               "body"                           { $$ = $1;
                                                       YYACCEPT; }; // *NOTE*: any following (entity) fragments will not be parsed
                    | "chunk" chunks headers "delimiter" { $$ = $1 + $2 + $3 + $4; // *TODO*: potential conflict here (i.e. incomplete chunk may be accepted)
                                                       YYACCEPT; };
                    |                                { $$ = 0;
                                                       YYACCEPT; }; // *TODO*: potential conflict here (i.e. incomplete chunk 
//                    | %empty                         { $$ = 0;
//                                                       YYACCEPT; }; // *TODO*: potential conflict here (i.e. incomplete chunk may be accepted)
chunks:             "chunk" chunks                   { $$ = $1 + $2; };
                    |                                { $$ = 0; };
//                    | %empty                         { $$ = 0; };
%%

/* void
yy::HTTP_Parser::error (const location_type& location_in,
                        const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::HTTP_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Parser::set"));

  yyscanner = context_in;
} */

void
yyerror (YYLTYPE* location_in,
         Net_IParser<HTTP_Record>* driver_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (location_in);
  ACE_ASSERT (driver_in);

  driver_in->error (*location_in, std::string (message_in));
}

void
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
    case VERSION:
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
}
