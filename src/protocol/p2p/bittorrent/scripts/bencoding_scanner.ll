%top{
  #include "bittorrent_iparser.h"
//  #include "bittorrent_bencoding_scanner.h"

/* This disables inclusion of unistd.h, which is not available using MSVC. The
 * C++ scanner uses STL streams instead. */
/*#define YY_NO_UNISTD_H*/

class BitTorrent_Bencoding_Scanner;

//yy::BitTorrent_Bencoding_Parser::symbol_type
/*#define YY_DECL                                                                              \
yy::BitTorrent_Bencoding_Parser::token_type                                                  \
BitTorrent_Bencoding_Scanner::yylex (yy::BitTorrent_Bencoding_Parser::semantic_type* yylval, \
                                     yy::location* location,                                 \
                                     BitTorrent_Bencoding_IParser* parser)*/
// ... and declare it for the parser's sake
//YY_DECL;

//#define YYLTYPE yy::location
//#define YYSTYPE yy::BitTorrent_Bencoding_Parser::semantic_type

//#define YY_EXTRA_TYPE
}

%{
  // *WORKAROUND*
  #include <iostream>
  //using namespace std;
  //// *IMPORTANT NOTE*: several ACE headers include ace/iosfwd.h, which introduces
  ////                   a problem in conjunction with the standard include headers
  ////                   when ACE_USES_OLD_IOSTREAMS is defined
  ////                   --> include the necessary headers manually (see above), and
  ////                       prevent ace/iosfwd.h from causing any harm
  //#define ACE_IOSFWD_H

  #include <sstream>

  #include "ace/Log_Msg.h"
  #include "ace/OS_Memory.h"

  #include "net_macros.h"

  #include "bittorrent_bencoding_parser.h"
  #include "bittorrent_bencoding_scanner.h"

  // the original yyterminate() macro returns int. Since this uses Bison 3
  // variants as tokens, redefine it to change type to `Parser::semantic_type`
//  #define yyterminate() yy::BitTorrent_Bencoding_Parser::make_END (location_)
  #define yyterminate() return yy::BitTorrent_Bencoding_Parser::token::END

  // this tracks the current scanner location. Action is called when length of
  // the token is known
  #define YY_USER_ACTION yylloc->columns (yyleng);
%}

%option yylineno yywrap
%option nomain nounput noyymore noreject nodefault nostdinit
/* %option noline nounistd */
%option nounistd

%option 8bit batch never-interactive stack
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full

%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, header-file extra-type'
*/
/*%option extra-type="Common_IScanner*"*/

%option c++
/*%option header-file="bittorrent_bencoding_scanner.h" outfile="bittorrent_bencoding_scanner.cpp"*/
%option outfile="bittorrent_bencoding_scanner.cpp"
%option prefix="BitTorrent_Bencoding_Scanner_"
%option yyclass="BitTorrent_Bencoding_Scanner"

/* *NOTE*: for protcol specification, see:
           - http://bittorrent.org/beps/bep_0003.html
           - https://wiki.theory.org/BitTorrentSpecification */

OCTET                             [\x00-\xFF]
DIGIT                             [-[:digit:]]
/* big-endian */
BEGIN                             {OCTET}{4}
INDEX                             {OCTET}{4}
LENGTH                            {OCTET}{4}

URL                               {OCTET}*
HASH                              {OCTET}{20}

/* this is called 'bencoding' in BitTorrent lingo */
STRING                            ({DIGIT}+:{OCTET}*)
INTEGER                           (i{DIGIT}+e)
LIST                              (l({INTEGER}|{STRING})*e)
DICTIONARY                        (d({STRING}({INTEGER}|{STRING}|{LIST}|{DICTIONARY}))*e)

/* METAINFO_KEY                      (announce|info) */
METAINFO_KEY                      (announce|announce-list|comment|created by|creation date|encoding|info)
METAINFO_ANNOUNCE_VALUE           {URL}
/* METAINFO_INFO_KEY                 (name|piece length|pieces|length|files) */
METAINFO_INFO_KEY                 (files|name|length|piece length|md5sum|pieces|private)
METAINFO_NAME_VALUE               {OCTET}*
METAINFO_PIECE_LENGTH_VALUE       {LENGTH}
METAINFO_PIECES_VALUE             {HASH}+
METAINFO_KEY_LENGTH_VALUE         {LENGTH}
/* METAINFO_KEY_FILES_DICTIONARY_KEY (length|path) */
METAINFO_KEY_FILES_DICTIONARY_KEY (length|md5sum|path)
METAINFO_KEY_FILES_VALUE          (ld({METAINFO_KEY_FILES_DICTIONARY_KEY}({INTEGER}|{STRING}))*e)

