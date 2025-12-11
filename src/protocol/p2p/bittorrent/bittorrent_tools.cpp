/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "stdafx.h"

#include "bittorrent_tools.h"

#include "openssl/sha.h"

#include <iomanip>
#include <regex>
#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_file_tools.h"

#include "common_math_tools.h"

#include "net_macros.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "bittorrent_bencoding_scanner.h"
#include "bittorrent_bencoding_parser_driver.h"
#include "bittorrent_defines.h"
#include "bittorrent_stream_common.h"

// initialize statics
int
BitTorrent_Tools::selector (const dirent* dirEntry_in)
{
  //NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::selector"));

  // *IMPORTANT NOTE*: select only files following the naming schema for
  //                   piece files: "<NUMBER>_XXXXXX.piece"

  // sanity check --> suffix ok ?
  std::string file_extension =
      Common_File_Tools::fileExtension (ACE_TEXT_ALWAYS_CHAR (dirEntry_in->d_name),
                                        true);
  if (ACE_OS::strncmp (file_extension.c_str (),
                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECE_FILENAME_SUFFIX),
                       ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECE_FILENAME_SUFFIX))))
    return 0;

  return 1;
}

//////////////////////////////////////////

std::string
BitTorrent_Tools::HandShakeToString (const struct BitTorrent_PeerHandShake& peerHandshake_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::HandShakeToString"));

  std::string result;
  std::ostringstream converter;

  result += peerHandshake_in.pstr;
  result += ACE_TEXT_ALWAYS_CHAR (" ");
  for (unsigned int i = 0;
       i < BITTORRENT_PEER_HANDSHAKE_RESERVED_SIZE;
       ++i)
  {
    result += ((peerHandshake_in.reserved[i] & 0x80) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ((peerHandshake_in.reserved[i] & 0x40) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ((peerHandshake_in.reserved[i] & 0x20) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ((peerHandshake_in.reserved[i] & 0x10) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ((peerHandshake_in.reserved[i] & 0x08) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ((peerHandshake_in.reserved[i] & 0x04) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ((peerHandshake_in.reserved[i] & 0x02) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ((peerHandshake_in.reserved[i] & 0x01) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
    result += ACE_TEXT_ALWAYS_CHAR (" ");
  } // end FOR
//  result += ACE_TEXT_ALWAYS_CHAR (" ");
  result += peerHandshake_in.info_hash;
  result += ACE_TEXT_ALWAYS_CHAR (" ");
  result += peerHandshake_in.peer_id;

  return result;
}
std::string
BitTorrent_Tools::RecordToString (const struct BitTorrent_PeerRecord& peerRecord_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::RecordToString"));

  std::string result;
  std::ostringstream converter;

  if (!peerRecord_in.length)
    result += ACE_TEXT_ALWAYS_CHAR ("type: KEEP-ALIVE");
  else
  {
    result += ACE_TEXT_ALWAYS_CHAR ("type: ");
    result += BitTorrent_Tools::TypeToString (peerRecord_in.type);
    switch (peerRecord_in.type)
    {
      case BITTORRENT_MESSAGETYPE_CHOKE:
      case BITTORRENT_MESSAGETYPE_UNCHOKE:
      case BITTORRENT_MESSAGETYPE_INTERESTED:
      case BITTORRENT_MESSAGETYPE_NOT_INTERESTED:
        break;
      case BITTORRENT_MESSAGETYPE_HAVE:
      {
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter << peerRecord_in.have;
        result += converter.str ();
        break;
      }
      case BITTORRENT_MESSAGETYPE_BITFIELD:
      {
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        unsigned int num_pieces_i = 0;
        for (BitTorrent_MessagePayload_BitfieldIterator iterator = peerRecord_in.bitfield.begin ();
             iterator != peerRecord_in.bitfield.end ();
             ++iterator)
        {
          num_pieces_i += (*iterator & 0x80) ? 1 : 0;
          num_pieces_i += (*iterator & 0x40) ? 1 : 0;
          num_pieces_i += (*iterator & 0x20) ? 1 : 0;
          num_pieces_i += (*iterator & 0x10) ? 1 : 0;
          num_pieces_i += (*iterator & 0x08) ? 1 : 0;
          num_pieces_i += (*iterator & 0x04) ? 1 : 0;
          num_pieces_i += (*iterator & 0x02) ? 1 : 0;
          num_pieces_i += (*iterator & 0x01) ? 1 : 0;

          //result += ((*iterator & 0x80) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ((*iterator & 0x40) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ((*iterator & 0x20) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ((*iterator & 0x10) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ((*iterator & 0x08) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ((*iterator & 0x04) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ((*iterator & 0x02) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ((*iterator & 0x01) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          //result += ACE_TEXT_ALWAYS_CHAR (" ");
        } // end FOR
        //result.erase (result.end () - 1);

        std::ostringstream converter;
        converter << num_pieces_i;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR (" pieces");

        break;
      }
      case BITTORRENT_MESSAGETYPE_REQUEST:
      {
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter << peerRecord_in.request.index;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << peerRecord_in.request.begin;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << peerRecord_in.request.length;
        result += converter.str ();
        break;
      }
      case BITTORRENT_MESSAGETYPE_PIECE:
      {
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter << peerRecord_in.piece.index;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << peerRecord_in.piece.begin;
        result += converter.str ();
        break;
      }
      case BITTORRENT_MESSAGETYPE_CANCEL:
      {
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter << peerRecord_in.cancel.index;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << peerRecord_in.cancel.begin;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR (" ");
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << peerRecord_in.cancel.length;
        result += converter.str ();
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), aborting\n"),
                    peerRecord_in.type));
        return result;
      }
    } // end SWITCH
  } // end ELSE
  result += '\n';

  return result;
}

std::string
BitTorrent_Tools::TypeToString (const enum BitTorrent_MessageType& type_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::TypeToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (type_in)
  {
    case BITTORRENT_MESSAGETYPE_CHOKE:
      result = ACE_TEXT_ALWAYS_CHAR ("CHOKE"); break;
    case BITTORRENT_MESSAGETYPE_UNCHOKE:
      result = ACE_TEXT_ALWAYS_CHAR ("UNCHOKE"); break;
    case BITTORRENT_MESSAGETYPE_INTERESTED:
      result = ACE_TEXT_ALWAYS_CHAR ("INTERESTED"); break;
    case BITTORRENT_MESSAGETYPE_NOT_INTERESTED:
      result = ACE_TEXT_ALWAYS_CHAR ("NOT INTERESTED"); break;
    case BITTORRENT_MESSAGETYPE_HAVE:
      result = ACE_TEXT_ALWAYS_CHAR ("HAVE"); break;
    case BITTORRENT_MESSAGETYPE_BITFIELD:
      result = ACE_TEXT_ALWAYS_CHAR ("BITFIELD"); break;
    case BITTORRENT_MESSAGETYPE_REQUEST:
      result = ACE_TEXT_ALWAYS_CHAR ("REQUEST"); break;
    case BITTORRENT_MESSAGETYPE_PIECE:
      result = ACE_TEXT_ALWAYS_CHAR ("PIECE"); break;
    case BITTORRENT_MESSAGETYPE_CANCEL:
      result = ACE_TEXT_ALWAYS_CHAR ("CANCEL"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
BitTorrent_Tools::MetaInfoToInfoHash (const Bencoding_Dictionary_t& metaInfo_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::MetaInfoToInfoHash"));

  std::string result;

  Bencoding_DictionaryIterator_t iterator = metaInfo_out.begin ();
  for (;
       iterator != metaInfo_out.end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_KEY))
      break;
  if (iterator == metaInfo_out.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("metainfo key (was: \"%s\") not found, aborting\n"),
                ACE_TEXT (BITTORRENT_METAINFO_INFO_KEY)));
    return result;
  } // end IF
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);

  std::string bencoded_string =
      Common_Parser_Bencoding_Tools::bencode (*(*iterator).second->dictionary);
  unsigned char info_hash[SHA_DIGEST_LENGTH];
  SHA1 (reinterpret_cast<const unsigned char*> (bencoded_string.c_str ()),
        bencoded_string.size (),
        info_hash);
  result.assign (reinterpret_cast<char*> (info_hash), BITTORRENT_PRT_INFO_HASH_SIZE);

  return result;
}

unsigned int
BitTorrent_Tools::MetaInfoToLength (const Bencoding_Dictionary_t& metaInfo_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::MetaInfoToLength"));

  Bencoding_DictionaryIterator_t iterator = metaInfo_in.begin ();
  for (;
       iterator != metaInfo_in.end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_KEY))
      break;
  ACE_ASSERT (iterator != metaInfo_in.end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);

  // single-file mode ?
  Bencoding_DictionaryIterator_t iterator_2 =
      (*iterator).second->dictionary->begin ();
  for (;
       iterator_2 != (*iterator).second->dictionary->end ();
       ++iterator_2)
    if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_LENGTH_KEY))
      break;
  if (iterator_2 != (*iterator).second->dictionary->end ())
  {
    ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
    return static_cast<unsigned int> ((*iterator_2).second->integer);
  } // end IF

  iterator_2 = (*iterator).second->dictionary->begin ();
  for (;
       iterator_2 != (*iterator).second->dictionary->end ();
       ++iterator_2)
    if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_KEY))
      break;
  ACE_ASSERT (iterator_2 != (*iterator).second->dictionary->end ());
  ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_LIST);

  unsigned int result = 0;
  for (Bencoding_ListIterator_t iterator_3 = (*iterator_2).second->list->begin ();
       iterator_3 != (*iterator_2).second->list->end ();
       ++iterator_3)
  {
    ACE_ASSERT ((*iterator_3)->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
    iterator = (*iterator_3)->dictionary->begin ();
    for (;
         iterator != (*iterator_3)->dictionary->end ();
         ++iterator)
      if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_LENGTH_KEY))
        break;
    ACE_ASSERT (iterator != (*iterator_3)->dictionary->end ());
    ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
    result += static_cast<unsigned int> ((*iterator).second->integer);
  } // end FOR

  return result;
}

