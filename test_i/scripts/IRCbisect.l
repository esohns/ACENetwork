%{
#include "stdafx.h"

// *WORKAROUND*
#include <iostream>
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "ace/Log_Msg.h"
%}

%option pointer 8bit reentrant never-interactive nounistd
/* %option c++ yyclass="IRC_Client_IRCBisect" stack noline bison-bridge bison-locations */
%option warn batch nomain noyywrap nounput nodefault
%option debug perf-report verbose yylineno ansi-definitions ansi-prototypes
%option full backup perf-report verbose
%option outfile="IRC_client_IRCbisect.cpp" header-file="IRC_client_IRCbisect.h" prefix="IRC_Client_IRCBisect_"

/* *NOTE*: "\r\n" is (ANSI-C) implementation-dependent --> use ASCII codes directly */
CRLF    \x0D\x0A
MESSAGE ([\x00-\xFF]{-}[\x0D\x0A])+

%s      end_of_frame

%%
<INITIAL>{
"\x0D"                 { BEGIN(end_of_frame); }
{MESSAGE}              { return yyleng; }
(?s:.)                 {
                         ACE_DEBUG ((LM_ERROR,
                                     ACE_TEXT ("ignoring invalid character: \"%c\" (%d)\n"),
                                     yytext[0],
                                     static_cast<int> (yytext[0])));
                       }
} // end <INITIAL>
<end_of_frame>{
"\x0A"                 { BEGIN(INITIAL);
                         /* (*yyextra)++; */
                         return -1; }
(?s:.)                 { BEGIN(INITIAL);
                         yymore(); }
} // end <end_of_frame>
<<EOF>>                { return 0; }
%%
