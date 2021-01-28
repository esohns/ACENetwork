%top{
//  #include "ace/Synch.h"
  #include "common_iparser.h"

  #include "m3u_parser.h"

#if defined (M3U_IN_HEADER)
class M3U_IParser;
#else
#define YY_STRUCT_YY_BUFFER_STATE
  #include "m3u_scanner.h"

  #include "m3u_iparser.h"
#undef YY_STRUCT_YY_BUFFER_STATE
#endif

#define YY_DECL                                     \
int                                                 \
M3U_lex (yy::parser::semantic_type* yylval,   \
         yy::parser::location_type* location, \
         M3U_IParser* parser,                 \
         yyscan_t yyscanner)
// ... and declare it for the parser's sake
extern YY_DECL;

/*#define YY_EXTRA_TYPE*/
}

%{
  // *WORKAROUND*
//  #include <iostream>
  #include <sstream>
  //using namespace std;
  //// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
  ////                   a problem in conjunction with the standard include headers
  ////                   when ACE_USES_OLD_IOSTREAMS is defined
  ////                   --> include the necessary headers manually (see above), and
  ////                       prevent ace/iosfwd.h from causing any harm
  //#define ACE_IOSFWD_H

//  #include "ace/Synch.h"
  #include "m3u_scanner.h"
  #include "m3u_parser.h"
  #include "test_i_m3u_parser_driver.h"

  // the original yyterminate() macro returns int. Since this uses Bison 3
  // variants as tokens, redefine it to change type to `Parser::semantic_type`
  #define yyterminate() return yy::parser::token::END;

  // this tracks the current scanner location. Action is called when length of
  // the token is known
  #define YY_USER_ACTION location->columns (yyleng);
%}

%option backup
%option batch
%option never-interactive
%option stack
%option pointer

%option nodefault
%option nomain
%option nostdinit
/*%option nounput*/
/*%option noyywrap*/

%option 8bit
%option align
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option read
%option full
%option ecs
%option reentrant

%option debug
%option perf-report
%option verbose
%option warn
%option yylineno

/*%option ansi-definitions ansi-prototypes*/
/*%option c++*/
%option header-file="m3u_scanner.h"
%option outfile="m3u_scanner.cpp"
%option prefix="M3U_"
/*%option yyclass="Common_BencodingScanner"*/

/* *NOTE*: for protcol specification, see:
           - http://bittorrent.org/beps/bep_0003.html
           - https://wiki.theory.org/BitTorrentSpecification */

DIGIT                             [[:digit:]]
OCTET                             [\x00-\xFF]
TO_COMMA                          [^,]
TO_END_OF_LINE                    [^\x0A]

%s state_x
%s state_inf
%s state_inf_key_value
%s state_inf_title
%s state_inf_url

%{
/* handle locations */
/*#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
                       yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng; */
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%% /* end of definitions */

%{
  /* code to place at the beginning of yylex() */

  // reset location
  location->step ();

  std::stringstream converter;

  // sanity check(s)
  ACE_ASSERT (parser);
  ACE_Message_Block* message_block_p = parser->buffer ();
  ACE_ASSERT (message_block_p);
  ACE_ASSERT (yyscanner);

//  location.columns (yyleng);
%}

<INITIAL>{
"#EXTM3U\x0A"          { parser->offset (yyleng);
                         ACE_NEW_NORETURN (yylval->lval,
                                           M3U_Playlist_t);
                         ACE_ASSERT (yylval->eval);
                         return yy::parser::token::BEGIN_ELEMS; }
"#EXT-X-TARGETDURATION:" { parser->offset (yyleng);
                           BEGIN (state_x); }
"#EXT-X-ALLOW-CACHE:"  { parser->offset (yyleng);
                         BEGIN (state_x); }
"#EXT-X-VERSION:"      { parser->offset (yyleng);
                         BEGIN (state_x); }
"#EXT-X-MEDIA-SEQUENCE:" { parser->offset (yyleng);
                           BEGIN (state_x); }
"#EXTINF:"             { parser->offset (yyleng);
                         BEGIN (state_inf);
                         ACE_NEW_NORETURN (yylval->eval,
                                           struct M3U_Element);
                         ACE_ASSERT (yylval->eval);
                         return yy::parser::token::BEGIN_INF; }
\x0A                   { parser->offset (yyleng); }
} // end <INITIAL>
<state_x>{
{TO_END_OF_LINE}+/\x0A { parser->offset (yyleng); }
\x0A                   { parser->offset (yyleng);
                         BEGIN (INITIAL); }
} // end <state_x>
<state_inf>{
{TO_COMMA}+/,          { parser->offset (yyleng);
                         converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                         converter.clear ();
                         converter << yytext;
                         converter >> yylval->ival;
                         return yy::parser::token::LENGTH; }
","                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         BEGIN (state_inf_title); }
} // end <state_inf>
<state_inf_title>{
{TO_END_OF_LINE}+/\x0A { parser->offset (yyleng);
                         ACE_NEW_NORETURN (yylval->sval,
                                           std::string);
                         ACE_ASSERT (yylval->sval);
                         yylval->sval->push_back (yytext[0]);
                         return yy::parser::token::TITLE; }
\x0A                   { parser->offset (yyleng);
                         BEGIN (state_inf_url); }
} // end <state_inf_title>
<state_inf_url>{
{TO_END_OF_LINE}+/\x0A { parser->offset (yyleng);
                         ACE_NEW_NORETURN (yylval->sval,
                                           std::string);
                         ACE_ASSERT (yylval->sval);
                         yylval->sval->append (yytext);
                         return yy::parser::token::URL; }
\x0A                   { parser->offset (yyleng);
                         BEGIN (INITIAL);
                         return yy::parser::token::END_OF_ELEMENT; }
} // end <state_inf_url>
<<EOF>>                { return yy::parser::token::END; }
<*>{OCTET}            { /* *TODO*: use (?s:.) ? */
                        if (!parser->isBlocking ())
                          return yy::parser::token::END;
                        // wait for more data fragment(s)
                        if (!parser->switchBuffer ())
                        { // *NOTE*: most probable reason: connection
                          //         has been closed --> session end
                          ACE_DEBUG ((LM_DEBUG,
                                      ACE_TEXT ("failed to Common_IParser::switchBuffer(), returning\n")));
                          yyterminate(); // not enough data, cannot proceed
                        } // end IF
                        yyless (0); }
%% /* end of rules */

#include "stream_macros.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
M3U_wrap (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::M3U_wrap"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  M3U_ParserDriver* driver =
    static_cast<M3U_ParserDriver*> (M3U_get_extra (yyscanner));

  // sanity check(s)
  ACE_ASSERT (driver);
  if (driver->hasFinished ())
    return 1; // done
//  ACE_ASSERT (driver->fragment_);

  // *NOTE*: there is more data
  // 1. gobble/save the rest
  // 2. switch buffers
  // 3. unput the rest
  // 4. continue scanning

  // step1
  std::string the_rest;
//  the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
  yyg->yy_c_buf_p += yyleng;
  yyg->yy_hold_char = *yyg->yy_c_buf_p;
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("the rest: \"%s\"\n"),
//              ACE_TEXT (the_rest.c_str ())));

  // step2
  if (!driver->switchBuffer ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_ParserBase_T::switchBuffer() [remaining data was: \"%s\"], aborting\n"),
                ACE_TEXT (the_rest.c_str ())));
    return 1;
  } // end IF

  // step3
//  for (std::string::reverse_iterator iterator = the_rest.rbegin ();
//       iterator != the_rest.rend ();
//       ++iterator)
//    unput (*iterator);

  // step4
  // yymore ();
  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
