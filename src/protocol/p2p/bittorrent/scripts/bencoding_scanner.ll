%top{
  #include <ace/Synch.h>
  #include "bittorrent_iparser.h"
//  #include "bittorrent_bencoding_scanner.h"

/* This disables inclusion of unistd.h, which is not available using MSVC. The
 * C++ scanner uses STL streams instead. */
/*#define YY_NO_UNISTD_H*/

class BitTorrent_Bencoding_Scanner;

//yy::BitTorrent_Bencoding_Parser::symbol_type
#define YY_DECL                                                                              \
yy::BitTorrent_Bencoding_Parser::token_type                                                  \
BitTorrent_Bencoding_Scanner::yylex (yy::BitTorrent_Bencoding_Parser::semantic_type* yylval, \
                                     yy::location* location,                                 \
                                     BitTorrent_Bencoding_IParser* parser)
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

  #include <ace/Synch.h>
  #include "bittorrent_bencoding_parser.h"
  #include "bittorrent_bencoding_scanner.h"

  // the original yyterminate() macro returns int. Since this uses Bison 3
  // variants as tokens, redefine it to change type to `Parser::semantic_type`
//  #define yyterminate() yy::BitTorrent_Bencoding_Parser::make_END (location_)
  #define yyterminate() return yy::BitTorrent_Bencoding_Parser::token::END

  // this tracks the current scanner location. Action is called when length of
  // the token is known
  #define YY_USER_ACTION location_.columns (yyleng);
%}

%option yylineno yywrap
%option nomain nounput noyymore noreject nodefault nostdinit
%option noline nounistd

%option 8bit batch never-interactive stack
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full

%option backup debug perf-report perf-report verbose warn

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
%s state_integer
%s state_list
%s state_dictionary_key
%s state_dictionary_value

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

  // reset location
  location_.step ();

  unsigned int string_length = 0;

  std::stringstream converter;
//  std::string regex_string;
//  std::regex regex;
//  std::smatch match_results;

  // sanity check(s)
  ACE_ASSERT (parser);
  ACE_Message_Block* message_block_p = parser->buffer ();
  ACE_ASSERT (message_block_p);

//  location_.columns (yyleng);
%}

<INITIAL>{
"d"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_dictionary_key);
                         ACE_NEW_NORETURN (yylval->dval,
                                           Bencoding_Dictionary_t ());
                         ACE_ASSERT (yylval->dval);
                         return yy::BitTorrent_Bencoding_Parser::token::DICTIONARY; }
} // end <INITIAL>
<state_string>{
{DIGIT}+               {
                         parser->offset (yyleng);
                         converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                         converter.clear ();
                         converter << yytext;
                         converter >> string_length; }
":"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         if (!string_length)
                         { // --> found an empty string
                           ACE_NEW_NORETURN (yylval->sval,
                                             std::string ());
                           ACE_ASSERT (yylval->sval);
                           yy_pop_state ();
                           return yy::BitTorrent_Bencoding_Parser::token::STRING;
                         } }
{OCTET}{1}             { ACE_ASSERT (string_length != 0);
                         parser->offset (string_length);
                         ACE_NEW_NORETURN (yylval->sval,
                                           std::string ());
                         ACE_ASSERT (yylval->sval);
                         yylval->sval->push_back (yytext[0]);
                         for (unsigned int i = 0; i < (string_length - 1); ++i)
                           yylval->sval->push_back (yyinput ());
                         yy_pop_state ();
                         return yy::BitTorrent_Bencoding_Parser::token::STRING; }
} // end <state_string>
<state_integer>{
"e"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_pop_state (); }
{DIGIT}+               {
                         parser->offset (yyleng);
                         converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                         converter.clear ();
                         converter << yytext;
                         converter >> yylval->ival;
                         return yy::BitTorrent_Bencoding_Parser::token::INTEGER; }
"i"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1); }
} // end <state_integer>
<state_list>{
"e"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_pop_state ();
                         return yy::BitTorrent_Bencoding_Parser::token::END_OF_LIST; }
{DIGIT}{1}             { yyless (0);
                         yy_push_state (state_string); }
"i"                    { ACE_ASSERT (yyleng == 1);
                         yyless (0);
                         yy_push_state (state_integer); }
"l"                    { ACE_ASSERT (yyleng == 1);
                         ACE_NEW_NORETURN (yylval->lval,
                                           Bencoding_List_t ());
                         ACE_ASSERT (yylval->lval);
                         return yy::BitTorrent_Bencoding_Parser::token::LIST; }
"d"                    { ACE_ASSERT (yyleng == 1);
                         yy_push_state (state_dictionary_key);
                         ACE_NEW_NORETURN (yylval->dval,
                                           Bencoding_Dictionary_t ());
                         ACE_ASSERT (yylval->dval);
                         return yy::BitTorrent_Bencoding_Parser::token::DICTIONARY; }
} // end <state_list>
<state_dictionary_key>{
"e"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_pop_state ();
                         return yy::BitTorrent_Bencoding_Parser::token::END_OF_DICTIONARY; }
{DIGIT}{1}             { yyless (0);
                         BEGIN(state_dictionary_value);
                         yy_push_state (state_string); }
} // end <state_dictionary_key>
<state_dictionary_value>{
{DIGIT}{1}             { yyless (0);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_string); }
"i"                    { ACE_ASSERT (yyleng == 1);
                         yyless (0);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_integer); }
"l"                    { ACE_ASSERT (yyleng == 1);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_list);
                         ACE_NEW_NORETURN (yylval->lval,
                                           Bencoding_List_t ());
                         ACE_ASSERT (yylval->lval);
                         return yy::BitTorrent_Bencoding_Parser::token::LIST; }
"d"                    { ACE_ASSERT (yyleng == 1);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_dictionary_key);
                         ACE_NEW_NORETURN (yylval->dval,
                                           Bencoding_Dictionary_t ());
                         ACE_ASSERT (yylval->dval);
                         return yy::BitTorrent_Bencoding_Parser::token::DICTIONARY; }
} // end <state_dictionary_value>
<<EOF>>                { return yy::BitTorrent_Bencoding_Parser::token::END; }
<*>{OCTET}             { /* *TODO*: use (?s:.) ? */
                         if (!parser->isBlocking ())
                           yyterminate(); // not enough data, cannot proceed

                         // wait for more data fragment(s)
                         if (!parser->switchBuffer ())
                         { // *NOTE*: most probable reason: connection
                           //         has been closed --> session end
                           ACE_DEBUG ((LM_DEBUG,
                                       ACE_TEXT ("failed to Net_IParser::switchBuffer(), returning\n")));
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