bool
BitTorrent_Tools::parseMetaInfoFile (const struct Common_FlexBisonParserConfiguration& configuration_in,
                                     const std::string& metaInfoFileName_in,
                                     Bencoding_Dictionary_t*& metaInfo_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::parseMetaInfoFile"));

  // initialize return value(s)
  if (metaInfo_out)
    Common_Parser_Bencoding_Tools::free (metaInfo_out);

  // sanity check(s)
  ACE_ASSERT (!metaInfo_out);

  bool block_in_parse = configuration_in.block;
#if defined (_DEBUG)
  BitTorrent_Bencoding_ParserDriver parser (configuration_in.debugScanner,
                                            configuration_in.debugParser);
#else
  BitTorrent_Bencoding_ParserDriver parser (false, false);
#endif // _DEBUG

  const_cast<struct Common_FlexBisonParserConfiguration&> (configuration_in).block =
    false;
  if (!parser.initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize parser, aborting\n")));
    return false;
  } // end IF
  const_cast<struct Common_FlexBisonParserConfiguration&> (configuration_in).block =
    block_in_parse;

  // slurp the whole file
  bool result = false;

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (Common_File_Tools::canRead (metaInfoFileName_in, ACE_TEXT_ALWAYS_CHAR ("")));
#else
  ACE_ASSERT (Common_File_Tools::canRead (metaInfoFileName_in, static_cast<uid_t> (-1)));
#endif // ACE_WIN32 || ACE_WIN64

  unsigned char* data_p = NULL;
  ACE_UINT64 file_size_i = 0;
  if (unlikely (!Common_File_Tools::load (metaInfoFileName_in,
                                          data_p,
                                          file_size_i,
                                          COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p);
  ACE_UINT64 size_i = Common_File_Tools::size (metaInfoFileName_in);
  ACE_Message_Block message_block (reinterpret_cast<char*> (data_p),
                                   size_i + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                   ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY);
  message_block.wr_ptr (size_i);

  if (!parser.parse (&message_block))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ParserBase_T::parse(\"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
    goto clean;
  } // end IF
  ACE_ASSERT (parser.bencoding_);
  ACE_ASSERT (parser.bencoding_->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);

  metaInfo_out = parser.bencoding_->dictionary;
  parser.bencoding_ = NULL;

  result = true;

clean:
  delete [] data_p; data_p = NULL;

  return result;
}

std::string
BitTorrent_Tools::generatePeerId ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::generatePeerId"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("-");
  result += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PEER_ID_CLIENT_ID);

  std::string library_version = ACE_TEXT_ALWAYS_CHAR ("0000");
#if defined (HAVE_CONFIG_H)
  // parse library version
//  library_version =
//      ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_VERSION);
//  std::string regex_string =
//    ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+)\\.([[:digit:]]+)\\.([[:digit:]]+)(.+)?$");
//  std::regex regex (regex_string);
//  std::smatch match_results;
//  if (!std::regex_match (library_version,
//                         match_results,
//                         regex,
//                         std::regex_constants::match_default))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid version string (was: \"%s\"), aborting\n"),
//                ACE_TEXT (library_version.c_str ())));
//    return std::string ();
//  } // end IF
////  ACE_ASSERT (match_results.ready () && !match_results.empty ());
//  ACE_ASSERT (!match_results.empty ());
//
//  library_version = match_results[1].str ();
//  if (library_version.size () < 4)
//    library_version += match_results[2].str ();
//  if (library_version.size () < 4)
//    library_version += match_results[3].str ();
//  if (library_version.size () < 4)
//    library_version += ACE_TEXT_ALWAYS_CHAR ("0");
  library_version.resize (4);
#endif // HAVE_CONFIG_H
  result += library_version;
  result += ACE_TEXT_ALWAYS_CHAR ("-");

  // generate 12 bytes of random data
  // *TODO*: ACE_Time_Value::sec() causes linkage trouble; find out why
  time_t now = COMMON_TIME_NOW.msec () / 1000;
  std::ostringstream converter;
  converter << now;
  std::string random_string = converter.str ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  char buffer_a[BUFSIZ];
  ACE_OS::memset (&buffer_a, 0, BUFSIZ);
  ACE_OS::strcpy (buffer_a, ACE_TEXT ("YYYYYYXXXXXX"));
  ACE_OS::mktemp (buffer_a);
  if (unlikely (!ACE_OS::strlen (buffer_a)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::mktemp(): \"%m\", aborting\n")));
    return std::string ();
  } // end IF
  result += buffer_a;
