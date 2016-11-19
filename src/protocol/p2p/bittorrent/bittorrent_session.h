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

//#include <ace/Asynch_Connector.h>
//#include <ace/config-macros.h>
//#include <ace/Connector.h>
#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
//#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>

#include "net_session_base.h"

#include "bittorrent_common.h"
#include "bittorrent_isession.h"
#include "bittorrent_stream_common.h"

// forward declarations
class ACE_Message_Block;

template <typename HandlerConfigurationType, // socket-
          typename ConfigurationType, // connection-
          typename ConnectionStateType,
          ////////////////////////////////
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          ////////////////////////////////
          typename PeerConnectionType,
          typename TrackerConnectionType,
          ////////////////////////////////
          typename ConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          ////////////////////////////////
          typename StateType,
          ////////////////////////////////
          typename UserDataType>
class BitTorrent_Session_T
 : public Net_SessionBase_T<ACE_INET_Addr,
                            ConfigurationType,
                            ConnectionStateType,
                            BitTorrent_RuntimeStatistic_t,
                            struct Net_SocketConfiguration,
                            HandlerConfigurationType,
                            typename PeerConnectionType::ICONNECTION_T,
                            ConnectionManagerType,
                            PeerConnectorType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  ConfigurationType,
                                                  ConnectionStateType,
                                                  BitTorrent_RuntimeStatistic_t,
                                                  struct Net_SocketConfiguration,
                                                  HandlerConfigurationType,
                                                  PeerStreamType,
                                                  StreamStatusType,
                                                  StateType> >
{
 public:
  BitTorrent_Session_T (const HandlerConfigurationType&, // socket handler configuration
                        ConnectionManagerType* = NULL,   // connection manager handle
                        bool = !NET_EVENT_USE_REACTOR);  // asynchronous ?
  virtual ~BitTorrent_Session_T ();

  // implement BitTorrent_ISession_T
  virtual void trackerConnect (const ACE_INET_Addr&);
  inline virtual void trackerDisconnect (const ACE_INET_Addr& address_in) { inherited::disconnect (address_in); };
  inline virtual void trackerConnect (Net_ConnectionId_t id_in) { inherited::connect (id_in); };
  inline virtual void trackerDisconnect (Net_ConnectionId_t id_in) { inherited::disconnect (id_in); };
  virtual void notify (const struct HTTP_Record&); // tracker message record
  virtual void notify (const struct BitTorrent_Record&, // message record
                       ACE_Message_Block* = NULL);      // data piece (if applicable)

 private:
  typedef Net_SessionBase_T<ACE_INET_Addr,
                            ConfigurationType,
                            ConnectionStateType,
                            BitTorrent_RuntimeStatistic_t,
                            struct Net_SocketConfiguration,
                            HandlerConfigurationType,
                            typename PeerConnectionType::ICONNECTION_T,
                            ConnectionManagerType,
                            PeerConnectorType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  ConfigurationType,
                                                  ConnectionStateType,
                                                  BitTorrent_RuntimeStatistic_t,
                                                  struct Net_SocketConfiguration,
                                                  HandlerConfigurationType,
                                                  PeerStreamType,
                                                  StreamStatusType,
                                                  StateType> > inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T (const BitTorrent_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T& operator= (const BitTorrent_Session_T&))

  // convenient types
//  typedef ACE_Connector<TrackerConnectionType,
//                        ACE_SOCK_CONNECTOR> TRACKER_CONNECTOR_T;
//  typedef ACE_Asynch_Connector<TrackerConnectionType> TRACKER_ASYNCH_CONNECTOR_T;
  typedef TrackerConnectorType TRACKER_CONNECTOR_T;

  void error (const struct BitTorrent_Record&);
  void log (const struct BitTorrent_Record&);

  bool logToFile_;
};

// include template definition
#include "bittorrent_session.inl"

#endif
