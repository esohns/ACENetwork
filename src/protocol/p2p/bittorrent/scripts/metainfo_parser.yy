/*%require                          "2.4.1"*/
%require                          "3.0"
/* %file-prefix                      "" */
%language                         "c++"
/*%language                         "C"*/
%locations
%no-lines
/*%skeleton                         "glr.c"*/
%skeleton                         "lalr1.cc"
%verbose
/* %yacc */

%defines                          "bittorrent_metainfo_parser.h"
%output                           "bittorrent_metainfo_parser.cpp"

/*%name-prefix                      "bittorrent_metainfo_"*/
/*%pure-parser*/
/*%token-table*/
/*%error-verbose*/
/*%debug*/

%code top {
#include "stdafx.h"

#include <ace/Synch.h>
#include "bittorrent_metainfo_parser.h"
}

/* %define location_type */
/* %define api.location.type         {} */
/* %define namespace                 {yy} */
%define api.namespace             {yy}
/* %define api.prefix                {yy} */
/* %define api.pure                  true */
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
%define api.token.constructor
/* %define api.token.prefix          {} */
%define api.value.type            variant
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

%define parse.assert              {true}
%define parse.error               verbose
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
%define parser_class_name         {BitTorrent_MetaInfo_Parser}
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%define parse.trace               {true}

%code requires {
// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef BitTorrent_MetaInfo_Parser_H
//#define BitTorrent_MetaInfo_Parser_H

/*#include <cstdio>
#include <string>*/

/*#include "bencoding_scanner.h"*/
/*#include "bittorrent_exports.h"*/
#include "bittorrent_iparser.h"

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
//struct YYLTYPE;
class Net_BencodingScanner;

/* #define YYSTYPE
typedef union YYSTYPE
{
  int          ival;
  std::string* sval;
} YYSTYPE; */
/*#undef YYSTYPE*/
//union YYSTYPE;

/*typedef void* yyscan_t;*/

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually; apparently, there is no easy way to add the export
//         symbol to the declaration
#define YYDEBUG 1
/*extern int BitTorrent_Export yydebug;*/
#define YYERROR_VERBOSE 1
}

// calling conventions / parameter passing
%parse-param              { BitTorrent_MetaInfo_IParser_t* parser }
%parse-param              { Net_BencodingScanner* scanner }
// *NOTE*: cannot use %initial-action, as it is scoped
// *TODO*: find a better way to do this
%parse-param              { std::string* dictionary_key }
/*%parse-param              { yyscan_t yyscanner }*/
/*%lex-param                { YYSTYPE* yylval }
%lex-param                { YYLTYPE* yylloc } */
%lex-param                { BitTorrent_MetaInfo_IParser_t* parser }
/*%lex-param                { Net_BencodingScanner* scanner }*/
/*%lex-param                { yyscan_t yyscanner }*/
/* %param                    { BitTorrent_MetaInfo_IParser_t* iparser_p }
%param                    { yyscan_t yyscanner } */

%initial-action
{
  // initialize the location
  @$.initialize (YY_NULLPTR, 1, 1);

  // sanity check(s)
  ACE_ASSERT (dictionary_key);
/*  std::string dictionary_key;*/
}

// symbols
/*%union
{
  int          ival;
  std::string* sval;
}*/

/*%token <int>         INTEGER;*/
/*%token <std::string> STRING; */

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
/*#if defined (YYDEBUG)
#include <iostream>
#endif*/
/*#include <regex>*/
#include <sstream>
#include <string>

// *WORKAROUND*
/*using namespace std;*/
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
/*#define ACE_IOSFWD_H*/

#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include "net_macros.h"

#include "bencoding_scanner.h"
#include "bittorrent_common.h"
#include "bittorrent_defines.h"
/*#include <ace/Synch.h>*/
#include "bittorrent_metainfo_parser_driver.h"
#include "bittorrent_tools.h"

