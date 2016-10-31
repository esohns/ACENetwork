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

#ifndef BITTORRENT_SESSION_H
#define BITTORRENT_SESSION_H

#include <ace/Asynch_Connector.h>
#include <ace/config-macros.h>
#include <ace/Connector.h>
#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>

#include "bittorrent_common.h"
#include "bittorrent_isession.h"
#include "bittorrent_stream_common.h"

// forward declarations
class ACE_Message_Block;

template <typename ConfigurationType,
          typename StateType,
          ////////////////////////////////
          typename StreamType,
          typename StreamStatusType,
          ////////////////////////////////
          typename SessionStateType,
          ////////////////////////////////
          typename PeerConnectionType,
          typename PeerConnectionManagerType,
          ////////////////////////////////
          typename TrackerConnectionType,
          typename TrackerConnectionManagerType>
class BitTorrent_Session_T
 : public BitTorrent_ISession_T<ACE_INET_Addr,
                                ConfigurationType,
                                StateType,
                                BitTorrent_RuntimeStatistic_t,
                                struct Net_SocketConfiguration,
                                struct Net_SocketHandlerConfiguration,
                                StreamType,
                                StreamStatusType,
                                SessionStateType>
{
 public:
  BitTorrent_Session_T (PeerConnectionManagerType* = NULL,     // peer connection manager handle
                        TrackerConnectionManagerType* = NULL); // tracker connection manager handle
  virtual ~BitTorrent_Session_T ();

  // implement Net_ISession_T
  //virtual const BitTorrent_SessionState& state () const;

  // implement BitTorrent_ISession_T
  virtual const SessionStateType& state (); // return value: state handle
  virtual void trackerConnect (const ACE_INET_Addr&); // peer address
  virtual void trackerDisconnect (const ACE_INET_Addr&); // peer address
  virtual void peerConnect (const ACE_INET_Addr&); // peer address
  virtual void peerDisconnect (const ACE_INET_Addr&); // peer address
  virtual void notify (const struct BitTorrent_Record&, // message record
                       ACE_Message_Block* = NULL);      // data piece (if applicable)

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T (const BitTorrent_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T& operator= (const BitTorrent_Session_T&))

  // convenient types
  typedef ACE_Connector<PeerConnectionType,
                        ACE_SOCK_CONNECTOR> PEER_CONNECTOR_T;
  typedef ACE_Asynch_Connector<PeerConnectionType> PEER_ASYNCH_CONNECTOR_T;
  typedef ACE_Connector<TrackerConnectionType,
                        ACE_SOCK_CONNECTOR> TRACKER_CONNECTOR_T;
  typedef ACE_Asynch_Connector<TrackerConnectionType> TRACKER_ASYNCH_CONNECTOR_T;

  void error (const struct BitTorrent_Record&);
  void log (const struct BitTorrent_Record&);

  ACE_SYNCH_MUTEX lock_;
  bool            logToFile_;
};

// include template definition
#include "bittorrent_session.inl"

#endif
