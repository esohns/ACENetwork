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

#include <openssl/sha.h>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_file_tools.h"

#include "net_macros.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "ace/Synch.h"
#include "bittorrent_bencoding_scanner.h"
#include "bittorrent_bencoding_parser_driver.h"
#include "bittorrent_defines.h"
#include "bittorrent_stream_common.h"

std::string
BitTorrent_Tools::HandShake2String (const struct BitTorrent_PeerHandShake& peerHandshake_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::HandShake2String"));

  std::string result;
  std::ostringstream converter;

  return result;
}
std::string
BitTorrent_Tools::Record2String (const struct BitTorrent_PeerRecord& peerRecord_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::Record2String"));

  std::string result;
  std::ostringstream converter;

  return result;
}

std::string
BitTorrent_Tools::Type2String (enum BitTorrent_MessageType& type_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::Type2String"));

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
                  ACE_TEXT ("invalid/unknown method (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
BitTorrent_Tools::MetaInfo2InfoHash (const Bencoding_Dictionary_t& metaInfo_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::MetaInfo2InfoHash"));

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
      BitTorrent_Tools::bencode (*(*iterator).second->dictionary);
  unsigned char info_hash[SHA_DIGEST_LENGTH + 1];
  SHA1 (reinterpret_cast<const unsigned char*> (bencoded_string.c_str ()),
        bencoded_string.size (),
        info_hash);
  info_hash[SHA_DIGEST_LENGTH] = '\0';
  result += reinterpret_cast<char*> (info_hash);

  return result;
}

unsigned int
BitTorrent_Tools::MetaInfo2Length (const Bencoding_Dictionary_t& metaInfo_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::MetaInfo2Length"));

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
    BitTorrent_Tools::free (metaInfo_out);

  // sanity check(s)
  ACE_ASSERT (!metaInfo_out);

  bool block_in_parse = configuration_in.block;
  BitTorrent_Bencoding_ParserDriver_T<BitTorrent_TrackerSessionMessage_t> parser (configuration_in.debugScanner,
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
  ACE_ASSERT (Common_File_Tools::isReadable (metaInfoFileName_in));

  unsigned char* data_p = NULL;
  if (!Common_File_Tools::load (metaInfoFileName_in,
                                data_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p);
  unsigned int size = Common_File_Tools::size (metaInfoFileName_in);
  ACE_Message_Block message_block (reinterpret_cast<char*> (data_p),
                                   size,
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
  delete [] data_p;

  return result;
}

void
BitTorrent_Tools::free (Bencoding_Dictionary_t*& dictionary_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::free"));

  if (!dictionary_inout)
    return;

  for (Bencoding_DictionaryIterator_t iterator = dictionary_inout->begin ();
       iterator != dictionary_inout->end ();
       ++iterator)
  {
    switch ((*iterator).second->type)
    {
      case Bencoding_Element::BENCODING_TYPE_INTEGER:
        break;
      case Bencoding_Element::BENCODING_TYPE_STRING:
        delete (*iterator).second->string; break;
      case Bencoding_Element::BENCODING_TYPE_LIST:
      {
        BitTorrent_Tools::free ((*iterator).second->list);
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      {
        BitTorrent_Tools::free ((*iterator).second->dictionary);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), continuing\n"),
                    ACE_TEXT ((*iterator).second->type)));
        break;
      }
    } // end SWITCH
  } // end FOR

  delete dictionary_inout;
  dictionary_inout = NULL;
}

std::string
BitTorrent_Tools::generatePeerId ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::generatePeerId"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("-");
  result += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PEER_ID_CLIENT_ID);

  std::string library_version = ACE_TEXT_ALWAYS_CHAR ("0000");
#ifdef HAVE_CONFIG_H
  // parse library version
  library_version =
      ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_VERSION);
  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+).([[:digit:]]+).([[:digit:]]+)(.+)?$");
  std::regex regex (regex_string);
  std::smatch match_results;
  if (!std::regex_match (library_version,
                         match_results,
                         regex,
                         std::regex_constants::match_default))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid version string (was: \"%s\"), aborting\n"),
                ACE_TEXT (library_version.c_str ())));
    return std::string ();
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());

  library_version = match_results[1];
  if (library_version.size () < 4)
    library_version += match_results[2];
  if (library_version.size () < 4)
    library_version += match_results[3];
  if (library_version.size () < 4)
    library_version += ACE_TEXT_ALWAYS_CHAR ("0");
  library_version.resize (4);
