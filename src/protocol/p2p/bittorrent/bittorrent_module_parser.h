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

#ifndef BitTorrent_Module_Parser_T_H
#define BitTorrent_Module_Parser_T_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_misc_parser.h"

#include "bittorrent_defines.h"
#include "bittorrent_bencoding_parser_driver.h"
#include "bittorrent_parser_driver.h"

// forward declaration(s)
class ACE_Message_Block;

extern const char libacenetwork_protocol_bittorrent_default_peer_parser_module_name_string[];
extern const char libacenetwork_protocol_bittorrent_default_tracker_parser_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename UserDataType>
class BitTorrent_Module_PeerParser_T
 : public Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 BitTorrent_ParserDriver,
                                 UserDataType>
{
  typedef Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 BitTorrent_ParserDriver,
                                 UserDataType> inherited;

 public:
  BitTorrent_Module_PeerParser_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~BitTorrent_Module_PeerParser_T () {}

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParser_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParser_T (const BitTorrent_Module_PeerParser_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParser_T& operator= (const BitTorrent_Module_PeerParser_T&))

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // implement (part of) BitTorrent_IParser_T
  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (struct BitTorrent_PeerRecord*&); // data record
  virtual void handshake (struct BitTorrent_PeerHandShake*&); // handshake

  struct Common_FlexBisonParserConfiguration parserConfiguration_;
};

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename UserDataType>
class BitTorrent_Module_TrackerParser_T
 : public Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 BitTorrent_Bencoding_ParserDriver,
                                 UserDataType>
{
  typedef Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 BitTorrent_Bencoding_ParserDriver,
                                 UserDataType> inherited;

 public:
  BitTorrent_Module_TrackerParser_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~BitTorrent_Module_TrackerParser_T () {}

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerParser_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerParser_T (const BitTorrent_Module_TrackerParser_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerParser_T& operator= (const BitTorrent_Module_TrackerParser_T&))

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // implement (part of) BitTorrent_IParser_T
  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (Bencoding_Dictionary_t*&); // data record
};

// include template definition
#include "bittorrent_module_parser.inl"

#endif
