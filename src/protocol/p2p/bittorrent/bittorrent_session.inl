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

#include <functional>
#include <random>
#include <vector>

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"

#include "net_defines.h"
#include "net_macros.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "net_client_defines.h"

#include "http_tools.h"

#include "bittorrent_defines.h"
#include "bittorrent_network.h"
#include "bittorrent_tools.h"

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::BitTorrent_Session_T ()
 : inherited ()
 , trackerConnectionConfiguration_ (NULL)
 , trackerConnectionManager_ (NULL)
 , logToFile_ (BITTORRENT_DEFAULT_SESSION_LOG)
 , metaInfoFileName_ ()
 , peerHandlerModule_ (NULL)
 , peerStreamHandler_ (this)
 , trackerHandlerModule_ (NULL)
 , trackerStreamHandler_ (this)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::BitTorrent_Session_T"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    inherited::state_.peerId = BitTorrent_Tools::generatePeerId ();
  } // end lock scope

  ACE_NEW_NORETURN (peerHandlerModule_,
                    PEER_MESSAGEHANDLER_MODULE_T (NULL,
                                                  ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_HANDLER_MODULE_NAME)));
  if (!peerHandlerModule_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  ACE_NEW_NORETURN (trackerHandlerModule_,
                    TRACKER_MESSAGEHANDLER_MODULE_T (NULL,
                                                     ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_HANDLER_MODULE_NAME)));
  if (!trackerHandlerModule_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::~BitTorrent_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::~BitTorrent_Session_T"));

  if (peerHandlerModule_)
    delete peerHandlerModule_;
  if (trackerHandlerModule_)
    delete trackerHandlerModule_;
}

