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
#include "bittorrent_isession.h"
#include "bittorrent_stream_common.h"
#include "bittorrent_streamhandler.h"

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

struct BitTorrent_PeerModuleHandlerConfiguration;
struct BitTorrent_PeerStreamConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct BitTorrent_PeerStreamConfiguration,
                               struct BitTorrent_PeerModuleHandlerConfiguration> BitTorrent_PeerStreamConfiguration_t;

class BitTorrent_PeerConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<BitTorrent_PeerStreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  BitTorrent_PeerConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
  {}
};

struct BitTorrent_TrackerModuleHandlerConfiguration;
struct BitTorrent_TrackerStreamConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct BitTorrent_TrackerStreamConfiguration,
                               struct BitTorrent_TrackerModuleHandlerConfiguration> BitTorrent_TrackerStreamConfiguration_t;
class BitTorrent_TrackerConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<BitTorrent_TrackerStreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  BitTorrent_TrackerConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
   ///////////////////////////////////////
  {
    //PDUSize = BITTORRENT_BUFFER_SIZE;
  }
};

//////////////////////////////////////////

typedef std::vector<ACE_INET_Addr> BitTorrent_PeerAddresses_t;
typedef BitTorrent_PeerAddresses_t::const_iterator BitTorrent_PeerAddressesIterator_t;
struct BitTorrent_SessionInitiationThreadData
{
  BitTorrent_SessionInitiationThreadData ()
   : addresses ()
   , lock (NULL)
   , session (NULL)
  {}

  BitTorrent_PeerAddresses_t addresses;
  ACE_SYNCH_MUTEX*           lock;
  Net_IInetSession_t*        session;
};

struct BitTorrent_SessionConfiguration
 : Net_SessionConfiguration
{
  BitTorrent_SessionConfiguration ()
   : Net_SessionConfiguration ()
   , allowMultipleConnectionsPerPeer (BITTORRENT_DEFAULT_ALLOW_MULTIPLE_CONNECTIONS_PER_PEER)
   , externalIPAddress (static_cast<u_short> (0),
                        static_cast<ACE_UINT32> (0))
   , metaInfo (NULL)
   , metaInfoFileName ()
   , parserConfiguration (NULL)
   , requestCompactPeerAddresses (BITTORRENT_DEFAULT_REQUEST_COMPACT_PEER_ADDRESSES)
   , sendBitfieldAfterHandshake (BITTORRENT_DEFAULT_SEND_BITFIELD_AFTER_PEER_HANDSHAKE)
   , subscriber (NULL)
   , peerModuleHandlerConfiguration (NULL)
   , peerStreamConfiguration (NULL)
   , trackerConnectionConfiguration (NULL)
   , trackerModuleHandlerConfiguration (NULL)
   , trackerStreamConfiguration (NULL)
  {}

  bool                                      allowMultipleConnectionsPerPeer;
  ACE_INET_Addr                             externalIPAddress;
  Bencoding_Dictionary_t*                   metaInfo;
  std::string                               metaInfoFileName;
  struct Common_ParserConfiguration*        parserConfiguration; // tracker stream handler
  bool                                      requestCompactPeerAddresses;
  bool                                      sendBitfieldAfterHandshake;
  BitTorrent_ISessionProgress*              subscriber; // session --> UI events

  struct Stream_ModuleHandlerConfiguration* peerModuleHandlerConfiguration;
  struct Stream_Configuration*              peerStreamConfiguration;
  struct Net_ConnectionConfigurationBase*   trackerConnectionConfiguration;
  struct Stream_ModuleHandlerConfiguration* trackerModuleHandlerConfiguration;
  struct Stream_Configuration*              trackerStreamConfiguration;
};

// *NOTE*: see also: https://wiki.theory.org/BitTorrentSpecification#Overview
struct BitTorrent_PeerStatus
{
  BitTorrent_PeerStatus ()
   : am_choking (true)
   , am_interested (false)
   , peer_choking (true)
   , peer_interested (false)
   //////////////////////////////////////
   , requesting_piece (0)
  {}

  // protocol-specific
  bool am_choking;
  bool am_interested;
  bool peer_choking;
  bool peer_interested;

  // other
  int requesting_piece;
};

struct BitTorrent_PeerState
{
  BitTorrent_PeerState ()
   : connections ()
   , pieces ()
   , status ()
  {}

  std::vector<Net_ConnectionId_t>    connections;
  BitTorrent_MessagePayload_Bitfield pieces;
  struct BitTorrent_PeerStatus       status;
};

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 BitTorrent_PeerConnectionConfiguration,
                                 struct BitTorrent_PeerConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> BitTorrent_PeerConnection_Manager_t;

