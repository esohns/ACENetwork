%top{
  #include <ace/Synch.h>
  #include "bittorrent_iparser.h"
  #include "bittorrent_metainfo_parser.h"

#if defined (Net_Bencoding_IN_HEADER)
/* This disables inclusion of unistd.h, which is not available using MSVC. The
 * C++ scanner uses STL streams instead. */
#define YY_NO_UNISTD_H

  #include <FlexLexer.h>

  #include "location.hh"

  class Net_BencodingScanner
   : public yyFlexLexer
   , public BitTorrent_MetaInfo_IScanner_t
  {
   public:
    Net_BencodingScanner (std::istream* in = NULL,
                          std::ostream* out = NULL)
     : yyFlexLexer (in, out)
     , location_ ()
     , parser_ (NULL)
    {};
    virtual ~Net_BencodingScanner () {};

    // implement BitTorrent_MetaInfo_IScanner_t
    inline virtual void set (BitTorrent_MetaInfo_IParser_t* parser_in) { parser_ = parser_in; };

    // override yyFlexLexer::yylex()
//    virtual int yylex ();
    virtual yy::BitTorrent_MetaInfo_Parser::symbol_type yylex (BitTorrent_MetaInfo_IParser_t*);

    yy::location                   location_;

   private:
    BitTorrent_MetaInfo_IParser_t* parser_;
  };
#else
#define YY_STRUCT_YY_BUFFER_STATE
  #include "bencoding_scanner.h"
#undef YY_STRUCT_YY_BUFFER_STATE
#endif

#define YY_DECL                             \
yy::BitTorrent_MetaInfo_Parser::symbol_type \
Net_BencodingScanner::yylex (BitTorrent_MetaInfo_IParser_t* parser)
// ... and declare it for the parser's sake
//YY_DECL;

//#define YYLTYPE yy::location
//#define YYSTYPE yy::BitTorrent_MetaInfo_Parser::semantic_type

#define YY_EXTRA_TYPE
}

%{
  // *WORKAROUND*
  #include <iostream>
  //using namespace std;
  //// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
  ////                   a problem in conjunction with the standard include headers
  ////                   when ACE_USES_OLD_IOSTREAMS is defined
  ////                   --> include the necessary headers manually (see above), and
  ////                       prevent ace/iosfwd.h from causing any harm
  //#define ACE_IOSFWD_H

  #include <ace/Synch.h>
  #include "bencoding_scanner.h"
  #include "bittorrent_metainfo_parser.h"

  // the original yyterminate() macro returns int. Since this uses Bison 3
  // variants as tokens, redefine it to change type to `Parser::semantic_type`
  #define yyterminate() yy::BitTorrent_MetaInfo_Parser::make_END (location_);

  // this tracks the current scanner location. Action is called when length of
  // the token is known
  #define YY_USER_ACTION location_.columns (yyleng);
%}

%option backup
%option batch
%option never-interactive
%option stack

%option nodefault
%option nomain
%option nostdinit
%option nounput
%option noyywrap

%option 8bit
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full

%option debug perf-report verbose warn yylineno

%option ansi-definitions ansi-prototypes
%option c++
%option header-file="bencoding_scanner.h" outfile="bencoding_scanner.cpp"
%option prefix="Net_Bencoding_"
%option yyclass="Net_BencodingScanner"

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
                         return yy::BitTorrent_MetaInfo_Parser::make_DICTIONARY (location_); }
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
                           yy_pop_state ();
                           return yy::BitTorrent_MetaInfo_Parser::make_STRING (std::string (), location_);
                         } }
{OCTET}{1}             { ACE_ASSERT (string_length != 0);
                         parser->offset (string_length);
                         std::string string_i;
                         string_i.push_back (yytext[0]);
                         for (unsigned int i = 0; i < (string_length - 1); ++i)
                           string_i.push_back (yyinput ());
                         yy_pop_state ();
                         return yy::BitTorrent_MetaInfo_Parser::make_STRING (string_i, location_); }
} // end <state_string>
<state_integer>{
"i"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1); }
"e"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_pop_state (); }
{DIGIT}+               {
                         parser->offset (yyleng);
                         converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                         converter.clear ();
                         converter << yytext;
                         int i = -1;
                         converter >> i;
                         return yy::BitTorrent_MetaInfo_Parser::make_INTEGER (i, location_); }
} // end <state_integer>
<state_list>{
"e"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_pop_state (); }
{DIGIT}{1}             { yyless (0);
                         yy_push_state (state_string); }
"i"                    { yyless (0);
                         ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_push_state (state_integer); }
"l"                    { yyless (0);
                         ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_push_state (state_list);
                         return yy::BitTorrent_MetaInfo_Parser::make_LIST (BitTorrent_MetaInfo_List_t (), location_); }
"d"                    { yyless (0);
                         ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_push_state (state_dictionary_key);
                         return yy::BitTorrent_MetaInfo_Parser::make_DICTIONARY (location_); }
} // end <state_list>
<state_dictionary_key>{
"e"                    { ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         yy_pop_state (); }
{DIGIT}{1}             { yyless (0);
                         BEGIN(state_dictionary_value);
                         yy_push_state (state_string); }
} // end <state_dictionary_key>
<state_dictionary_value>{
{DIGIT}{1}             { yyless (0);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_string); }
"i"                    { yyless (0);
                         ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_integer); }
"l"                    { yyless (0);
                         ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_list);
                         return yy::BitTorrent_MetaInfo_Parser::make_LIST (BitTorrent_MetaInfo_List_t (), location_); }
"d"                    { yyless (0);
                         ACE_ASSERT (yyleng == 1);
                         parser->offset (1);
                         BEGIN(state_dictionary_key);
                         yy_push_state (state_dictionary_key);
                         return yy::BitTorrent_MetaInfo_Parser::make_DICTIONARY (location_); }
} // end <state_dictionary_value>
<<EOF>>                { return yy::BitTorrent_MetaInfo_Parser::make_END (location_); }
<*>{OCTET}             { /* *TODO*: use (?s:.) ? */
                         if (!parser->isBlocking ())
                           return yy::BitTorrent_MetaInfo_Parser::make_END_OF_FRAGMENT (location_);

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
