/*%require                          "3.4.1"*/
%require                          "3.0"
/* %file-prefix                      "" */
/*%language                         "c++"*/
%language                         "C"
%locations
%no-lines
/*%skeleton                         "glr.c"*/
%skeleton                         "lalr1.cc"
%verbose
/* %yacc */

%defines                          "m3u_parser.h"
%output                           "m3u_parser.cpp"

/*%define "parser_class_name"       "Bencoding_Parser"*/
/* *NOTE*: this is the namespace AND the (f)lex prefix */
/*%name-prefix                      "y"*/
/*%pure-parser*/
/*%token-table*/
%define parse.error               verbose
%debug

%code top {
#include "stdafx.h"

#include "m3u_parser.h"
}

/* %define location_type */
/* %define api.location.type         {} */
/* %define api.namespace             {yy} */
/* %define api.prefix                {yy} */
//%define api.pure                    true
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/*%define api.token.constructor*/
%define api.token.prefix            {}
/*%define api.value.type            variant*/
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/*%define parse.assert              {true}*/
/*%define parse.error               verbose*/
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/*%define parser_class_name         {BitTorrent_Bencoding_Parser}*/
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
/*%define parse.trace               {true}*/

%code requires {
// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef M3U_Parser_H
//#define M3U_Parser_H

#include "m3u_iparser.h"

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually; apparently, there is no easy way to add the export
//         symbol to the declaration
#define YYDEBUG 1
/*extern int BitTorrent_Export yydebug;*/
#define YYERROR_VERBOSE 1
}

// calling conventions / parameter passing
%parse-param              { M3U_IParser* iparser }
%parse-param              { M3U_IScanner_t* iscanner }
// *NOTE*: cannot use %initial-action, as it is scoped
%lex-param                { M3U_IParser* iparser }
%lex-param                { yyscan_t iscanner->getR ().context }

%initial-action
{
  // initialize the location
  @$.initialize (NULL);
}

// symbols
%union
{
  ACE_INT32       ival;
  std::string*    sval;
  M3U_Element*    eval;
  M3U_Playlist_t* lval;
}
//%token <ACE_INT32>   INTEGER;
//%token <std::string> STRING;
//%token <M3U_Element> ELEMENT;
//%token EOF 0;

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
/*#if defined (YYDEBUG)
#include <iostream>
#endif*/
/*#include <regex>*/
#include <sstream>
#include <string>

// *WORKAROUND*
/*using namespace std;*/
// *IMPORTANT NOTE*: several ACE headers include ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
/*#define ACE_IOSFWD_H*/

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_macros.h"

#include "m3u_scanner.h"
#include "test_i_m3u_parser_driver.h"
#include "test_i_m3u_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex M3U_lex
//#define yylex iscanner->lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

// terminals
%token
              END_OF_ELEMENT       "element_end"
              END 0                "end"
;
%token <sval> KEY                  "key"
%token <ival> LENGTH               "length"
%token <sval> TITLE                "title"
%token <sval> URL                  "URL"
%token <sval> VALUE                "value"
%token <eval> BEGIN_EXTINF         "begin_ext_inf"
%token <eval> BEGIN_EXT_STREAM_INF "begin_ext_stream_inf"
%token <lval> BEGIN_ELEMS          "begin_elements"

// non-terminals
%nterm  <ival> elements key_values key_value
%nterm  <eval> element
%nterm  <sval> ext_stream_inf_rest_1 ext_stream_inf_rest_3 ext_stream_inf_rest_4
%nterm  <sval> ext_inf_rest_1 ext_inf_rest_2 ext_inf_rest_3 ext_inf_rest_4
%nterm  <lval> playlist

/*%precedence element inf_rest_1 inf_rest_2 inf_rest_3 inf_rest_4
%precedence playlist elements*/

%code provides {
/*void BitTorrent_Export yysetdebug (int);
void yyerror (YYLTYPE*, M3U_IParser*, yyscan_t, const char*);
int yyparse (M3U_IParser*, yyscan_t);
void yyprint (FILE*, yytokentype, YYSTYPE);*/

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // M3U_Parser_H
}