// *TODO*: this shouldn't be necessary
/*#define yylex bencoding_lex*/
#define yylex scanner->yylex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token
 DICTIONARY      "dictionary"
 END_OF_FRAGMENT "end_of_fragment"
 END 0           "end"
;
%token <std::string>                STRING  "string"
%token <int>                        INTEGER "integer"
%token <BitTorrent_MetaInfo_List_t> LIST    "list"
%type  <unsigned int> metainfo
%type  <unsigned int> dictionary_items dictionary_item dictionary_value
%type  <unsigned int> list_items list_item

%code provides {
/*void BitTorrent_Export yysetdebug (int);
void BitTorrent_Export yyerror (YYLTYPE*, BitTorrent_MetaInfo_IParser*, yyscan_t, const char*);
int BitTorrent_Export yyparse (BitTorrent_MetaInfo_IParser*, yyscan_t);
void BitTorrent_Export yyprint (FILE*, yytokentype, YYSTYPE);*/

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // BitTorrent_MetaInfo_Parser_H
}

/*%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), $$->c_str ()); } <sval>
%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %d"), $$); } <ival>
%destructor { delete $$; $$ = NULL; } <sval>
%destructor { $$ = 0; } <ival>*/
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */
/*%printer    { yyoutput << $$; } <*>;*/
%printer    { yyoutput << $$; } <std::string>
%printer    { yyoutput << $$; } <unsigned int>
%printer    { for (BitTorrent_MetaInfo_ListIterator_t iterator = $$.begin();
                   iterator != $$.end ();
                   ++iterator)
                yyoutput << *iterator << ','; } <BitTorrent_MetaInfo_List_t>

%%
%start            metainfo   ;
metainfo:         dictionary_items { $$ = $1;
                                     struct BitTorrent_MetaInfo& metainfo_r =
                                       parser->current ();
                                     struct BitTorrent_MetaInfo* metainfo_p =
                                       &metainfo_r;
                                     try {
                                       parser->record (metainfo_p);
                                     } catch (...) {
                                       ACE_DEBUG ((LM_ERROR,
                                                   ACE_TEXT ("caught exception in BitTorrent_MetaInfo_IParser::record(), continuing\n")));
                                     } };
list_items:       list_item list_items { $$ = $1 + $2; };
                  | %empty             { $$ = 0; };
list_item:        "string"     { $$ = $1.size ();

                                 struct BitTorrent_MetaInfo& metainfo_r =
                                   parser->current ();
                                 if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("announce-list"))
                                 {
                                   BitTorrent_MetaInfo_AnnounceList_t& current_r =
                                     metainfo_r.announceList.back ();
                                   current_r.push_back ($1);
                                 } };
                  | "integer" { $$ = 0; };
                  | "list" list_items { $$ = $2;

                                        struct BitTorrent_MetaInfo& metainfo_r =
                                          const_cast<struct BitTorrent_MetaInfo&> (parser->current ());
                                        if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("announce-list"))
                                        {
                                          BitTorrent_MetaInfo_AnnounceList_t announce_list;
                                          metainfo_r.announceList.push_back (announce_list);
                                        } };
                  | "dictionary" dictionary_items { $$ = $2; };
dictionary_items: dictionary_item dictionary_items { $$ = $1 + $2; };
                  | %empty                         { $$ = 0; };
dictionary_item:  "string" dictionary_value { $$ = $1.size () + $2;
                                              *dictionary_key = $1; };
