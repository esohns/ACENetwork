%defines                          "pcp_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "pcp_parser.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/* %skeleton                         "lalr1.cc" */
%token-table
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
}

/* %define location_type */
/* %define api.location.type         {} */
/* %define namespace                 {yy} */
/* %define api.namespace             {yy} */
%name-prefix                      "yy"
/* %define api.prefix                {yy} */
%pure-parser
/* %define api.pure                  true */
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/* %define api.token.constructor */
/* %define api.token.prefix          {} */
/* %define api.value.type            variant */
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/* %define parse.assert              {true} */
%error-verbose
/* %define parse.error               verbose */
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/* %define parser_class_name         {DHCP_Parser} */
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               {true} */

%code requires {
#include <cstdio>
#include <string>

//enum yytokentype
//{
//  END = 0,
//  OP = 258,
//  HTYPE = 259,
//  HLEN = 260,
//  HOPS = 261,
//  XID = 262,
//  SECS = 263,
//  FLAGS = 264,
//  CIADDR = 265,
//  YIADDR = 266,
//  SIADDR = 267,
//  GIADDR = 268,
//  CHADDR = 269,
//  SNAME = 270,
//  FILE_ = 271,
//  COOKIE = 272,
//  OPTION_KEY = 273,
//  OPTION_VALUE = 274
//};
//#define YYTOKENTYPE
class PCP_ParserDriver;
//class PCP_Scanner;
struct YYLTYPE;
//union YYSTYPE;

union yytoken
{
  ACE_UINT32 ival;
};
typedef yytoken yytoken_t;
#define YYSTYPE yytoken_t

typedef void* yyscan_t;

//#define YYERROR_VERBOSE
extern void yyerror (YYLTYPE*, PCP_ParserDriver*, yyscan_t, const char*);
extern int yyparse (PCP_ParserDriver*, yyscan_t);

#undef YYPRINT
//extern void yyprint (FILE*, yytokentype, YYSTYPE);
}

// calling conventions / parameter passing
%parse-param              { PCP_ParserDriver* driver }
%parse-param              { yyscan_t yyscanner }
/* %lex-param                { YYSTYPE* yylval } */
/* %lex-param                { YYLTYPE* yylloc } */
%lex-param                { PCP_ParserDriver* driver }
%lex-param                { yyscan_t yyscanner }

%initial-action
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  //@$.begin.filename = @$.end.filename = &driver->file;
  ACE_OS::memset (&@$, 0, sizeof (YYLTYPE));

  // initialize the token value container
  $$.ival = 0;
};

// symbols
%union
{
  unsigned int ival;
};
/* %token <int>         INTEGER; */

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif // YYDEBUG

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "pcp_common.h"
#include "pcp_defines.h"
#include "pcp_parser_driver.h"
#include "pcp_scanner.h"
#include "pcp_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex PCP_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <ival> VERSION                                  "version"
%token <ival> OPCODE                                   "opcode"
%token <ival> RESERVED                                 "reserved"
%token <ival> RESULT_CODE                              "result_code"
%token <ival> LIFETIME                                 "lifetime"
%token <ival> EPOCH_TIME                               "epoch_time"
%token <ival> RESERVED_2                               "reserved_2"
%token <ival> OPTION_MAP_NONCE                         "option_map_nonce"
%token <ival> OPTION_MAP_PROTOCOL                      "option_map_protocol"
%token <ival> OPTION_MAP_RESERVED                      "option_map_reserved"
%token <ival> OPTION_MAP_INTERNAL_PORT                 "option_map_internal_port"
%token <ival> OPTION_MAP_ASSIGNED_EXTERNAL_PORT        "option_map_assigned_external_port"
%token <ival> OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS  "option_map_assigned_external_ip_address"
%token <ival> OPTION_PEER_NONCE                        "option_peer_nonce"
%token <ival> OPTION_PEER_PROTOCOL                     "option_peer_protocol"
%token <ival> OPTION_PEER_RESERVED                     "option_peer_reserved"
%token <ival> OPTION_PEER_INTERNAL_PORT                "option_peer_internal_port"
%token <ival> OPTION_PEER_ASSIGNED_EXTERNAL_PORT       "option_peer_assigned_external_port"
%token <ival> OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS "option_peer_assigned_external_ip_address"
%token <ival> OPTION_PEER_REMOTE_PEER_PORT             "option_peer_remote_port"
%token <ival> OPTION_PEER_RESERVED_2                   "option_peer_reserved_2"
%token <ival> OPTION_PEER_REMOTE_PEER_IP_ADDRESS       "option_peer_remote_peer_address"
%token <ival> OPTION_CODE                              "option_code"
%token <ival> OPTION_RESERVED                          "option_reserved"
%token <ival> OPTION_LENGTH                            "option_length"
%token <ival> OPTION_THIRD_PARTY_ADDRESS               "option_third_party_address"
%token <ival> OPTION_FILTER_RESERVED                   "option_filter_reserved"
%token <ival> OPTION_FILTER_PREFIX_LENGTH              "option_filter_prefix_length"
%token <ival> OPTION_FILTER_REMOTE_PEER_PORT           "option_filter_remote_peer_port"
%token <ival> OPTION_FILTER_REMOTE_PEER_IP_ADDRESS     "option_filter_remote_peer_address"
%token <ival> END 0                                    "end"

