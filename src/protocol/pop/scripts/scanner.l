%top{
#undef YYTOKENTYPE
#include "pop_parser.h"

/*
#define YY_DECL                                               \
yy::POP_Parser::token_type                                    \
POP_Scanner_lex (yy::POP_Parser::semantic_type* yylval_param, \
                 yy::POP_Parser::location_type* yylloc_param, \
                 POP_IParser* driver,                         \
                 yyscan_t yyscanner) */
/* yytokentype */
#define YY_DECL                         \
int                                     \
POP_Scanner_lex (YYSTYPE* yylval_param, \
                 YYLTYPE* yylloc_param, \
                 POP_IParser* driver,   \
                 yyscan_t yyscanner)
// ... and declare it for the parser's sake
extern YY_DECL;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int POP_Scanner_reset (yyscan_t);
#ifdef __cplusplus
}
#endif /* __cplusplus */
//extern void POP_Scanner_set_column (int, yyscan_t);

//using namespace yy;
//#define YYLTYPE POP_Parser::location_type
//#define YYSTYPE POP_Parser::semantic_type
}

%{
#include <sstream>
#include <string>

#include "ace/OS_Memory.h"

#include "net_macros.h"

#include "pop_common.h"
#include "pop_iparser.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
//#define yyterminate() return yy::POP_Parser::token::END
#define yyterminate() return yytokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option prefix="POP_Scanner_"

%option header-file="pop_scanner.h" outfile="pop_scanner.cpp"
%option batch 8bit never-interactive yylineno
%option nodefault nostdinit yywrap nomain nounput
/* %option c++ yyclass="POP_Scanner" */
/* %option ansi-definitions ansi-prototypes bison-bridge */
/* *NOTE*: the 'line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
%option bison-locations
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
%option reentrant pointer
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, header-file extra-type'
*/
%option extra-type="POP_IParser*"
/* %option nounput */

/* *NOTE*: see RFC 1531 page 13ff. */
OCTET          [\x00-\xFF]
OCTET_NO_CR    [\x00-\xFF]{-}[\x0D]
DOT            \x2E
ERR            -ERR
OK             \+OK
CRLF           \x0D\x0A
MINUS          \x2D
PLUS           \x2B
SPACE          \x20

%s             reply_code
%s             text_begin
%s             text
%s             multiline_text
%s             multiline_text_begin
%s             multiline_text_dot

%{
/* handle locations */
/*#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;                        \
                         yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
                         yycolumn += yyleng; */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno;                        \
                       yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%%
%{
  //yylloc->step ();
%}

<INITIAL>{
{PLUS}                     {
                             BEGIN(reply_code);
                             yycolumn -= yyleng;
                             yyless (0); }
{MINUS}                    {
                             BEGIN(reply_code);
                             yycolumn -= yyleng;
                             yyless (0); }
} // end <INITIAL>
<reply_code>{
{OK}                       { driver->offset (yyleng);
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->append (yytext);
                             BEGIN(text_begin);
                             return yytokentype::REPLY_CODE;
                             /* return yy::POP_Parser::token::REPLY_CODE; */ }
{ERR}                      { driver->offset (yyleng);
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->append (yytext);
                             BEGIN(text_begin);
                             return yytokentype::REPLY_CODE;
                             /* return yy::POP_Parser::token::REPLY_CODE; */ }
} // end <reply_code>
<text_begin>{
{SPACE}+                   { driver->offset (yyleng);
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             if (driver->expectMultiline ())
                               BEGIN(multiline_text_begin);
                             else
                               BEGIN(text);
                           }
{CRLF}                     { driver->offset (yyleng);
                             if (driver->expectMultiline ())
                               BEGIN(multiline_text_begin);
                             else
                             {
                               BEGIN(reply_code);
                               return yytokentype::REPLY_TEXT_END;
                             }
                             /* return yy::POP_Parser::token::REPLY_TEXT_END; */ }
} // end <text_begin>
<multiline_text_begin>{
{DOT}/{CRLF}               { driver->offset (yyleng);
                             BEGIN(reply_code);
                             return yytokentype::REPLY_TEXT_END;
                             /* return yy::POP_Parser::token::REPLY_TEXT_END; */ }
{DOT}                      { driver->offset (yyleng); /* byte-stuffing */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             BEGIN(multiline_text); }
{OCTET}{1}                 { driver->offset (yyleng);
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->append (yytext);
                             BEGIN(multiline_text); }
} // end <multiline_text_begin>
<text>{
{CRLF}                     { driver->offset (yyleng);
                             BEGIN(reply_code);
                             return yytokentype::REPLY_TEXT_END;
                             /* return yy::POP_Parser::token::REPLY; */ }
{OCTET_NO_CR}*/{CRLF}      { driver->offset (yyleng);
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext);
                             return yytokentype::REPLY_TEXT;
                             /* return yy::POP_Parser::token::REPLY_TEXT; */ }
} // end <text>
<multiline_text>{
{CRLF}                     { driver->offset (yyleng);
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->append (yytext);
                             BEGIN(multiline_text_begin);
                             return yytokentype::REPLY_TEXT;
                             /* return yy::POP_Parser::token::REPLY_TEXT; */ }
{OCTET_NO_CR}*/{CRLF}      { driver->offset (yyleng);
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->append (yytext); }
} // end <multiline_text>
<<EOF>>                    { yyterminate(); }
<*>{OCTET}{1}              { /* *NOTE*: should not be reached, unless something
                                        went terribly wrong */
                             ACE_ASSERT (false);

                             // debug info
                             std::string error_message =
                               ACE_TEXT_ALWAYS_CHAR ("invalid character: \"");
                             error_message += yytext[0];
                             error_message += ACE_TEXT_ALWAYS_CHAR ("\", aborting");
                             driver->error (*yylloc, error_message);
                             yyterminate(); }
%%

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
POP_Scanner_reset (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::POP_Scanner_reset"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  POP_IParser* driver_p = POP_Scanner_get_extra (yyscanner);
  ACE_ASSERT (driver_p);

  BEGIN(INITIAL);

  return 0;
}

int
POP_Scanner_wrap (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::POP_Scanner_wrap"));

  // sanity check(s)
  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  ACE_ASSERT (yyg);
  POP_IParser* driver_p = POP_Scanner_get_extra (yyscanner);
  ACE_ASSERT (driver_p);

  if (!driver_p->isBlocking ())
    return 1; // not enough data, cannot proceed
//  if (driver_p->hasFinished ())
//  {
//    BEGIN(INITIAL);
//    return 1; // done
//  } // end IF

  // *NOTE*: there is more data
  // 1. gobble/save the rest
  // 2. switch buffers
  // 3. unput the rest
  // 4. continue scanning

  // step1
  //std::string the_rest;
  //the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
  //yyg->yy_c_buf_p += yyleng;
  //yyg->yy_hold_char = *yyg->yy_c_buf_p;
  //if (yy_flex_debug)
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("the rest: \"%s\" (%d byte(s))\n"),
  //              ACE_TEXT (the_rest.c_str ()),
  //              the_rest.size ()));

  // step2
  if (!driver_p->switchBuffer ())
  {
    // *NOTE*: most probable reason: received session end message
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_IParser::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  // step3
  //for (std::string::reverse_iterator iterator = the_rest.rbegin ();
  //     iterator != the_rest.rend ();
  //     ++iterator)
  //  unput (*iterator);

  // step4
  //yyg->yy_did_buffer_switch_on_eof = 1;
  // yymore ();

  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