/* aka '.torrent' files */
METAINFO_FILE                     {DICTIONARY}

%s state_string
%s state_string_2
%s state_integer
%s state_list
%s state_dictionary_key
%s state_dictionary_key_2
%s state_dictionary_value
%s state_dictionary_value_2

%{
/* handle locations */
/*#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
                       yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng; */
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%% /* end of definitions */

%{
  /* code to place at the beginning of yylex() */

  // sanity check(s)
  ACE_ASSERT (parser_);
  ACE_ASSERT (yylloc);

  // reset location
  yylloc->step ();

  unsigned int string_length = 0;
  std::stringstream converter;

/*  ACE_Message_Block* message_block_p = parser_->buffer ();
  ACE_ASSERT (message_block_p);*/
%}

<INITIAL>{
"i"                    { parser_->offset (1);
                         yylval->ival = 0;
                         yy_push_state (state_integer); }
{DIGIT}{1}             { yyless (0);
                         yy_push_state (state_string); }
"l"                    { parser_->offset (1);
                         yy_push_state (state_list);
                         ACE_NEW_NORETURN (yylval->lval,
                                           Bencoding_List_t ());
                         ACE_ASSERT (yylval->lval);
                         return yy::BitTorrent_Bencoding_Parser::token::LIST; }
"d"                    { parser_->offset (1);
                         yy_push_state (state_dictionary_key);
                         ACE_NEW_NORETURN (yylval->dval,
                                           Bencoding_Dictionary_t ());
                         ACE_ASSERT (yylval->dval);
                         return yy::BitTorrent_Bencoding_Parser::token::DICTIONARY; }
} // end <INITIAL>
<state_integer>{
"e"                    { yy_pop_state ();
                         return yy::BitTorrent_Bencoding_Parser::token::INTEGER; }
{DIGIT}+               { parser_->offset (yyleng);
                         converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                         converter.clear ();
                         converter << yytext;
                         converter >> yylval->ival; }
} // end <state_integer>
<state_string>{
{DIGIT}+               { parser_->offset (yyleng);
                         converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                         converter.clear ();
                         converter << yytext;
                         converter >> string_length; }
":"                    { parser_->offset (1);
                         ACE_NEW_NORETURN (yylval->sval,
                                           std::string ());
                         ACE_ASSERT (yylval->sval);
                         if (!string_length)
                         { // --> found an empty string
                           yy_pop_state ();
                           return yy::BitTorrent_Bencoding_Parser::token::STRING;
                         } // end IF
                         BEGIN(state_string_2); }
} // end <state_string>
<state_string_2>{
{OCTET}{1}             { ACE_ASSERT (string_length != 0);
                         parser_->offset (yyleng);
                         yylval->sval->push_back (yytext[0]);
                         --string_length;
                         if (!string_length)
                         {
                           yy_pop_state ();
                           return yy::BitTorrent_Bencoding_Parser::token::STRING;
                         } // end IF
                       }
} // end <state_string_2>
<state_list>{
"e"                    { parser_->offset (1);
                         yy_pop_state ();
                         return yy::BitTorrent_Bencoding_Parser::token::END_OF_LIST; }
"i"                    { parser_->offset (1);
                         yylval->ival = 0;
                         yy_push_state (state_integer); }
{DIGIT}{1}             { yyless (0);
                         yy_push_state (state_string); }
"l"                    { parser_->offset (1);
                         ACE_NEW_NORETURN (yylval->lval,
                                           Bencoding_List_t ());
                         ACE_ASSERT (yylval->lval);
                         yy_push_state (state_list);
                         return yy::BitTorrent_Bencoding_Parser::token::LIST; }
"d"                    { parser_->offset (1);
                         ACE_NEW_NORETURN (yylval->dval,
                                           Bencoding_Dictionary_t ());
                         ACE_ASSERT (yylval->dval);
                         yy_push_state (state_dictionary_key);
                         return yy::BitTorrent_Bencoding_Parser::token::DICTIONARY; }
} // end <state_list>
<state_dictionary_key>{
"e"                    { parser_->offset (1);
                         yy_pop_state ();
                         return yy::BitTorrent_Bencoding_Parser::token::END_OF_DICTIONARY; }
{DIGIT}+               { parser_->offset (yyleng);
                         converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                         converter.clear ();
                         converter << yytext;
                         converter >> string_length; }
":"                    { parser_->offset (1);
                         ACE_NEW_NORETURN (yylval->sval,
                                           std::string ());
                         ACE_ASSERT (yylval->sval);
                         if (!string_length)
                         { // --> found an empty string
                           yy_push_state (state_dictionary_value);
                           return yy::BitTorrent_Bencoding_Parser::token::STRING;
                         } // end IF
                         BEGIN(state_dictionary_key_2); }
} // end <state_string>
<state_dictionary_key_2>{
{OCTET}{1}             { ACE_ASSERT (string_length != 0);
                         parser_->offset (yyleng);
                         yylval->sval->push_back (yytext[0]);
                         --string_length;
                         if (!string_length)
                         {
                           yy_push_state (state_dictionary_value);
                           return yy::BitTorrent_Bencoding_Parser::token::STRING;
                         } // end IF
                       }
} // end <state_dictionary_key_2>
<state_dictionary_value>{
"i"                    { parser_->offset (1);
                         yylval->ival = 0;
                         yy_push_state (state_dictionary_value_2);
                         yy_push_state (state_integer); }
{DIGIT}{1}             { yyless (0);
                         yy_push_state (state_dictionary_value_2);
                         yy_push_state (state_string); }
"l"                    { parser_->offset (1);
                         yy_push_state (state_dictionary_value_2);
                         yy_push_state (state_list);
                         ACE_NEW_NORETURN (yylval->lval,
                                           Bencoding_List_t ());
                         ACE_ASSERT (yylval->lval);
                         return yy::BitTorrent_Bencoding_Parser::token::LIST; }
"d"                    { parser_->offset (1);
                         yy_push_state (state_dictionary_value_2);
                         yy_push_state (state_dictionary_key);
                         ACE_NEW_NORETURN (yylval->dval,
                                           Bencoding_Dictionary_t ());
                         ACE_ASSERT (yylval->dval);
                         return yy::BitTorrent_Bencoding_Parser::token::DICTIONARY; }
} // end <state_dictionary_value>
<state_dictionary_value_2>{
{OCTET}{1}             { yyless (0);
                         yy_pop_state (); // state_dictionary_value_2
                         yy_pop_state (); // state_dictionary_value
                         BEGIN(state_dictionary_key); }
} // end <state_dictionary_value_2>
<<EOF>>                { return yy::BitTorrent_Bencoding_Parser::token::END; }
<*>{OCTET}             { /* *TODO*: use (?s:.) ? */
                         if (!isBlocking ())
                           yyterminate(); // not enough data, cannot proceed

                         // wait for more data fragment(s)
                         if (!switchBuffer ())
                         { // *NOTE*: most probable reason: connection
                           //         has been closed --> session end
                           ACE_DEBUG ((LM_DEBUG,
                                       ACE_TEXT ("failed to Common_IScanner::switchBuffer(), returning\n")));
                           yyterminate(); // not enough data, cannot proceed
                         } // end IF
                         yyless (0); }

%% /* end of rules */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
yyFlexLexer::yywrap ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_Scanner_FlexLexer::yywrap"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
/*int
BitTorrent_Bencoding_Scanner::yywrap ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_Scanner::yywrap"));

  // sanity check(s)
  ACE_ASSERT (parser_);
  if (!parser_->isBlocking ())
    return 1; // not enough data, cannot proceed

  // *NOTE*: there is more data
  // 1. gobble/save the rest
  // 2. switch buffers
  // 3. unput the rest
  // 4. continue scanning

  // step1
  //std::string the_rest;
  //the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
  //yyg->yy_c_buf_p += yyleng;
  //yyg->yy_hold_char = *yyg->yy_c_buf_p;
  //if (yy_flex_debug)
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("the rest: \"%s\" (%d byte(s))\n"),
  //              ACE_TEXT (the_rest.c_str ()),
  //              the_rest.size ()));

  // step2
  if (!parser_->switchBuffer ())
  {
    // *NOTE*: most probable reason: received session end message
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_IParser::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  // step3
  //for (std::string::reverse_iterator iterator = the_rest.rbegin ();
  //     iterator != the_rest.rend ();
  //     ++iterator)
  //  unput (*iterator);

  // step4
  //yyg->yy_did_buffer_switch_on_eof = 1;
  // yymore ();

  return 0;
}*/
#ifdef __cplusplus
}
#endif /* __cplusplus */