%type  <ival> message header opcode_specific opcode_specific_map opcode_specific_peer options option option_data option_data_third_party option_data_filter

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival> */
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), $$); }             <ival>
/* %printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (*$$).c_str ()); } <sval> */
//%destructor               { ACE_OS::memset ($$, 0, 16); } <aval>
%destructor               { $$ = 0; }                     <ival>
/* %destructor               { delete $$; $$ = NULL; } <sval> */
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start                   message;

message:                 header opcode_specific options             { $$ = $1 + $2 + $3; }
header:                  "version"                                  { driver->record_->version =
                                                                        static_cast<enum PCP_Codes::VersionType> ($1); }
                         "opcode"                                   { driver->record_->opcode = $3; }
                         "reserved"                                 { driver->record_->reserved = $5; }
                         "result_code"                              { driver->record_->result_code =
                                                                        static_cast<enum PCP_Codes::ResultCodeType> ($7); }
                         "lifetime"                                 { driver->record_->lifetime = $9; }
                         "epoch_time"                               { driver->record_->epoch_time = $11; }
                         "reserved_2"                               { $$ = $1 + $3 + $5 + $7 + $9 + $11 + $13;
                                                                      driver->record_->reserved_2 = $13; }
opcode_specific:         opcode_specific_map                        { $$ = $1; }
                         | opcode_specific_peer                     { $$ = $1; }
                         | /* empty */                              { $$ = 0; }
