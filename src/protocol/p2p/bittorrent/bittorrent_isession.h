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

#ifndef BITTORRENT_ISESSION_T_H
#define BITTORRENT_ISESSION_T_H

#include "net_common.h"
#include "net_iconnection.h"

#include "common_parser_bencoding_common.h"

#include "bittorrent_common.h"

// forward declarations
class ACE_Message_Block;

template <typename AddressType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename SocketConfigurationType,
          typename PeerHandlerConfigurationType, // peer socket-
          typename TrackerHandlerConfigurationType, // tracker socket-
          ////////////////////////////////
          typename PeerStreamType,
          typename StreamStatusType,
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType>
class BitTorrent_ISession_T
 : public Net_ISession_T<AddressType,
                         PeerConnectionConfigurationType,
                         PeerConnectionStateType,
                         StatisticContainerType,
                         SocketConfigurationType,
                         PeerHandlerConfigurationType,
                         PeerStreamType,
                         StreamStatusType,
                         ConfigurationType,
                         StateType>
{
 public:
  inline virtual ~BitTorrent_ISession_T () {}

  virtual void interested (Net_ConnectionId_t) = 0; // connection id
  virtual void request (Net_ConnectionId_t, // connection id
                        unsigned int,       // index (piece#)
                        unsigned int,       // begin (offset)
                        unsigned int) = 0;  // length (bytes)
  virtual void scrape () = 0;

  virtual void trackerConnect (const AddressType&) = 0; // tracker address
  virtual void trackerDisconnect (const AddressType&) = 0; // tracker address
  virtual AddressType trackerAddress () = 0; // return value: tracker address
  virtual Net_ConnectionId_t trackerConnectionId () = 0; // return value: tracker connection id

  ////////////////////////////////////////
  // callbacks
  // *TODO*: make these 'private'
  virtual void trackerConnect (Net_ConnectionId_t) = 0;    // connection id
  virtual void trackerDisconnect (Net_ConnectionId_t) = 0; // connection id
  virtual void trackerRedirect (Net_ConnectionId_t,      // connection id
                                const std::string&) = 0; // HTTP response "Location" header

  //--------------------------------------

  virtual void notify (const Bencoding_Dictionary_t&) = 0; // tracker (scrape-/)response record
  virtual void notify (Net_ConnectionId_t,                          // connection id
                       const struct BitTorrent_PeerHandShake&) = 0; // peer handshake record
  virtual void notify (Net_ConnectionId_t,                  // connection id
                       const struct BitTorrent_PeerRecord&, // message record
                       ACE_Message_Block* = NULL) = 0;      // data piece (if applicable)
};

#endif