#endif
  result += library_version;
  result += ACE_TEXT_ALWAYS_CHAR ("-");

  // generate 14 bytes of random data
  time_t now = COMMON_TIME_NOW.sec ();
  std::ostringstream converter;
  converter << now;
  std::string random_string = converter.str ();
  unsigned char random_hash[SHA_DIGEST_LENGTH + 1];
  SHA1 (reinterpret_cast<const unsigned char*> (random_string.c_str ()),
        random_string.size (),
        random_hash);
  random_hash[SHA_DIGEST_LENGTH] = '\0';
  result += reinterpret_cast<char*> (random_hash);
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
  time_t now = COMMON_TIME_NOW.sec ();
  std::ostringstream converter;
  converter << now;
  std::string random_string = converter.str ();
  unsigned char random_hash[SHA_DIGEST_LENGTH + 1];
  SHA1 (reinterpret_cast<const unsigned char*> (random_string.c_str ()),
        random_string.size (),
        random_hash);
  random_hash[SHA_DIGEST_LENGTH] = '\0';
  result += reinterpret_cast<char*> (random_hash);
  result.resize (20);

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
BitTorrent_Tools::AnnounceURL2ScrapeURL (const std::string& announceURL_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::AnnounceURL2ScrapeURL"));

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

std::string
BitTorrent_Tools::Dictionary2String (const Bencoding_Dictionary_t& dictionary_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::Dictionary2String"));

  std::string result (ACE_TEXT_ALWAYS_CHAR ("dictionary:\n"));

  for (Bencoding_DictionaryIterator_t iterator = dictionary_in.begin ();
       iterator != dictionary_in.end ();
       ++iterator)
  {
    result += ACE_TEXT_ALWAYS_CHAR ("\"");
    result += *(*iterator).first;
    result += ACE_TEXT_ALWAYS_CHAR ("\": ");

    switch ((*iterator).second->type)
    {
      case Bencoding_Element::BENCODING_TYPE_INTEGER:
      {
        std::ostringstream converter;
        converter << (*iterator).second->integer;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR ("\n");
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_STRING:
      {
        result += ACE_TEXT_ALWAYS_CHAR ("\"");
        result += *(*iterator).second->string;
        result += ACE_TEXT_ALWAYS_CHAR ("\"\n");
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_LIST:
      {
        result += BitTorrent_Tools::List2String (*(*iterator).second->list);
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      {
        result += BitTorrent_Tools::Dictionary2String (*(*iterator).second->dictionary);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), continuing\n"),
                    ACE_TEXT ((*iterator).second->type)));
        break;
      }
    } // end SWITCH
  } // end FOR

  return result;
}
std::string
BitTorrent_Tools::List2String (const Bencoding_List_t& list_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::List2String"));

  std::string result (ACE_TEXT_ALWAYS_CHAR ("list:\n"));

  for (Bencoding_ListIterator_t iterator = list_in.begin ();
       iterator != list_in.end ();
       ++iterator)
  {
    switch ((*iterator)->type)
    {
      case Bencoding_Element::BENCODING_TYPE_INTEGER:
      {
        std::ostringstream converter;
        converter << (*iterator)->integer;
        result += converter.str ();
        result += ACE_TEXT_ALWAYS_CHAR ("\n");
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_STRING:
      {
        result += ACE_TEXT_ALWAYS_CHAR ("\"");
        result += *(*iterator)->string;
        result += ACE_TEXT_ALWAYS_CHAR ("\"\n");
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_LIST:
      {
        result += BitTorrent_Tools::List2String (*(*iterator)->list);
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      {
        result += BitTorrent_Tools::Dictionary2String (*(*iterator)->dictionary);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), continuing\n"),
                    ACE_TEXT ((*iterator)->type)));
        break;
      }
    } // end SWITCH
  } // end FOR

  return result;
}

std::string
BitTorrent_Tools::MetaInfo2String (const Bencoding_Dictionary_t& metaInfo_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::MetaInfo2String"));

  return BitTorrent_Tools::Dictionary2String (metaInfo_in);
}

std::string
BitTorrent_Tools::bencode (const Bencoding_Dictionary_t& dictionary_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::bencode"));

  // initialize return value
  std::string result = ACE_TEXT_ALWAYS_CHAR ("d");

  std::ostringstream converter;

  for (Bencoding_DictionaryIterator_t iterator = dictionary_in.begin ();
       iterator != dictionary_in.end ();
       ++iterator)
  {
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << (*iterator).first->size ();
    result += converter.str ();
    result += ':';
    result += *(*iterator).first;

    switch ((*iterator).second->type)
    {
      case Bencoding_Element::BENCODING_TYPE_INTEGER:
      {
        result += 'i';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).second->integer;
        result += converter.str ();
        result += 'e';
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_STRING:
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator).second->string->size ();
        result += converter.str ();
        result += ':';
        result += *(*iterator).second->string;
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_LIST:
      {
        result += BitTorrent_Tools::bencode (*(*iterator).second->list);
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      {
        result += BitTorrent_Tools::bencode (*(*iterator).second->dictionary);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), continuing\n"),
                    ACE_TEXT ((*iterator).second->type)));
        break;
      }
    } // end SWITCH
  } // end FOR
  result += 'e';

  return result;
}
std::string
BitTorrent_Tools::bencode (const Bencoding_List_t& list_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::bencode"));

  // initialize return value
  std::string result = ACE_TEXT_ALWAYS_CHAR ("l");

  std::ostringstream converter;

  for (Bencoding_ListIterator_t iterator = list_in.begin ();
       iterator != list_in.end ();
       ++iterator)
  {
    switch ((*iterator)->type)
    {
      case Bencoding_Element::BENCODING_TYPE_INTEGER:
      {
        result += 'i';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator)->integer;
        result += converter.str ();
        result += 'e';
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_STRING:
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << (*iterator)->string->size ();
        result += converter.str ();
        result += ':';
        result += *(*iterator)->string;
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_LIST:
      {
        result += BitTorrent_Tools::bencode (*(*iterator)->list);
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      {
        result += BitTorrent_Tools::bencode (*(*iterator)->dictionary);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), continuing\n"),
                    ACE_TEXT ((*iterator)->type)));
        break;
      }
    } // end SWITCH
  } // end FOR
  result += 'e';

  return result;
}

void
BitTorrent_Tools::free (Bencoding_List_t*& list_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::free"));

  if (!list_inout)
    return;

  for (Bencoding_ListIterator_t iterator = list_inout->begin ();
       iterator != list_inout->end ();
       ++iterator)
  {
    switch ((*iterator)->type)
    {
      case Bencoding_Element::BENCODING_TYPE_INTEGER:
        break;
      case Bencoding_Element::BENCODING_TYPE_STRING:
        delete (*iterator)->string; break;
      case Bencoding_Element::BENCODING_TYPE_LIST:
      {
        BitTorrent_Tools::free ((*iterator)->list);
        break;
      }
      case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      {
        BitTorrent_Tools::free ((*iterator)->dictionary);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), continuing\n"),
                    ACE_TEXT ((*iterator)->type)));
        break;
      }
    } // end SWITCH
  } // end FOR

  delete list_inout;
  list_inout = NULL;
}