#else
  unsigned char random_hash[SHA256_DIGEST_LENGTH + 1];
  ACE_OS::memset (&random_hash, 0, SHA256_DIGEST_LENGTH + 1);
  SHA256_CTX context;
  SHA256_Init (&context);
  SHA256_Update (&context,
                 reinterpret_cast<const void*> (random_string.c_str ()),
                 random_string.size ());
  SHA256_Final (random_hash, &context);
  //random_hash[SHA256_DIGEST_LENGTH] = '\0';
  result = reinterpret_cast<char*> (random_hash);
#endif // ACE_WIN32 || ACE_WIN64
  result.resize (BITTORRENT_PRT_PEER_ID_LENGTH);

  return result;
}

std::string
BitTorrent_Tools::generateKey ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::generateKey"));

  // initialize return value(s)
  std::string result;

  // generate 20 bytes of random data
  // *TODO*: ACE_Time_Value::sec() causes linkage trouble; find out why
  time_t now = COMMON_TIME_NOW.msec () / 1000;
  std::ostringstream converter;
  converter << now;
  std::string random_string = converter.str ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  char buffer_a[BUFSIZ];
  ACE_OS::memset (&buffer_a, 0, BUFSIZ);
  ACE_OS::strcpy (buffer_a, ACE_TEXT_ALWAYS_CHAR ("XXXXXXXXXXXXXXXXXXXX"));
  ACE_OS::mktemp (buffer_a);
  if (unlikely (!ACE_OS::strlen (buffer_a)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::mktemp(): \"%m\", aborting\n")));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF
  result += buffer_a;
#else
  unsigned char random_hash[SHA256_DIGEST_LENGTH + 1];
  ACE_OS::memset (&random_hash, 0, SHA256_DIGEST_LENGTH + 1);
  SHA256_CTX context;
  SHA256_Init (&context);
  SHA256_Update (&context,
                 reinterpret_cast<const void*> (random_string.c_str ()),
                 random_string.size ());
  SHA256_Final (random_hash, &context);
  //random_hash[SHA256_DIGEST_LENGTH] = '\0';
  result = reinterpret_cast<char*> (random_hash);
  result.resize (20);
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

