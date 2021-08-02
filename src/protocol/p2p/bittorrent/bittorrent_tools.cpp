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

#include <regex>
#include <sstream>

#include "openssl/sha.h"

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_file_tools.h"

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
        for (BitTorrent_MessagePayload_BitfieldIterator iterator = peerRecord_in.bitfield.begin ();
             iterator != peerRecord_in.bitfield.end ();
             ++iterator)
        {
          result += ((*iterator & 0x80) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x40) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x20) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x10) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x08) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x04) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x02) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x01) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ACE_TEXT_ALWAYS_CHAR (" ");
        } // end FOR
        result.erase (result.end () - 1);
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
  unsigned char info_hash[SHA_DIGEST_LENGTH + 1];
  SHA1 (reinterpret_cast<const unsigned char*> (bencoded_string.c_str ()),
        bencoded_string.size (),
        info_hash);
  info_hash[SHA_DIGEST_LENGTH] = '\0';
  result += reinterpret_cast<char*> (info_hash);

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
BitTorrent_Tools::parseMetaInfoFile (const struct Common_ParserConfiguration& configuration_in,
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
  BitTorrent_Bencoding_ParserDriver parser (configuration_in.debugScanner,
                                            configuration_in.debugParser);

  const_cast<struct Common_ParserConfiguration&> (configuration_in).block =
    false;
  if (!parser.initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize parser, aborting\n")));
    return false;
  } // end IF
  const_cast<struct Common_ParserConfiguration&> (configuration_in).block =
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
  unsigned int file_size_i = 0;
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
  unsigned int size = Common_File_Tools::size (metaInfoFileName_in);
  ACE_Message_Block message_block (reinterpret_cast<char*> (data_p),
                                   size + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                   ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY);
  message_block.wr_ptr (size);

  if (!parser.parse (&message_block))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ParserBase_T::parse(\"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
    goto clean;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsed meta-info file \"%s\"...\n"),
//              ACE_TEXT (metaInfoFileName_in.c_str ())));

  metaInfo_out = parser.bencoding_;

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

  // generate 14 bytes of random data
  // *TODO*: ACE_Time_Value::sec() causes linkage trouble; find out why
  time_t now = COMMON_TIME_NOW.msec () / 1000;
  std::ostringstream converter;
  converter << now;
  std::string random_string = converter.str ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  char buffer_a[BUFSIZ];
  ACE_OS::memset (&buffer_a, 0, BUFSIZ);
  ACE_OS::strcpy (buffer_a, ACE_TEXT ("XXXXXXXXXXXXXX"));
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
  ACE_OS::strcpy (buffer_a, ACE_TEXT ("XXXXXXXXXXXXXXXXXXXX"));
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

  for (BitTorrent_PieceChunksIterator_t iterator = chunks_in.begin ();
       iterator != chunks_in.end ();
       ++iterator)
  { ACE_ASSERT ((*iterator).data);
    total_length += (*iterator).data->total_length ();
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
    if (!BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                            (*iterator).chunks) &&
        BitTorrent_Tools::havePiece (std::distance (pieces_in.begin (), iterator),
                                     bitfield_in))
      return true;

  return false;
}

