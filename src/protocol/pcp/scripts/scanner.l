%top{
#include "pcp_parser.h"

/*
#define YY_DECL                                               \
yy::PCP_Parser::token_type                                    \
PCP_Scanner_lex (yy::PCP_Parser::semantic_type* yylval_param, \
                 yy::PCP_Parser::location_type* yylloc_param, \
                 PCP_ParserDriver* driver,                    \
                 yyscan_t yyscanner) */
/* yytokentype */
#define YY_DECL                              \
int                                          \
PCP_Scanner_lex (YYSTYPE* yylval_param,     \
                 YYLTYPE* yylloc_param,     \
                 PCP_ParserDriver* driver, \
                 yyscan_t yyscanner)
// ... and declare it for the parser's sake
extern YY_DECL;

extern int PCP_Scanner_reset (yyscan_t);
extern void PCP_Scanner_set_column (int, yyscan_t);

//using namespace yy;
//#define YYLTYPE PCP_Parser::location_type
//#define YYSTYPE PCP_Parser::semantic_type
}

%{
#include <sstream>
#include <string>

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "net_macros.h"

#include "pcp_common.h"
#include "pcp_parser_driver.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
//#define yyterminate() return yy::DHCP_Parser::token::END
#define yyterminate() return yytokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option prefix="PCP_Scanner_"

%option header-file="pcp_scanner.h" outfile="pcp_scanner.cpp"
%option batch 8bit never-interactive yylineno
%option nodefault nostdinit noyywrap nomain nounput
/* %option c++ yyclass="PCP_Scanner" */
/* %option ansi-definitions ansi-prototypes bison-bridge */
/* *NOTE*: the 'line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
%option ansi-definitions ansi-prototypes bison-locations
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
%option reentrant pointer
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, header-file extra-type'
*/
%option extra-type="PCP_ParserDriver*"
/* %option nounput */

/* *NOTE*: see RFC 1531 page 13ff. */
OCTET          [\x00-\xFF]
/* STRING         [[:alpha:]\x00] */
/* DIGIT          [[:digit:]] */
/* NUL            [\x00] */

%s             version
%s             op
%s             reserved
%s             result_code
%s             lifetime
%s             epoch_time
%s             reserved_2
%s             opcode_specific_response_data_map
%s             opcode_specific_response_data_map_protocol
%s             opcode_specific_response_data_map_reserved
%s             opcode_specific_response_data_map_internal_port
%s             opcode_specific_response_data_map_assigned_external_port
%s             opcode_specific_response_data_map_assigned_external_ip_address
%s             opcode_specific_response_data_peer
%s             opcode_specific_response_data_peer_protocol
%s             opcode_specific_response_data_peer_reserved
%s             opcode_specific_response_data_peer_internal_port
%s             opcode_specific_response_data_peer_assigned_external_port
%s             opcode_specific_response_data_peer_assigned_external_ip_address
%s             opcode_specific_response_data_peer_remote_peer_port
%s             opcode_specific_response_data_peer_reserved_2
%s             opcode_specific_response_data_peer_remote_peer_ip_address
%s             option_code
%s             option_reserved
%s             option_length
%s             option_data
%s             option_data_2

%{
/* handle locations */
/*#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;                      \
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

  unsigned int padding_bytes = 0;
%}

<INITIAL>{
{OCTET}{1}                 {
                             BEGIN(version);
                             yycolumn -= yyleng;
                             yyless (0); }
} // end <INITIAL>
<version>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(op);
                             return yytokentype::VERSION;
                             /* return yy::PCP_Parser::token::VERSION; */ }
} // end <version>
<op>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(reserved);
                             return yytokentype::OP;
                             /* return yy::PCP_Parser::token::OP; */ }
} // end <op>
<reserved>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(result_code);
                             return yytokentype::RESERVED;
                             /* return yy::PCP_Parser::token::RESERVED; */ }
} // end <reserved>
<result_code>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(lifetime);
                             return yytokentype::RESULT_CODE;
                             /* return yy::PCP_Parser::token::RESULT_CODE; */ }
} // end <result_code>
<lifetime>{
{OCTET}{4}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<ACE_UINT32*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(epoch_time);
                             return yytokentype::LIFETIME;
                             /* return yy::PCP_Parser::token::LIFETIME; */ }
} // end <lifetime>
<epoch_time>{
{OCTET}{4}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<ACE_UINT32*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(reserved_2);
                             return yytokentype::EPOCH_TIME;
                             /* return yy::PCP_Parser::token::EPOCH_TIME; */ }
} // end <epoch_time>
<reserved_2>{
{OCTET}{12}                { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<unsigned char*> (yytext + 8) << 24);
                             yylval->ival |=
                                 (*reinterpret_cast<unsigned char*> (yytext + 9) << 16);
                             yylval->ival |=
                                 (*reinterpret_cast<unsigned char*> (yytext + 10) << 8);
                             yylval->ival |=
                                 *reinterpret_cast<unsigned char*> (yytext + 11);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (yylval->ival)
                                                                        : yylval->ival);
                             switch (parser->record_.opcode)
                             {
                               case PCP_Codes::PCP_OPCODE_MAP:
                                 BEGIN(opcode_specific_response_data_map); break;
                               case PCP_Codes::PCP_OPCODE_PEER:
                                 BEGIN(opcode_specific_response_data_peer); break;
                               case PCP_Codes::PCP_OPCODE_ANNOUNCE:
                                 BEGIN(option_code); break;
                               default:
                               {
                                 ACE_DEBUG ((LM_ERROR,
                                             ACE_TEXT ("invalid/unknown opcode (was: %d), aborting\n"),
                                             parser->record_.opcode));
                                 yyterminate();
                               }
                             } // end SWITCH
                             return yytokentype::RESERVED_2;
                             /* return yy::PCP_Parser::token::RESERVED_2; */ }
} // end <reserved_2>