bool
BitTorrent_Tools::torrentSupportsScrape (const std::string& announceURL_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::torrentSupportsScrape"));

  std::string::size_type position =
      announceURL_in.find_last_of ('/',
                                   std::string::npos);
  if (position == std::string::npos)
    return false;
  ++position;

  return (announceURL_in.find (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY),
                               position) == position);
}
std::string
BitTorrent_Tools::AnnounceURLToScrapeURL (const std::string& announceURL_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::AnnounceURLToScrapeURL"));

  std::string result;

  // sanity check(s)
  if (!BitTorrent_Tools::torrentSupportsScrape (announceURL_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("tracker does not support scraping this torrent (announce URL was: \"%s\"), aborting\n"),
                ACE_TEXT (announceURL_in.c_str ())));
    return result;
  } // end IF

  std::string::size_type position =
      announceURL_in.find_last_of ('/',
                                   std::string::npos);
  ACE_ASSERT (position != std::string::npos);

  result = announceURL_in;
  result.replace (position,
                  ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY)),
                  ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_SCRAPE_URI_PREFIX));

  return result;
}

bool
BitTorrent_Tools::isPieceComplete (unsigned int length_in,
                                   const BitTorrent_PieceChunks_t& chunks_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::isPieceComplete"));

  unsigned int total_length = 0;

  for (BitTorrent_PieceChunksConstIterator_t iterator = chunks_in.begin ();
       iterator != chunks_in.end ();
       ++iterator)
  { ACE_ASSERT ((*iterator).data);
    total_length +=
      static_cast<unsigned int> ((*iterator).data->total_length ());
  } // end FOR

  return (length_in == total_length);
}

bool
BitTorrent_Tools::havePiece (unsigned int index_in,
                             const BitTorrent_MessagePayload_Bitfield& bitfield_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::havePiece"));

  unsigned int array_index_i = index_in / (sizeof (ACE_UINT8) * 8);
  unsigned int array_offset_i = index_in % (sizeof (ACE_UINT8) * 8);

  // sanity check(s)
  ACE_ASSERT (bitfield_in.size () >= array_index_i);

  return bitfield_in[array_index_i] & (0x80 >> array_offset_i);
}

bool
BitTorrent_Tools::hasMissingPiece (const BitTorrent_Pieces_t& pieces_in,
                                   const BitTorrent_MessagePayload_Bitfield& bitfield_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::hasMissingPiece"));

  for (BitTorrent_PiecesConstIterator_t iterator = pieces_in.begin ();
       iterator != pieces_in.end ();
       ++iterator)
  {
    if ((*iterator).onDisk)
      continue;
    if (!BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                            (*iterator).chunks) &&
        BitTorrent_Tools::havePiece (static_cast<unsigned int> (std::distance (pieces_in.begin (), iterator)),
                                     bitfield_in))
      return true;
  } // end FOR

  return false;
}

bool
BitTorrent_Tools::validatePieceHash (const struct BitTorrent_Piece& piece_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::validatePieceHash"));

  ACE_UINT8* data_p = NULL, *data_2 = NULL;
  ACE_Message_Block* message_block_p = NULL;

  // concatenate the data
  ACE_NEW_NORETURN (data_p,
                    ACE_UINT8[piece_in.length]);
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (was: %u byte(s)), aborting\n"),
                sizeof (ACE_UINT8) * piece_in.length));
    return false;
  } // end IF

  data_2 = data_p;
  for (BitTorrent_PieceChunksConstIterator_t iterator = piece_in.chunks.begin ();
       iterator != piece_in.chunks.end ();
       ++iterator)
  {
    message_block_p = (*iterator).data;
    while (message_block_p)
    {
      ACE_OS::memcpy (data_2,
                      message_block_p->rd_ptr (),
                      message_block_p->length ());
      data_2 += message_block_p->length ();
      message_block_p = message_block_p->cont ();
    } // end WHILE
  } // end FOR
  ACE_ASSERT (data_2 == data_p + piece_in.length);

  // hash and compare
  ACE_UINT8 piece_hash[SHA_DIGEST_LENGTH];
  SHA1 (data_p,
        piece_in.length,
        piece_hash);
  delete [] data_p; data_p = NULL;
  return (ACE_OS::memcmp (piece_hash,
                          piece_in.hash,
                          SHA_DIGEST_LENGTH) == 0);
}

bool
BitTorrent_Tools::savePiece (const std::string& metaInfoFileName_in,
                             unsigned int numberOfPieces_in,
                             unsigned int index_in,
                             struct BitTorrent_Piece& piece_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::savePiece"));

  std::string filename =
      Common_File_Tools::getUserDownloadDirectory (ACE_TEXT_ALWAYS_CHAR (""));
  filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  filename += metaInfoFileName_in;
  filename += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECES_DIRECTORY_SUFFIX);
  // sanity check(s): directory exists ?
  // No ? --> (try to) create it then
  if (unlikely (!Common_File_Tools::isDirectory (filename)))
  {
    if (!Common_File_Tools::createDirectory (filename))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::createDirectory(\"%s\"), aborting\n"),
                  ACE_TEXT (filename.c_str ())));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("created directory: \"%s\"\n"),
                ACE_TEXT (filename.c_str ())));
  } // end IF
  filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  std::ostringstream converter;
  converter << std::setfill ('0');
  converter <<
    std::setw (Common_Math_Tools::digitsBase10 (static_cast<int> (numberOfPieces_in)));
  converter << index_in;
  filename += converter.str ();
  filename += ACE_TEXT_ALWAYS_CHAR ("_");
  filename += piece_inout.filename;
  filename += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECE_FILENAME_SUFFIX);

  int flags_i =  (O_BINARY |
                  O_CREAT  |
                  O_TRUNC  |
                  O_WRONLY);
  ACE_FILE_IO stream;
  BitTorrent_PieceChunksConstIterator_t iterator = piece_inout.chunks.begin ();
  ACE_Message_Block* message_block_p = (*iterator).data;
  ssize_t result = 0;
  size_t bytes_transferred_i = 0;
  int result_2 = 0;
  bool result_3 = true;
  if (!Common_File_Tools::open (filename,
                                flags_i,
                                stream))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::open(\"%s\", %d), aborting\n"),
                ACE_TEXT (filename.c_str ()),
                flags_i));
    return false;
  } // end IF
  ++iterator;
  while (iterator != piece_inout.chunks.end ())
  {
    message_block_p->next ((*iterator).data);
    message_block_p = (*iterator).data;
    ++iterator;
  } // end FOR
  result = stream.send_n ((*piece_inout.chunks.begin ()).data,
                          NULL,
                          &bytes_transferred_i);
  if (unlikely (bytes_transferred_i != piece_inout.length))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_IO::send_n(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (filename.c_str ())));
    result_3 = false;
  } // end IF
  ACE_UNUSED_ARG (result);
  result_2 = stream.close ();
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_IO::close(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (filename.c_str ())));
    result_3 = false;
  } // end IF
  else
  {
    piece_inout.onDisk = true;
    BitTorrent_Tools::freeChunks (piece_inout.chunks);

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: saved piece %u to \"%s\" (%u byte(s))...\n"),
                ACE::basename (ACE_TEXT (metaInfoFileName_in.c_str ()),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                index_in,
                ACE_TEXT (filename.c_str ()),
                piece_inout.length));
  } // end ELSE

  return result_3;
}

