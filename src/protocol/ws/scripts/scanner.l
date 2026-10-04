%top{
#include "ws_common.h"
//#include "ws_exports.h"
#include "ws_iparser.h"
//#undef YYTOKENTYPE
#include "ws_parser.h"

#if !defined (YY_DECL)
/*#define YY_DECL                                           \
yy::WebSocket_Parser::token_type                            \
WebSocket_Scanner_lex (yy::WebSocket_Parser::semantic_type* yylval, \
                       yy::WebSocket_Parser::location_type* yylloc, \
                       WebSocket_IParser* iparser_p,                \
                       yyscan_t yyscanner)*/
#define YY_DECL                                      \
enum yytokentype                                     \
WebSocket_Scanner_lex (YYSTYPE* yylval,              \
                       YYLTYPE* yylloc,              \
                       WebSocket_IParser* iparser_p, \
                       yyscan_t yyscanner)
YY_DECL;
#endif

//using namespace yy;
/*#define YYLTYPE yy::location
#define YYSTYPE yy::WebSocket_Parser::semantic_type*/

void WebSocket_Scanner_set_column (int, yyscan_t);

//#define FLEXINT_H
}

%{
#include <cctype>
#include <regex>
#include <sstream>
#include <string>

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "common_string_tools.h"
#include "common_tools.h"

#include "net_macros.h"

#include "ws_common.h"
#include "ws_defines.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
//#define yyterminate() return yy::WebSocket_Parser::token::END
#define yyterminate() return yytokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option prefix="WebSocket_Scanner_"

%option header-file="ws_scanner.h" outfile="ws_scanner.cpp"
%option yylineno yywrap
%option nomain nounput noyymore noreject nodefault nostdinit noline
/* %option nostack noline nounistd */
%option nostack nounistd
/* %option c++ yyclass="WebSocket_Scanner" */
/* *NOTE*: the '#line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
/* %option ansi-definitions ansi-prototypes */
/* *IMPORTANT NOTE*: do NOT mess with these (it's broken) */
/* %option bison-bridge bison-locations */
%option 8bit batch never-interactive
%option reentrant pointer
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, trace header-file extra-type'
*/
%option extra-type="WebSocket_IParser*"
/* *NOTE*: option 'tables-file' breaks 'yylineno' (flex 2.6.3), which crashes
           the lexer when 'debug' is set (apparently, array yy_rule_linenum is
           not initialized in this case) */
/* %option tables-file="ws_scanner.tab" */

/* *NOTE*: see RFC 1945 page 10 */
OCTET          [\x00-\xFF]
/* *NOTE*: (US-)ASCII */
CHAR           [\x00-\x7F]
UPALPHA        [A-Z]
LOALPHA        [a-z]
ALPHA          [[:alpha:]]
DIGIT          [[:digit:]]
CTL            [\x00-\x1F\x7F]
CR             \x0D
LF             \x0A
SP             \x20
/* *NOTE*: (horizontal) TAB */
HT             \x09
/* DOUBLE_QUOTE   \x22 */

CRLF           \x0D\x0A
LWS            (\x0D\x0A(\x20|\x09){1})
/* *NOTE*: flex 2.5.4 does not support class unions ({+}) */
TEXT           ([\x00-\xFF]{-}[\x00-\x1F]{+}[\x09\x0A\x0D])*
HEX            [a-fA-F[:digit:]]

%s             mask_payload_size_first
%s             payload_size_16
%s             payload_size_64
%s             payload
%s             data

%{
/* handle locations */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
                       yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;
/*#define YY_USER_ACTION yylloc->begin.line = yylloc->end.line = yylineno; \
                       yylloc->begin.column = yycolumn; yylloc->end.column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;*/
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%%
%{
%}

<INITIAL>{
{OCTET}{1}                 { iparser_p->offset (1);
                             uint8_t byte_i = *yytext;
                             struct WebSocket_Record& record_r =
                               iparser_p->current ();
                             record_r.final = byte_i & 0x80;
                             record_r.opcode =
                               static_cast<WebSocket_OpCode_t> (byte_i & 0x0F);
                             BEGIN (mask_payload_size_first);
                           }
} // end <INITIAL>
<mask_payload_size_first>{
{OCTET}{1}                 { iparser_p->offset (1);
                             uint8_t byte_i = *yytext;
                             ACE_ASSERT ((byte_i & 0x80) == 0); // --> frame is not masked
                             struct WebSocket_Record& record_r =
                               iparser_p->current ();
                             record_r.payloadSize = byte_i & 0x7F;
                             if (record_r.payloadSize <= 125)
                             {
                               iparser_p->dataSize (record_r.payloadSize);
                               BEGIN (record_r.payloadSize ? payload : INITIAL);
                               if (!record_r.payloadSize)
                               {
                                 yylval->ival = 0;
//                               return yy::WebSocket_Parser::token::FRAME;
                                 return yytokentype::FRAME;
                               } // end IF
                             } // end IF
                             else if (record_r.payloadSize == 126)
                               BEGIN (payload_size_16);
                             else if (record_r.payloadSize == 127)
                               BEGIN (payload_size_64);
                           }
} // end <mask_payload_size_first>
<payload_size_16>{
{OCTET}{2}                 { iparser_p->offset (2);
                             uint16_t word_i = *reinterpret_cast<uint16_t*> (yytext);
                             if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
                               word_i = ACE_SWAP_WORD (word_i);
                             struct WebSocket_Record& record_r =
                               iparser_p->current ();
                             record_r.payloadSize = word_i;
                             iparser_p->dataSize (record_r.payloadSize);
                             BEGIN (payload); }
} // end <payload_size_16>
<payload_size_64>{
{OCTET}{4}                 { iparser_p->offset (4);
                             uint64_t quad_word_i = *reinterpret_cast<uint64_t*> (yytext);
                             if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
                               quad_word_i = ACE_SWAP_LONG_LONG (quad_word_i);
                             struct WebSocket_Record& record_r =
                               iparser_p->current ();
                             record_r.payloadSize = quad_word_i;
                             ACE_ASSERT ((quad_word_i & 0x8000000000000000) == 0); // --> MSB is 0
                             iparser_p->dataSize (record_r.payloadSize);
                             BEGIN (payload); }
} // end <payload_size_64>
<payload>{
{OCTET}{1}                 {
                             struct WebSocket_Record& record_r =
                               iparser_p->current ();
/*                             ACE_DEBUG ((LM_DEBUG,
                                         ACE_TEXT ("found payload @%d of %Q byte(s)\n"),
                                         iparser_p->offset (),
                                         record_r.payloadSize)); */
                             ACE_UINT64 missing_bytes = iparser_p->dataBytesToSkip ();
                             if (!missing_bytes)
                             {
                               yylval->ival = 1;
                               BEGIN (INITIAL);
                               return yytokentype::FRAME;
                             }
                             --missing_bytes;
                             iparser_p->dataBytesSkipped (1);
/*                             char c = 0;
                             for (unsigned int i = 0;
                                  i < missing_bytes;
                                  ++i)
                             {
                               c = yyinput (yyscanner);
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;
                             } // end FOR */
                             BEGIN (data);
                           }
} // end <payload>
<data>{
{OCTET}{1}                 {
                             iparser_p->dataBytesSkipped (1);
                             if (unlikely (!iparser_p->dataBytesToSkip ()))
                             {
                               try {
                                 iparser_p->finished ();
                               } catch (...) {
                                 ACE_DEBUG ((LM_ERROR,
                                             ACE_TEXT ("caught exception in WebSocket_IParser::finished(), continuing\n")));
                               }

                               BEGIN (INITIAL);
                               yylval->ival = iparser_p->dataSize ();
//                               return yy::WebSocket_Parser::token::FRAME;
                               return yytokentype::FRAME;
                             } // end IF
                           }
} // end <data>
<<EOF>>                    { yyterminate(); } // *NOTE*: yywrap returned non-zero
<*>{OCTET}                 { /* *TODO*: use (?s:.) ? */
                             if (!iparser_p->isBlocking ())
                             { // *IMPORTANT NOTE*: put the data back
                               *yy_cp = yyg->yy_hold_char;
                               yyterminate (); // not enough data, cannot proceed
                             } // end IF

                             // wait for more data fragment(s)
                             if (!iparser_p->switchBuffer ())
                             { // *NOTE*: most probable reason: connection
                               //         has been closed --> session end
                               ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("failed to Net_IParser::switchBuffer(), returning\n")));
                               // *IMPORTANT NOTE*: put the data back
                               *yy_cp = yyg->yy_hold_char;
                               yyterminate (); // not enough data, cannot proceed
                             } // end IF
                             yyless (0); }
%%
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
WebSocket_Scanner_wrap (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::WebSocket_Scanner_wrap"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  WebSocket_IParser* iparser_p = WebSocket_Scanner_get_extra (yyscanner);

  // sanity check(s)
  ACE_ASSERT (iparser_p);
  if (!iparser_p->isBlocking ())
    return 1; // not enough data, cannot proceed
  if (iparser_p->hasFinished ())
    return 1; // done

  if (!iparser_p->switchBuffer ())
  {
    // *NOTE*: most probable reason: received session end message
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_IParser::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
