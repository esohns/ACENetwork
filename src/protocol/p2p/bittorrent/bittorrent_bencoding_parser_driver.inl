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

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"
#include "bittorrent_message.h"
#include "bittorrent_tools.h"

template <typename SessionMessageType>
BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::BitTorrent_Bencoding_ParserDriver_T (bool traceScanning_in,
                                                                                              bool traceParsing_in)
 : inherited (traceScanning_in,
              traceParsing_in)
 , bencoding_ (NULL)
 , dictionaries_ ()
 , lists_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::BitTorrent_Bencoding_ParserDriver_T"));

}

template <typename SessionMessageType>
BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::~BitTorrent_Bencoding_ParserDriver_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::~BitTorrent_Bencoding_ParserDriver_T"));

}

template <typename SessionMessageType>
void
//BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::error (const YYLTYPE& location_in,
BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::error (const yy::location& location_in,
                                                                const std::string& message_in)
//BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::error (const std::string& message_in)
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
  while (message_block_p->prev ()) message_block_p = message_block_p->prev ();
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
//template <typename SessionMessageType>
//void
//BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::error (const std::string& message_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::error"));

//  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("\": \"%s\"...\n"),
//              ACE_TEXT (message_in.c_str ())));
////   ACE_DEBUG((LM_ERROR,
////              ACE_TEXT("failed to parse \"%s\": \"%s\"...\n"),
////              std::string(fragment_->rd_ptr(), fragment_->length()).c_str(),
////              message_in.c_str()));

////   std::clog << message_in << std::endl;
//}

template <typename SessionMessageType>
void
BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::record (Bencoding_Dictionary_t*& bencoding_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::record"));

  // sanity check(s)
  ACE_ASSERT (bencoding_in);
  ACE_ASSERT (bencoding_in == bencoding_);

//#if defined (_DEBUG)
//  dump_state ();
//#endif
}

template <typename SessionMessageType>
void
//BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::set (Bencoding_Dictionary_t* dictionary_in)
BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::pushDictionary (Bencoding_Dictionary_t* dictionary_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::pushDictionary"));

  dictionaries_.push (dictionary_in);

  if (!bencoding_)
    bencoding_ = dictionary_in;
}

template <typename SessionMessageType>
void
BitTorrent_Bencoding_ParserDriver_T<SessionMessageType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Bencoding_ParserDriver_T::dump_state"));

  // sanity check(s)
  ACE_ASSERT (bencoding_);

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (BitTorrent_Tools::MetaInfoToString (*bencoding_).c_str ())));
}
