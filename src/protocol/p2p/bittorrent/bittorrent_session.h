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
#include "bittorrent_streamhandler.h"

// forward declarations
class ACE_Message_Block;

template <typename HandlerConfigurationType, // socket-
          typename ConnectionConfigurationType,
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
          typename ConfigurationType,
          typename StateType,
          ////////////////////////////////
          typename UserDataType,
          ////////////////////////////////
          typename CBDataType> // ui feedback data type
class BitTorrent_Session_T
 : public Net_SessionBase_T<ACE_INET_Addr,
                            ConnectionConfigurationType,
                            ConnectionStateType,
                            BitTorrent_RuntimeStatistic_t,
                            struct Net_SocketConfiguration,
                            HandlerConfigurationType,
                            typename PeerConnectionType::ICONNECTION_T,
                            ConnectionManagerType,
                            PeerConnectorType,
                            ConfigurationType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  ConnectionConfigurationType,
                                                  ConnectionStateType,
                                                  BitTorrent_RuntimeStatistic_t,
                                                  struct Net_SocketConfiguration,
                                                  HandlerConfigurationType,
                                                  PeerStreamType,
                                                  StreamStatusType,
                                                  ConfigurationType,
                                                  StateType> >
{
 public:
  // convenient types
  typedef typename PeerConnectionType::ISTREAM_CONNECTION_T ISTREAM_CONNECTION_T;
  typedef typename TrackerConnectionType::ISTREAM_CONNECTION_T ITRACKER_STREAM_CONNECTION_T;

  BitTorrent_Session_T ();
  virtual ~BitTorrent_Session_T ();

  // override/implement (part of) BitTorrent_ISession_T
  virtual bool initialize (const ConfigurationType&);
  virtual void connect (const ACE_INET_Addr&);
  virtual void trackerConnect (const ACE_INET_Addr&);
  inline virtual void trackerDisconnect (const ACE_INET_Addr& address_in) { inherited::disconnect (address_in); };

 private:
  typedef Net_SessionBase_T<ACE_INET_Addr,
                            ConnectionConfigurationType,
                            ConnectionStateType,
                            BitTorrent_RuntimeStatistic_t,
                            struct Net_SocketConfiguration,
                            HandlerConfigurationType,
                            typename PeerConnectionType::ICONNECTION_T,
                            ConnectionManagerType,
                            PeerConnectorType,
                            ConfigurationType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  ConnectionConfigurationType,
                                                  ConnectionStateType,
                                                  BitTorrent_RuntimeStatistic_t,
                                                  struct Net_SocketConfiguration,
                                                  HandlerConfigurationType,
                                                  PeerStreamType,
                                                  StreamStatusType,
                                                  ConfigurationType,
                                                  StateType> > inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T (const BitTorrent_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T& operator= (const BitTorrent_Session_T&))

  // convenient types
//  typedef ACE_Connector<TrackerConnectionType,
//                        ACE_SOCK_CONNECTOR> TRACKER_CONNECTOR_T;
//  typedef ACE_Asynch_Connector<TrackerConnectionType> TRACKER_ASYNCH_CONNECTOR_T;
  typedef TrackerConnectorType TRACKER_CONNECTOR_T;
  typedef BitTorrent_PeerStreamHandler_T<typename PeerStreamType::SESSION_DATA_T,
                                         UserDataType,
                                         typename inherited::ISESSION_T,
                                         CBDataType> PEER_STREAM_HANDLER_T;
  typedef BitTorrent_TrackerStreamHandler_T<typename TrackerStreamType::SESSION_DATA_T,
                                            UserDataType,
                                            typename inherited::ISESSION_T,
                                            CBDataType> TRACKER_STREAM_HANDLER_T;

  // implement (part of) BitTorrent_ISession_T
  inline virtual void trackerConnect (Net_ConnectionId_t id_in) { inherited::connect (id_in); };
  inline virtual void trackerDisconnect (Net_ConnectionId_t id_in) { inherited::disconnect (id_in); };
  virtual void notify (const struct HTTP_Record&); // tracker message record
  virtual void notify (const struct BitTorrent_Record&, // message record
                       ACE_Message_Block* = NULL);      // data piece (if applicable)

  void error (const struct BitTorrent_Record&);
  void log (const struct BitTorrent_Record&);

  bool                              logToFile_;
  BitTorrent_PeerHandler_Module*    peerHandlerModule_;
  PEER_STREAM_HANDLER_T             peerStreamHandler_;
  BitTorrent_TrackerHandler_Module* trackerHandlerModule_;
  TRACKER_STREAM_HANDLER_T          trackerStreamHandler_;
};

// include template definition
#include "bittorrent_session.inl"

#endif
