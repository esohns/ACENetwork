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

//#include "ace/Synch.h"
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

std::string
BitTorrent_Tools::HandShakeToString (const struct BitTorrent_PeerHandShake& peerHandshake_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::HandShakeToString"));

  std::string result;
  std::ostringstream converter;

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
          result += ((*iterator & 0x01) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x02) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x04) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x08) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x10) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x20) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x40) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ((*iterator & 0x80) ? ACE_TEXT_ALWAYS_CHAR ("1") : ACE_TEXT_ALWAYS_CHAR ("0"));
          result += ACE_TEXT_ALWAYS_CHAR (" ");
        } // end FOR
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

  std::string key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_KEY);
//  Bencoding_DictionaryIterator_t iterator = metaInfo_out.find (&key);
  Bencoding_DictionaryIterator_t iterator = metaInfo_out.begin ();
  for (;
       iterator != metaInfo_out.end ();
       ++iterator)
    if (*(*iterator).first == key) break;
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

  std::string key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_KEY);
//  Bencoding_DictionaryIterator_t iterator = metaInfo_in.find (&key);
  Bencoding_DictionaryIterator_t iterator = metaInfo_in.begin ();
  for (;
       iterator != metaInfo_in.end ();
       ++iterator)
    if (*(*iterator).first == key) break;
  ACE_ASSERT (iterator != metaInfo_in.end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);

  // single-file mode ?
  key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_LENGTH_KEY);
//  Bencoding_DictionaryIterator_t iterator_2 =
//      (*iterator).second->dictionary->find (&key);
  Bencoding_DictionaryIterator_t iterator_2 =
      (*iterator).second->dictionary->begin ();
  for (;
       iterator_2 != (*iterator).second->dictionary->end ();
       ++iterator_2)
    if (*(*iterator_2).first == key) break;
  if (iterator_2 != (*iterator).second->dictionary->end ())
  {
    ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
    return static_cast<unsigned int> ((*iterator_2).second->integer);
  } // end IF

  key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_FILES_KEY);
//  iterator_2 = (*iterator).second->dictionary->find (&key);
  iterator_2 = (*iterator).second->dictionary->begin ();
  for (;
       iterator_2 != (*iterator).second->dictionary->end ();
       ++iterator_2)
    if (*(*iterator_2).first == key) break;
  ACE_ASSERT (iterator_2 != (*iterator).second->dictionary->end ());
  ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_LIST);

  key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_LENGTH_KEY);
  unsigned int result = 0;
  for (Bencoding_ListIterator_t iterator_3 = (*iterator_2).second->list->begin ();
       iterator_3 != (*iterator_2).second->list->end ();
       ++iterator_3)
  {
    ACE_ASSERT ((*iterator_3)->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
//    iterator = (*iterator_3)->dictionary->find (&key);
    iterator = (*iterator_3)->dictionary->begin ();
    for (;
         iterator != (*iterator_3)->dictionary->end ();
         ++iterator)
      if (*(*iterator).first == key) break;
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
  BitTorrent_Bencoding_ParserDriver parser (false, //configuration_in.debugScanner,
                                            false /*configuration_in.debugParser*/);

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

  return (announceURL_in.find (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY),
                               ++position) == position);
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

