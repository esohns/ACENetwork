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

#include <map>
#include <string>
#include <utility>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_timer_handler.h"

#include "net_connection_configuration.h"
#include "net_session_base.h"

#include "bittorrent_common.h"
#include "bittorrent_isession.h"
#include "bittorrent_stream_common.h"
#include "bittorrent_streamhandler.h"

// forward declarations
class ACE_Message_Block;

ACE_THR_FUNC_RETURN net_bittorrent_session_setup_function (void*);

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          ////////////////////////////////
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          ////////////////////////////////
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          ////////////////////////////////
          typename PeerConnectionType,
          typename TrackerConnectionType,
          ////////////////////////////////
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          ////////////////////////////////
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          ////////////////////////////////
          typename ControllerInterfaceType, // derived from BitTorrent_IControl_T
          ////////////////////////////////
          typename CBDataType> // ui feedback data type
class BitTorrent_Session_T
 : public Net_SessionBase_T<ACE_INET_Addr,
                            PeerConnectionConfigurationType,
                            PeerConnectionStateType,
                            Net_StreamStatistic_t,
                            typename PeerConnectionType::ICONNECTION_T,
                            PeerConnectionManagerType,
                            PeerConnectorType,
                            ConfigurationType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  PeerConnectionConfigurationType,
                                                  TrackerConnectionConfigurationType,
                                                  PeerConnectionStateType,
                                                  Net_StreamStatistic_t,
                                                  PeerStreamType,
                                                  StreamStatusType,
                                                  ConfigurationType,
                                                  StateType> >
 , public Common_Timer_Handler
{
  typedef Net_SessionBase_T<ACE_INET_Addr,
                            PeerConnectionConfigurationType,
                            PeerConnectionStateType,
                            Net_StreamStatistic_t,
                            typename PeerConnectionType::ICONNECTION_T,
                            PeerConnectionManagerType,
                            PeerConnectorType,
                            ConfigurationType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  PeerConnectionConfigurationType,
                                                  TrackerConnectionConfigurationType,
                                                  PeerConnectionStateType,
                                                  Net_StreamStatistic_t,
                                                  PeerStreamType,
                                                  StreamStatusType,
                                                  ConfigurationType,
                                                  StateType> > inherited;
  typedef Common_Timer_Handler inherited2;

 public:
  // convenient types
  typedef typename PeerConnectionType::ICONNECTION_T ICONNECTION_T;
  typedef typename TrackerConnectionType::ICONNECTION_T ITRACKER_CONNECTION_T;
  typedef typename PeerConnectionType::ISTREAM_CONNECTION_T ISTREAM_CONNECTION_T;
  typedef typename TrackerConnectionType::ISTREAM_CONNECTION_T ITRACKER_STREAM_CONNECTION_T;
  typedef ControllerInterfaceType ICONTROLLER_T;
  typedef ACE_Singleton<PeerConnectionManagerType,
                        ACE_SYNCH_MUTEX> PEER_CONNECTION_MANAGER_SINGLETON_T;
  typedef ACE_Singleton<TrackerConnectionManagerType,
                        ACE_SYNCH_MUTEX> TRACKER_CONNECTION_MANAGER_SINGLETON_T;
  typedef typename TrackerStreamType::MESSAGE_T TRACKER_MESSAGE_T;

  BitTorrent_Session_T ();
  virtual ~BitTorrent_Session_T ();

  // override/implement (part of) BitTorrent_ISession_T
  inline virtual unsigned int numberOfPieces () const { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, 0); return static_cast<unsigned int> (inherited::state_.pieces.size ()); }
  virtual bool initialize (const ConfigurationType&);
  virtual void connect (const ACE_INET_Addr&);
  virtual void trackerConnect (const ACE_INET_Addr&);
  inline virtual void trackerDisconnect (const ACE_INET_Addr& address_in) { inherited::disconnect (address_in); }
  inline virtual Net_ConnectionId_t trackerConnectionId () { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, 0); return inherited::state_.trackerConnectionId; }
  inline virtual ACE_INET_Addr trackerAddress () { ACE_INET_Addr dummy; ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, dummy); return inherited::state_.trackerAddress; }

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T (const BitTorrent_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T& operator= (const BitTorrent_Session_T&))

  // convenient types
  typedef TrackerConnectorType TRACKER_CONNECTOR_T;

  typedef Stream_Module_Aggregator_ReaderTask_T<ACE_MT_SYNCH,
                                                Common_TimePolicy_t,
                                                typename PeerStreamType::CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T,
                                                typename PeerStreamType::CONTROL_MESSAGE_T,
                                                typename PeerStreamType::MESSAGE_T,
                                                typename PeerStreamType::SESSION_MESSAGE_T> PEER_MESSAGEHANDLER_READER_T;
  typedef Stream_Module_MessageHandlerA_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          typename PeerStreamType::CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T,
                                          typename PeerStreamType::CONTROL_MESSAGE_T,
                                          typename PeerStreamType::MESSAGE_T,
                                          typename PeerStreamType::SESSION_MESSAGE_T,
                                          typename PeerStreamType::SESSION_DATA_T,
                                          PeerStreamUserDataType> PEER_MESSAGEHANDLER_WRITER_T;
  typedef Stream_Module_Aggregator_ReaderTask_T<ACE_MT_SYNCH,
                                                Common_TimePolicy_t,
                                                typename TrackerStreamType::CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T,
                                                typename TrackerStreamType::CONTROL_MESSAGE_T,
                                                typename TrackerStreamType::MESSAGE_T,
                                                typename TrackerStreamType::SESSION_MESSAGE_T> TRACKER_MESSAGEHANDLER_READER_T;
  typedef Stream_Module_MessageHandlerA_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          typename TrackerStreamType::CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T,
                                          typename TrackerStreamType::CONTROL_MESSAGE_T,
                                          typename TrackerStreamType::MESSAGE_T,
                                          typename TrackerStreamType::SESSION_MESSAGE_T,
                                          typename TrackerStreamType::SESSION_DATA_T,
                                          TrackerStreamUserDataType> TRACKER_MESSAGEHANDLER_WRITER_T;
  typedef Stream_StreamModuleA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 typename PeerStreamType::SESSION_DATA_T,
                                 enum Stream_SessionMessageType,
                                 typename PeerStreamType::CONFIGURATION_T::MODULE_CONFIGURATION_T,
                                 typename PeerStreamType::CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T,
                                 libacenetwork_default_bittorrent_handler_module_name_string,
                                 Stream_INotify_t,
                                 PEER_MESSAGEHANDLER_READER_T,
                                 PEER_MESSAGEHANDLER_WRITER_T> PEER_MESSAGEHANDLER_MODULE_T;
  typedef Stream_StreamModuleA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 typename TrackerStreamType::SESSION_DATA_T,
                                 enum Stream_SessionMessageType,
                                 typename TrackerStreamType::CONFIGURATION_T::MODULE_CONFIGURATION_T,
                                 typename TrackerStreamType::CONFIGURATION_T::MODULEHANDLER_CONFIGURATION_T,
                                 libacenetwork_default_bittorrent_handler_module_name_string,
                                 Stream_INotify_t,
                                 TRACKER_MESSAGEHANDLER_READER_T,
                                 TRACKER_MESSAGEHANDLER_WRITER_T> TRACKER_MESSAGEHANDLER_MODULE_T;

  // implement/override (part of) BitTorrent_ISession_T
  virtual void connect (Net_ConnectionId_t); // connection id
  virtual void disconnect (Net_ConnectionId_t); // connection id
  virtual void choke (Net_ConnectionId_t, // connection id
                      bool);              // choke ? : unchoke
  virtual void interested (Net_ConnectionId_t, // connection id
                           bool);              // interested ? : not interested
  virtual void have (unsigned int);  // index (piece#)
  virtual void request (Net_ConnectionId_t, // connection id
                        unsigned int,       // index (piece#)
                        unsigned int,       // begin (offset)
                        unsigned int);      // length (bytes)
  virtual void piece (Net_ConnectionId_t, // connection id
                      unsigned int,       // index (piece#)
                      unsigned int,       // begin (offset)
                      unsigned int);      // length (bytes)
  virtual void scrape ();
  virtual void trackerConnect (Net_ConnectionId_t);
  virtual void trackerDisconnect (Net_ConnectionId_t);
  virtual void trackerRedirect (Net_ConnectionId_t,  // connection id
                                const std::string&); // HTTP response "Location" header
  virtual void trackerError (Net_ConnectionId_t,         // connection id
                             const struct HTTP_Record&); // HTTP response
  virtual void notify (const Bencoding_Dictionary_t&); // tracker response/scrape record
  virtual void notify (Net_ConnectionId_t,                      // connection id
                       const struct BitTorrent_PeerHandShake&); // peer handshake record
  virtual void notify (Net_ConnectionId_t,                  // connection id
                       const struct BitTorrent_PeerRecord&, // message record
                       ACE_Message_Block* = NULL);          // data piece (if applicable)

  void error (const struct BitTorrent_PeerRecord&);
  void log (const struct BitTorrent_PeerRecord&);

  bool getConnectionAndMessage (Net_ConnectionId_t,                    // connection id
                                ISTREAM_CONNECTION_T*&,                // return value: connection handle
                                typename PeerStreamType::MESSAGE_T*&); // return value: message handle
  bool requestNextPiece (Net_ConnectionId_t); // connection id
  void populatePeerPiecesBitfield (Net_ConnectionId_t); // connection id
  ACE_UINT32 connectionIdToPeerAddress (Net_ConnectionId_t); // connection id

  // implement Common_ITimerHandler
  virtual void handle (const void*); // asynchronous completion token

  //bool                             logToFile_;
  PEER_MESSAGEHANDLER_MODULE_T*    peerHandlerModule_;
  TRACKER_MESSAGEHANDLER_MODULE_T* trackerHandlerModule_;
  long                             timerId_;
};

// include template definition
#include "bittorrent_session.inl"

#endif
