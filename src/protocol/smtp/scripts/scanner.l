%top{
#include "smtp_parser.h"

/*
#define YY_DECL                                                 \
yy::SMTP_Parser::token_type                                     \
SMTP_Scanner_lex (yy::SMTP_Parser::semantic_type* yylval_param, \
                  yy::SMTP_Parser::location_type* yylloc_param, \
                  SMTP_ParserDriver* driver,                    \
                  yyscan_t yyscanner) */
/* yytokentype */
#define YY_DECL                              \
int                                          \
SMTP_Scanner_lex (YYSTYPE* yylval_param,     \
                  YYLTYPE* yylloc_param,     \
                  SMTP_ParserDriver* driver, \
                  yyscan_t yyscanner)
// ... and declare it for the parser's sake
extern YY_DECL;

extern int SMTP_Scanner_reset (yyscan_t);
extern void SMTP_Scanner_set_column (int, yyscan_t);

//using namespace yy;
//#define YYLTYPE SMTP_Parser::location_type
//#define YYSTYPE SMTP_Parser::semantic_type
}

%{
#include <sstream>
#include <string>

#include "ace/OS_Memory.h"

#include "net_macros.h"

#include "smtp_common.h"
#include "smtp_parser_driver.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
//#define yyterminate() return yy::DHCP_Parser::token::END
#define yyterminate() return yytokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option prefix="SMTP_Scanner_"

%option header-file="smtp_scanner.h" outfile="smtp_scanner.cpp"
%option batch 8bit never-interactive yylineno
%option nodefault nostdinit yywrap nomain nounput
/* %option c++ yyclass="SMTP_Scanner" */
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
%option extra-type="SMTP_ParserDriver*"
/* %option nounput */

/* *NOTE*: see RFC 1531 page 13ff. */
OCTET          [\x00-\xFF]
/* STRING         [[:alpha:]\x00] */
DIGIT          [[:digit:]]
/* NUL            [\x00] */
CRLF           \x0D\x0A
MINUS          \x2D
SPACE          \x20

%s             reply_code
%s             text_begin
%s             text
%s             multiline_text

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
  yy_flex_debug = driver->getDebugScanner ();
  std::istringstream converter;
%}

<INITIAL>{
{DIGIT}{1}                 {
                             BEGIN(reply_code);
                             yycolumn -= yyleng;
                             yyless (0); }
} // end <INITIAL>
<reply_code>{
{DIGIT}{3}                 { driver->offset_ += yyleng;
                             converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                             converter.clear ();
                             converter.str (yytext);
                             converter >> yylval->ival;
                             BEGIN(text_begin);
                             return yytokentype::REPLY_CODE;
                             /* return yy::SMTP_Parser::token::REPLY_CODE; */ }
} // end <reply_code>
<text_begin>{
{CRLF}                     { // *NOTE*: this is a protocol violation
                             driver->offset_ += yyleng;
                             return yytokentype::REPLY;
                             /* return yy::SMTP_Parser::token::REPLY; */ }
{MINUS}                    { driver->offset_ += yyleng;
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             BEGIN(multiline_text);
                           }
{SPACE}                    { driver->offset_ += yyleng;
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             BEGIN(text);
                           }
} // end <text_begin>
<multiline_text>{
{CRLF}                     { driver->offset_ += yyleng;
                             BEGIN(reply_code);
                             return yytokentype::REPLY_TEXT;
                             /* return yy::SMTP_Parser::token::REPLY_TEXT; */ }
{OCTET}*/{CRLF}            { driver->offset_ += yyleng;
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext);
                           }
} // end <multiline_text>
<text>{
{CRLF}                     { driver->offset_ += yyleng;
                             BEGIN(reply_code);
                             return yytokentype::REPLY;
                             /* return yy::SMTP_Parser::token::REPLY; */ }
{OCTET}*/{CRLF}            { driver->offset_ += yyleng;
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext);
                             return yytokentype::REPLY_TEXT;
                             /* return yy::SMTP_Parser::token::REPLY_TEXT; */ }
} // end <text>
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

int
SMTP_Scanner_reset (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::SMTP_Scanner_reset"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  SMTP_ParserDriver* driver = SMTP_Scanner_get_extra (yyscanner);

  // sanity check(s)
  ACE_ASSERT (driver);
  ACE_ASSERT (driver->fragment_);

  BEGIN(INITIAL);

  return 0;
}
