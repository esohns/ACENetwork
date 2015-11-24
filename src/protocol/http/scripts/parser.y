%defines                          "http_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
/* %no-lines */
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
/* %name-prefix                      "yy" */
%define api.prefix                {yy}
/* %pure-parser */
%define api.pure                  true
/* %define api.push-pull             {pull} */
/* %define api.token.constructor */
%define api.token.prefix          {}
/* %define api.value.type            variant */
/* %define api.value.union.name      YYSTYPE */
%define lr.default-reduction      most
%define lr.keep-unreachable-state false
%define lr.type                   lalr

/* %define parse.assert              {true} */
/* %error-verbose */
%define parse.error               verbose
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/* %define parser_class_name         {HTTP_Parser} */
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
/* %debug */
%define parse.trace               {true}

%code requires {
#include <cstdio>
#include <string>

enum yytokentype;
class HTTP_ParserDriver;
//class HTTP_Scanner;
struct YYLTYPE;
union YYSTYPE;

typedef void* yyscan_t;

//#define YYERROR_VERBOSE
void yyerror (YYLTYPE*, HTTP_ParserDriver*, yyscan_t, const char*);
void yyprint (FILE*, yytokentype, YYSTYPE);
}

// calling conventions / parameter passing
/* %parse-param              { HTTP_ParserDriver* driver }
%parse-param              { yyscan_t yyscanner }
%lex-param                { YYSTYPE* yylval }
%lex-param                { YYLTYPE* yylloc } */
%param                    { HTTP_ParserDriver* driver }
%param                    { yyscan_t yyscanner }

%initial-action
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  //@$.begin.filename = @$.end.filename = &driver->file;
  ACE_OS::memset (&@$, 0, sizeof (YYLTYPE));

  // initialize the token value container
  $$.ival = 0;
  $$.sval = NULL;
};

// symbols
%union
{
  int          ival;
  std::string* sval;
};
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

#include "http_parser_driver.h"
#include "http_record.h"
#include "http_scanner.h"
#include "http_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex HTTP_Scanner_lex

#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <sval> METHOD      "method"
%token <sval> VERSION     "version"
%token <sval> REQUEST     "request_line"
%token <sval> RESPONSE    "status_line"
%token <sval> HEADER      "header"
%token <ival> DELIMITER   "delimiter"
%token <ival> BODY        "body"
/* %type  <sval> METHOD VERSION REQUEST RESPONSE HEADER
%type  <ival> DELIMITER BODY */
/* %token <std::string> METHOD      "method"
%token <std::string> VERSION     "version"
%token <std::string> REQUEST     "request_line"
%token <std::string> RESPONSE    "status_line"
%token <std::string> HEADER      "header"
%token <std::string> DELIMITER   "delimiter"
%token <int>         BODY        "body" */
%token               END 0       "end_of_message"

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival> */
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (*$$).c_str ()); } <sval>
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %d"), $$); }  <ival>
%destructor               { delete $$; $$ = NULL; }   <sval>
%destructor               { $$ = 0; }                 <ival>
%destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <>

%%
%start        message;
/* %nonassoc                 ':' '!' '@'; */

message:      head "delimiter" body "end_of_message" /* default */
head:         "method" request                       {
                                                       driver->record_->method_ =
                                                         HTTP_Tools::Method2Type (*$1);
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set method: \"%s\"\n"),
                                                                   ACE_TEXT ((*$1).c_str ()))); };
              | "version" response                   { driver->record_->version_ =
                                                         HTTP_Tools::Version2Type (*$1);
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set version: \"%s\"\n"),
                                                                   ACE_TEXT ((*$1).c_str ()))); };
request:      "request_line" headers                 { /* *TODO*: modify the scanner so it emits the proper fields itself */
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^([[^\\s]+)\\s([[^\\s]+)\\sHTTP/([[:digit:]]+)\\.([[:digit:]]+)$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*$1,
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP request-line (was: \"%s\"), continuing\n"),
                                                                     ACE_TEXT ((*$1).c_str ())));
                                                       } // end IF
                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());

                                                       ACE_ASSERT (match_results[1].matched);
                                                       driver->record_->method_ =
                                                         HTTP_Tools::Method2Type (match_results[1]);
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set method: \"%s\"\n"),
                                                                   ACE_TEXT (match_results[1].str ().c_str ())));
                                                       ACE_ASSERT (match_results[2].matched);
                                                       driver->record_->URI_ = match_results[2];
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set URI: \"%s\"\n"),
                                                                   ACE_TEXT (match_results[2].str ().c_str ())));
                                                       ACE_ASSERT (match_results[3].matched);
                                                       driver->record_->version_ =
                                                         HTTP_Tools::Version2Type (match_results[3]);
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set version: \"%s\"\n"),
                                                                   ACE_TEXT (match_results[3].str ().c_str ()))); };
response:     "status_line" headers                  { /* *TODO*: modify the scanner so it emits the proper fields itself */
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^([[^\\s]+)\\s([[:digit:]{3})\\s(.+)$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*$1,
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP status-line (was: \"%s\"), continuing\n"),
                                                                     ACE_TEXT ((*$1).c_str ())));
                                                       } // end IF
                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());

                                                       ACE_ASSERT (match_results[1].matched);
                                                       driver->record_->version_ =
                                                         HTTP_Tools::Version2Type (match_results[1]);
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set version: \"%s\"\n"),
                                                                   ACE_TEXT (match_results[1].str ().c_str ())));
                                                       ACE_ASSERT (match_results[2].matched);
                                                       std::stringstream converter;
                                                       converter.str (match_results[2].str ().c_str ());
                                                       int status;
                                                       converter >> status;
                                                       driver->record_->status_ =
                                                         static_cast<HTTP_Status_t> (status);
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set status: \"%s\"\n"),
                                                                   ACE_TEXT (match_results[2].str ().c_str ())));
                                                       ACE_ASSERT (match_results[3].matched);
                                                       /* driver->record_->reason_ = match_results[3];
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set reason: \"%s\"\n"),
                                                                   ACE_TEXT (match_results[3].str ().c_str ()))); */ };
headers:      "header" headers                       { /* *TODO*: modify the scanner so it emits the proper fields itself */
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^([^:]+):\\s(.+)$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*$1,
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP header (was: \"%s\"), continuing\n"),
                                                                     ACE_TEXT ((*$1).c_str ())));
                                                       } // end IF
                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());

                                                       ACE_ASSERT (match_results[1].matched);
                                                       HTTP_HeadersIterator_t iterator =
                                                         driver->record_->headers_.find (match_results[1]);
                                                       ACE_ASSERT (iterator == driver->record_->headers_.end ());
                                                       ACE_ASSERT (match_results[2].matched);
                                                       ACE_ASSERT (!match_results[2].str ().empty ());
                                                       driver->record_->headers_[match_results[1]] =
                                                         match_results[2];
                                                       ACE_DEBUG ((LM_DEBUG,
                                                                   ACE_TEXT ("set header: \"%s\" to \"%s\"\n"),
                                                                   ACE_TEXT (match_results[1].str ().c_str ()),
                                                                   ACE_TEXT (match_results[2].str ().c_str ()))); };
              | %empty                               /* empty */ /* *TODO*: enforce the standard here */
body:         "body"                                 /* default */
              | %empty                               /* empty */
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
         HTTP_ParserDriver* driver_in,
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
    case VERSION:
    case REQUEST:
    case RESPONSE:
    case HEADER:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case BODY:
    case DELIMITER:
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
