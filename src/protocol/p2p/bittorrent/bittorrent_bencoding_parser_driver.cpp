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

#include "bittorrent_bencoding_parser_driver.h"

#include "ace/Log_Msg.h"

#include "common_parser_bencoding_tools.h"

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"
#include "bittorrent_message.h"

BitTorrent_Bencoding_ParserDriver::BitTorrent_Bencoding_ParserDriver (bool traceScanning_in,
                                                                      bool traceParsing_in)
 : inherited (traceScanning_in,
              traceParsing_in)
 , bencoding_ (NULL)
 , isFirst_ (true)
 , current_ ()
 , key_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::BitTorrent_Bencoding_ParserDriver_T"));

}

void
BitTorrent_Bencoding_ParserDriver::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::dump_state"));

  // sanity check(s)
  if (!bencoding_)
    return;

  std::string bencoding_string;
  switch (bencoding_->type)
  {
    case Bencoding_Element::BENCODING_TYPE_INTEGER:
    {
      std::ostringstream converter;
      converter << bencoding_->integer;
      bencoding_string = converter.str ();
      break;
    }
    case Bencoding_Element::BENCODING_TYPE_STRING:
      bencoding_string = *bencoding_->string;
      break;
    case Bencoding_Element::BENCODING_TYPE_LIST:
      bencoding_string =
          Common_Parser_Bencoding_Tools::ListToString (*bencoding_->list);
      break;
    case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      bencoding_string =
          Common_Parser_Bencoding_Tools::DictionaryToString (*bencoding_->dictionary);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid element type (was: %d), continuing\n"),
                  bencoding_->type));
      break;
    }
  } // end SWITCH
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (bencoding_string.c_str ())));
}

void
BitTorrent_Bencoding_ParserDriver::error (const struct YYLTYPE& location_in,
                                          const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::error"));

  //std::ostringstream converter;
  //converter << location_in;

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
              location_in.first_line, location_in.first_column,
              location_in.last_line, location_in.last_column,
              ACE_TEXT (message_in.c_str ())));
//  ACE_DEBUG ((LM_ERROR,
////              ACE_TEXT ("failed to parse \"%s\" (@%s): \"%s\"\n"),
//              ACE_TEXT ("failed to BitTorrent_Parser::parse(): \"%s\"\n"),
////              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
////              converter.str ().c_str (),
//              message_in.c_str ()));

  // dump message
  ACE_Message_Block* message_block_p = inherited::fragment_;
  while (message_block_p->prev ())
    message_block_p = message_block_p->prev ();
  ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (message_block_p);
  if (idump_state_p)
  {
    try {
      idump_state_p->dump_state ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
    }
  } // end IF

  //std::clog << location_in << ": " << message_in << std::endl;
}

void
//BitTorrent_Bencoding_ParserDriver::error (const YYLTYPE& location_in,
BitTorrent_Bencoding_ParserDriver::error (const yy::location& location_in,
                                          const std::string& message_in)
//BitTorrent_Bencoding_ParserDriver::error (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::error"));

  std::ostringstream converter;
  converter << location_in;

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
              location_in.begin.line, location_in.begin.column,
              location_in.end.line, location_in.end.column,
              ACE_TEXT (message_in.c_str ())));
//  ACE_DEBUG ((LM_ERROR,
////              ACE_TEXT ("failed to parse \"%s\" (@%s): \"%s\"\n"),
//              ACE_TEXT ("failed to BitTorrent_Parser::parse(): \"%s\"\n"),
////              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
////              converter.str ().c_str (),
//              message_in.c_str ()));

  // dump message
  ACE_Message_Block* message_block_p = inherited::fragment_;
  while (message_block_p->prev ())
    message_block_p = message_block_p->prev ();
  ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (message_block_p);
  if (idump_state_p)
  {
    try {
      idump_state_p->dump_state ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
    }
  } // end IF

  //std::clog << location_in << ": " << message_in << std::endl;
}