bool
BitTorrent_Tools::savePiece (const std::string& metaInfoFileName_in,
                             unsigned int index_in,
                             struct BitTorrent_Piece& piece_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::savePiece"));

  // step1: validate piece hash
  unsigned int total_length = 0;
  for (BitTorrent_PieceChunksIterator_t iterator = piece_in.chunks.begin ();
       iterator != piece_in.chunks.end ();
       ++iterator)
    total_length += (*iterator).data->total_length ();
  unsigned char* data_p = NULL, *data_2 = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    unsigned char[total_length]);
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (was: %u byte(s)), aborting\n"),
                sizeof (unsigned char[total_length])));
    return false;
  } // end IF
  struct bittorrent_piece_chunks_less less_s;
  std::sort (piece_in.chunks.begin (),
             piece_in.chunks.end (),
             less_s);
  data_2 = data_p;
  for (BitTorrent_PieceChunksIterator_t iterator = piece_in.chunks.begin ();
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
  unsigned char piece_hash[SHA_DIGEST_LENGTH + 1];
  SHA1 (data_p,
        total_length,
        piece_hash);
  piece_hash[SHA_DIGEST_LENGTH] = '\0';
  if (ACE_OS::memcmp (piece_hash,
                      piece_in.hash.c_str (),
                      SHA_DIGEST_LENGTH))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to validate piece hash; freeing chunk(s), aborting\n")));
    delete [] data_p; data_p = NULL;
    BitTorrent_Tools::freeChunks (piece_in.chunks);
    return false;
  } // end IF

  // step2: write the piece to a temporary file
  std::string filename =
      Common_File_Tools::getUserDownloadDirectory (ACE_TEXT_ALWAYS_CHAR (""));
  filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  filename += metaInfoFileName_in;
  // sanity check(s): directory exists ?
  // No ? --> (try to) create it then
  if (unlikely (!Common_File_Tools::isDirectory (filename)))
  {
    if (!Common_File_Tools::createDirectory (filename))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::createDirectory(\"%s\"), aborting\n"),
                  ACE_TEXT (filename.c_str ())));
      delete [] data_p; data_p = NULL;
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("created directory: \"%s\"\n"),
                ACE_TEXT (filename.c_str ())));
  } // end IF
  filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  std::ostringstream converter;
  converter << index_in;
  filename += converter.str ();
  filename += ACE_TEXT_ALWAYS_CHAR ("_");
  filename += piece_in.filename;
  filename += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_PIECE_FILENAME_SUFFIX);
  if (!Common_File_Tools::store (filename,
                                 data_p,
                                 total_length))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), aborting\n"),
                ACE_TEXT (filename.c_str ())));
    delete [] data_p; data_p = NULL;
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: saved piece to \"%s\"...\n"),
              ACE_TEXT (metaInfoFileName_in.c_str ()),
              ACE_TEXT (filename.c_str ())));
  delete [] data_p; data_p = NULL;

  return true;
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
  Common_File_IdentifierList_t piece_files =
    Common_File_Tools::files (pieces_path,
                              BitTorrent_Tools::selector);
  // sanity check(s)
  ACE_ASSERT (Common_File_Tools::size (piece_files) == BitTorrent_Tools::MetaInfoToLength (metaInfo_in));

  // step2: sort by name
  struct common_file_identifier_less file_identifier_less;
  std::sort (piece_files.begin (),
             piece_files.end (),
             file_identifier_less);

  // step3: process piece(s)
  uint8_t* data_p = NULL;
  unsigned int file_size = 0;
  Bencoding_DictionaryIterator_t iterator = metaInfo_in.begin ();
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
    if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_KEY))
      break;
  ACE_ASSERT (iterator_2 != (*iterator).second->dictionary->end ());
  ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_LIST);
  Bencoding_ListIterator_t iterator_3 = (*iterator_2).second->list->begin ();
  ACE_ASSERT (iterator_3 != (*iterator_2).second->list->end ());
  ACE_ASSERT ((*iterator_3)->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
  Bencoding_DictionaryIterator_t iterator_4 =
      (*iterator_3)->dictionary->begin ();
  for (;
       iterator_4 != (*iterator_3)->dictionary->end ();
       ++iterator_4)
    if (*(*iterator_4).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_LENGTH_KEY))
      break;
  ACE_ASSERT (iterator_4 != (*iterator_3)->dictionary->end ());
  ACE_ASSERT ((*iterator_4).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
  unsigned int current_file_length = (*iterator_4).second->integer;
  iterator_4 = (*iterator_3)->dictionary->begin ();
  for (;
       iterator_4 != (*iterator_3)->dictionary->end ();
       ++iterator_4)
    if (*(*iterator_4).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_PATH_KEY))
      break;
  ACE_ASSERT (iterator_4 != (*iterator_3)->dictionary->end ());
  ACE_ASSERT ((*iterator_4).second->type == Bencoding_Element::BENCODING_TYPE_LIST);
  std::string current_file_path =
      BitTorrent_Tools::listToPath (*(*iterator_4).second->list);
  std::string current_file_path_on_disk = pieces_path;
  current_file_path_on_disk += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  current_file_path_on_disk += current_file_path;
  unsigned int bytes_to_write = 0;
  for (Common_File_IdentifierListIterator_t iterator_5 = piece_files.begin ();
       iterator_5 != piece_files.end ();
       ++iterator_5)
  {
    // step1: load the current piece file into memory
    if (!Common_File_Tools::load ((*iterator_5).identifier,
                                  data_p,
                                  file_size,
                                  0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                  ACE_TEXT ((*iterator_5).identifier.c_str ())));
      return false;
    } // end IF

    // step2: write to the current file
write_file:
    bytes_to_write = ((file_size >= current_file_length) ? current_file_length
                                                         : file_size);
    if (!Common_File_Tools::store (current_file_path_on_disk,
                                   data_p,
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
    file_size -= bytes_to_write;
    if (current_file_length)
    { // *NOTE*: wrote at least file_size bytes here
      ACE_ASSERT (!file_size);
      // --> load the next piece and continue
      delete [] data_p; data_p = NULL;
      continue;
    } // end IF
    // done with the current file
    // --> get the next filename/size ?
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
      current_file_length = (*iterator_4).second->integer;
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
      current_file_path_on_disk += current_file_path;
      if (file_size)
        goto write_file; // there is data left
      // --> load the next piece and continue
    } // end IF
    ACE_ASSERT (!file_size);
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
    for (BitTorrent_PieceChunksIterator_t iterator_2 = (*iterator).chunks.begin ();
         iterator_2 != (*iterator).chunks.end ();
         ++iterator_2)
      result += BitTorrent_Tools::chunkLength (*iterator_2);

  return result;
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