//////////////////////////////////////////

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
bool
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.controller);
  ACE_ASSERT (configuration_in.parserConfiguration);
  ACE_ASSERT (configuration_in.metaInfo);

  // *TODO*: remove type inferences
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, false);

    inherited::state_.controller = configuration_in.controller;
    inherited::state_.metaInfo = configuration_in.metaInfo;
  } // end lock scope
  trackerConnectionConfiguration_ =
      const_cast<ConfigurationType&> (configuration_in).trackerConnectionConfiguration;
  trackerConnectionManager_ = configuration_in.trackerConnectionManager;
  metaInfoFileName_ = configuration_in.metaInfoFileName;
  if (!trackerStreamHandler_.initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_TrackerStreamHandler_T::initialize(), aborting\n")));
    return false;
  } // end IF

  return inherited::initialize (configuration_in);
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::connect"));

  // step0: subscribe to notifications
  PEERMODULEHANDLERCONFIGURATIONITERATOR_T iterator;
  PeerConnectionConfigurationType* configuration_p = NULL;
  PeerUserDataType* user_data_p = NULL;
  typename PeerStreamType::IDATA_NOTIFY_T* subscriber_p = NULL;
  bool clone_module = false;
  bool delete_module = false;
  Stream_Module_t* module_p = NULL;
  if (inherited::connectionManager_)
  {
    inherited::connectionManager_->get (configuration_p,
                                        user_data_p);
    ACE_ASSERT (configuration_p);
    // *TODO*: remove type inferences
    ACE_ASSERT (configuration_p->streamConfiguration_);

    // step1: set up configuration
    configuration_p->socketHandlerConfiguration.socketConfiguration_2.address =
        address_in;

    iterator =
        configuration_p->streamConfiguration_->find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator != configuration_p->streamConfiguration_->end ());
    subscriber_p = (*iterator).second.second.subscriber;
    (*iterator).second.second.subscriber = &peerStreamHandler_;

    clone_module =
      configuration_p->streamConfiguration_->configuration_.cloneModule;
    delete_module =
      configuration_p->streamConfiguration_->configuration_.deleteModule;
    module_p =
      configuration_p->streamConfiguration_->configuration_.module;

    configuration_p->streamConfiguration_->configuration_.cloneModule =
      false;
    configuration_p->streamConfiguration_->configuration_.deleteModule =
      false;
    configuration_p->streamConfiguration_->configuration_.module =
      peerHandlerModule_;
  } // end IF

  // step1: (try to) connect
  inherited::connect (address_in);

  // step2: reset configuration
  if (inherited::connectionManager_)
  {
    ACE_ASSERT (configuration_p);
    // *TODO*: remove type inferences
    ACE_ASSERT (configuration_p->streamConfiguration_);

    ACE_ASSERT (iterator != configuration_p->streamConfiguration_->end ());
    (*iterator).second.second.subscriber = subscriber_p;

    configuration_p->streamConfiguration_->configuration_.cloneModule =
      clone_module;
    configuration_p->streamConfiguration_->configuration_.deleteModule =
      delete_module;
    configuration_p->streamConfiguration_->configuration_.module = module_p;
  } // end IF
};

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::connect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::connect"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration->messageAllocator);
  ACE_ASSERT (inherited::connectionManager_);

  inherited::connect (id_in);

  struct BitTorrent_PeerStatus peer_status;
  struct BitTorrent_PeerHandShake* record_p = NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  typename PeerStreamType::MESSAGE_T* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ICONNECTION_T* iconnection_p = NULL;
  ISTREAM_CONNECTION_T* istream_connection_p = NULL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    inherited::state_.peerStatus.insert (std::make_pair (id_in, peer_status));
  } // end lock scope

  // send handshake
  ACE_NEW_NORETURN (record_p,
                    struct BitTorrent_PeerHandShake ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    // sanity check(s)
    ACE_ASSERT (inherited::state_.metaInfo);

    record_p->info_hash =
        BitTorrent_Tools::MetaInfoToInfoHash (*inherited::state_.metaInfo);
    record_p->peer_id = inherited::state_.peerId;
  } // end lock scope

  ACE_NEW_NORETURN (data_p,
                    struct BitTorrent_PeerMessageData ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  data_p->handShakeRecord = record_p;
  record_p = NULL;

  // *IMPORTANT NOTE*: fire-and-forget API (data_p)
  ACE_NEW_NORETURN (data_container_p,
                    typename PeerStreamType::MESSAGE_T::DATA_T (data_p,
                                                                true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  data_p = NULL;
allocate:
  message_p =
    static_cast<typename PeerStreamType::MESSAGE_T*> (inherited::configuration_->connectionConfiguration->messageAllocator->malloc (inherited::configuration_->connectionConfiguration->PDUSize));
  // keep retrying ?
  if (!message_p &&
      !inherited::configuration_->connectionConfiguration->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));

    // clean up
    data_container_p->decrease ();

    goto error;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (data_container_p)
  message_p->initialize (data_container_p,
                         1,
                         NULL);

  iconnection_p = inherited::connectionManager_->get (id_in);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve connection handle (id was: %d), aborting\n"),
                id_in));
    goto error;
  } // end IF
  istream_connection_p = dynamic_cast<ISTREAM_CONNECTION_T*> (iconnection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    goto error;
  } // end IF

  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  message_block_p = message_p;
  try {
    istream_connection_p->send (message_block_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IStreamConnection_T::send(), returning\n")));
    goto error;
  }
  message_p = NULL;

  // step5: clean up
  iconnection_p->decrease ();

  return;

error:
  if (record_p)
    delete record_p;
  if (data_p)
    delete data_p;
  if (message_p)
    delete message_p;
  if (iconnection_p)
  {
    iconnection_p->close ();
    iconnection_p->decrease ();
  } // end IF
}
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::disconnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::disconnect"));

  inherited::disconnect (id_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    BitTorrent_PeerStatusIterator_t iterator =
        inherited::state_.peerStatus.find (id_in);
    if (iterator != inherited::state_.peerStatus.end ())
      inherited::state_.peerStatus.erase (iterator);

    if (inherited::state_.connections.empty () &&
        inherited::state_.controller)
    {
      try {
        inherited::state_.controller->notify (metaInfoFileName_,
                                              BITTORRENT_EVENT_CANCELLED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
      }
    } // end IF
  } // end lock scope
}


