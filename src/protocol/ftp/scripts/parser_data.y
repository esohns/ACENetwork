%defines                          "ftp_parser_data.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "ftp_parser_data.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/* %skeleton                         "lalr1.cc" */
%token-table
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
#undef YYTOKENTYPE

// *TODO*: this shouldn't be necessary
#define zzlex FTP_Scanner_Data_lex
}

/* %define location_type */
/* %define api.location.type         {} */
/* %define namespace                 {yy} */
/* %define api.namespace             {yy} */
%name-prefix                      "zz"
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
/* %define parser_class_name         {FTP_Parser_Data} */
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               {true} */

%code requires {
#include <cstdio>
#include <string>

class FTP_IParserData;
//class FTP_Scanner;
struct YYLTYPE;
//union YYSTYPE;

union zztoken
{
  ACE_UINT16   ival;
  std::string* sval;
};
typedef zztoken zztoken_t;
#define YYSTYPE zztoken_t

typedef void* yyscan_t;

//#define YYERROR_VERBOSE
extern void yyerror (YYLTYPE*, FTP_IParserData*, yyscan_t, const char*);
extern int yyparse (FTP_IParserData*, yyscan_t);

#undef YYPRINT
//extern void yyprint (FILE*, yytokentype, YYSTYPE);
}

// calling conventions / parameter passing
%parse-param              { FTP_IParserData* driver }
%parse-param              { yyscan_t yyscanner }
/* %lex-param                { YYSTYPE* yylval } */
/* %lex-param                { YYLTYPE* yylloc } */
%lex-param                { FTP_IParserData* driver }
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
#include "ftp_parser_data_driver.h"
#include "ftp_scanner_data.h"
#include "ftp_tools.h"

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <sval> LIST_DIRECTORY                            "directory"
%token <sval> LIST_FILE                                 "file"
%token <ival> RAW_DATA                                  "data"
%token <ival> END 0                                     "end"

%type  <ival> stream
%type  <ival> items item

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
%start                   stream;

stream:                  items                                     { $$ = $1; }
items:                   item items                                { $$ = $1 + $2; }
                         | /* empty */                             { $$ = 0; }
item:                    "directory"                               { $$ = $1->length () + 2;
                                                                     driver->directory (*$1); }
                         | "file"                                  { $$ = $1->length () + 2;
                                                                     driver->file (*$1); }
                         | "data"                                  { $$ = $1;
                                                                     driver->data ();
                                                                     YYACCEPT; }
%%

/* void
zz::FTP_Parser_Data::error (const location_type& location_in,
                            const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Parser_Data::error"));

  driver->error (location_in, message_in);
}

void
zz::FTP_Parser_Data::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Parser_Data::set"));

  yyscanner = context_in;
} */

void
zzerror (YYLTYPE* location_in,
         FTP_IParserData* driver_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::zzerror"));

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