void
BitTorrent_Bencoding_ParserDriver::record (struct Bencoding_Element*& bencoding_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::record"));

  // sanity check(s)
  ACE_ASSERT (bencoding_in);
  ACE_ASSERT (bencoding_in == bencoding_);

//#if defined (_DEBUG)
//  dump_state ();
//#endif
}

void
BitTorrent_Bencoding_ParserDriver::pushInteger (ACE_INT64 integer_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::pushInteger"));

  struct Bencoding_Element* element_p = NULL;
  ACE_NEW_NORETURN (element_p,
                    struct Bencoding_Element);
  ACE_ASSERT (element_p);
  element_p->type = Bencoding_Element::BENCODING_TYPE_INTEGER;
  element_p->integer = integer_in;

  push (element_p);
}

void
BitTorrent_Bencoding_ParserDriver::pushString (std::string* string_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::pushString"));

  struct Bencoding_Element* element_p = NULL;
  ACE_NEW_NORETURN (element_p,
                    struct Bencoding_Element);
  ACE_ASSERT (element_p);
  element_p->type = Bencoding_Element::BENCODING_TYPE_STRING;
  element_p->string = string_in;

  push (element_p);
}

void
BitTorrent_Bencoding_ParserDriver::pushList (Bencoding_List_t* list_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::pushList"));

  struct Bencoding_Element* element_p = NULL;
  ACE_NEW_NORETURN (element_p,
                    struct Bencoding_Element);
  ACE_ASSERT (element_p);
  element_p->type = Bencoding_Element::BENCODING_TYPE_LIST;
  element_p->list = list_in;

  push (element_p);
}

void
BitTorrent_Bencoding_ParserDriver::pushDictionary (Bencoding_Dictionary_t* dictionary_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::pushDictionary"));

  struct Bencoding_Element* element_p = NULL;
  ACE_NEW_NORETURN (element_p,
                    struct Bencoding_Element);
  ACE_ASSERT (element_p);
  element_p->type = Bencoding_Element::BENCODING_TYPE_DICTIONARY;
  element_p->dictionary = dictionary_in;

  push (element_p);
}

void
BitTorrent_Bencoding_ParserDriver::push (struct Bencoding_Element* element_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::push"));

  if (unlikely (isFirst_))
  {
    isFirst_ = false;
    ACE_ASSERT (!bencoding_);
    bencoding_ = element_in;
  } // end IF

  if (current_.empty ())
  {
    switch (element_in->type)
    {
      case Bencoding_Element::BENCODING_TYPE_INTEGER:
      case Bencoding_Element::BENCODING_TYPE_STRING:
        break;
      case Bencoding_Element::BENCODING_TYPE_LIST:
      case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
        current_.push (element_in); break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid element type (was: %d), continuing\n"),
                    element_in->type));
        break;
      }
    } // end SWITCH
    return;
  } // end IF

  struct Bencoding_Element*& current_p = current_.top ();
  switch (current_p->type)
  {
    case Bencoding_Element::BENCODING_TYPE_LIST:
    {
      current_p->list->push_back (element_in);
      break;
    }
    case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
    { ACE_ASSERT (key_);
      current_p->dictionary->push_back (std::make_pair (key_, element_in));
      key_ = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid element type (was: %d), continuing\n"),
                  current_p->type));
      ACE_ASSERT (false);
      break;
    }
  } // end SWITCH

  switch (element_in->type)
  {
    case Bencoding_Element::BENCODING_TYPE_INTEGER:
    case Bencoding_Element::BENCODING_TYPE_STRING:
      break;
    case Bencoding_Element::BENCODING_TYPE_LIST:
    case Bencoding_Element::BENCODING_TYPE_DICTIONARY:
      current_.push (element_in); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid element type (was: %d), continuing\n"),
                  element_in->type));
      break;
    }
  } // end SWITCH
}
