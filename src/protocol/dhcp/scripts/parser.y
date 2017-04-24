%defines                          "dhcp_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "dhcp_parser.cpp"
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
class DHCP_ParserDriver;
//class DHCP_Scanner;
struct YYLTYPE;
//union YYSTYPE;

union yytoken
{
  unsigned char aval[16];
  int           ival;
  std::string*  sval;
};
typedef yytoken yytoken_t;
#define YYSTYPE yytoken_t

typedef void* yyscan_t;

//#define YYERROR_VERBOSE
extern void yyerror (YYLTYPE*, DHCP_ParserDriver*, yyscan_t, const char*);
extern int yyparse (DHCP_ParserDriver*, yyscan_t);

#undef YYPRINT
//extern void yyprint (FILE*, yytokentype, YYSTYPE);
}

// calling conventions / parameter passing
%parse-param              { DHCP_ParserDriver* driver }
%parse-param              { yyscan_t yyscanner }
/* %lex-param                { YYSTYPE* yylval } */
/* %lex-param                { YYLTYPE* yylloc } */
%lex-param                { DHCP_ParserDriver* driver }
%lex-param                { yyscan_t yyscanner }

%initial-action
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  //@$.begin.filename = @$.end.filename = &driver->file;
  ACE_OS::memset (&@$, 0, sizeof (YYLTYPE));

  // initialize the token value container
//  $$.aval = {};
  $$.ival = 0;
  $$.sval = NULL;
};

// symbols
%union
{
  unsigned char aval[16];
  int           ival;
  std::string*  sval;
};
/* %token <int>         INTEGER;
%token <std::string> STRING; */

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif
//#include <regex>
//#include <sstream>
#include <string>
#include <utility>

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

#include "dhcp_common.h"
#include "dhcp_defines.h"
#include "dhcp_parser_driver.h"
#include "dhcp_scanner.h"
#include "dhcp_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex DHCP_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <ival> OP           "op"
%token <ival> HTYPE        "htype"
%token <ival> HLEN         "hlen"
%token <ival> HOPS         "hops"
%token <ival> XID          "xid"
%token <ival> SECS         "secs"
%token <ival> FLAGS        "flags"
%token <ival> CIADDR       "ciaddr"
%token <ival> YIADDR       "yiaddr"
%token <ival> SIADDR       "siaddr"
%token <ival> GIADDR       "giaddr"
%token <aval> CHADDR       "chaddr"
%token <sval> SNAME        "sname"
%token <sval> FILE_        "file"
%token <ival> COOKIE       "cookie"
%token <ival> OPTION_TAG   "tag"
%token <sval> OPTION_VALUE "value"
%token <ival> END 0        "end"

%type  <ival> message header options option

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << *$$; } <sval>
%printer                  { debug_stream () << $$; }  <ival> */
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), Net_Common_Tools::MACAddressToString ($$).c_str ()); } <aval>
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %d"), $$); }                                                 <ival>
%printer                  { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (*$$).c_str ()); }                                     <sval>
//%destructor               { ACE_OS::memset ($$, 0, 16); } <aval>
//%destructor               { $$ = 0; }                     <ival>
%destructor               { delete $$; $$ = NULL; }       <sval>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start        message;

message:            header "cookie" options          { $$ = $1 + 4 + 308;
                                                       driver->record_->cookie = $2;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set cookie: %d\n"),
//                                                                   driver->record_->cookie));
                                                       ACE_ASSERT (driver->record_->cookie == DHCP_MAGIC_COOKIE);
                                                     };
header:             "op" "htype" "hlen" "hops" "xid" "secs" "flags" "ciaddr" "yiaddr" "siaddr" "giaddr" "chaddr" "sname" "file" { $$ = $1 + $2 + $3 + $4 + $5 + $6 + $7 + $8 + $9 + $10 + $11 + 16 + 64 + 128;
                                                       driver->record_->op =
                                                           static_cast <DHCP_Codes::OpType> ($1);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set op: \"%s\"\n"),
//                                                                   ACE_TEXT (DHCP_Tools::Op2String (driver->record_->op).c_str ())));
                                                       driver->record_->htype = $2;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set htype: %d\n"),
//                                                                   static_cast<int> (driver->record_->htype)));
                                                       driver->record_->hlen = $3;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set hlen: %d\n"),