void
BitTorrent_Tools::freeChunks (BitTorrent_PieceChunks_t& chunks_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::freeChunks"));

  for (BitTorrent_PieceChunksIterator_t iterator = chunks_in.begin ();
       iterator != chunks_in.end ();
       ++iterator)
    (*iterator).data->release ();
  chunks_in.clear ();
}

bool
BitTorrent_Tools::assembleFiles (const std::string& metaInfoFileName_in,
                                 const Bencoding_Dictionary_t& metaInfo_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::assembleFiles"));

  // step1: retrieve all piece file(s)
  std::string pieces_path =
      Common_File_Tools::getUserDownloadDirectory (ACE_TEXT_ALWAYS_CHAR (""));
  pieces_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  pieces_path += metaInfoFileName_in;
  pieces_path += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECES_DIRECTORY_SUFFIX);
  Common_File_IdentifierList_t piece_files =
    Common_File_Tools::files (pieces_path,
                              BitTorrent_Tools::selector);
  // sanity check(s)
  //ACE_ASSERT (Common_File_Tools::size (piece_files) == BitTorrent_Tools::MetaInfoToLength (metaInfo_in));

  // step2: sort by name
  struct common_file_identifier_less file_identifier_less;
  std::sort (piece_files.begin (),
             piece_files.end (),
             file_identifier_less);

  // step3: process piece(s)
  uint8_t* data_p = NULL, *data_2 = NULL;
  ACE_UINT64 file_size_i = 0;
  Bencoding_DictionaryIterator_t iterator = metaInfo_in.begin ();
  std::string directory_name_string;
  for (;
       iterator != metaInfo_in.end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_KEY))
      break;
  ACE_ASSERT (iterator != metaInfo_in.end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
  Bencoding_DictionaryIterator_t iterator_2 =
    (*iterator).second->dictionary->begin ();
  for (;
    iterator_2 != (*iterator).second->dictionary->end ();
    ++iterator_2)
    if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_NAME_KEY))
      break;
  ACE_ASSERT (iterator_2 != (*iterator).second->dictionary->end ());
  ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
  directory_name_string = *(*iterator_2).second->string;
  iterator_2 =
      (*iterator).second->dictionary->begin ();
  for (;
       iterator_2 != (*iterator).second->dictionary->end ();
       ++iterator_2)
    if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_KEY))
      break;
  bool is_single_file_b = false;
  ACE_UINT64 current_file_length = 0;
  std::string current_file_path;
  Bencoding_ListIterator_t iterator_3;
  Bencoding_DictionaryIterator_t iterator_4;
  if (iterator_2 == (*iterator).second->dictionary->end()) // torrent contains just a single file
  {
    is_single_file_b = true;
    iterator_2 =
      (*iterator).second->dictionary->begin ();
    for (;
         iterator_2 != (*iterator).second->dictionary->end ();
         ++iterator_2)
      if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_LENGTH_KEY))
        break;
    ACE_ASSERT (iterator_2 != (*iterator).second->dictionary->end());
    ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
    current_file_length =
      static_cast<ACE_UINT64> ((*iterator_2).second->integer);
    current_file_path = directory_name_string;
    directory_name_string.clear ();
  } // end IF
  else
  {
    ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_LIST);
    iterator_3 = (*iterator_2).second->list->begin ();
    ACE_ASSERT (iterator_3 != (*iterator_2).second->list->end ());
    ACE_ASSERT ((*iterator_3)->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
    iterator_4 = (*iterator_3)->dictionary->begin ();
    for (;
         iterator_4 != (*iterator_3)->dictionary->end ();
         ++iterator_4)
      if (*(*iterator_4).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_LENGTH_KEY))
        break;
    ACE_ASSERT (iterator_4 != (*iterator_3)->dictionary->end ());
    ACE_ASSERT ((*iterator_4).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
    current_file_length =
      static_cast<ACE_UINT64> ((*iterator_4).second->integer);
    iterator_4 = (*iterator_3)->dictionary->begin();
    for (;
         iterator_4 != (*iterator_3)->dictionary->end();
         ++iterator_4)
      if (*(*iterator_4).first == ACE_TEXT_ALWAYS_CHAR(BITTORRENT_METAINFO_INFO_FILES_PATH_KEY))
        break;
    ACE_ASSERT(iterator_4 != (*iterator_3)->dictionary->end());
    ACE_ASSERT((*iterator_4).second->type == Bencoding_Element::BENCODING_TYPE_LIST);
    current_file_path =
        BitTorrent_Tools::listToPath (*(*iterator_4).second->list);
  } // end ELSE
  std::string current_file_path_on_disk = pieces_path;
  if (is_single_file_b)
  {
    current_file_path_on_disk += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    current_file_path_on_disk += current_file_path;
  } // end IF
  else
  {
    current_file_path_on_disk += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    current_file_path_on_disk += directory_name_string;
    current_file_path_on_disk += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    current_file_path_on_disk += current_file_path;
  } // end ELSE
  unsigned int bytes_to_write = 0;
  std::string current_piece_file;
  for (Common_File_IdentifierListIterator_t iterator_5 = piece_files.begin ();
       iterator_5 != piece_files.end ();
       ++iterator_5)
  {
    current_piece_file = pieces_path;
    current_piece_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    current_piece_file += (*iterator_5).identifier;
    // step1: load the current piece file into memory
    if (!Common_File_Tools::load (current_piece_file,
                                  data_p,
                                  file_size_i,
                                  0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                  ACE_TEXT ((*iterator_5).identifier.c_str ())));
      return false;
    } // end IF
    data_2 = data_p;

    // step2: write to the current file