dictionary_value: "string" { $$ = $1.size ();
                             ACE_DEBUG ((LM_DEBUG,
                                         ACE_TEXT ("key: \"%s\": \"%s\"\n"),
                                         ACE_TEXT (dictionary_key->c_str ()),
                                         ACE_TEXT ($1.c_str ())));

                             struct BitTorrent_MetaInfo& metainfo_r =
                               parser->current ();
                             if (metainfo_r.singleFileMode)
                             { ACE_ASSERT (metainfo_r.info.single_file);
                               if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("pieces"))
                                 metainfo_r.info.single_file->pieces = $1;
                               else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("name"))
                                 metainfo_r.info.single_file->name = $1;
                               else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("md5sum"))
                                 metainfo_r.info.single_file->md5sum = $1;
                             } // end ELSE IF
                             else
                             { ACE_ASSERT (metainfo_r.info.multiple_file);
                               if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("pieces"))
                                 metainfo_r.info.multiple_file->pieces = $1;
                               else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("name"))
                                 metainfo_r.info.multiple_file->name = $1;
                               else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("md5sum"))
                               {
                                 struct BitTorrent_MetaInfo_InfoDictionary_MultipleFile_File& current_r =
                                   metainfo_r.info.multiple_file->files.back ();
                                 current_r.md5sum = $1;
                               } // end ELSE IF
                             } };
                  | "integer" { $$ = $1;
                                ACE_DEBUG ((LM_DEBUG,
                                            ACE_TEXT ("key: \"%s\": %d\n"),
                                            ACE_TEXT (dictionary_key->c_str ()),
                                            $1));

                                struct BitTorrent_MetaInfo& metainfo_r =
                                  parser->current ();

                                if (metainfo_r.singleFileMode)
                                { ACE_ASSERT (metainfo_r.info.single_file);
                                  if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("piece length"))
                                    metainfo_r.info.single_file->pieceLength = $1;
                                  else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("private"))
                                    metainfo_r.info.single_file->_private = $1;
                                  else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("length"))
                                    metainfo_r.info.single_file->length = $1;
                                }
                                else
                                { ACE_ASSERT (metainfo_r.info.multiple_file);
                                  if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("piece length"))
                                    metainfo_r.info.multiple_file->pieceLength = $1;
                                  else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("private"))
                                    metainfo_r.info.multiple_file->_private = $1;
                                  else if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("length"))
                                  {
                                    struct BitTorrent_MetaInfo_InfoDictionary_MultipleFile_File& current_r =
                                      metainfo_r.info.multiple_file->files.back ();
                                    current_r.length = $1;
                                  }
                                } };
                  | "list" list_items { $$ = $2;

                                        struct BitTorrent_MetaInfo& metainfo_r =
                                          parser->current ();

                                        if (metainfo_r.singleFileMode) {}
                                        else
                                        { ACE_ASSERT (metainfo_r.info.multiple_file);
                                          if (*dictionary_key == ACE_TEXT_ALWAYS_CHAR ("path"))
                                          {
                                            struct BitTorrent_MetaInfo_InfoDictionary_MultipleFile_File& current_r =
                                              metainfo_r.info.multiple_file->files.back ();
                                            BitTorrent_MetaInfo_ListIterator_t iterator =
                                              $1.begin ();
                                            current_r.path = *iterator;
                                            for (;
                                                 iterator != $1.end ();
                                                 ++iterator)
                                            {
                                              current_r.path += ACE_DIRECTORY_SEPARATOR_STR;
                                              current_r.path += *iterator;
                                            } // end FOR
                                          } // end IF
                                        } };
                  | "dictionary" dictionary_items { $$ = $2; };
%%

/* void
yy::BitTorrent_MetaInfo_Parser::error (const location_type& location_in,
                                       const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_MetaInfo_Parser::error"));

  try {
    iparser_p->error (location_in, message_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_MetaInfo_IParser::error(), continuing\n")));
  }
}

void
yy::BitTorrent_MetaInfo_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_MetaInfo_Parser::set"));

  yyscanner = context_in;
} */

/*void
yysetdebug (int debug_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yysetdebug"));

  yydebug = debug_in;
}*/

/*void
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

  try {
//    iparser_p->error (*location_in, message_in);
    iparser_p->error (message_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_MetaInfo_IParser::error(), continuing\n")));
  }
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
    case STRING:
    case INTEGER:
    case LIST:
    case DICTIONARY:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
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
}*/