//                                                                   static_cast<int> (driver->record_->hlen)));
                                                       driver->record_->hops = $4;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set hops: %d\n"),
//                                                                   static_cast<int> (driver->record_->hops)));
                                                       driver->record_->xid =
                                                         ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG ($5) : $5);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set xid: %d\n"),
//                                                                   driver->record_->xid));
                                                       driver->record_->secs = $6;
                                                       if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
                                                         ACE_SWAP_WORD (driver->record_->secs);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set secs: %d\n"),
//                                                                   static_cast<int> (driver->record_->secs)));
                                                       driver->record_->flags = $7;
                                                       if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
                                                         ACE_SWAP_WORD (driver->record_->flags);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set flags: %d\n"),
//                                                                   static_cast<int> (driver->record_->flags)));
                                                       driver->record_->ciaddr = $8;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set ciaddr: \"%s\"\n"),
//                                                                   ACE_TEXT (Net_Tools::IP2String (driver->record_->ciaddr).c_str ())));
                                                       driver->record_->yiaddr = $9;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set yiaddr: \"%s\"\n"),
//                                                                   ACE_TEXT (Net_Tools::IP2String (driver->record_->yiaddr).c_str ())));
                                                       driver->record_->siaddr = $10;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set siaddr: \"%s\"\n"),
//                                                                   ACE_TEXT (Net_Tools::IP2String (driver->record_->siaddr).c_str ())));
                                                       driver->record_->giaddr = $11;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set giaddr: \"%s\"\n"),
//                                                                   ACE_TEXT (Net_Tools::IP2String (driver->record_->giaddr).c_str ())));
                                                       ACE_OS::memcpy (driver->record_->chaddr, $12, DHCP_CHADDR_SIZE);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set chaddr: %s\n"),
//                                                                   ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (driver->record_->chaddr).c_str ())));
                                                       ACE_ASSERT ($13);
                                                       driver->record_->sname = *$13;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set sname: \"%s\"\n"),
//                                                                   ACE_TEXT (driver->record_->sname.c_str ())));
                                                       ACE_ASSERT ($14);
                                                       driver->record_->file = *$14;
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set file: \"%s\"\n"),
//                                                                   ACE_TEXT (driver->record_->file.c_str ())));
                                                     };
options:                                             /* empty */
                    | option options                 { $$ = $1 + $2; };
option:             "tag"                            { $$ = $1;
                                                       if ($1 == 255)
                                                         YYACCEPT; };
                    | "tag" "value"                  { $$ = $1 + 1 + (*$2).size ();
                                                       driver->record_->options.insert (std::make_pair (static_cast<unsigned char> ($1),
                                                                                                        *$2)); };
%%

/* void
yy::DHCP_Parser::error (const location_type& location_in,
                        const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::DHCP_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Parser::set"));

  yyscanner = context_in;
} */

void
yyerror (YYLTYPE* location_in,
         DHCP_ParserDriver* driver_in,
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
//    case OP:
//    case HTYPE:
//    case HLEN:
//    case HOPS:
//    case XID:
//    case SECS:
//    case FLAGS:
//    {
//      result = ACE_OS::fprintf (file_in,
//                                ACE_TEXT (" %d"),
//                                value_in.ival);
//      if (result < 0)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
//      break;
//    }
//    case CIADDR:
//    case YIADDR:
//    case SIADDR:
//    case GIADDR:
//    {
//      std::string address_string =
//          Net_Common_Tools::IPAddress2String (0, value_in.ival);
//      result = ACE_OS::fprintf (file_in,
//                                ACE_TEXT (" %s"),
//                                address_string.c_str ());
//      if (result < 0)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
//      break;
//    }
//    case CHADDR:
//    {
//      std::string address_string =
//          Net_Common_Tools::MACAddress2String (value_in.aval);
//      result = ACE_OS::fprintf (file_in,
//                                ACE_TEXT (" %s"),
//                                address_string.c_str ());
//      if (result < 0)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
//      break;
//    }
//    case SNAME:
//    case FILE_:
//    case OPTION_KEY:
//    case OPTION_VALUE:
//    {
//      ACE_ASSERT (value_in.sval);
//      result = ACE_OS::fprintf (file_in,
//                                ACE_TEXT (" %s"),
//                                value_in.sval->c_str ());
//      if (result < 0)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
//                  type_in));
//      return;
//    }
//  } // end SWITCH
//}