write_file:
    bytes_to_write =
      static_cast<unsigned int> ((file_size_i >= current_file_length) ? current_file_length
                                                                      : file_size_i);
    if (!Common_File_Tools::store (current_file_path_on_disk,
                                   data_2,
                                   bytes_to_write,
                                   true)) // append if exists
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), aborting\n"),
                  ACE_TEXT (current_file_path_on_disk.c_str ())));
      delete [] data_p; data_p = NULL;
      return false;
    } // end IF
    current_file_length -= bytes_to_write;
    file_size_i -= bytes_to_write;
    data_2 += bytes_to_write;
    if (current_file_length)
    { // *NOTE*: wrote at least file_size bytes here
      ACE_ASSERT (!file_size_i);
      // --> load the next piece and continue
      delete[] data_p; data_p = NULL; data_2 = NULL;
      continue;
    } // end IF
    // done with the current file
    // --> get the next filename/size ?
    if (!is_single_file_b)
    {
      ++iterator_3;
      if (iterator_3 != (*iterator_2).second->list->end ())
      { ACE_ASSERT ((*iterator_3)->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
        iterator_4 = (*iterator_3)->dictionary->begin ();
        for (;
             iterator_4 != (*iterator_3)->dictionary->end ();
             ++iterator_4)
          if (*(*iterator_4).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_LENGTH_KEY))
            break;
        ACE_ASSERT (iterator_4 != (*iterator_3)->dictionary->end ());
        ACE_ASSERT ((*iterator_4).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
        current_file_length =
          static_cast<unsigned int> ((*iterator_4).second->integer);
        iterator_4 = (*iterator_3)->dictionary->begin ();
        for (;
             iterator_4 != (*iterator_3)->dictionary->end ();
             ++iterator_4)
          if (*(*iterator_4).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_PATH_KEY))
            break;
        ACE_ASSERT (iterator_4 != (*iterator_3)->dictionary->end ());
        ACE_ASSERT ((*iterator_4).second->type == Bencoding_Element::BENCODING_TYPE_LIST);
        current_file_path =
            BitTorrent_Tools::listToPath (*(*iterator_4).second->list);
        current_file_path_on_disk = pieces_path;
        current_file_path_on_disk += ACE_DIRECTORY_SEPARATOR_CHAR_A;
        current_file_path_on_disk += directory_name_string;
        current_file_path_on_disk += ACE_DIRECTORY_SEPARATOR_CHAR_A;
        current_file_path_on_disk += current_file_path;
        if (file_size_i)
          goto write_file; // there is data left
        // --> load the next piece and continue
      } // end IF
    } // end IF
    ACE_ASSERT (!file_size_i);
    delete [] data_p; data_p = NULL;
  } // end FOR

  // clean up
  Common_File_Tools::deleteFiles (piece_files);

  return true;
}

unsigned int
BitTorrent_Tools::receivedBytes (const BitTorrent_Pieces_t& pieces_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::receivedBytes"));

  unsigned int result = 0;

  for (BitTorrent_PiecesConstIterator_t iterator = pieces_in.begin ();
       iterator != pieces_in.end ();
       ++iterator)
    for (BitTorrent_PieceChunksConstIterator_t iterator_2 = (*iterator).chunks.begin ();
         iterator_2 != (*iterator).chunks.end ();
         ++iterator_2)
      result += BitTorrent_Tools::chunkLength (*iterator_2);

  return result;
}

