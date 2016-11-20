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

#include <sstream>

#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>

#include "common_file_tools.h"

#include "net_macros.h"

#include "bittorrent_defines.h"
#include <ace/Synch.h>
#include "bittorrent_metainfo_parser_driver.h"

std::string
BitTorrent_Tools::Handshake2String (const struct BitTorrent_PeerHandshake& peerHandshake_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::Handshake2String"));

  std::string result;
  std::ostringstream converter;

  return result;
}
std::string
BitTorrent_Tools::Record2String (const struct BitTorrent_Record& record_in)
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

//bool
//BitTorrent_Tools::parseURL (const std::string& URL_in,
//                      ACE_INET_Addr& address_out,
//                      std::string& URI_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::parseURL"));

//  bool use_SSL = false;
//  std::string hostname;
//  unsigned short port = BitTorrent_DEFAULT_SERVER_PORT;
//  std::istringstream converter;
//  //std::string dotted_decimal_string;
//  int result = -1;

//  // step1: split protocol/hostname/port
//  std::string regex_string =
//    ACE_TEXT_ALWAYS_CHAR ("^(?:http(s)?://)?([[:alnum:]-.]+)(?:\\:([[:digit:]]{1,5}))?(.+)?$");
//  std::regex regex (regex_string);
//  std::smatch match_results;
//  if (!std::regex_match (URL_in,
//                         match_results,
//                         regex,
//                         std::regex_constants::match_default))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid URL string (was: \"%s\"), aborting\n"),
//                ACE_TEXT (URL_in.c_str ())));
//    return false;
//  } // end IF
//  ACE_ASSERT (match_results.ready () && !match_results.empty ());

//  if (match_results[1].matched)
//    use_SSL = true;
//  ACE_UNUSED_ARG (use_SSL);
//  ACE_ASSERT (match_results[2].matched);
//  hostname = match_results[2];
//  if (match_results[3].matched)
//  {
//    converter.clear ();
//    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//    converter.str (match_results[3].str ());
//    converter >> port;
//  } // end IF
//  ACE_ASSERT (match_results[4].matched);
//  URI_out = match_results[4];

//  // step2: validate address/verify host name exists
//  //        --> resolve
//  // *TODO*: support IPv6 as well
//  //regex_string =
//  //  ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{1,3}\\.){4}$");
//  //regex = regex_string;
//  //std::smatch match_results_2;
//  //if (std::regex_match (hostname,
//  //                      match_results_2,
//  //                      regex,
//  //                      std::regex_constants::match_default))
//  //{
//  //  ACE_ASSERT (match_results_2.ready () &&
//  //              !match_results_2.empty () &&
//  //              match_results_2[1].matched);
//  //  dotted_decimal_string = hostname;
//  //} // end IF
//  result = address_out.set (port,
//                            hostname.c_str (),
//                            1,
//                            ACE_ADDRESS_FAMILY_INET);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::set (): \"%m\", aborting\n")));
//    return false;
//  } // end IF

//  // step3: validate URI
//  regex_string =
//    ACE_TEXT_ALWAYS_CHAR ("^(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
//  //regex_string =
//  //    ACE_TEXT_ALWAYS_CHAR ("^(?:http(?:s)?://)?((.+\\.)+([^\\/]+))(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
//  regex.assign (regex_string,
//                (std::regex_constants::ECMAScript |
//                 std::regex_constants::icase));
//  std::smatch match_results_3;
//  if (!std::regex_match (URI_out,
//                         match_results_3,
//                         regex,
//                         std::regex_constants::match_default))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid URI (was: \"%s\"), aborting\n"),
//                ACE_TEXT (URI_out.c_str ())));
//    return false;
//  } // end IF
//  ACE_ASSERT (match_results_3.ready () && !match_results_3.empty ());

//  return true;
//}

bool
BitTorrent_Tools::parseMetaInfoFile (const std::string& metaInfoFileName_in,
                                     Bencoding_Dictionary_t*& metaInfo_out,
                                     bool traceScanning_in,
                                     bool traceParsing_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::parseMetaInfoFile"));

  // initialize return value(s)
  if (metaInfo_out)
    BitTorrent_Tools::free (metaInfo_out);

  // sanity check(s)
  ACE_ASSERT (!metaInfo_out);

  BitTorrent_MetaInfo_ParserDriver_T<BitTorrent_SessionMessage_t> parser (traceScanning_in,
                                                                          traceParsing_in);
  parser.initialize (traceScanning_in, // trace flex scanner ?
                     traceParsing_in,  // trace bison parser ?
                     NULL,             // data buffer queue (yywrap)
                     true);            // block in parse ?

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

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("parsed meta-info file \"%s\"...\n"),
              ACE_TEXT (metaInfoFileName_in.c_str ())));

  metaInfo_out = parser.metaInfo_;

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
BitTorrent_Tools::Dictionary2String (const Bencoding_Dictionary_t& dictionary_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Tools::Dictionary2String"));

  std::string result (ACE_TEXT_ALWAYS_CHAR ("dictionary:\n"));

  for (Bencoding_DictionaryIterator_t iterator = dictionary_in.begin ();
       iterator != dictionary_in.end ();
       ++iterator)
  {
    result += ACE_TEXT_ALWAYS_CHAR ("\"");
    result += (*iterator).first;
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