template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::scrape ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::scrape"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->metaInfo);
  ACE_ASSERT (trackerConnectionConfiguration_);
  ACE_ASSERT (trackerConnectionConfiguration_->messageAllocator);
  ACE_ASSERT (trackerConnectionManager_);

  struct HTTP_Record* record_p = NULL;
  Bencoding_DictionaryIterator_t iterator;
  std::string key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY);
  std::string host_name_string;
  std::string user_agent;
  Net_ConnectionId_t tracker_connection_id = 0;
  ITRACKER_CONNECTION_T* iconnection_p = NULL;
  ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  bool use_SSL = false;

  ACE_NEW_NORETURN (record_p,
                    struct HTTP_Record ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF

//  iterator = configuration_->metaInfo->find (&key);
  iterator = inherited::configuration_->metaInfo->begin ();
  for (;
       iterator != inherited::configuration_->metaInfo->end ();
       ++iterator)
    if (*(*iterator).first == key)
      break;
  ACE_ASSERT (iterator != inherited::configuration_->metaInfo->end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
  if (!HTTP_Tools::parseURL (*(*iterator).second->string,
                             host_name_string,
                             record_p->URI,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                ACE_TEXT (*(*iterator).second->string->c_str ())));
    goto error;
  } // end IF
  record_p->URI = BitTorrent_Tools::AnnounceURLToScrapeURL (record_p->URI);

  // step4: send request to the tracker
  record_p->method = HTTP_Codes::HTTP_METHOD_GET;
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER),
                                         HTTP_Tools::URLEncode (BitTorrent_Tools::MetaInfoToInfoHash (*inherited::configuration_->metaInfo))));

#ifdef HAVE_CONFIG_H
  user_agent  = ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_NAME);
  user_agent += ACE_TEXT_ALWAYS_CHAR ("/");
  user_agent += ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_VERSION);
#endif
  if (!user_agent.empty ())
    record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_AGENT_STRING),
                                              user_agent));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                            host_name_string));
//                                            HTTP_Tools::IPAddress2HostName (tracker_address).c_str ()));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_STRING),
                                            ACE_TEXT_ALWAYS_CHAR ("*/*")));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_ENCODING_STRING),
                                            ACE_TEXT_ALWAYS_CHAR ("gzip;q=1.0, deflate, identity")));

  // *IMPORTANT NOTE*: fire-and-forget API (record_p)
  ACE_NEW_NORETURN (data_container_p,
                    typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T (record_p,
                                                                                        true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  record_p = NULL;
allocate:
  message_p =
    static_cast<typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T*> (inherited::configuration_->trackerConnectionConfiguration->messageAllocator->malloc (inherited::configuration_->trackerConnectionConfiguration->PDUSize));
  // keep retrying ?
  if (!message_p &&
      !inherited::configuration_->trackerConnectionConfiguration->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate request message: \"%m\", returning\n")));

    // clean up
    data_container_p->decrease ();

    goto error;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (data_container_p)
  message_p->initialize (data_container_p,
                         1,
                         NULL);
  data_container_p = NULL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    ACE_ASSERT (inherited::state_.trackerConnectionId);
    tracker_connection_id = inherited::state_.trackerConnectionId;
  } // end lock scope
  iconnection_p = trackerConnectionManager_->get (tracker_connection_id);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve tracker connection handle (id was: %d), aborting\n"),
                tracker_connection_id));
    goto error;
  } // end IF
  istream_connection_p =
      dynamic_cast<ITRACKER_STREAM_CONNECTION_T*> (iconnection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    goto error;
  } // end IF

  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  message_block_p = message_p;
  try {
    istream_connection_p->send (message_block_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IStreamConnection_T::send(), returning\n")));
    goto error;
  }
  message_p = NULL;

  // step5: clean up
  iconnection_p->decrease ();

  return;

error:
  if (iconnection_p)
    iconnection_p->decrease ();
  if (record_p)
    delete record_p;
  if (data_container_p)
    data_container_p->decrease ();
  if (message_p)
    delete message_p;
}
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::trackerConnect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerConnect"));

  TrackerConnectorType connector (trackerConnectionManager_,
                                  ACE_Time_Value::zero);
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // step0: subscribe to notifications
  TRACKERMODULEHANDLERCONFIGURATIONITERATOR_T iterator;
  TrackerConnectionConfigurationType* configuration_p = NULL;
  TrackerUserDataType* user_data_p = NULL;
  typename TrackerStreamType::IDATA_NOTIFY_T* subscriber_p = NULL;
  bool clone_module = false;
  bool delete_module = false;
  Stream_Module_t* module_p = NULL;
  if (trackerConnectionManager_)
  {
    trackerConnectionManager_->get (configuration_p,
                                    user_data_p);
    ACE_ASSERT (configuration_p);
    // *TODO*: remove type inferences
    ACE_ASSERT (configuration_p->streamConfiguration_);

    // step1: set up configuration
    configuration_p->socketHandlerConfiguration.socketConfiguration_2.address =
        address_in;

    iterator =
        configuration_p->streamConfiguration_->find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator != configuration_p->streamConfiguration_->end ());
    subscriber_p = (*iterator).second.second.subscriber;
    ACE_ASSERT (!subscriber_p);
    (*iterator).second.second.subscriber = &trackerStreamHandler_;

    clone_module =
      configuration_p->streamConfiguration_->configuration_.cloneModule;
    delete_module =
      configuration_p->streamConfiguration_->configuration_.deleteModule;
    module_p =
      configuration_p->streamConfiguration_->configuration_.module;
    ACE_ASSERT (!module_p);

    configuration_p->streamConfiguration_->configuration_.cloneModule =
      false;
    configuration_p->streamConfiguration_->configuration_.deleteModule =
      false;
    configuration_p->streamConfiguration_->configuration_.module =
      trackerHandlerModule_;
  } // end IF

  ACE_Time_Value deadline;
  ACE_Time_Value initialization_timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT,
                                         0);
  Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
  typename TrackerConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
      NULL;

  // step1: initialize connector
  typename TrackerConnectorType::ICONNECTOR_T* iconnector_p = &connector;
  typename TrackerConnectorType::ICONNECTION_T* iconnection_p = NULL;
  int result = -1;
  ACE_ASSERT (trackerConnectionConfiguration_);
  if (!iconnector_p->initialize (*trackerConnectionConfiguration_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
    goto error;
  } // end IF

  // step2: try to connect
  handle = iconnector_p->connect (address_in);
  if (handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to tracker %s: \"%m\", returning\n"),
                ACE::basename (metaInfoFileName_.c_str (),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    goto error;
  } // end IF
  if (inherited::isAsynch_)
  {
    deadline =
        (COMMON_TIME_NOW +
         ACE_Time_Value (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0));
    ACE_Time_Value delay (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT_INTERVAL,
                          0);
    do
    {
      // *TODO*: this does not work
      iconnection_p = trackerConnectionManager_->get (address_in);
      if (iconnection_p)
        break; // done

      result = ACE_OS::sleep (delay);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &delay));
    } while (COMMON_TIME_NOW < deadline);
  } // end ELSE
  else
    iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      trackerConnectionManager_->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
      trackerConnectionManager_->get (static_cast<Net_ConnectionId_t> (handle));