opcode_specific_map:     "option_map_nonce"                         { driver->record_->map.nonce = $1; }
                         "option_map_protocol"                      { driver->record_->map.protocol = $3; }
                         "option_map_reserved"                      { driver->record_->map.reserved = $5; }
                         "option_map_internal_port"                 { driver->record_->map.internal_port = $7; }
                         "option_map_assigned_external_port"        { driver->record_->map.external_port = $9; }
                         "option_map_assigned_external_ip_address"  { $$ = $1 + $3 + $5 + $7 + $9 + $11;
                                                                      int result =
                                                                        driver->record_->map.external_address.set (driver->record_->map.external_port,
                                                                                                                   $11,
                                                                                                                   1,
                                                                                                                   AF_INET);
                                                                      if (result == -1)
                                                                      {
                                                                        ACE_DEBUG ((LM_ERROR,
                                                                                    ACE_TEXT ("failed to ACE_INET_Addr::set(%u,%u): \"%m\", aborting\n"),
                                                                                    driver->record_->map.external_port,
                                                                                    $11));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
opcode_specific_peer:    "option_peer_nonce"                        { driver->record_->peer.nonce = $1; }
                         "option_peer_protocol"                     { driver->record_->peer.protocol = $3; }
                         "option_peer_reserved"                     { driver->record_->peer.reserved = $5; }
                         "option_peer_internal_port"                { driver->record_->peer.internal_port = $7; }
                         "option_peer_assigned_external_port"       { driver->record_->peer.external_port = $9; }
                         "option_peer_assigned_external_ip_address" { int result =
                                                                      driver->record_->peer.external_address.set (driver->record_->peer.external_port,
                                                                                                                  $11,
                                                                                                                  1,
                                                                                                                  AF_INET);
                                                                      if (result == -1)
                                                                      {
                                                                        ACE_DEBUG ((LM_ERROR,
                                                                                    ACE_TEXT ("failed to ACE_INET_Addr::set(%u,%u): \"%m\", aborting\n"),
                                                                                    driver->record_->peer.external_port,
                                                                                    $11));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
                         "option_peer_remote_port"                  { driver->record_->peer.remote_peer_port = $13; }
                         "option_peer_reserved_2"                   { driver->record_->peer.reserved_2 = $15; }
                         "option_peer_remote_peer_address"          { $$ = $1 + $3 + $5 + $7 + $9 + $11 + $13 + $15 + $17;
                                                                      int result =
                                                                        driver->record_->peer.remote_peer_address.set (driver->record_->peer.remote_peer_port,
                                                                                                                       $17,
                                                                                                                       1,
                                                                                                                       AF_INET);
                                                                      if (result == -1)
                                                                      {
                                                                        ACE_DEBUG ((LM_ERROR,
                                                                                    ACE_TEXT ("failed to ACE_INET_Addr::set(%u,%u): \"%m\", aborting\n"),
                                                                                    driver->record_->peer.remote_peer_port,
                                                                                    $17));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
options:                 option options                             { $$ = $1 + $2; }
                         | /* empty */                              { $$ = 0; }
option:                  "option_code"                              { struct PCPOption option_s;
                                                                      option_s.code = static_cast<enum PCP_Codes::OptionType> ($1);
                                                                      driver->record_->options.push_back (option_s); }
                         "option_reserved"                          { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.reserved = $3; }
                         "option_length"                            { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.length = $5; }
                         option_data                                { $$ = $1 + $3 + $5 + $7; }
option_data:             option_data_third_party                    { $$ = $1; }
                         | option_data_filter                       { $$ = $1; }
                         | /* empty */                              { $$ = 0; }
option_data_third_party: "option_third_party_address"               { $$ = $1;
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      int result =
                                                                        option_r.third_party.address.set (0,
                                                                                                          $1,
                                                                                                          0,
                                                                                                          AF_INET);
                                                                      if (result == -1)
                                                                      {
                                                                        ACE_DEBUG ((LM_ERROR,
                                                                                    ACE_TEXT ("failed to ACE_INET_Addr::set(%u,%u): \"%m\", aborting\n"),
                                                                                    0,
                                                                                    $1));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
option_data_filter:      "option_filter_reserved"                   { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.reserved = $1; }
                         "option_filter_prefix_length"              { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.prefix_length = $3; }
                         "option_filter_remote_peer_port"           { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.remote_peer_port = $5; }
                         "option_filter_remote_peer_address"        { $$ = $1 + $3 + $5 + $7;
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      int result =
                                                                        option_r.filter.remote_peer_address.set (option_r.filter.remote_peer_port,
                                                                                                                 $7,
                                                                                                                 0,
                                                                                                                 AF_INET);
                                                                      if (result == -1)
                                                                      {
                                                                        ACE_DEBUG ((LM_ERROR,
                                                                                    ACE_TEXT ("failed to ACE_INET_Addr::set(%u,%u): \"%m\", aborting\n"),
                                                                                    option_r.filter.remote_peer_port,
                                                                                    $7));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
%%

/* void
yy::PCP_Parser::error (const location_type& location_in,
                       const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::PCP_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Parser::set"));

  yyscanner = context_in;
} */

void
yyerror (YYLTYPE* location_in,
         PCP_ParserDriver* driver_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (location_in);
  ACE_ASSERT (driver_in);

  driver_in->error (*location_in, std::string (message_in));
}

//void
//yyprint (FILE* file_in,
//         yytokentype type_in,
//         YYSTYPE value_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::yyprint"));
//
//  int result = -1;
//
//  std::string format_string;
//  switch (type_in)
//  {
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
//                  type_in));
//      return;
//    }
//  } // end SWITCH
//}
