/* *NOTE*: this version of the scanner is rather straight-forward, as it assumes
           a continuous buffer containing the whole frame and hence makes no
           provisions for switching buffers, backing-up and the like. In this
           sense, it is optimized for speed and efficiency */

%{
//#include <stdlib.h>

// *WORKAROUND*
//#include <iostream>
#include <sstream>
#include <string>

// *WORKAROUND*
//using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
//#define ACE_IOSFWD_H

#include "ace/ace_wchar.h"
#include "ace/OS_Memory.h"

#include "ace/Synch.h"
#include "irc_module_parser.h"
#include "irc_parser_driver.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
#define yyterminate() return yy::IRC_Parser::token::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option 8bit align backup batch
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option debug nodefault fast never-interactive perf-report noread
/* %option c++ yyclass="IRC_Scanner" stack noline bison-bridge */
/*         bison-locations */
%option verbose warn
%option pointer
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd, */
/*                   ansi-definitions, ansi-prototypes, header-file' */
%option reentrant
%option header-file="irc_scanner.h"
/* *IMPORTANT NOTE*: 'yylineno' is not compatible with 'read' */
%option nomain nostdinit noyywrap
%option outfile="irc_scanner.cpp" prefix="IRC_Scanner_"
%option nounput
%option extra-type="IRC_ParserDriver*"

/* *NOTE*: "see RFC 952 [DNS:4] for details on allowed hostnames..."
RFC 952
1. A "name" (Net, Host, Gateway, or Domain name) is a text string up
   to 24 characters drawn from the alphabet (A-Z), digits (0-9), minus
   sign (-), and period (.).  Note that periods are only allowed when
   they serve to delimit components of "domain style names".
RFC 1123
The syntax of a legal Internet host name was specified in RFC-952.
One aspect of host name syntax is hereby changed: the restriction on
the first character is relaxed to allow either a letter or a digit.

And so on...
--> [[:alpha:][:digit:].-] is just a somewhat informed and tolerant suggestion.
    A more CORRECT implementation would probably look something like this...
<hostname>    = <label> | <hostname>.<label>
<label>       = <let-dig> | <let-dig><let-dig> | <let-dig><ldh-string><let-dig>
<ldh-string>  = <let-dig-hyp> | <ldh-string><let-dig-hyp>
<let-dig-hyp> = <let-dig> | '-'
<let-dig>     = <letter> | <digit>
<digit>       = [[:digit:]]
<letter>      = [[:alpha:]]*/

CHAR           [\x00-\xFF]
SPACE          [ ]
HOST           ([[:alpha:][:digit:].\-])+
SERVERNAME     {HOST}
/* LETTER         [[:alpha:]] */
/*LETTER    ==     [a-zA-Z]*/
/* NUMBER         [[:digit:]] */
/*NUMBER    ==     [0-9]*/
/* *NOTE*: violating the spec, some implementations allow '_'... */
/* SPECIAL        [-\[\]\\`^{}_] */
NICK           [[:alpha:]]([[:alpha:][:digit:]\-\[\]\\`^{}_])*
/*ORIGIN         {SERVERNAME}|{NICK}*/
/* *NOTE*: "\r\n" is (ANSI-C) implementation-dependent --> use ASCII codes directly */
/* *NOTE*: in order to pick up the hostname in a sequence "!user[@host] ",
           disallow "@" within a username... */
USER           [^ \t\0\x0D\x0A@]+
/* NONWHITE       [\x00-\xFF]{-}[ \t\0\r\n] */
/* USER           {NONWHITE}+ */
/* CHSTRING       ([\x00-\xFF]{-}[ \b\0\x0D\x0A,])+ */
/* MASK           [#$]{([\x00-\xFF]{-}[ \b\0\x0D\x0A,])}+ */
/* CHANNEL        [#&]{([\x00-\xFF]{-}[ \b\0\x0D\x0A,])}+ */
PARAM          ([^: \0\x0D\x0A])([^ \0\x0D\x0A])*
TRAILING_PARAM [^\0\x0D\x0A]*

/* CRLF           \x0D\x0A */

%s             prefix
%s             user
%s             host
%s             command
%s             params
%s             trailing
%s             end_of_frame

%{
#define YY_USER_ACTION yylloc->columns(yyleng);
%}

/* *NOTE*: it seems that the following "exceptions" are allowed:
           - the trailing param does NOT need a ':' prefix IF it doesn't contain
             <SPACE>s...
*/

%%
%{
  yylloc->step();
  yy_flex_debug = driver->getDebugScanner();
  std::stringstream converter;
%}

<INITIAL>{
":"                        { yylloc->step();
                             BEGIN(prefix);
                             yylval->ival = 1;
                             return yy::IRC_Parser::token_type(yytext[0]); }
"\x0D"                     { yylloc->step();
                             BEGIN(end_of_frame);
                             yylval->ival = 1; }
} // end <INITIAL>
<prefix>{
{SPACE}+                   { yylloc->step();
                             BEGIN(command);
                             yylval->ival = yyleng;
                             return yy::IRC_Parser::token::SPACE; }
"!"                        { yylloc->step();
                             BEGIN(user);
                             yylval->ival = 1;
                             return yy::IRC_Parser::token_type(yytext[0]); }
"@"                        { yylloc->step();
                             BEGIN(host);
                             yylval->ival = 1;
                             return yy::IRC_Parser::token_type(yytext[0]); }
{NICK}|{HOST}              { yylloc->step();
                             ACE_NEW_NORETURN(yylval->sval,
                                              std::string);
                             yylval->sval->append(yytext, yyleng);
                             return yy::IRC_Parser::token::ORIGIN; }
} // end <prefix>
<user>{
{SPACE}+                   { yylloc->step();
                             BEGIN(command);
                             yylval->ival = yyleng;
                             return yy::IRC_Parser::token::SPACE; }
"@"                        { yylloc->step();
                             BEGIN(host);
                             yylval->ival = 1;
                             ACE_NEW_NORETURN(yylval->sval,
                                              std::string);
                             return yy::IRC_Parser::token_type(yytext[0]); }
{USER}                     { yylloc->step();
                             BEGIN(prefix);
                             ACE_NEW_NORETURN(yylval->sval,
                                              std::string);
                             yylval->sval->append(yytext, yyleng);
                             return yy::IRC_Parser::token::USER; }
} // end <user>
<host>{
{SPACE}+                   { yylloc->step();
                             BEGIN(command);
                             yylval->ival = yyleng;
                             return yy::IRC_Parser::token::SPACE; }
{HOST}                     { yylloc->step();
                             BEGIN(prefix);
                             ACE_NEW_NORETURN(yylval->sval,
                                              std::string);
                             yylval->sval->append(yytext, yyleng);
                             return yy::IRC_Parser::token::HOST; }
} // end <host>
<INITIAL,command>{
{SPACE}+                   { yylloc->step();
                             BEGIN(params);
                             yylval->ival = yyleng;
                             return yy::IRC_Parser::token::SPACE; }
[[:digit:]]{3}             { yylloc->step();
                             converter.str(ACE_TEXT_ALWAYS_CHAR(""));
                             converter.clear();
                             converter << yytext;
                             converter >> yylval->ival;
                             return yy::IRC_Parser::token::CMD_NUMERIC; }
[[:alpha:]]+               { yylloc->step();
                             ACE_NEW_NORETURN(yylval->sval,
                                              std::string);
                             yylval->sval->append(yytext, yyleng);
                             return yy::IRC_Parser::token::CMD_STRING; }
} // end <INITIAL, command>
<params>{
{SPACE}+                   { yylloc->step();
                             yylval->ival = yyleng;
                             return yy::IRC_Parser::token::SPACE; }
":"                        { yylloc->step();
                             BEGIN(trailing);
                             yylval->ival = 1;
                             return yy::IRC_Parser::token_type(yytext[0]); }
"\x0D"                     { yylloc->step(); /* *NOTE*: non-compliant (see above) */
                             BEGIN(end_of_frame); }
{PARAM}                    { yylloc->step();
                             ACE_NEW_NORETURN(yylval->sval,
                                              std::string);
                             yylval->sval->append(yytext, yyleng);
                             return yy::IRC_Parser::token::PARAM; }
} // end <params>
<trailing>{
"\x0D"                     { yylloc->step();
                             BEGIN(end_of_frame); }
{TRAILING_PARAM}           { yylloc->step();
                             if (yyleng > 0)
                             {
                               ACE_NEW_NORETURN(yylval->sval,
                                                std::string);
                               yylval->sval->append(yytext, yyleng);
                               return yy::IRC_Parser::token::PARAM;
                             } // end IF
                           }
} // end <trailing>
<end_of_frame>{
"\x0A"                     { yylloc->step();
                             BEGIN(INITIAL);
                             yylval->ival = 2;
                             (*messageCounter)++; }
}
<<EOF>>                    { yyterminate(); }
<*>{CHAR}                  { /* *TODO*: use (?s:.) ? */
                             yylloc->step();

                             // debug info
                             std::string error_message = ACE_TEXT("invalid character: \"");
                             error_message += yytext[0];
                             error_message += ACE_TEXT("\", aborting");
                             driver->error(*yylloc, error_message);

                             yyterminate(); }
%%

// #ifdef __cplusplus
// extern "C"
// {
// #endif /* __cplusplus */
// int
// IRCScannerwrap(yyscan_t yyscanner)
// {
//   ACE_TRACE(ACE_TEXT("::IRCScannerwrap"));
//
//   struct yyguts_t* yyg = ACE_static_cast(struct yyguts_t*, yyscanner);
//
//   IRC_ParserDriver* driver = IRC_Client_IRCscanner_get_extra (yyscanner);
//   // sanity check
//   ACE_ASSERT(driver);
//   if (!driver->moreData())
//     return 1;
//
//   // *NOTE*: there SHOULD be more data in a
//   // continuation, so:
//   // 1. gobble/save the rest
//   // 2. switch buffers
//   // 3. unput the rest
//   // 4. continue matching
//
//   // step1
//   std::string the_rest;
//   the_rest += yytext[0];
//   for (char c = yyinput(yyscanner);
//        c != EOF;
//        c = yyinput(yyscanner))
//     the_rest += c;
//
//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("the rest: \"%s\"\n"),
//              the_rest.c_str()));
//
//   // step2
//   if (!driver->switchBuffer())
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to IRC_ParserDriver::switchBuffer() [data was: \"%s\"], aborting\n"),
//                the_rest.c_str()));
//
//     return 1;
//   } // end IF
//
//   // step3
//   for (std::string::reverse_iterator iterator = the_rest.rbegin();
//        iterator != the_rest.rend();
//        iterator++)
//     unput(*iterator);
//
//   // step4
//  // yymore();
//
//   return 0;
// }
// #ifdef __cplusplus
// }
// #endif /* __cplusplus */
