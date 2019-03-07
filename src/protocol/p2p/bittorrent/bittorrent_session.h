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
#include "ace/Synch_Traits.h"

#include "net_session_base.h"

#include "bittorrent_common.h"
#include "bittorrent_isession.h"
#include "bittorrent_stream_common.h"
#include "bittorrent_streamhandler.h"

// forward declarations
class ACE_Message_Block;

ACE_THR_FUNC_RETURN net_bittorrent_session_setup_function (void*);

template <typename PeerHandlerConfigurationType, // socket-
          typename TrackerHandlerConfigurationType, // socket-
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          ////////////////////////////////
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          ////////////////////////////////
          typename PeerModuleHandlerConfigurationType,
          typename TrackerModuleHandlerConfigurationType,
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
          typename PeerUserDataType,
          typename TrackerUserDataType,
          ////////////////////////////////
          typename ControllerInterfaceType // derived from BitTorrent_IControl_T
          ////////////////////////////////
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
class BitTorrent_Session_T
 : public Net_SessionBase_T<ACE_INET_Addr,
                            PeerConnectionConfigurationType,
                            PeerConnectionStateType,
                            BitTorrent_Statistic_t,
                            struct Net_SocketConfiguration,
                            PeerHandlerConfigurationType,
                            typename PeerConnectionType::ICONNECTION_T,
                            PeerConnectionManagerType,
                            PeerConnectorType,
                            ConfigurationType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  PeerConnectionConfigurationType,
                                                  TrackerConnectionConfigurationType,
                                                  PeerConnectionStateType,
                                                  BitTorrent_Statistic_t,
                                                  struct Net_SocketConfiguration,
                                                  PeerHandlerConfigurationType,
                                                  TrackerHandlerConfigurationType,
                                                  PeerStreamType,
                                                  StreamStatusType,
                                                  ConfigurationType,
                                                  StateType> >
{
 public:
  // convenient types
  typedef typename PeerConnectionType::ICONNECTION_T ICONNECTION_T;
  typedef typename TrackerConnectionType::ICONNECTION_T ITRACKER_CONNECTION_T;
  typedef typename PeerConnectionType::ISTREAM_CONNECTION_T ISTREAM_CONNECTION_T;
  typedef typename TrackerConnectionType::ISTREAM_CONNECTION_T ITRACKER_STREAM_CONNECTION_T;
  typedef ControllerInterfaceType ICONTROLLER_T;

  BitTorrent_Session_T ();
  virtual ~BitTorrent_Session_T ();

  // override/implement (part of) BitTorrent_ISession_T
  virtual bool initialize (const ConfigurationType&);
  virtual void connect (const ACE_INET_Addr&);
  virtual void trackerConnect (const ACE_INET_Addr&);
  inline virtual void trackerDisconnect (const ACE_INET_Addr& address_in) { inherited::disconnect (address_in); }

 protected:
  TrackerConnectionConfigurationType* trackerConnectionConfiguration_;
  TrackerConnectionManagerType*       trackerConnectionManager_;

 private:
  typedef Net_SessionBase_T<ACE_INET_Addr,
                            PeerConnectionConfigurationType,
                            PeerConnectionStateType,
                            BitTorrent_Statistic_t,
                            struct Net_SocketConfiguration,
                            PeerHandlerConfigurationType,
                            typename PeerConnectionType::ICONNECTION_T,
                            PeerConnectionManagerType,
                            PeerConnectorType,
                            ConfigurationType,
                            StateType,
                            BitTorrent_ISession_T<ACE_INET_Addr,
                                                  PeerConnectionConfigurationType,
                                                  TrackerConnectionConfigurationType,
                                                  PeerConnectionStateType,
                                                  BitTorrent_Statistic_t,
                                                  struct Net_SocketConfiguration,
                                                  PeerHandlerConfigurationType,
                                                  TrackerHandlerConfigurationType,
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

  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         PeerModuleHandlerConfigurationType,
                                         typename PeerStreamType::CONTROL_MESSAGE_T,
                                         typename PeerStreamType::MESSAGE_T,
                                         typename PeerStreamType::SESSION_MESSAGE_T,
                                         Stream_SessionId_t,
                                         typename PeerStreamType::SESSION_DATA_T,
                                         PeerUserDataType> PEER_MESSAGEHANDLER_T;
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         TrackerModuleHandlerConfigurationType,
                                         typename TrackerStreamType::CONTROL_MESSAGE_T,
                                         typename TrackerStreamType::MESSAGE_T,
                                         typename TrackerStreamType::SESSION_MESSAGE_T,
                                         Stream_SessionId_t,
                                         typename TrackerStreamType::SESSION_DATA_T,
                                         TrackerUserDataType> TRACKER_MESSAGEHANDLER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Stream_SessionId_t,
                                         typename PeerStreamType::SESSION_DATA_T,
                                         enum Stream_SessionMessageType,
                                         struct Stream_ModuleConfiguration,
                                         PeerModuleHandlerConfigurationType,
                                         libacenetwork_default_bittorrent_handler_module_name_string,
                                         Stream_INotify_t,
                                         PEER_MESSAGEHANDLER_T> PEER_MESSAGEHANDLER_MODULE_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Stream_SessionId_t,
                                         typename TrackerStreamType::SESSION_DATA_T,
                                         enum Stream_SessionMessageType,
                                         struct Stream_ModuleConfiguration,
                                         TrackerModuleHandlerConfigurationType,
                                         libacenetwork_default_bittorrent_handler_module_name_string,
                                         Stream_INotify_t,
                                         TRACKER_MESSAGEHANDLER_T> TRACKER_MESSAGEHANDLER_MODULE_T;

  typedef BitTorrent_PeerStreamHandler_T<typename PeerStreamType::SESSION_DATA_T,
                                         PeerUserDataType,
                                         typename inherited::ISESSION_T
#if defined (GUI_SUPPORT)
                                         ,CBDataType> PEER_HANDLER_T;
#else
                                         > PEER_HANDLER_T;
#endif // GUI_SUPPORT
  typedef BitTorrent_TrackerStreamHandler_T<typename TrackerStreamType::SESSION_DATA_T,
                                            TrackerUserDataType,
                                            typename inherited::ISESSION_T
#if defined (GUI_SUPPORT)
                                            ,CBDataType> TRACKER_HANDLER_T;
#else
                                            > TRACKER_HANDLER_T;
#endif // GUI_SUPPORT

  typedef typename std::map<std::string,
                            std::pair <struct Stream_ModuleConfiguration,
                                       PeerModuleHandlerConfigurationType> >::iterator PEERMODULEHANDLERCONFIGURATIONITERATOR_T;
  typedef typename std::map<std::string,
                            std::pair <struct Stream_ModuleConfiguration,
                                       TrackerModuleHandlerConfigurationType> >::iterator TRACKERMODULEHANDLERCONFIGURATIONITERATOR_T;

  // implement/override (part of) BitTorrent_ISession_T
  virtual void connect (Net_ConnectionId_t);
  virtual void disconnect (Net_ConnectionId_t);
  virtual void scrape ();
  virtual void trackerConnect (Net_ConnectionId_t);
  virtual void trackerDisconnect (Net_ConnectionId_t);
  virtual void notify (const Bencoding_Dictionary_t&); // tracker message record
  virtual void notify (const struct BitTorrent_PeerHandShake&); // peer handshake record
  virtual void notify (const struct BitTorrent_PeerRecord&, // message record
                       ACE_Message_Block* = NULL);          // data piece (if applicable)

  void error (const struct BitTorrent_PeerRecord&);
  void log (const struct BitTorrent_PeerRecord&);

  bool                             logToFile_;
  std::string                      metaInfoFileName_;
  PEER_MESSAGEHANDLER_MODULE_T*    peerHandlerModule_;
  PEER_HANDLER_T                   peerStreamHandler_;
  TRACKER_MESSAGEHANDLER_MODULE_T* trackerHandlerModule_;
  TRACKER_HANDLER_T                trackerStreamHandler_;
};

// include template definition
#include "bittorrent_session.inl"

#endif
