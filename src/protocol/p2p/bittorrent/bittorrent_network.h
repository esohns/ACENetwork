/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef BITTORRENT_NETWORK_H
#define BITTORRENT_NETWORK_H

#include <string>
#include <vector>

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpsockethandler.h"
#include "net_tcpconnection_base.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "bittorrent_common.h"
#include "bittorrent_stream_common.h"

struct BitTorrent_SessionConfiguration;
struct BitTorrent_SessionState;
struct BitTorrent_PeerConnectionState
 : Net_StreamConnectionState
{
  BitTorrent_PeerConnectionState ()
   : Net_StreamConnectionState ()
   , configuration (NULL)
   //, connection (NULL)
   , handshake (NULL)
   //, session (NULL)
  {}

  struct BitTorrent_Configuration* configuration;
  //BitTorrent_IPeerConnection_t*    connection;
  struct BitTorrent_PeerHandshake* handshake;
  //BitTorrent_ISession_t*           session;
};
struct BitTorrent_TrackerConnectionState
 : Net_StreamConnectionState
{
  BitTorrent_TrackerConnectionState ()
   : Net_StreamConnectionState ()
   , configuration (NULL)
   //, connection (NULL)
   //, session (NULL)
  {}

  struct BitTorrent_Configuration* configuration;
  //BitTorrent_ITrackerConnection_t* connection;
  //BitTorrent_ISession_t*           session;
};

//struct BitTorrent_AllocatorConfiguration;
struct BitTorrent_PeerStreamConfiguration;
class BitTorrent_PeerConnectionConfiguration
 : public Net_ConnectionConfiguration_T<struct Net_AllocatorConfiguration,
                                        struct BitTorrent_PeerStreamConfiguration,
                                        NET_TRANSPORTLAYER_TCP>
{
 public:
  BitTorrent_PeerConnectionConfiguration ()
   : Net_ConnectionConfiguration_T ()
  {
    //PDUSize = BITTORRENT_BUFFER_SIZE;
  }
};

struct BitTorrent_TrackerStreamConfiguration;
class BitTorrent_TrackerConnectionConfiguration
 : public Net_ConnectionConfiguration_T<struct Net_AllocatorConfiguration,
                                        struct BitTorrent_TrackerStreamConfiguration,
                                        NET_TRANSPORTLAYER_TCP>
{
 public:
  BitTorrent_TrackerConnectionConfiguration ()
   : Net_ConnectionConfiguration_T ()
   ///////////////////////////////////////
  {
    //PDUSize = BITTORRENT_BUFFER_SIZE;
  }
};

typedef std::vector<ACE_INET_Addr> BitTorrent_PeerAddresses_t;
typedef BitTorrent_PeerAddresses_t::const_iterator BitTorrent_PeerAddressesIterator_t;
struct BitTorrent_SessionInitiationThreadData
{
  BitTorrent_SessionInitiationThreadData ()
   : addresses (NULL)
   , lock (NULL)
   , session (NULL)
  {};

  BitTorrent_PeerAddresses_t* addresses;
  ACE_SYNCH_MUTEX*            lock;
  Net_IInetSession_t*         session;
};

template <typename SessionInterfaceType>
class BitTorrent_IControl_T;
//typedef BitTorrent_IControl_T<BitTorrent_ISession_t> BitTorrent_IControl_t;
struct BitTorrent_SessionConfiguration
 : Net_SessionConfiguration
{
  BitTorrent_SessionConfiguration ()
   : Net_SessionConfiguration ()
   , metaInfo (NULL)
   , metaInfoFileName ()
  {};

  Bencoding_Dictionary_t* metaInfo;
  std::string             metaInfoFileName;
};

// *NOTE*: see also: https://wiki.theory.org/BitTorrentSpecification#Overview
struct BitTorrent_PeerStatus
{
  BitTorrent_PeerStatus ()
   : am_choking (true)
   , am_interested (false)
   , peer_choking (true)
   , peer_interested (false)
  {};

  bool am_choking;
  bool am_interested;
  bool peer_choking;
  bool peer_interested;
};
typedef std::map<Net_ConnectionId_t, struct BitTorrent_PeerStatus> BitTorrent_PeerStatus_t;
typedef BitTorrent_PeerStatus_t::iterator BitTorrent_PeerStatusIterator_t;
struct BitTorrent_SessionState
{
  BitTorrent_SessionState ()
   : connections ()
   //, controller (NULL)
   , fileName ()
   , key ()
   , metaInfo (NULL)
   , peerId ()
   , peerStatus ()
   , trackerConnectionId (0)
   , trackerId ()
   , trackerRequestResponse (NULL)
   , trackerScrapeResponse (NULL)
  {};

  Net_ConnectionIds_t     connections;
  //BitTorrent_IControl_t*  controller;
  std::string             fileName; // .torrent file
  std::string             key; // tracker-
  Bencoding_Dictionary_t* metaInfo;
  std::string             peerId;
  BitTorrent_PeerStatus_t peerStatus;
  Net_ConnectionId_t      trackerConnectionId;
  std::string             trackerId;
  Bencoding_Dictionary_t* trackerRequestResponse;
  Bencoding_Dictionary_t* trackerScrapeResponse;
};

#endif