#endif
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to tracker %s: \"%m\", returning\n"),
                ACE::basename (metaInfoFileName_.c_str (),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    goto error;
  } // end IF

  // step3a: wait for the connection to finish initializing
  // *TODO*: avoid tight loop here
  deadline = COMMON_TIME_NOW + initialization_timeout;
  do
  {
    status = iconnection_p->status ();
    if (status == NET_CONNECTION_STATUS_OK)
      break;
  } while (COMMON_TIME_NOW < deadline);
  if (status != NET_CONNECTION_STATUS_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (to tracker: %s) failed to initialize (status was: %d), returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
                status));
    goto error;
  } // end IF
  // step3b: wait for the connection stream to finish initializing
  istream_connection_p =
    dynamic_cast<typename TrackerConnectorType::ISTREAM_CONNECTION_T*> (iconnection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    goto error;
  } // end IF
  istream_connection_p->wait (STREAM_STATE_RUNNING,
                              NULL); // <-- block

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("connected to tracker %s: %u\n"),
  //            ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
  //            iconnection_p->id ()));

  iconnection_p->decrease ();

error:
  if (trackerConnectionManager_)
  {
    ACE_ASSERT (configuration_p);
    ACE_ASSERT (configuration_p->streamConfiguration_);

    ACE_ASSERT (iterator != configuration_p->streamConfiguration_->end ());
    (*iterator).second.second.subscriber = subscriber_p;

    configuration_p->streamConfiguration_->configuration_.cloneModule =
      clone_module;
    configuration_p->streamConfiguration_->configuration_.deleteModule =
      delete_module;
    configuration_p->streamConfiguration_->configuration_.module =
      module_p;
  } // end IF
};

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::trackerConnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerConnect"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    ACE_ASSERT (!inherited::state_.trackerConnectionId);
    inherited::state_.trackerConnectionId = id_in;
  } // end lock scope

  inherited::connect (id_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: new tracker connection (id: %d)\n"),
              ACE::basename (metaInfoFileName_.c_str (),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in));
}
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::trackerDisconnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerDisconnect"));

  inherited::disconnect (id_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: tracker connection closed (id was: %d)\n"),
              ACE::basename (metaInfoFileName_.c_str (),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    ACE_ASSERT (inherited::state_.trackerConnectionId == id_in);
    inherited::state_.trackerConnectionId = 0;

    if (inherited::state_.connections.empty () &&
        inherited::state_.controller)
    {
      try {
        inherited::state_.controller->notify (metaInfoFileName_,
                                              BITTORRENT_EVENT_CANCELLED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
      }
    } // end IF
  } // end lock scope
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::notify (const Bencoding_Dictionary_t& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

//#if defined (_DEBUG)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s\n"),
//              ACE_TEXT (BitTorrent_Tools::Dictionary2String (record_in).c_str ())));
//#endif

  // *NOTE*: this could be the response to either a request or a 'scrape', the
  //         type can be deduced from the dictionary schema
  Bencoding_DictionaryIterator_t iterator;
  std::string key =
      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_SCRAPE_RESPONSE_FILES_HEADER);
  iterator = record_in.begin ();
  for (;
       iterator != record_in.end ();
       ++iterator)
    if (*(*iterator).first == key) break;
  if (iterator != record_in.end ())
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    if (inherited::state_.trackerScrapeResponse)
      BitTorrent_Tools::free (inherited::state_.trackerScrapeResponse);
    inherited::state_.trackerScrapeResponse =
        &const_cast<Bencoding_Dictionary_t&> (record_in);

    return;
  } // end IF

  // response is regular --> (try to) connect to all peers

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    if (inherited::state_.trackerRequestResponse)
      BitTorrent_Tools::free (inherited::state_.trackerRequestResponse);
    inherited::state_.trackerRequestResponse =
        &const_cast<Bencoding_Dictionary_t&> (record_in);
  } // end lock scope

  key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_HEADER);
  BitTorrent_PeerAddresses_t peer_addresses;
  int result = -1;
  ACE_INET_Addr inet_address;