void
BitTorrent_Tools::sanitizeChunks (BitTorrent_PieceChunks_t& chunks_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::sanitizeChunks"));

  // step1: sort all data by offset and length
  struct bittorrent_piece_chunks_less less_s;
  std::sort (chunks_in.begin (),
             chunks_in.end (),
             less_s);

  BitTorrent_PieceChunksIterator_t iterator_3, iterator_4;
  unsigned int total_length_i = 0;
  unsigned int overlap_i = 0;
  ACE_Message_Block* message_block_p = NULL, *message_block_2 = NULL;
  // step2: merge any overlaps
  iterator_3 = chunks_in.begin ();
  if (iterator_3 == chunks_in.end ())
    return; // no chunks yet
  iterator_4 = iterator_3;
  std::advance (iterator_4, 1);
  bool done_b = false;
  while (iterator_4 != chunks_in.end ())
  { // step2a: remove 'duplicates'
    while ((*iterator_3).offset == (*iterator_4).offset)
    { ACE_ASSERT ((*iterator_4).data);
      (*iterator_4).data->release (); (*iterator_4).data = NULL;
      iterator_4 = chunks_in.erase (iterator_4);
      if (iterator_4 == chunks_in.end ())
      {
        done_b = true;
        break;
      } // end IF
    } // end WHILE
    if (done_b)
      break;
    // step2b: handle overlaps
    total_length_i =
      static_cast<unsigned int> ((*iterator_3).data->total_length ());
    overlap_i =
      (((*iterator_3).offset + total_length_i) > (*iterator_4).offset ? (*iterator_3).offset + total_length_i - (*iterator_4).offset
                                                                      : 0);
    if (overlap_i)
    {
      // step2ba: crop the last continuation until the last continuation
      //          contains all (!) the (remaining) overlap
      do
      {
        // step2baa: retrieve last continuation
        message_block_p = (*iterator_3).data;
        message_block_2 = NULL;
        while (message_block_p->cont ())
        {
          message_block_2 = message_block_p;
          message_block_p = message_block_p->cont ();
        } // end WHILE
        // step2bab: last continuation contains the overlap ?
        if (static_cast<unsigned int> (message_block_p->length ()) > overlap_i)
          break; // proceed
        // step2bac: no --> crop and continue
        overlap_i -= static_cast<unsigned int> (message_block_p->length ());
        ACE_ASSERT (message_block_2);
        message_block_2->cont (NULL);
        message_block_p->release ();
      } while (overlap_i);
      // step2bb: crop the remaining overlap (if any)
      if (overlap_i)
      { ACE_ASSERT (message_block_p);
        message_block_p->rd_ptr (overlap_i);
      } // end IF
    } // end IF
    std::advance (iterator_3, 1);
    std::advance (iterator_4, 1);
  } // end WHILE
}

std::string
BitTorrent_Tools::listToPath (const Bencoding_List_t& list_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::listToPath"));

  std::string result;

  for (Bencoding_ListIterator_t iterator = list_in.begin ();
       iterator != list_in.end ();
       ++iterator)
  { ACE_ASSERT ((*iterator)->type == Bencoding_Element::BENCODING_TYPE_STRING);
    result += *(*iterator)->string;
    result += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  } // end FOR
  result.erase (--result.end ());

  return result;
}

unsigned int
BitTorrent_Tools::pieceFileNameToIndex (const std::string& fileName_in)
{
  unsigned int result = 0;

  std::string filename = ACE::basename (ACE_TEXT (fileName_in.c_str ()),
                                        ACE_DIRECTORY_SEPARATOR_CHAR);
  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+)(?:_[[:alnum:]]{6}");
  regex_string +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECE_FILENAME_SUFFIX);
  regex_string += ACE_TEXT_ALWAYS_CHAR (")$");
  std::regex::flag_type flags = std::regex_constants::ECMAScript;
  std::regex regex;
  std::smatch match_results;
  regex.assign (regex_string, flags);
  if (unlikely (!std::regex_match (filename,
                                   match_results,
                                   regex,
                                   std::regex_constants::match_default)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid filename (was: \"%s\"), aborting\n"),
                ACE_TEXT (fileName_in.c_str ())));
    return -1;
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  std::istringstream converter;
  converter.str (match_results[1].str ());
  converter >> result;

  return result;
}

bool
BitTorrent_Tools::loadPiece (const std::string& metaInfoFileName_in,
                             unsigned int index_in,
                             struct BitTorrent_Piece& piece_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::loadPiece"));

  // sanity check(s)
  ACE_ASSERT (piece_inout.chunks.empty () && piece_inout.onDisk);

  std::string pieces_path =
      Common_File_Tools::getUserDownloadDirectory (ACE_TEXT_ALWAYS_CHAR (""));
  pieces_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  pieces_path +=
    ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (metaInfoFileName_in.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR));
  pieces_path +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECES_DIRECTORY_SUFFIX);
  Common_File_IdentifierList_t piece_files =
    Common_File_Tools::files (pieces_path,
                              BitTorrent_Tools::selector);

  // step2: sort by name
  struct common_file_identifier_less file_identifier_less;
  std::sort (piece_files.begin (),
             piece_files.end (),
             file_identifier_less);

  std::string filename_string;
  uint8_t* data_p = NULL;
  ACE_UINT64 file_size_i = 0;
  ACE_Message_Block* message_block_p = NULL;
  struct BitTorrent_Piece_Chunk chunk_s;
  unsigned int piece_index_i = 0;
  for (Common_File_IdentifierListIterator_t iterator = piece_files.begin ();
       iterator != piece_files.end ();
       ++iterator)
  {
    piece_index_i =
      BitTorrent_Tools::pieceFileNameToIndex ((*iterator).identifier);
    if (likely (piece_index_i != index_in))
      continue;

    // load the current piece file into memory
    filename_string = pieces_path;
    filename_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    filename_string += (*iterator).identifier;
    if (!Common_File_Tools::load (filename_string,
                                  data_p,
                                  file_size_i,
                                  0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                  ACE_TEXT ((*iterator).identifier.c_str ())));
      return false;
    } // end IF
    ACE_NEW_NORETURN (message_block_p,
                      ACE_Message_Block (reinterpret_cast<char*> (data_p),
                                         file_size_i,
                                         ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY));
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      delete [] data_p; data_p = NULL;
      return false;
    } // end IF
    message_block_p->wr_ptr (file_size_i);
    message_block_p->clr_flags (ACE_Message_Block::DONT_DELETE); // release with message block

    chunk_s.data = message_block_p;
    piece_inout.chunks.push_back (chunk_s);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: loaded piece %u (%Q byte(s))...\n"),
                ACE::basename (ACE_TEXT (metaInfoFileName_in.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
                index_in,
                file_size_i));
    break; // done
  } // end FOR

  return true;
}