typedef BitTorrent_PeerStream_T<struct BitTorrent_PeerStreamState,
                                struct BitTorrent_PeerStreamConfiguration,
                                struct Stream_Statistic,
                                Common_Timer_Manager_t,
                                struct BitTorrent_PeerModuleHandlerConfiguration,
                                struct BitTorrent_PeerSessionData,
                                BitTorrent_PeerSessionData_t,
                                Stream_ControlMessage_t,
                                BitTorrent_Message_t,
                                BitTorrent_PeerSessionMessage_t,
                                BitTorrent_PeerConnectionConfiguration,
                                struct BitTorrent_PeerConnectionState,
                                Net_TCPSocketConfiguration_t,
                                struct BitTorrent_SessionState,
                                BitTorrent_PeerConnection_Manager_t,
                                struct Stream_UserData> BitTorrent_PeerStream_t;

typedef BitTorrent_ISession_T<ACE_INET_Addr,
                              BitTorrent_PeerConnectionConfiguration,
                              BitTorrent_TrackerConnectionConfiguration,
                              struct BitTorrent_PeerConnectionState,
                              Net_StreamStatistic_t,
                              BitTorrent_PeerStream_t,
                              enum Stream_StateMachine_ControlState,
                              struct BitTorrent_SessionConfiguration,
                              struct BitTorrent_SessionState> BitTorrent_ISession_t;

//----------------------------------------

typedef BitTorrent_PeerStreamHandler_T<struct BitTorrent_PeerSessionData,
                                       struct Stream_UserData,
                                       BitTorrent_ISession_t,
                                       void> BitTorrent_PeerStreamHandler_t;
typedef BitTorrent_TrackerStreamHandler_T<struct BitTorrent_TrackerSessionData,
                                          struct Stream_UserData,
                                          BitTorrent_ISession_t,
                                          void> BitTorrent_TrackerStreamHandler_t;

typedef std::map<ACE_UINT32, struct BitTorrent_PeerState> BitTorrent_PeerState_t;
typedef BitTorrent_PeerState_t::iterator BitTorrent_PeerStateIterator_t;

struct BitTorrent_SessionState
{
  BitTorrent_SessionState ()
   : aborted (false)
   , connections ()
   , fileName ()
   , metaInfo (NULL)
   , pieces ()
   ///////////////////////////////////////
   , peerConnectionConfiguration (NULL)
   , peerId ()
   , peerModuleHandlerConfiguration (NULL)
   , peerState ()
   , peerStreamConfiguration (NULL)
   , peerStreamHandler (NULL)
   ///////////////////////////////////////
   , key ()
   , trackerAddress ()
   , trackerBaseURI ()
   , trackerConnectionConfiguration (NULL)
   , trackerConnectionId (0)
   , trackerId ()
   , trackerModuleHandlerConfiguration (NULL)
   , trackerRequestResponse (NULL)
   , trackerScrapeResponse (NULL)
   , trackerStreamConfiguration (NULL)
   , trackerStreamHandler (NULL)
  {}

  bool                                      aborted;
  Net_ConnectionIds_t                       connections;
  std::string                               fileName; // .torrent file
  Bencoding_Dictionary_t*                   metaInfo;
  BitTorrent_Pieces_t                       pieces;

  struct Net_ConnectionConfigurationBase*   peerConnectionConfiguration;
  std::string                               peerId;
  struct Stream_ModuleHandlerConfiguration* peerModuleHandlerConfiguration;
  BitTorrent_PeerState_t                    peerState;
  struct Stream_Configuration*              peerStreamConfiguration;
  BitTorrent_PeerStreamHandler_t*           peerStreamHandler;

  std::string                               key; // tracker-
  ACE_INET_Addr                             trackerAddress; // *NOTE*: might be redirected
  std::string                               trackerBaseURI;
  struct Net_ConnectionConfigurationBase*   trackerConnectionConfiguration;
  Net_ConnectionId_t                        trackerConnectionId;
  std::string                               trackerId;
  struct Stream_ModuleHandlerConfiguration* trackerModuleHandlerConfiguration;
  Bencoding_Dictionary_t*                   trackerRequestResponse;
  Bencoding_Dictionary_t*                   trackerScrapeResponse;
  struct Stream_Configuration*              trackerStreamConfiguration;
  BitTorrent_TrackerStreamHandler_t*        trackerStreamHandler;
};

#endif
