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
#undef YYTOKENTYPE
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

class PCP_ParserDriver;
//class PCP_Scanner;
struct YYLTYPE;
//union YYSTYPE;

union yytoken
{
  ACE_UINT64 ival;
  ACE_UINT8* aval;
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
  ACE_UINT64 ival;
  ACE_UINT8* aval;
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

%token <ival> VERSION                                   "version"
%token <ival> OPCODE                                    "opcode"
%token <ival> RESERVED                                  "reserved"
%token <ival> RESULT_CODE                               "result_code"
%token <ival> LIFETIME                                  "lifetime"
%token <ival> EPOCH_TIME                                "epoch_time"
%token <ival> RESERVED_2                                "reserved_2"
%token <ival> OPTION_MAP_NONCE                          "option_map_nonce"
%token <ival> OPTION_MAP_PROTOCOL                       "option_map_protocol"
%token <ival> OPTION_MAP_RESERVED                       "option_map_reserved"
%token <ival> OPTION_MAP_INTERNAL_PORT                  "option_map_internal_port"
%token <ival> OPTION_MAP_ASSIGNED_EXTERNAL_PORT         "option_map_assigned_external_port"
%token <ival> OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS   "option_map_assigned_external_ip_address"
%token <ival> OPTION_PEER_NONCE                         "option_peer_nonce"
%token <ival> OPTION_PEER_PROTOCOL                      "option_peer_protocol"
%token <ival> OPTION_PEER_RESERVED                      "option_peer_reserved"
%token <ival> OPTION_PEER_INTERNAL_PORT                 "option_peer_internal_port"
%token <ival> OPTION_PEER_ASSIGNED_EXTERNAL_PORT        "option_peer_assigned_external_port"
%token <ival> OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS  "option_peer_assigned_external_ip_address"
%token <ival> OPTION_PEER_REMOTE_PEER_PORT              "option_peer_remote_port"
%token <ival> OPTION_PEER_RESERVED_2                    "option_peer_reserved_2"
%token <ival> OPTION_PEER_REMOTE_PEER_IP_ADDRESS        "option_peer_remote_peer_address"
%token <ival> OPTION_AUTHENTICATION_SESSION_ID          "option_authentication_session_id" // rfc7652
%token <ival> OPTION_AUTHENTICATION_SEQUENCE_NUMBER     "option_authentication_sequence_number" // rfc7652
%token <ival> OPTION_CODE                               "option_code"
%token <ival> OPTION_RESERVED                           "option_reserved"
%token <ival> OPTION_LENGTH                             "option_length"
%token <ival> OPTION_THIRD_PARTY_ADDRESS                "option_third_party_address"
%token <ival> OPTION_FILTER_RESERVED                    "option_filter_reserved"
%token <ival> OPTION_FILTER_PREFIX_LENGTH               "option_filter_prefix_length"
%token <ival> OPTION_FILTER_REMOTE_PEER_PORT            "option_filter_remote_peer_port"
%token <ival> OPTION_FILTER_REMOTE_PEER_IP_ADDRESS      "option_filter_remote_peer_address"
%token <ival> OPTION_NONCE_NONCE                        "option_nonce_nonce" // rfc7652
%token <ival> OPTION_AUTHENTICATION_TAG_SESSION_ID      "option_authentication_tag_session_id" // rfc7652
%token <ival> OPTION_AUTHENTICATION_TAG_SEQUENCE_NUMBER "option_authentication_tag_sequence_number" // rfc7652
%token <ival> OPTION_AUTHENTICATION_TAG_KEY_ID          "option_authentication_tag_key_id" // rfc7652
%token <aval> OPTION_AUTHENTICATION_TAG_DATA            "option_authentication_tag_data" // rfc7652
%token <ival> OPTION_PA_AUTHENTICATION_TAG_KEY_ID       "option_pa_authentication_tag_key_id" // rfc7652
%token <aval> OPTION_PA_AUTHENTICATION_TAG_DATA         "option_pa_authentication_tag_data" // rfc7652
%token <aval> OPTION_EAP_PAYLOAD_DATA                   "option_eap_payload_data" // rfc7652
%token <ival> OPTION_PSEUDO_RANDOM_FUNCTION_ID          "option_pseudo_random_function_id" // rfc7652
%token <ival> OPTION_MAC_ALGORITHM_ID                   "option_mac_algorithm_id" // rfc7652
%token <ival> OPTION_SESSION_LIFETIME_LIFETIME          "option_session_lifetime_lifetime" // rfc7652
%token <ival> OPTION_RECEIVED_PAK_SEQUENCE_NUMBER       "option_received_pak_sequence_number" // rfc7652
%token <aval> OPTION_ID_INDICATOR_DATA                  "option_id_indicator_data" // rfc7652
%token <ival> END 0                                     "end"

%type  <ival> message header opcode_specific opcode_specific_map opcode_specific_peer opcode_specific_authentication options option
%type  <ival> option_data option_data_third_party option_data_filter option_data_nonce option_data_authentication_tag option_data_pa_authentication_tag
%type  <ival> option_data_eap_payload option_data_pseudo_random_function option_data_mac_algorithm option_data_session_lifetime
%type  <ival> option_data_received_pak option_data_id_indicator

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival> */
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), $$); }   <ival>
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), ($$)); } <aval>
//%destructor               { ACE_OS::memset ($$, 0, 16); } <aval>
%destructor               { $$ = 0; }                 <ival>
%destructor               { delete[] $$; $$ = NULL; } <aval>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start                   message;

