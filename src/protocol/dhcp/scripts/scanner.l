%top{
#include "dhcp_parser.h"

/*
#define YY_DECL                                                 \
yy::DHCP_Parser::token_type                                     \
DHCP_Scanner_lex (yy::DHCP_Parser::semantic_type* yylval_param, \
                  yy::DHCP_Parser::location_type* yylloc_param, \
                  DHCP_ParserDriver* driver,                    \
                  yyscan_t yyscanner) */
/* yytokentype */
#define YY_DECL                              \
int                                          \
DHCP_Scanner_lex (YYSTYPE* yylval_param,     \
                  YYLTYPE* yylloc_param,     \
                  DHCP_ParserDriver* driver, \
                  yyscan_t yyscanner)
// ... and declare it for the parser's sake
YY_DECL;

void DHCP_Scanner_set_column (int, yyscan_t);

//using namespace yy;
//#define YYLTYPE DHCP_Parser::location_type
//#define YYSTYPE DHCP_Parser::semantic_type
}

%{
#include <sstream>
#include <string>

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "net_macros.h"

#include "dhcp_common.h"
#include "dhcp_parser_driver.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
//#define yyterminate() return yy::DHCP_Parser::token::END
#define yyterminate() return yytokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option header-file="dhcp_scanner.h" outfile="dhcp_scanner.cpp"
%option batch 8bit nodefault never-interactive nostdinit yylineno yywrap
/* %option c++ yyclass="DHCP_Scanner" */
/* %option ansi-definitions ansi-prototypes bison-bridge */
/* *NOTE*: the 'line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
%option ansi-definitions ansi-prototypes bison-locations
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
%option reentrant pointer prefix="DHCP_Scanner_" nomain
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, header-file extra-type'
*/
%option extra-type="DHCP_ParserDriver*"
/* %option nounput */

/* *NOTE*: see RFC 1531 page 13ff. */
OCTET          [\x00-\xFF]
STRING         [[:alpha:]\x00]
/* DIGIT          [[:digit:]] */
NUL            [\x00]

%s             header
%s             op
%s             htype
%s             hlen
%s             hops
%s             xid
%s             secs
%s             flags
%s             ciaddr
%s             yiaddr
%s             siaddr
%s             giaddr
%s             chaddr
%s             sname
%s             file_
%s             cookie
%s             option_tag
%s             option_length

%{
/* handle locations */
/*#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
                       yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng; */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
                       yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%%
%{
  //yylloc->step ();
  yy_flex_debug = driver->getDebugScanner ();

  std::istringstream converter;
  unsigned int option_size = 0;
%}

<INITIAL>{
{OCTET}                    {
                             BEGIN (header);
                             yycolumn -= yyleng;
                             /* yymore (); */ yyless (0); }
} // end <INITIAL>
<header>{
{OCTET}                    {
                             BEGIN (op);
                             yycolumn -= yyleng;
                             /* yymore (); */ yyless (0); }
} // end <header>
<op>{
{OCTET}{1}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (htype);
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::OP;
                             /* return yy::DHCP_Parser::token::OP; */ }
} // end <op>
<htype>{
{OCTET}{1}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (hlen);
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::HTYPE;
                             /* return yy::DHCP_Parser::token::VERSION; */ }
} // end <version>
<hlen>{
{OCTET}{1}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (hops);
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::HLEN;
                             /* return yy::DHCP_Parser::token::HEADER; */ }
} // end <hlen>
<hops>{
{OCTET}{1}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (xid);
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             /* *TODO*: error handling */
                             //ACE_ASSERT (yylval->ival);
                             return yytokentype::HOPS;
                             /* return yy::DHCP_Parser::token::VERSION; */ }
} // end <hops>
<xid>{
{OCTET}{4}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (secs);
                             yylval->ival =
                                 *reinterpret_cast<unsigned int*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::XID;
                             /* return yy::DHCP_Parser::token::STATUS; */ }
} // end <xid>
<secs>{
{OCTET}{2}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (flags);
                             yylval->ival =
                                 *reinterpret_cast<unsigned short*> (yytext);
                             /* *TODO*: error handling */
                             //ACE_ASSERT (yylval->ival);
                             return yytokentype::SECS;
                             /* return yy::DHCP_Parser::token::REASON; */ }
} // end <secs>
<flags>{
{OCTET}{2}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (ciaddr);
                             yylval->ival =
                                 *reinterpret_cast<unsigned short*> (yytext);
                             /* *TODO*: error handling */
                             //ACE_ASSERT (yylval->ival);
                             return yytokentype::FLAGS;
                             /* return yy::DHCP_Parser::token::HEADER; */ }
} // end <flags>
<ciaddr>{
{OCTET}{4}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (yiaddr);
                             yylval->ival =
                                 *reinterpret_cast<unsigned int*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::CIADDR;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <ciaddr>
<yiaddr>{
{OCTET}{4}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (siaddr);
                             yylval->ival =
                                 *reinterpret_cast<unsigned int*> (yytext);
                             /* *TODO*: error handling */
                             //ACE_ASSERT (yylval->ival);
                             return yytokentype::YIADDR;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <yiaddr>
<siaddr>{
{OCTET}{4}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (giaddr);
                             yylval->ival =
                                 *reinterpret_cast<unsigned int*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::SIADDR;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <siaddr>
<giaddr>{
{OCTET}{4}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (chaddr);
                             yylval->ival =
                                 *reinterpret_cast<unsigned int*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::GIADDR;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <giaddr>
<chaddr>{
{OCTET}{16}                { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             ACE_ASSERT (yyleng == DHCP_CHADDR_SIZE);
                             BEGIN (sname);
                             ACE_OS::memcpy (yylval->aval, yytext, DHCP_CHADDR_SIZE);
                             /* *TODO*: error handling */
                             return yytokentype::CHADDR;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <chaddr>
<sname>{
{OCTET}{64}                { driver->fragment_->rd_ptr (DHCP_SNAME_SIZE);
                             driver->offset_ += yyleng;
                             ACE_ASSERT (yyleng == DHCP_SNAME_SIZE);
                             BEGIN (file_);
                             yylval->ival = yyleng;
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             *yylval->sval = yytext;
                             return yytokentype::SNAME;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <sname>
<file_>{
{OCTET}{128}               { driver->fragment_->rd_ptr (DHCP_FILE_SIZE);
                             driver->offset_ += yyleng;
                             ACE_ASSERT (yyleng == DHCP_FILE_SIZE);
                             BEGIN (cookie);
                             yylval->ival = yyleng;
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             *yylval->sval = yytext;
                             return yytokentype::FILE_;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <file_>
<cookie>{
{OCTET}{4}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             ACE_ASSERT (yyleng == 4);
                             BEGIN (option_tag);
                             yylval->ival =
                               *reinterpret_cast<unsigned int*> (yytext);
                             /* *TODO*: error handling */
                             ACE_ASSERT (yylval->ival);
                             return yytokentype::COOKIE;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <cookie>
<option_tag>{
{OCTET}{1}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             yylval->ival =
                               *reinterpret_cast<unsigned char*> (yytext);
                             /* *TODO*: support other encodings */
                             if ((yylval->ival == 0) || (yylval->ival == 255))
                               return yytokentype::OPTION_TAG;

                             BEGIN (option_length);
                             return yytokentype::OPTION_TAG;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <option_tag>
<option_length>{
{OCTET}{1}                 { driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             option_size =
                               *reinterpret_cast<unsigned char*> (yytext);

                             /* undo the effects of YY_DO_BEFORE_ACTION */                             *yy_cp = yyg->yy_hold_char;
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->append (driver->fragment_->rd_ptr (), option_size);

                             // skip over value bytes                             yyg->yy_c_buf_p += (yyleng + option_size);                             yyg->yy_hold_char = *yyg->yy_c_buf_p;//                             YY_FLUSH_BUFFER; // --> refill scan buffer//                             char c;//                             for (unsigned int i = 0;//                                  i <= option_size;//                                  ++i)//                               c = yyinput (yyscanner);//                             ACE_UNUSED_ARG (c);                             driver->fragment_->rd_ptr (option_size);                             driver->offset_ += option_size;
                             BEGIN (option_tag);
                             return yytokentype::OPTION_VALUE;
                             /* return yy::DHCP_Parser::token::BODY; */ }
} // end <option_length>
<<EOF>>                    { yyterminate(); }
<*>{OCTET}                 { /* *TODO*: use (?s:.) ? */

                             /* *NOTE*: should not be reached, unless something
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
DHCP_Scanner_wrap (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::DHCP_Scanner_wrap"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  DHCP_ParserDriver* driver = DHCP_Scanner_get_extra (yyscanner);

  // sanity check(s)
  ACE_ASSERT (driver);
  if (driver->finished_)
    return 1; // done
  ACE_ASSERT (driver->fragment_);

  // *NOTE*: there is more data
  // 1. gobble/save the rest
  // 2. switch buffers
  // 3. unput the rest
  // 4. continue scanning

  // step1
  std::string the_rest;
  the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
  yyg->yy_c_buf_p += yyleng;
  yyg->yy_hold_char = *yyg->yy_c_buf_p;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("the rest: \"%s\"\n"),
              ACE_TEXT (the_rest.c_str ())));

  // step2
  if (!driver->switchBuffer ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DHCP_ParserDriver::switchBuffer() [remaining data was: \"%s\"], aborting\n"),
                ACE_TEXT (the_rest.c_str ())));
    return 1;
  } // end IF

  // step3
  for (std::string::reverse_iterator iterator = the_rest.rbegin ();
       iterator != the_rest.rend ();
       ++iterator)
    unput (*iterator);

  // step4
  // yymore ();
  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
