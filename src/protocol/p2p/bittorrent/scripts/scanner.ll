%top{
  #include "bittorrent_common.h"
  #include <ace/Synch.h>
  #include "bittorrent_iparser.h"
//  #include "bittorrent_scanner.h"

/* This disables inclusion of unistd.h, which is not available using MSVC. The
 * C++ scanner uses STL streams instead. */
/*#define YY_NO_UNISTD_H*/

class BitTorrent_Scanner;

//yy::BitTorrent_Parser::symbol_type
#define YY_DECL                                                          \
yy::BitTorrent_Parser::token_type                                        \
BitTorrent_Scanner::yylex (yy::BitTorrent_Parser::semantic_type* yylval, \
                           yy::location* location,                       \
                           BitTorrent_IParser_t* parser)
// ... and declare it for the parser's sake
//YY_DECL;

//#define YYLTYPE yy::location
//#define YYSTYPE yy::BitTorrent_Parser::semantic_type

//#define YY_EXTRA_TYPE
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
  #include "bittorrent_parser.h"
  #include "bittorrent_scanner.h"

  // the original yyterminate() macro returns int. Since this uses Bison 3
  // variants as tokens, redefine it to change type to `Parser::semantic_type`
//  #define yyterminate() yy::BitTorrent_Bencoding_Parser::make_END (location_)
  #define yyterminate() return yy::BitTorrent_Parser::token::END

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
%option header-file="bittorrent_scanner.h" outfile="bittorrent_scanner.cpp"
%option prefix="BitTorrent_Scanner_"
%option yyclass="BitTorrent_Scanner"

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

/* METAINFO_KEY                      (announce|info)*/
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
METAINFO_FILE                     {DICTIONARY}*/

TRACKER_GET_STATUS                (completed|empty|started|stopped)
/* TRACKER_GET_KEY                   (info_hash|peer_id|ip|port|uploaded|downloaded|left|{TRACKER_GET_STATUS}) */
TRACKER_GET_KEY                   (compact|downloaded|info_hash|ip|key|left|no_peer_id|numwant|peer_id|port|trackerid|uploaded|{TRACKER_GET_STATUS})
TRACKER_RESPONSE_PEERS_KEY        (ip|peer id|port)
/* TRACKER_RESPONSE_KEY              (interval|peers) */
TRACKER_RESPONSE_KEY              (complete|incomplete|failure_reason|interval|min interval|peers|tracker id|warning_message)
TRACKER_SCRAPE_FILES_KEY          (complete|downloaded|incomplete|name)
TRACKER_SCRAPE_FLAGS_KEY          (min_request_interval)
TRACKER_SCRAPE_KEY                (failure reason|files|flags)

/* version 1.0 of the BitTorrent protocol */
PEER_HANDSHAKE_PREFIX             \x13BitTorrent\ protocol
PEER_HANDSHAKE_RESERVED           \x00{8}
/* "...sha1 hash of the bencoded form of the info value from the metainfo file.
 * (This is the same value which is announced as info_hash to the tracker, only
 *  here it's raw instead of quoted here). ..." */
PEER_HANDSHAKE_HASH               {OCTET}{20}
/* "...After the download hash comes the 20-byte peer id which is reported in
 *  tracker requests and contained in peer lists in tracker responses. ..." */
PEER_HANDSHAKE_PEER_ID            {OCTET}{20}
/* PEER_HANDSHAKE                    {PEER_HANDSHAKE_PREFIX}{PEER_HANDSHAKE_RESERVED}{PEER_HANDSHAKE_HASH}{PEER_HANDSHAKE_PEER_ID} */

TYPE                              [0-8]
BITFIELD_PAYLOAD                  {OCTET}{1}
CANCEL_PAYLOAD                    {REQUEST_PAYLOAD}
HAVE_PAYLOAD                      {INDEX}
PIECE_PAYLOAD                     {INDEX}{BEGIN}{OCTET}{1}
PORT_PAYLOAD                      {OCTET}{2}
REQUEST_PAYLOAD                   {INDEX}{BEGIN}{LENGTH}
PAYLOAD                           ({BITFIELD_PAYLOAD}|{CANCEL_PAYLOAD}|{HAVE_PAYLOAD}|{PIECE_PAYLOAD}|{PORT_PAYLOAD}|{REQUEST_PAYLOAD})
/* "...That's it for handshaking, next comes an alternating stream of length
 * prefixes and messages. Messages of length zero are keepalives, and ignored.
 * Keepalives are generally sent once every two minutes, but note that timeouts
 * can be done much more quickly when data is expected. ..." */
MESSAGE                           {LENGTH}{TYPE}{0-1}{PAYLOAD}{0-1}

%s state_reserved
%s state_hash
%s state_peer_id
%s state_length
%s state_type
%s state_bitfield_payload
%s state_cancel_payload
%s state_have_payload
%s state_piece
%s state_piece_payload
%s state_port_payload
%s state_request_payload

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

  unsigned int bitfield_counter = 0;

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
{PEER_HANDSHAKE_PREFIX}   { ACE_ASSERT (yyleng == 19);
                            parser->offset (yyleng);
                            /* *TODO*: error handling */
                            ACE_NEW_NORETURN (yylval->handshake,
                                              struct BitTorrent_PeerHandshake ());
                            ACE_ASSERT (yylval->handshake);
                            yylval->handshake->version.append (yytext, 19);
                            BEGIN (state_reserved); }
} // end <INITIAL>
<state_reserved>{
{PEER_HANDSHAKE_RESERVED} { ACE_ASSERT (yyleng == 8);
                            ACE_ASSERT (yylval->handshake);
                            parser->offset (yyleng);
                            ACE_OS::memcpy (yylval->handshake->reserved, yytext, 8);
                            BEGIN (state_hash); }
} // end <state_reserved>
<state_hash>{
{PEER_HANDSHAKE_HASH}     { ACE_ASSERT (yyleng == 20);
                            ACE_ASSERT (yylval->handshake);
                            parser->offset (yyleng);
                            yylval->handshake->hash.append (yytext, 20);
                            BEGIN (state_peer_id); }
} // end <state_hash>
<state_peer_id>{
{PEER_HANDSHAKE_PEER_ID}  { ACE_ASSERT (yyleng == 20);
                            ACE_ASSERT (yylval->handshake);
                            parser->offset (yyleng);
                            yylval->handshake->peer_id.append (yytext, 20);
                            BEGIN (state_length);
                            return yy::BitTorrent_Parser::token::HANDSHAKE; }
} // end <state_peer_id>
<state_length>{
{LENGTH}                  { ACE_ASSERT (yyleng == 4);
                            parser->offset (yyleng);
                            /* *TODO*: error handling */
                            ACE_NEW_NORETURN (yylval->record,
                                              struct BitTorrent_Record ());
                            ACE_ASSERT (yylval->record);
                            yylval->record->length =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext));
                            if (!yylval->record->length)
                              return yy::BitTorrent_Parser::token::KEEP_ALIVE;
                            BEGIN (state_type); }
} // end <state_length>
<state_type>{
{TYPE}                    { ACE_ASSERT (yyleng == 1);
                            parser->offset (yyleng);
                            yylval->record->type =
                              static_cast<enum BitTorrent_MessageType> (*reinterpret_cast<ACE_UINT8*> (yytext));
                            switch (yylval->record->type)
                            {
                              case BITTORRENT_MESSAGETYPE_CHOKE:
                                BEGIN (state_length);
                                return yy::BitTorrent_Parser::token::CHOKE;
                              case BITTORRENT_MESSAGETYPE_UNCHOKE:
                                BEGIN (state_length);
                                return yy::BitTorrent_Parser::token::UNCHOKE;
                              case BITTORRENT_MESSAGETYPE_INTERESTED:
                                BEGIN (state_length);
                                return yy::BitTorrent_Parser::token::INTERESTED;
                              case BITTORRENT_MESSAGETYPE_NOT_INTERESTED:
                                BEGIN (state_length);
                                return yy::BitTorrent_Parser::token::NOT_INTERESTED;
                              case BITTORRENT_MESSAGETYPE_HAVE:
                                BEGIN (state_have_payload);
                                break;
                              case BITTORRENT_MESSAGETYPE_BITFIELD:
                                bitfield_counter = yylval->record->length - 1;
                                BEGIN (state_bitfield_payload);
                                break;
                              case BITTORRENT_MESSAGETYPE_REQUEST:
                                BEGIN (state_request_payload);
                                break;
                              case BITTORRENT_MESSAGETYPE_PIECE:
                                BEGIN (state_piece_payload);
                                break;
                              case BITTORRENT_MESSAGETYPE_CANCEL:
                                BEGIN (state_cancel_payload);
                                break;
                              default:
                              {
                                ACE_DEBUG ((LM_ERROR,
                                            ACE_TEXT ("invalid/unknown message type (was: %d), aborting\n"),
                                            yylval->record->type));
                                yyterminate ();
                              }
                            } // end SWITCH
                          }
} // end <state_type>
<state_have_payload>{
{HAVE_PAYLOAD}            { ACE_ASSERT (yyleng == 4);
                            parser->offset (yyleng);
                            yylval->record->have =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext));
                            BEGIN (state_length);
                            return yy::BitTorrent_Parser::token::HAVE; }
} // end <state_have_payload>
<state_bitfield_payload>{
{BITFIELD_PAYLOAD}        { ACE_ASSERT (yyleng == 1);
                            ACE_ASSERT (bitfield_counter);
                            parser->offset (yyleng);
                            yylval->record->bitfield.push_back (*reinterpret_cast<ACE_UINT8*> (yytext));
                            --bitfield_counter;
                            if (!bitfield_counter)
                            {
                              BEGIN (state_length);
                              return yy::BitTorrent_Parser::token::BITFIELD;
                            } }
} // end <state_bitfield_payload>
<state_request_payload>{
{REQUEST_PAYLOAD}         { ACE_ASSERT (yyleng == 12);
                            parser->offset (yyleng);
                            yylval->record->request.index =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext));
                            yylval->record->request.begin =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext + 4))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext + 4));
                            yylval->record->request.length =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext + 8))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext + 8));
                            BEGIN (state_length);
                            return yy::BitTorrent_Parser::token::REQUEST; }
} // end <state_request_payload>
<state_cancel_payload>{
{CANCEL_PAYLOAD}          { ACE_ASSERT (yyleng == 12);
                            parser->offset (yyleng);
                            yylval->record->cancel.index =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext));
                            yylval->record->cancel.begin =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext + 4))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext + 4));
                            yylval->record->cancel.length =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext + 8))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext + 8));
                            BEGIN (state_length);
                            return yy::BitTorrent_Parser::token::CANCEL; }
} // end <state_cancel_payload>
<state_piece_payload>{
{PIECE_PAYLOAD}           { ACE_ASSERT (yyleng == 8);
                            parser->offset (yyleng);
                            yylval->record->piece.index =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext));
                            yylval->record->piece.begin =
                              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (*reinterpret_cast<ACE_UINT32*> (yytext + 4))
                                                                     : *reinterpret_cast<ACE_UINT32*> (yytext + 4));
                            BEGIN (state_piece); }
} // end <state_piece_payload>
<state_piece>{
{OCTET}{1}                { ACE_ASSERT (yyleng == 1);
                            /* undo the effects of YY_DO_BEFORE_ACTION */
                            *yy_cp = (yy_hold_char);

                            // (frame and) skip over piece data
                            message_block_p->rd_ptr (parser->offset ());

                            unsigned int bytes_to_skip =
                              yylval->record->length - 9;
                            ACE_Message_Block* message_p, *message_2;
                            unsigned int remainder = message_block_p->length ();
                            if (bytes_to_skip <= remainder)
                            { // the current fragment contains the whole piece
                              // --> insert buffer
                              message_p = message_block_p->duplicate ();
                              ACE_ASSERT (message_p);
                              message_2 = message_block_p->cont ();
                              if (message_2)
                                message_p->cont (message_2);
                              message_block_p->cont (message_p);

                              message_block_p->wr_ptr (message_block_p->rd_ptr () + bytes_to_skip);
                              ACE_ASSERT (message_block_p->length () == bytes_to_skip);
                              message_p->rd_ptr (bytes_to_skip);
                              ACE_ASSERT (message_p->length () == remainder);
                              // set bytes to skip by the scanner (see below)
                              remainder = 0;
                            } // end IF
                            else
                            {
                              // skip over trailing piece data fragment(s)
                              unsigned int skipped_bytes = 0;
                              while (skipped_bytes <= bytes_to_skip)
                              {
                                if (!parser->switchBuffer ())
                                {
                                  ACE_DEBUG ((LM_ERROR,
                                              ACE_TEXT ("failed to Net_IParser::switchBuffer(), aborting\n")));
                                  yyterminate ();
                                } // end IF
                                message_block_p = parser->buffer ();
                                skipped_bytes += message_block_p->length ();
                              } // end WHILE
                              remainder = (skipped_bytes - bytes_to_skip);

                              // this piece ends in the current fragment
                              // --> insert buffer, adjust writer pointer,
                              //     switch buffers one more time
                              message_p = message_block_p->duplicate ();
                              ACE_ASSERT (message_p);
                              message_2 = message_block_p->cont ();
                              if (message_2)
                                message_p->cont (message_2);
                              message_block_p->cont (message_p);
                              message_block_p->wr_ptr (message_block_p->rd_ptr () + remainder);
                              ACE_ASSERT (message_block_p->length () == remainder);
                            } // end ELSE
                            if (!parser->switchBuffer (true))
                            {
                              ACE_DEBUG ((LM_ERROR,
                                          ACE_TEXT ("failed to Net_IParser::switchBuffer(), aborting\n")));
                              yyterminate ();
                            } // end IF
                            parser->buffer ()->rd_ptr (remainder);
                            parser->offset (remainder);

                            // gobble initial bytes (if any)
                            char c = 0;
                            for (unsigned int i = 0; i < remainder; ++i)
                              c = yyinput ();
                            ACE_UNUSED_ARG (c);

                            BEGIN (state_length);
                            return yy::BitTorrent_Parser::token::PIECE; }
} // end <state_piece>
<<EOF>>                   { yyterminate(); } // *NOTE*: yywrap returned non-zero
<*>{OCTET}                { /* *TODO*: use (?s:.) ? */
                            if (!parser->isBlocking ())
                              return yy::BitTorrent_Parser::token::END_OF_FRAGMENT; // not enough data, cannot proceed

                            // wait for more data fragment(s)
                            if (!parser->switchBuffer ())
                            { // *NOTE*: most probable reason: connection has
                              //         been closed --> session end
                              ACE_DEBUG ((LM_DEBUG,
                                          ACE_TEXT ("failed to Net_IParser::switchBuffer(), returning\n")));
                              return yy::BitTorrent_Parser::token::END_OF_FRAGMENT; // not enough data, cannot proceed
                            } // end IF
                            yyless (0); }
%%
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
yyFlexLexer::yywrap ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Scanner_FlexLexer::yywrap"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
int
BitTorrent_Scanner::yywrap ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Scanner::yywrap"));

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
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