/*%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), $$->c_str ()); } <sval>
%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %d"), $$); } <ival>
%destructor { delete $$; $$ = NULL; } <sval>
%destructor { $$ = 0; } <ival>*/
/*%destructor { $$ = 0; } <eval>
%destructor { $$ = 0; } <lval>*/
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */
/*%printer    { yyoutput << $$; } <*>;*/
%printer    { debug_stream () << $$; } <ival>
%printer    { debug_stream () << *$$; } <sval>
/*%printer    { yyoutput << Test_I_M3U_Tools::ElementToString (*$$); } <eval>*/
%printer    { if ($$) debug_stream () << Test_I_M3U_Tools::ElementToString (*$$); } <eval>
%printer    { if ($$) debug_stream () << Test_I_M3U_Tools::PlaylistToString (*$$); } <lval>

%%
%start            playlist;
playlist:         "begin_elements" {
                    iparser->setP ($1);
                  } elements {
                    M3U_Playlist_t& playlist_r = iparser->current ();
                    M3U_Playlist_t* playlist_p = &playlist_r;
                    try {
                      iparser->record (playlist_p);
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in M3U_IParser::record(), continuing\n")));
                    }
                    YYACCEPT;
                  }

elements:         elements element
/*                  |                    { }*/
                  | %empty             { }
element:          "begin_ext_inf" {
                    iparser->setP_2 ($1);
                  } ext_inf_rest_1 { }
                  | "begin_ext_stream_inf" {
                    iparser->setP_2 ($1);
                  } ext_stream_inf_rest_1 { }

ext_inf_rest_1:   "length" {
                    struct M3U_Element& element_r = iparser->current_2 ();
                    element_r.Length = $1; } ext_inf_rest_2 { }
ext_inf_rest_2:   "title" {
                    struct M3U_Element& element_r = iparser->current_2 ();
                    element_r.Title = *$1; } ext_inf_rest_3 { }
                  | ext_inf_rest_3 { }
ext_inf_rest_3:   "URL" {
                    struct M3U_Element& element_r = iparser->current_2 ();
                    element_r.URL = *$1; } ext_inf_rest_4 { }
ext_inf_rest_4:   "element_end" {
                    M3U_Playlist_t& playlist_r = iparser->current ();
                    struct M3U_Element& element_r = iparser->current_2 ();
                    playlist_r.push_back (element_r); }
/*                  |                    { }*/
                  | %empty             { }

ext_stream_inf_rest_1: key_values ext_stream_inf_rest_3 { }
key_values: key_values key_value
/*            |                    { }*/
            | %empty             { }
key_value:   "key" {
  struct M3U_Element& element_r = iparser->current_2 ();
  element_r.key = *$1; } "value" {
  struct M3U_Element& element_r = iparser->current_2 ();
  element_r.keyValues.push_back (std::make_pair (element_r.key, *$3)); element_r.key.clear (); }
ext_stream_inf_rest_3:   "URL" {
  struct M3U_Element& element_r = iparser->current_2 ();
  element_r.URL = *$1; } ext_stream_inf_rest_4 { }
ext_stream_inf_rest_4:   "element_end" {
  M3U_Playlist_t& playlist_r = iparser->current ();
  struct M3U_Element& element_r = iparser->current_2 ();
  playlist_r.push_back (element_r); }
/*                  |                    { }*/
| %empty             { }
%%

void
yy::parser::error (const location_type& location_in,
                   const std::string& message_in)
{
  COMMON_TRACE (ACE_TEXT ("yy::parser::error"));

  // sanity check(s)
  ACE_ASSERT (iparser);

  try {
    iparser->error (location_in, message_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Bencoding_IParser_t::error(), continuing\n")));
  }
}

void
yyerror (yy::location* location_in,
         M3U_IParser_t* iparser_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

//  ACE_UNUSED_ARG (location_in);
  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (iparser_in);

  try {
    iparser_in->error (*location_in, message_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Bencoding_IParser::error(), continuing\n")));
  }
}

void
yyprint (FILE* file_in,
         yy::parser::token::yytokentype type_in,
         yy::parser::semantic_type value_in)
{
  COMMON_TRACE (ACE_TEXT ("::yyprint"));

  int result = -1;

  std::string format_string;
  switch (type_in)
  {
    case yy::parser::token::TITLE:
    case yy::parser::token::URL:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case yy::parser::token::LENGTH:
    case yy::parser::token::END:
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