message:                 header opcode_specific options             { $$ = $1 + $2 + $3; }
header:                  "version"                                  { driver->record_->version =
                                                                        static_cast<enum PCP_Codes::VersionType> ($1); }
                         "opcode"                                   { driver->record_->opcode = static_cast<ACE_UINT8> ($3); }
                         "reserved"                                 { driver->record_->reserved = static_cast<ACE_UINT8> ($5); }
                         "result_code"                              { driver->record_->result_code =
                                                                        static_cast<enum PCP_Codes::ResultCodeType> ($7); }
                         "lifetime"                                 { driver->record_->lifetime = static_cast<ACE_UINT32> ($9); }
                         "epoch_time"                               { driver->record_->epoch_time = static_cast<ACE_UINT32> ($11); }
                         "reserved_2"                               { $$ = $1 + $3 + $5 + $7 + $9 + $11 + $13;
                                                                      driver->record_->reserved_2 = static_cast<ACE_UINT32> ($13); }
opcode_specific:         opcode_specific_map                        { $$ = $1; }
                         | opcode_specific_peer                     { $$ = $1; }
                         | opcode_specific_authentication           { $$ = $1; } // rfc7652
                         | /* empty */                              { $$ = 0; }
opcode_specific_map:     "option_map_nonce"                         { driver->record_->map.nonce = $1; }
                         "option_map_protocol"                      { driver->record_->map.protocol = static_cast<ACE_UINT8> ($3); }
                         "option_map_reserved"                      { driver->record_->map.reserved = static_cast<ACE_UINT32> ($5); }
                         "option_map_internal_port"                 { driver->record_->map.internal_port = static_cast<ACE_UINT16> ($7); }
                         "option_map_assigned_external_port"        { driver->record_->map.external_port = static_cast<ACE_UINT16> ($9); }
                         "option_map_assigned_external_ip_address"  { $$ = $1 + $3 + $5 + $7 + $9 + $11;
                                                                      ACE_NEW_NORETURN (driver->record_->map.external_address,
                                                                                        ACE_INET_Addr (driver->record_->map.external_port,
                                                                                                       static_cast<ACE_UINT32> ($11)));
                                                                      if (!driver->record_->map.external_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
opcode_specific_peer:    "option_peer_nonce"                        { driver->record_->peer.nonce = $1; }
                         "option_peer_protocol"                     { driver->record_->peer.protocol = static_cast<ACE_UINT8> ($3); }
                         "option_peer_reserved"                     { driver->record_->peer.reserved = static_cast<ACE_UINT32> ($5); }
                         "option_peer_internal_port"                { driver->record_->peer.internal_port = static_cast<ACE_UINT16> ($7); }
                         "option_peer_assigned_external_port"       { driver->record_->peer.external_port = static_cast<ACE_UINT16> ($9); }
                         "option_peer_assigned_external_ip_address" { ACE_NEW_NORETURN (driver->record_->peer.external_address,
                                                                                        ACE_INET_Addr (driver->record_->peer.external_port,
                                                                                                       static_cast<ACE_UINT32> ($11)));
                                                                      if (!driver->record_->peer.external_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
                         "option_peer_remote_port"                  { driver->record_->peer.remote_peer_port = static_cast<ACE_UINT16> ($13); }
                         "option_peer_reserved_2"                   { driver->record_->peer.reserved_2 = static_cast<ACE_UINT16> ($15); }
                         "option_peer_remote_peer_address"          { $$ = $1 + $3 + $5 + $7 + $9 + $11 + $13 + $15 + $17;
                                                                      ACE_NEW_NORETURN (driver->record_->peer.remote_peer_address,
                                                                                        ACE_INET_Addr (driver->record_->peer.remote_peer_port,
                                                                                                       static_cast<ACE_UINT32> ($17)));
                                                                      if (!driver->record_->peer.remote_peer_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
opcode_specific_authentication: "option_authentication_session_id"  { driver->record_->authentication.session_id = static_cast<ACE_UINT32> ($1); } // rfc7652
                                "option_authentication_sequence_number" { $$ = $1 + $3; // rfc7652
                                                                          driver->record_->authentication.sequence_number = static_cast<ACE_UINT32> ($3); }
options:                 option options                             { $$ = $1 + $2; }
                         | /* empty */                              { $$ = 0; }
option:                  "option_code"                              { struct PCPOption option_s;
                                                                      option_s.code = static_cast<enum PCP_Codes::OptionType> ($1);
                                                                      driver->record_->options.push_back (option_s); }
                         "option_reserved"                          { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.reserved = static_cast<ACE_UINT8> ($3); }
                         "option_length"                            { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.length = static_cast<ACE_UINT16> ($5); }
                         option_data                                { $$ = $1 + $3 + $5 + $7; }
option_data:             option_data_third_party                    { $$ = $1; }
                         | option_data_filter                       { $$ = $1; }
                         | option_data_nonce                        { $$ = $1; } // rfc7652
                         | option_data_authentication_tag           { $$ = $1; } // rfc7652
                         | option_data_pa_authentication_tag        { $$ = $1; } // rfc7652
                         | option_data_eap_payload                  { $$ = $1; } // rfc7652
                         | option_data_pseudo_random_function       { $$ = $1; } // rfc7652
                         | option_data_mac_algorithm                { $$ = $1; } // rfc7652
                         | option_data_session_lifetime             { $$ = $1; } // rfc7652
                         | option_data_received_pak                 { $$ = $1; } // rfc7652
                         | option_data_id_indicator                 { $$ = $1; } // rfc7652
                         | /* empty */                              { $$ = 0; }
option_data_third_party: "option_third_party_address"               { $$ = $1;
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      ACE_NEW_NORETURN (option_r.third_party.address,
                                                                                        ACE_INET_Addr (static_cast<u_short> (0),
                                                                                                       static_cast<ACE_UINT32> ($1)));
                                                                      if (!option_r.third_party.address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
option_data_filter:      "option_filter_reserved"                   { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.reserved = static_cast<ACE_UINT8> ($1); }
                         "option_filter_prefix_length"              { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.prefix_length = static_cast<ACE_UINT8> ($3); }
                         "option_filter_remote_peer_port"           { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.remote_peer_port = static_cast<ACE_UINT16> ($5); }
                         "option_filter_remote_peer_address"        { $$ = $1 + $3 + $5 + $7;
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      ACE_NEW_NORETURN (option_r.filter.remote_peer_address,
                                                                                        ACE_INET_Addr (option_r.filter.remote_peer_port,
                                                                                                       static_cast<ACE_UINT32> ($7)));
                                                                      if (!option_r.filter.remote_peer_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
option_data_nonce:       "option_nonce_nonce"                       { $$ = $1; // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.nonce.nonce = static_cast<ACE_UINT32> ($1); }
option_data_authentication_tag: "option_authentication_tag_session_id" { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                         option_r.authentication_tag.session_id = static_cast<ACE_UINT32> ($1); }
                                "option_authentication_tag_sequence_number" { struct PCPOption& option_r = driver->record_->options.back ();
                                                                              option_r.authentication_tag.sequence_number = static_cast<ACE_UINT32> ($3); }
                                "option_authentication_tag_key_id"  { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.authentication_tag.key_id = static_cast<ACE_UINT32> ($5); }
                                "option_authentication_tag_data"    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      $$ = $1 + $3 + $5 + (option_r.length - 12);
                                                                      option_r.authentication_tag.data = $7; }
option_data_pa_authentication_tag: "option_pa_authentication_tag_key_id" { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                           option_r.pa_authentication_tag.key_id = static_cast<ACE_UINT32> ($1); }
                                   "option_pa_authentication_tag_data" { struct PCPOption& option_r = driver->record_->options.back ();
                                                                         $$ = $1 + (option_r.length - 4);
                                                                         option_r.pa_authentication_tag.data = $3; }
option_data_eap_payload: "option_eap_payload_data"                  { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                      $$ = option_r.length;
                                                                      option_r.eap_payload.data = $1; }
option_data_pseudo_random_function: "option_pseudo_random_function_id" { $$ = $1; // rfc7652
                                                                         struct PCPOption& option_r = driver->record_->options.back ();
                                                                         option_r.pseudo_random_function.id = static_cast<ACE_UINT32> ($1); }
option_data_mac_algorithm: "option_mac_algorithm_id"                { $$ = $1; // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.mac_algorithm.id = static_cast<ACE_UINT32> ($1); }
option_data_session_lifetime: "option_session_lifetime_lifetime"    { $$ = $1; // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.session_lifetime.lifetime = static_cast<ACE_UINT32> ($1); }
option_data_received_pak: "option_received_pak_sequence_number"     { $$ = $1; // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.received_pak.sequence_number = static_cast<ACE_UINT32> ($1); }
option_data_id_indicator: "option_id_indicator_data"                { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                      $$ = option_r.length;
                                                                      option_r.id_indicator.data = $1; }
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