//    iterator = record_in.find (key);
  iterator = record_in.begin ();
  for (;
       iterator != record_in.end ();
       ++iterator)
    if (*(*iterator).first == key) break;
  ACE_ASSERT (iterator != record_in.end ());
  // can be 'dictionary' or 'binary' model
  if ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY)
  {
    ACE_ASSERT (false);
    ACE_NOTSUP;
    ACE_NOTREACHED (return;)
  } // end IF
  else
  { ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
    const char* char_p = (*iterator).second->string->c_str ();
    for (unsigned int i = 0;
         i < ((*iterator).second->string->size () / 6);
         ++i)
    {
      result = inet_address.set (*reinterpret_cast<const u_short*> (char_p + 4),
                                 *reinterpret_cast<const ACE_UINT32*> (char_p),
                                 0,
                                 0);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
        return;
      } // end IF
      peer_addresses.push_back (inet_address);
      char_p += 6;
    } // end FOR
  } // end ELSE
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connecting to %u peer(s)...\n"),
              peer_addresses.size ()));

  std::uniform_int_distribution<int> random_distribution (1, peer_addresses.size ());
  std::default_random_engine         random_engine;
  std::function<int ()>              random_generator =
      std::bind (random_distribution, random_engine);
  int random_number = random_generator ();
  BitTorrent_PeerAddressesIterator_t iterator_2 = peer_addresses.begin ();
  iterator_2 += random_number;
  inet_address = *iterator_2;
  peer_addresses.clear ();
  peer_addresses.push_back (inet_address);

  struct BitTorrent_SessionInitiationThreadData thread_data;
  thread_data.addresses = &peer_addresses;
  thread_data.lock = &(inherited::lock_);
  thread_data.session = this;
  ACE_Thread_Manager* thread_manager_p = NULL;
  const char** thread_names_p = NULL;
  char* thread_name_p = NULL;
  std::string buffer;
  std::ostringstream converter;
  ACE_THR_FUNC function_p =
      static_cast<ACE_THR_FUNC> (::session_setup_function);
  void* arg_p = &thread_data;
  int group_id = (COMMON_EVENT_THREAD_GROUP_ID + 1); // *TODO*
  ACE_thread_t* thread_ids_p = NULL;
  ACE_hthread_t* thread_handles_p = NULL;
  // *TODO*: use ACE_NEW_MALLOC_ARRAY (as soon as the NORETURN variant becomes
  //         available)
  ACE_NEW_NORETURN (thread_ids_p,
                    ACE_thread_t[thread_data.addresses->size ()]);
  if (!thread_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory(%u), aborting\n"),
                (sizeof (ACE_thread_t) * thread_data.addresses->size ())));
    goto error;
  } // end IF
  ACE_NEW_NORETURN (thread_handles_p,
                    ACE_hthread_t[thread_data.addresses->size ()]);
  if (!thread_handles_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory(%u), aborting\n"),
                (sizeof (ACE_hthread_t) * thread_data.addresses->size ())));
    goto error;
  } // end IF
