%defines                          "ftp_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "ftp_parser.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/* %skeleton                         "lalr1.cc" */
%token-table
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
#undef YYTOKENTYPE
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
/* %define parser_class_name         {FTP_Parser} */
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               {true} */

%code requires {
#include <cstdio>
#include <string>

class FTP_IParser;
//class FTP_Scanner;
struct YYLTYPE;
//union YYSTYPE;

union yytoken
{
  ACE_UINT16   ival;
  std::string* sval;
};
typedef yytoken yytoken_t;
#define YYSTYPE yytoken_t

typedef void* yyscan_t;

//#define YYERROR_VERBOSE
extern void yyerror (YYLTYPE*, FTP_IParser*, yyscan_t, const char*);
extern int yyparse (FTP_IParser*, yyscan_t);

#undef YYPRINT
//extern void yyprint (FILE*, yytokentype, YYSTYPE);
}

// calling conventions / parameter passing
%parse-param              { FTP_IParser* driver }
%parse-param              { yyscan_t yyscanner }
/* %lex-param                { YYSTYPE* yylval } */
/* %lex-param                { YYLTYPE* yylloc } */
%lex-param                { FTP_IParser* driver }
%lex-param                { yyscan_t yyscanner }

%initial-action
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  //@$.begin.filename = @$.end.filename = &driver->file;
  ACE_OS::memset (&@$, 0, sizeof (YYLTYPE));

  // initialize the token value container
  $$.ival = 0;
};

// symbols
%union
{
  ACE_UINT16   ival;
  std::string* sval;
};
/* %token <int>         INTEGER; */

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif // YYDEBUG

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

#include "ftp_common.h"
#include "ftp_defines.h"
#include "ftp_parser_driver.h"
#include "ftp_scanner.h"
#include "ftp_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex FTP_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <ival> REPLY_CODE                                "reply_code"
%token <sval> REPLY_TEXT                                "text"
%token <ival> REPLY                                     "reply_end"
%token <ival> END 0                                     "end"

%type  <ival> message
%type  <ival> text_lines text_line multi_text_line final_text_line

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival> */
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), $$); }   <ival>
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), ($$->c_str ())); } <sval>
//%destructor               { ACE_OS::memset ($$, 0, 16); } <aval>
%destructor               { $$ = 0; }               <ival>
%destructor               { delete $$; $$ = NULL; } <sval>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start                   message;

message:                 "reply_code"                               { driver->current ().code = static_cast<SMTP_Code_t> ($1); }
                         text_lines "reply_end"                     { $$ = 3 + $3;
                                                                      struct SMTP_Record* record_p = &driver->current ();
                                                                      driver->record (record_p);
                                                                      YYACCEPT; }
text_lines:              text_line text_lines                       { $$ = $1 + $2; }
                         | /* empty */                              { $$ = 2; }
text_line:               multi_text_line
                         | final_text_line                          { $$ = $1; }
multi_text_line:         "reply_code" "text"                        { $$ = 3 + 1 + $2->length () + 2;
                                                                      driver->current ().text.push_back (*$2); }
final_text_line:         "text"                                     { $$ = 1 + $1->length () + 2;
                                                                      driver->current ().text.push_back (*$1); }
%%

/* void
yy::FTP_Parser::error (const location_type& location_in,
                       const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::FTP_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Parser::set"));

  yyscanner = context_in;
} */

void
yyerror (YYLTYPE* location_in,
         FTP_IParser* driver_in,
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

//void
//yyprint (FILE* file_in,
//         yytokentype type_in,
//         YYSTYPE value_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::yyprint"));
//
//  int result = -1;
//
//  std::string format_string;
//  switch (type_in)
//  {
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
//                  type_in));
//      return;
//    }
//  } // end SWITCH
//}