bool
BitTorrent_Tools::loadPieces (const std::string& metaInfoFileName_in,
                              BitTorrent_Pieces_t& pieces_inout,
                              bool loadToMemory_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::loadPieces"));

  // sanity check(s)
  ACE_ASSERT (!pieces_inout.empty ());

  // step1: retrieve all piece file(s)
  std::string pieces_path =
      Common_File_Tools::getUserDownloadDirectory (ACE_TEXT_ALWAYS_CHAR (""));
  pieces_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  pieces_path +=
    ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (metaInfoFileName_in.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR));
  pieces_path +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECES_DIRECTORY_SUFFIX);
  Common_File_IdentifierList_t piece_files =
    Common_File_Tools::files (pieces_path,
                              BitTorrent_Tools::selector);

  // step2: sort by name
  struct common_file_identifier_less file_identifier_less;
  std::sort (piece_files.begin (),
             piece_files.end (),
             file_identifier_less);

  std::string filename_string;
  uint8_t* data_p = NULL;
  ACE_UINT64 file_size_i = 0;
  ACE_Message_Block* message_block_p = NULL;
  struct BitTorrent_Piece_Chunk chunk_s;
  BitTorrent_PiecesIterator_t iterator_2;
  unsigned int piece_index_i = 0;
  for (Common_File_IdentifierListIterator_t iterator = piece_files.begin ();
       iterator != piece_files.end ();
       ++iterator)
  {
    piece_index_i =
      BitTorrent_Tools::pieceFileNameToIndex ((*iterator).identifier);
    iterator_2 = pieces_inout.begin ();
    std::advance (iterator_2, piece_index_i);
    ACE_ASSERT (iterator_2 != pieces_inout.end ());

    if (!loadToMemory_in)
    {
      (*iterator_2).onDisk = true;

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: found piece %u...\n"),
      //            ACE::basename (ACE_TEXT (metaInfoFileName_in.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
      //            piece_index_i));

      continue;
    } // end IF

    // load the current piece file into memory
    filename_string = pieces_path;
    filename_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    filename_string += (*iterator).identifier;
    if (!Common_File_Tools::load (filename_string,
                                  data_p,
                                  file_size_i,
                                  0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                  ACE_TEXT ((*iterator).identifier.c_str ())));
      goto error;
    } // end IF
    ACE_NEW_NORETURN (message_block_p,
                      ACE_Message_Block (reinterpret_cast<char*> (data_p),
                                         file_size_i,
                                         ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY));
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      delete [] data_p; data_p = NULL;
      goto error;
    } // end IF
    message_block_p->wr_ptr (file_size_i);
    message_block_p->clr_flags (ACE_Message_Block::DONT_DELETE); // release with message block

    chunk_s.data = message_block_p;
    (*iterator_2).chunks.push_back (chunk_s);

    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: loaded piece %u (%Q byte(s))...\n"),
    //            ACE::basename (ACE_TEXT (metaInfoFileName_in.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
    //            piece_index_i,
    //            file_size_i));
  } // end FOR

  return true;

error:
  for (iterator_2 = pieces_inout.begin ();
       iterator_2 != pieces_inout.end ();
       ++iterator_2)
    BitTorrent_Tools::freeChunks ((*iterator_2).chunks);
  return false;
}

std::vector<unsigned int>
BitTorrent_Tools::getPieceIndexes (const BitTorrent_Pieces_t& pieces_in,
                                   bool getMissing_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::getPieceIndexes"));

  std::vector<unsigned int> result;

  unsigned int index_i = 0;
  bool is_complete_b = false;
  for (BitTorrent_PiecesConstIterator_t iterator = pieces_in.begin ();
       iterator != pieces_in.end ();
       ++iterator, ++index_i)
  {
    is_complete_b = (*iterator).onDisk ||
                    BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                                       (*iterator).chunks);
    if ((getMissing_in && !is_complete_b) || (!getMissing_in && is_complete_b))
      result.push_back (index_i);
  } // end FOR

  return result;
}

unsigned int
BitTorrent_Tools::chunksLength (const BitTorrent_PieceChunks_t& chunks_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::chunksLength"));

  unsigned int result = 0;

  for (BitTorrent_PieceChunksConstIterator_t iterator = chunks_in.begin ();
       iterator != chunks_in.end ();
       ++iterator)
    result += BitTorrent_Tools::chunkLength (*iterator);

  return result;
}

std::vector<struct BitTorrent_MessagePayload_Request>
BitTorrent_Tools::getMissingChunks (unsigned int pieceLength_in,
                                    const BitTorrent_PieceChunks_t& chunks_in)
{
  std::vector<struct BitTorrent_MessagePayload_Request> result;

  struct BitTorrent_MessagePayload_Request request_s;
  unsigned int offset = 0;
  unsigned int missing_data = 0;
  BitTorrent_PieceChunksConstIterator_t iterator = chunks_in.begin ();
  for (;
       iterator != chunks_in.end ();
       ++iterator)
  {
    if ((*iterator).offset != offset)
    { ACE_ASSERT ((*iterator).offset > offset);
      missing_data = (*iterator).offset - offset;
      while (missing_data)
      {
        request_s.begin = offset;
        request_s.length =
          std::min (missing_data,
                    static_cast<unsigned int> (BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX));
        result.push_back (request_s);
        offset += request_s.length;
        missing_data -= request_s.length;
      } // end WHILE
      continue;
    } // end IF
    offset = (*iterator).offset + BitTorrent_Tools::chunkLength (*iterator);
  } // end FOR
  missing_data = pieceLength_in - offset;
  ACE_ASSERT (missing_data >= 0);
  while (missing_data)
  {
    request_s.begin = offset;
    request_s.length =
      std::min (missing_data,
                static_cast<unsigned int> (BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX));
    result.push_back (request_s);
    offset += request_s.length;
    missing_data -= request_s.length;
  } // end WHILE
  return result;
}