<opcode_specific_response_data_map>{
{OCTET}{12}                { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<unsigned char*> (yytext + 8) << 24);
                             yylval->ival |=
                                 (*reinterpret_cast<unsigned char*> (yytext + 9) << 16);
                             yylval->ival |=
                                 (*reinterpret_cast<unsigned char*> (yytext + 10) << 8);
                             yylval->ival |=
                                 *reinterpret_cast<unsigned char*> (yytext + 11);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_map_protocol);
                             return yytokentype::OPTION_MAP_NONCE;
                             /* return yy::PCP_Parser::token::OPTION_MAP_NONCE; */ }
} // end <opcode_specific_response_data_map>
<opcode_specific_response_data_map_protocol>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(opcode_specific_response_data_map_reserved);
                             return yytokentype::OPTION_MAP_PROTOCOL;
                             /* return yy::PCP_Parser::token::OPTION_MAP_PROTOCOL; */ }
} // end <opcode_specific_response_data_map_protocol>
<opcode_specific_response_data_map_reserved>{
{OCTET}{3}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(opcode_specific_response_data_map_internal_port);
                             return yytokentype::OPTION_MAP_RESERVED;
                             /* return yy::PCP_Parser::token::OPTION_MAP_RESERVED; */ }
} // end <opcode_specific_response_data_map_reserved>
<opcode_specific_response_data_map_internal_port>{
{OCTET}{2}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT16*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_map_assigned_external_port);
                             return yytokentype::OPTION_MAP_INTERNAL_PORT;
                             /* return yy::PCP_Parser::token::OPTION_MAP_INTERNAL_PORT; */ }
} // end <opcode_specific_response_data_map_internal_port>
<opcode_specific_response_data_map_assigned_external_port>{
{OCTET}{2}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT16*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_map_assigned_external_ip_address);
                             return yytokentype::OPTION_MAP_ASSIGNED_EXTERNAL_PORT;
                             /* return yy::PCP_Parser::token::OPTION_MAP_ASSIGNED_EXTERNAL_PORT; */ }
} // end <opcode_specific_response_data_map_assigned_external_port>
<opcode_specific_response_data_map_assigned_external_ip_address>{
{OCTET}{16}                { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT32*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(option_code);
                             return yytokentype::OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS;
                             /* return yy::PCP_Parser::token::OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS; */ }
} // end <opcode_specific_response_data_map_assigned_external_ip_address>

