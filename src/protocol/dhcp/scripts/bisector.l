%{
// *WORKAROUND*
#include <iostream>
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

//#include "ace/Log_Msg.h"
%}

%option 8bit align backup batch
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option debug nodefault fast never-interactive perf-report noread
/* %option c++ yyclass="IRC_Bisect" stack noline bison-bridge */
/*         bison-locations */
%option verbose warn
%option pointer
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd, */
/*                   ansi-definitions, ansi-prototypes, header-file' */
%option reentrant
%option header-file="http_bisector.h"
/* *IMPORTANT NOTE*: 'yylineno' is not compatible with 'read' */
/* *NOTE*: flex 2.5.39 does not recognize 'noline' */
%option nomain nostdinit noyywrap
%option outfile="http_bisector.cpp" prefix="HTTP_Bisector_"
%option nounput

/* *NOTE*: "\r\n" is (ANSI-C) implementation-dependent */
/*         --> use ASCII codes directly */
CHAR                   [\x00-\xFF]
/* CRLF                   \x0D\x0A */
MESSAGE                [^\x0D\x0A]+

%s                     end_of_frame

%% /* end of definitions */

<INITIAL>{
"\x0D"                 { BEGIN(end_of_frame); }
{MESSAGE}              { return yyleng; }
{CHAR}                 { /* *TODO*: use (?s:.) ? */
//                         ACE_DEBUG ((LM_ERROR,
//                                     ACE_TEXT ("ignoring invalid character: \"%c\" (%d)\n"),
//                                     yytext[0],
//                                     static_cast<int> (yytext[0])));
                         yymore();
                         /* yyless (0); */
                         return yyleng;
                       }
} // end <INITIAL>
<end_of_frame>{
"\x0A"                 { BEGIN(INITIAL);
                         /* (*yyextra)++; */
                         return -1;
                       }
{CHAR}                 { /* *TODO*: use (?s:.) ? */
                         BEGIN(INITIAL);
                         yymore();
                         /* yyless (0); */
                         return yyleng;
                       }
} // end <end_of_frame>
<<EOF>>                { return 0; }

%% /* end of rules */
