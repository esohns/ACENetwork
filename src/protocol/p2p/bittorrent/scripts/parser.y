%defines                          "bittorrent_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "bittorrent_parser.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/* %skeleton                         "lalr1.cc" */
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
}

/* %define location_type */
/* %define api.location.type         {} */
/* %define namespace                 {yy} */
/*%define api.namespace             {yy}*/
%name-prefix                      "bittorrent_"
/* %define api.prefix                {yy} */
%pure-parser
/* %define api.pure                  true */
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/* %define api.token.constructor */
/* %define api.token.prefix          {} */
%token-table
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
/*%define parser_class_name         {BitTorrent_Parser}*/
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               {true} */

%code requires {
// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef BITTORRENT_PARSER_H
//#define BITTORRENT_PARSER_H

#include <cstdio>
#include <string>

#include "bittorrent_exports.h"

/* enum yytokentype
{
  END = 0,
  HANDSHAKE = 258,
  MESSAGE_BITFIELD = 260,
  MESSAGE_CANCEL = 262,
  MESSAGE_HAVE = 259,
  MESSAGE_PIECE = 263,
  MESSAGE_PORT = 263,
  MESSAGE_REQUEST = 261,
}; */
//#define YYTOKENTYPE
/*#undef YYTOKENTYPE*/
/* enum yytokentype; */
class BitTorrent_IParser;
struct BitTorrent_PeerHandshake;
struct BitTorrent_Record;
//class BitTorrent_Scanner;
//struct YYLTYPE;

/* #define YYSTYPE
typedef union YYSTYPE
{
  int          ival;
  std::string* sval;
} YYSTYPE; */
#undef YYSTYPE
//union YYSTYPE;

typedef void* yyscan_t;

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually; apparently, there is no easy way to add the export
//         symbol to the declaration
#define YYDEBUG 1
extern int BitTorrent_Export yydebug;
#define YYERROR_VERBOSE 1
}

// calling conventions / parameter passing
%parse-param              { BitTorrent_IParser* iparser_p }
%parse-param              { yyscan_t yyscanner }
/*%lex-param                { YYSTYPE* yylval }
%lex-param                { YYLTYPE* yylloc } */
%lex-param                { BitTorrent_IParser* iparser_p }
%lex-param                { yyscan_t yyscanner }
/* %param                    { BitTorrent_IParser* iparser_p }
%param                    { yyscan_t yyscanner } */

%initial-action
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  ACE_OS::memset (&@$, 0, sizeof (YYLTYPE));

  // initialize the token value container
  $$.handshake = NULL;
  $$.record = NULL;
  $$.size = 0;
}

// symbols
%union
{
  struct BitTorrent_PeerHandshake* handshake;
  struct BitTorrent_Record*        record;
  unsigned int                     size;
}

/* %token <int>         INTEGER;
%token <std::string> STRING; */

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif
#include <regex>
#include <sstream>
#include <string>

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"
#include <ace/Synch.h>
#include "bittorrent_parser_driver.h"
#include "bittorrent_scanner.h"
#include "bittorrent_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex bittorrent_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <handshake> HANDSHAKE       "handshake"
%token <record>    BITFIELD        "bitfield"
%token <record>    CANCEL          "cancel"
%token <record>    CHOKE           "choke"
%token <record>    HAVE            "have"
%token <record>    INTERESTED      "interested"
%token <record>    KEEP_ALIVE      "keep-alive"
%token <record>    NOT_INTERESTED  "not_interested"
%token <record>    PIECE           "piece"
%token <record>    PORT            "port"
%token <record>    REQUEST         "request"
%token <record>    UNCHOKE         "unchoke"
%token <size>      END_OF_FRAGMENT "end_of_fragment"
%token <size>      END 0           "end"

%type  <size>      session messages message

%code provides {
void BitTorrent_Export yysetdebug (int);
void BitTorrent_Export yyerror (YYLTYPE*, BitTorrent_IParser*, yyscan_t, const char*);
int BitTorrent_Export yyparse (BitTorrent_IParser*, yyscan_t);
void BitTorrent_Export yyprint (FILE*, yytokentype, YYSTYPE);

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // BITTORRENT_PARSER_H
}

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival> */
%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), $$->version.c_str ()); } <handshake>
%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), BitTorrent_Tools::Type2String ($$->type).c_str ()); } <record>
%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %d"), $$); } <size>
/*%destructor { delete $$; $$ = NULL; } <handshake>*/
/*%destructor { delete $$; $$ = NULL; } <record>*/
%destructor { $$ = NULL; } <handshake>
%destructor { $$ = NULL; } <record>
%destructor { $$ = 0; } <size>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start    session              ;
session:  "handshake" messages { $$ = 67 + $2; // 19 + 8 + 20 + 20
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_PeerHandshake* handshake_p =
                                   const_cast<struct BitTorrent_PeerHandshake*> ($1);
                                 try {
                                   iparser_p->handshake (handshake_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::handshake(), continuing\n")));
                                 } };
messages: message messages     { $$ = $1 + $2; };
          | %empty             { $$ = 0; };
message:  "bitfield"           { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "cancel"           { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "choke"            { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "have"             { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "interested"       { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "keep-alive"       { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "not_interested"   { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "piece"            { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "port"             { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "request"          { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "unchoke"          { $$ = $1->length + 4;
                                 ACE_ASSERT ($1);
                                 struct BitTorrent_Record* record_p =
                                   const_cast<struct BitTorrent_Record*> ($1);
                                 BitTorrent_Message_t& message_r =
                                   iparser_p->current ();
                                 typename BitTorrent_IParser::DATA_CONTAINER_T& data_container_r =
                                   const_cast<typename BitTorrent_IParser::DATA_CONTAINER_T&> (message_r.get ());
                                 data_container_r.set (record_p);
                                 BitTorrent_Message_t* message_p = &message_r;
                                 try {
                                   iparser_p->message (message_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::message(), continuing\n")));
                                 } };
          | "end_of_fragment"  { $$ = $1;
                                 YYACCEPT; };
%%

/* void
yy::BitTorrent_Parser::error (const location_type& location_in,
                              const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::BitTorrent_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Parser::set"));

  yyscanner = context_in;
} */

void
yysetdebug (int debug_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yysetdebug"));

  yydebug = debug_in;
}

void
yyerror (YYLTYPE* location_in,
         BitTorrent_IParser* iparser_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

  ACE_UNUSED_ARG (location_in);
  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (iparser_in);

//  iparser_in->error (*location_in, std::string (message_in));
  iparser_in->error (std::string (message_in));
}

void
yyprint (FILE* file_in,
         yytokentype type_in,
         YYSTYPE value_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyprint"));

  int result = -1;

  std::string format_string;
  switch (type_in)
  {
    case HANDSHAKE:
    case BITFIELD:
    case CANCEL:
    case CHOKE:
    case HAVE:
    case INTERESTED:
    case KEEP_ALIVE:
    case NOT_INTERESTED:
    case PIECE:
    case PORT:
    case REQUEST:
    case UNCHOKE:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case END_OF_FRAGMENT:
    case END:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %d");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH

  result = ACE_OS::fprintf (file_in,
                            ACE_TEXT (format_string.c_str ()),
                            value_in);
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
}