<opcode_specific_response_data_peer>{
{OCTET}{12}                { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<unsigned char*> (yytext + 8) << 24);
                             yylval->ival |=
                                 (*reinterpret_cast<unsigned char*> (yytext + 9) << 16);
                             yylval->ival |=
                                 (*reinterpret_cast<unsigned char*> (yytext + 10) << 8);
                             yylval->ival |=
                                 *reinterpret_cast<unsigned char*> (yytext + 11);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_peer_protocol);
                             return yytokentype::OPTION_PEER_NONCE;
                             /* return yy::PCP_Parser::token::OPTION_PEER_NONCE; */ }
} // end <opcode_specific_response_data_peer>
<opcode_specific_response_data_peer_protocol>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(opcode_specific_response_data_map_reserved);
                             return yytokentype::OPTION_PEER_PROTOCOL;
                             /* return yy::PCP_Parser::token::OPTION_PEER_PROTOCOL; */ }
} // end <opcode_specific_response_data_peer_protocol>
<opcode_specific_response_data_peer_reserved>{
{OCTET}{3}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(opcode_specific_response_data_peer_internal_port);
                             return yytokentype::OPTION_PEER_RESERVED;
                             /* return yy::PCP_Parser::token::OPTION_PEER_RESERVED; */ }
} // end <opcode_specific_response_data_peer_reserved>
<opcode_specific_response_data_peer_internal_port>{
{OCTET}{2}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT16*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_peer_assigned_external_port);
                             return yytokentype::OPTION_PEER_INTERNAL_PORT;
                             /* return yy::PCP_Parser::token::OPTION_PEER_INTERNAL_PORT; */ }
} // end <opcode_specific_response_data_peer_internal_port>
<opcode_specific_response_data_peer_assigned_external_port>{
{OCTET}{2}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT16*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_peer_assigned_external_ip_address);
                             return yytokentype::OPTION_PEER_ASSIGNED_EXTERNAL_PORT;
                             /* return yy::PCP_Parser::token::OPTION_PEER_ASSIGNED_EXTERNAL_PORT; */ }
} // end <opcode_specific_response_data_peer_assigned_external_port>
<opcode_specific_response_data_peer_assigned_external_ip_address>{
{OCTET}{16}                { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT32*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_peer_remote_peer_port);
                             return yytokentype::OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS;
                             /* return yy::PCP_Parser::token::OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS; */ }
} // end <opcode_specific_response_data_peer_assigned_external_ip_address>
<opcode_specific_response_data_peer_remote_peer_port>{
{OCTET}{2}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT16*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_peer_reserved_2);
                             return yytokentype::OPTION_PEER_REMOTE_PEER_PORT;
                             /* return yy::PCP_Parser::token::OPTION_PEER_REMOTE_PEER_PORT; */ }
} // end <opcode_specific_response_data_peer_remote_peer_port>
<opcode_specific_response_data_peer_reserved_2>{
{OCTET}{2}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT16*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(opcode_specific_response_data_peer_remote_peer_ip_address);
                             return yytokentype::OPTION_PEER_RESERVED_2;
                             /* return yy::PCP_Parser::token::OPTION_PEER_RESERVED_2; */ }
} // end <opcode_specific_response_data_peer_reserved_2>
<opcode_specific_response_data_peer_remote_peer_ip_address>{
{OCTET}{16}                { driver->offset_ += yyleng;
                             yylval->ival =
                                 (*reinterpret_cast<ACE_UINT32*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(option_code);
                             return yytokentype::OPTION_PEER_REMOTE_PEER_IP_ADDRESS;
                             /* return yy::PCP_Parser::token::OPTION_PEER_REMOTE_PEER_IP_ADDRESS; */ }
} // end <opcode_specific_response_data_peer_remote_peer_ip_address>

<option_code>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(option_reserved);
                             return yytokentype::OPTION_CODE;
                             /* return yy::PCP_Parser::token::OPTION_CODE; */ }
} // end <option_code>
<option_reserved>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<unsigned char*> (yytext);
                             BEGIN(option_length);
                             return yytokentype::OPTION_RESERVED;
                             /* return yy::PCP_Parser::token::OPTION_RESERVED; */ }
} // end <option_reserved>
<option_length>{
{OCTET}{2}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<ACE_UINT16*> (yytext);
                             yylval->ival =
                                 ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (yylval->ival)
                                                                        : yylval->ival);
                             BEGIN(option_data);
                             return yytokentype::OPTION_LENGTH;
                             /* return yy::PCP_Parser::token::OPTION_LENGTH; */ }
} // end <option_length>
<option_data>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             yylval->ival =
                                 *reinterpret_cast<unsigned int*> (yytext);
                             ACE_UINT16 option_length_i = 0;
                             PCP_OptionsIterator_t iterator =
                               driver->record_->options.find (driver->currentOption (option_length_i));
                             ACE_ASSERT (iterator != driver->record_->options.end ());
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->append (yytext[0]);
                             /* undo the effects of YY_DO_BEFORE_ACTION */
                             *yy_cp = yyg->yy_hold_char;
                             char c;
                             for (ACE_UINT16 i = 0;
                                  i < option_length_i;
                                  ++i)
                             {
                               c = yyinput (yyscanner);
                               yylval->sval->append (c);
                               *yy_cp = yyg->yy_hold_char;
                             } // end FOR
                             padding_bytes = option_length_i % 4;
                             if (!padding_bytes)
                             {
                               BEGIN(option_code);
                               return yytokentype::OPTION_DATA;
                               /* return yy::PCP_Parser::token::OPTION_DATA; */
                             } // end IF
                             padding_bytes = 4 - padding_bytes;
                             BEGIN(option_data_2); }
} // end <option_data>
<option_data_2>{
{OCTET}{1}                 { driver->offset_ += yyleng;
                             ACE_ASSERT (padding_bytes);
                             --padding_bytes;
                             char c;
                             for (unsigned int i = 0;
                                  i < padding_bytes;
                                  ++i)
                               c = yyinput (yyscanner);
                             ACE_UNUSED_ARG (c);
                             BEGIN(option_code); }
} // end <option_data_2>
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
PCP_Scanner_reset (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::PCP_Scanner_reset"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  PCP_ParserDriver* driver = PCP_Scanner_get_extra (yyscanner);

  // sanity check(s)
  ACE_ASSERT (driver);
  ACE_ASSERT (driver->fragment_);

  BEGIN(INITIAL);

  return 0;
}