//  ACE_OS::memset (thread_handles_p, 0, sizeof (thread_handles_p));
  // *TODO*: use ACE_NEW_MALLOC_ARRAY (as soon as the NORETURN variant becomes
  //         available)
  ACE_NEW_NORETURN (thread_names_p,
                    const char*[thread_data.addresses->size ()]);
  if (!thread_names_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory(%u), aborting\n"),
                (sizeof (char*) * thread_data.addresses->size ())));
    goto error;
  } // end IF
  ACE_OS::memset (thread_names_p, 0, sizeof (thread_names_p));
  for (unsigned int i = 0;
       i < thread_data.addresses->size ();
       i++)
  {
    thread_name_p = NULL;
    // *TODO*: use ACE_NEW_MALLOC_ARRAY (as soon as the NORETURN variant becomes
    //         available)
    ACE_NEW_NORETURN (thread_name_p,
                      char[BUFSIZ]);
    if (!thread_name_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, aborting\n")));
      goto error;
    } // end IF
    ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (i + 1);
    buffer = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_SESSION_THREAD_NAME);
    buffer += ACE_TEXT_ALWAYS_CHAR (" #");
    buffer += converter.str ();
    ACE_OS::strcpy (thread_name_p, buffer.c_str ());
    thread_names_p[i] = thread_name_p;
  } // end FOR
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  group_id =
    thread_manager_p->spawn_n (thread_ids_p,                   // id(s)
                               thread_data.addresses->size (), // # threads
                               function_p,                     // function
                               arg_p,                          // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),            // flags
                               ACE_DEFAULT_THREAD_PRIORITY,    // priority
                               group_id,                       // group id
                               NULL,                           // stack(s)
                               NULL,                           // stack size(s)
                               thread_handles_p,               // handle(s)
                               NULL,                           // task
                               thread_names_p);                // name(s)
  if (group_id == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn_n(%u): \"%m\", aborting\n"),
                thread_data.addresses->size ()));
    goto error;
  } // end IF

  // clean up
  if (thread_ids_p)
    delete [] thread_ids_p;
  delete [] thread_handles_p;
  for (unsigned int i = 0; i < thread_data.addresses->size (); i++)
    delete [] thread_names_p[i];
  delete [] thread_names_p;

  result = thread_manager_p->wait_grp (group_id); // name(s)
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", aborting\n"),
                thread_data.addresses->size ()));
    goto error;
  } // end IF

  return;

error:
  if (thread_ids_p)
    delete [] thread_ids_p;
  if (thread_handles_p)
    delete [] thread_handles_p;
  for (unsigned int j = 0; j < thread_data.addresses->size (); j++)
    delete [] thread_names_p[j];
  if (thread_names_p)
    delete [] thread_names_p;
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::notify (const struct BitTorrent_PeerHandShake& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (BitTorrent_Tools::HandShakeToString (record_in).c_str ())));
#endif
}
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::notify (const struct BitTorrent_PeerRecord& record_in,
                                          ACE_Message_Block* messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (BitTorrent_Tools::RecordToString (record_in).c_str ())));
#endif

  switch (record_in.type)
  {
    case BITTORRENT_MESSAGETYPE_PIECE:
    {
      // sanity check(s)
      ACE_ASSERT (messageBlock_in);

      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), returning\n"),
                  record_in.type));
      return;
    }
  } // end SWITCH
}

//////////////////////////////////////////

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::error (const struct BitTorrent_PeerRecord& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::error"));

  std::string message_text = BitTorrent_Tools::RecordToString (record_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received error message: \"%s\"\n"),
              ACE_TEXT (message_text.c_str ())));
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::log (const struct BitTorrent_PeerRecord& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::log"));

  std::string message_text = BitTorrent_Tools::RecordToString (record_in);
  log (std::string (), // --> server log
       message_text);
}
