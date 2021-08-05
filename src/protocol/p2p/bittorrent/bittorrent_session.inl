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

#include "common_file_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#ifdef HAVE_CONFIG_H
#include "ACENetwork_config.h"
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::BitTorrent_Session_T ()
#else
                     >::BitTorrent_Session_T ()
#endif // GUI_SUPPORT
 : inherited ()
 , logToFile_ (BITTORRENT_DEFAULT_SESSION_LOG)
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::~BitTorrent_Session_T ()
#else
                     >::~BitTorrent_Session_T ()
#endif // GUI_SUPPORT
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::initialize (const ConfigurationType& configuration_in)
#else
                     >::initialize (const ConfigurationType& configuration_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.controller);
  ACE_ASSERT (configuration_in.metaInfo);
  ACE_ASSERT (configuration_in.parserConfiguration);

  // *TODO*: remove type inferences
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, false);
    inherited::state_.metaInfo = configuration_in.metaInfo;
    // initialize piece data
    // *TODO*: load completed piece data from somewhere
    Bencoding_DictionaryIterator_t iterator =
        configuration_in.metaInfo->begin ();
    for (;
         iterator != configuration_in.metaInfo->end ();
         ++iterator)
      if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_KEY))
        break;
    ACE_ASSERT (iterator != configuration_in.metaInfo->end ());
    ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
    Bencoding_DictionaryIterator_t iterator_2 =
        (*iterator).second->dictionary->begin ();
    for (;
         iterator_2 != (*iterator).second->dictionary->end ();
         ++iterator_2)
      if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_PIECES_KEY))
        break;
    ACE_ASSERT (iterator_2 != (*iterator).second->dictionary->end ());
    ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
    ACE_ASSERT (!((*iterator_2).second->string->size () % BITTORRENT_PRT_INFO_PIECE_HASH_SIZE));
    std::string piece_hashes = *(*iterator_2).second->string;
    Bencoding_DictionaryIterator_t iterator_3 =
        (*iterator).second->dictionary->begin ();
    for (;
         iterator_3 != (*iterator).second->dictionary->end ();
         ++iterator_3)
      if (*(*iterator_3).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_PIECE_LENGTH_KEY))
        break;
    ACE_ASSERT (iterator_3 != (*iterator).second->dictionary->end ());
    ACE_ASSERT ((*iterator_3).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
    struct BitTorrent_Piece piece_s;
    unsigned int total_length =
        BitTorrent_Tools::MetaInfoToLength (*configuration_in.metaInfo);
    while (!piece_hashes.empty ())
    {
      piece_s.filename =
          Common_File_Tools::getTempFilename (ACE_TEXT_ALWAYS_CHAR (""), // no prefix
                                              false);                    // filename only
      ACE_ASSERT (!piece_s.filename.empty ());
      piece_s.hash =
          piece_hashes.substr (0, BITTORRENT_PRT_INFO_PIECE_HASH_SIZE);
      piece_hashes.erase (0, BITTORRENT_PRT_INFO_PIECE_HASH_SIZE);
      piece_s.length =
          (piece_hashes.empty () ? total_length % static_cast<unsigned int> ((*iterator_3).second->integer)
                                 : static_cast<unsigned int> ((*iterator_3).second->integer));
      piece_s.length =
        (piece_s.length ? piece_s.length
                        : static_cast<unsigned int> ((*iterator_3).second->integer));
      inherited::state_.pieces.push_back (piece_s);
    } // end WHILE
  } // end lock scope
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::connect (const ACE_INET_Addr& address_in)
#else
                     >::connect (const ACE_INET_Addr& address_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::connect"));

  // step0: subscribe to notifications
  PEERMODULEHANDLERCONFIGURATIONITERATOR_T iterator;
  PeerConnectionConfigurationType* configuration_p = NULL;
  PeerUserDataType* user_data_p = NULL;
  typename PeerStreamType::IDATA_NOTIFY_T* subscriber_p = NULL;
  bool clone_module = false;
//  bool delete_module = false;
  Stream_Module_t* module_p = NULL;
  inherited::CONNECTION_MANAGER_SINGLETON_T::instance ()->get (configuration_p,
                                                               user_data_p);
  ACE_ASSERT (configuration_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_p->streamConfiguration);

  // step1: set up configuration
  configuration_p->address = address_in;

  iterator =
      configuration_p->streamConfiguration->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_p->streamConfiguration->end ());
  subscriber_p = (*iterator).second.second.subscriber;
  (*iterator).second.second.subscriber = &peerStreamHandler_;

  clone_module =
    configuration_p->streamConfiguration->configuration->cloneModule;
  //delete_module =
  //  configuration_p->streamConfiguration->configuration->deleteModule;
  module_p =
    configuration_p->streamConfiguration->configuration->module;

  configuration_p->streamConfiguration->configuration->cloneModule =
    false;
  //configuration_p->streamConfiguration->configuration->deleteModule =
  //  false;
  configuration_p->streamConfiguration->configuration->module =
    peerHandlerModule_;

  // step1: (try to) connect
  inherited::connect (address_in);

  // step2: reset configuration
  ACE_ASSERT (configuration_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_p->streamConfiguration);

  ACE_ASSERT (iterator != configuration_p->streamConfiguration->end ());
  (*iterator).second.second.subscriber = subscriber_p;

  configuration_p->streamConfiguration->configuration->cloneModule =
    clone_module;
  //configuration_p->streamConfiguration->configuration->deleteModule =
  //  delete_module;
  configuration_p->streamConfiguration->configuration->module = module_p;
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::connect (Net_ConnectionId_t id_in)
#else
                     >::connect (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::connect"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration->messageAllocator);

  inherited::connect (id_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: new peer connection (id: %d)\n"),
              ACE_TEXT (ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
                                       ACE_DIRECTORY_SEPARATOR_CHAR)),
              id_in));

  BitTorrent_MessagePayload_Bitfield peer_pieces;
  struct BitTorrent_PeerStatus peer_status;
  struct BitTorrent_PeerHandShake* record_p = NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  typename PeerStreamType::MESSAGE_T* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ISTREAM_CONNECTION_T* istream_connection_p = NULL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    inherited::state_.peerPieces.insert (std::make_pair (id_in, peer_pieces));
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
    ACE_ASSERT (inherited::state_.metaInfo);
    record_p->info_hash =
        BitTorrent_Tools::MetaInfoToInfoHash (*inherited::state_.metaInfo);
    record_p->peer_id = inherited::state_.peerId;
  } // end lock scope

  if (!getConnectionAndMessage (id_in,
                                istream_connection_p,
                                message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to BitTorrent_Session_T::getConnectionAndMessage, returning\n")));
    goto error;
  } // end IF
  ACE_ASSERT (istream_connection_p); ACE_ASSERT (message_p);
  data_container_p =
      &const_cast<typename PeerStreamType::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
      &const_cast<struct BitTorrent_PeerMessageData&> (data_container_p->getR ());
  data_p->handShakeRecord = record_p;
  record_p = NULL;

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
  istream_connection_p->decrease ();

  return;

error:
  if (record_p)
    delete record_p;
  if (message_p)
    message_p->release ();
  if (istream_connection_p)
    istream_connection_p->decrease ();
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::disconnect (Net_ConnectionId_t id_in)
#else
                     >::disconnect (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::disconnect"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  inherited::disconnect (id_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: peer connection closed (id was: %d)\n"),
              ACE_TEXT (ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
                                       ACE_DIRECTORY_SEPARATOR_CHAR)),
              id_in));

  bool notify_controller_b = false;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    BitTorrent_PeerPiecesIterator_t iterator =
        inherited::state_.peerPieces.find (id_in);
    ACE_ASSERT (iterator != inherited::state_.peerPieces.end ());
    inherited::state_.peerPieces.erase (iterator);
    BitTorrent_PeerStatusIterator_t iterator_2 =
        inherited::state_.peerStatus.find (id_in);
    ACE_ASSERT (iterator_2 != inherited::state_.peerStatus.end ());
    inherited::state_.peerStatus.erase (iterator_2);
    if (unlikely (inherited::state_.connections.empty ()))
      notify_controller_b = true;
  } // end lock scope
  if (notify_controller_b)
  { ACE_ASSERT (inherited::configuration_->controller);
    try {
      inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                     BITTORRENT_EVENT_CANCELLED,
                                                     ACE_TEXT_ALWAYS_CHAR (""));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
    }
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::interested (Net_ConnectionId_t id_in)
#else
                     >::interested (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::interested"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration->allocatorConfiguration);

  struct BitTorrent_PeerRecord* record_p = NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  typename PeerStreamType::MESSAGE_T* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ISTREAM_CONNECTION_T* istream_connection_p = NULL;

  ACE_NEW_NORETURN (record_p,
                    struct BitTorrent_PeerRecord ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  record_p->type = BITTORRENT_MESSAGETYPE_INTERESTED;

  if (!getConnectionAndMessage (id_in,
                                istream_connection_p,
                                message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to BitTorrent_Session_T::getConnectionAndMessage, returning\n")));
    goto error;
  } // end IF
  ACE_ASSERT (istream_connection_p); ACE_ASSERT (message_p);
  data_container_p =
      &const_cast<typename PeerStreamType::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
      &const_cast<struct BitTorrent_PeerMessageData&> (data_container_p->getR ());
  data_p->peerRecord = record_p;
  record_p = NULL;

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

  // clean up
  istream_connection_p->decrease ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: interested in peer (id was: %d)...\n"),
              ACE_TEXT (ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
                                       ACE_DIRECTORY_SEPARATOR_CHAR)),
              id_in));

  return;

error:
  if (record_p)
    delete record_p;
  if (message_p)
    message_p->release ();
  if (istream_connection_p)
    istream_connection_p->decrease ();
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::request (Net_ConnectionId_t id_in,
#else
                     >::request (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                            unsigned int index_in,
                                            unsigned int begin_in,
                                            unsigned int length_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::request"));

  // sanity check(s)
  ACE_ASSERT (id_in);
  ACE_ASSERT (length_in);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration->allocatorConfiguration);

  struct BitTorrent_PeerRecord* record_p = NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  typename PeerStreamType::MESSAGE_T* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ISTREAM_CONNECTION_T* istream_connection_p = NULL;

  ACE_NEW_NORETURN (record_p,
                    struct BitTorrent_PeerRecord ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  record_p->type = BITTORRENT_MESSAGETYPE_REQUEST;
  record_p->request.index = index_in;
  record_p->request.begin = begin_in;
  record_p->request.length = length_in;

  if (!getConnectionAndMessage (id_in,
                                istream_connection_p,
                                message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to BitTorrent_Session_T::getConnectionAndMessage, returning\n")));
    goto error;
  } // end IF
  ACE_ASSERT (istream_connection_p); ACE_ASSERT (message_p);
  data_container_p =
      &const_cast<typename PeerStreamType::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
      &const_cast<struct BitTorrent_PeerMessageData&> (data_container_p->getR ());
  data_p->peerRecord = record_p;
  record_p = NULL;

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

  // clean up
  istream_connection_p->decrease ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: requested chunk (piece %u, begin %u, length %u) from peer (id was: %d)...\n"),
              ACE_TEXT (ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
                                       ACE_DIRECTORY_SEPARATOR_CHAR)),
              index_in,
              begin_in,
              length_in,
              id_in));

  return;

error:
  if (record_p)
    delete record_p;
  if (message_p)
    message_p->release ();
  if (istream_connection_p)
    istream_connection_p->decrease ();
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::scrape ()
#else
                     >::scrape ()
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::scrape"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->metaInfo);

  Bencoding_DictionaryIterator_t iterator;
  std::string host_name_string;
  std::string user_agent;
  Net_ConnectionId_t tracker_connection_id = 0;
  ITRACKER_CONNECTION_T* iconnection_p = NULL;
  ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
      NULL;
  typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  bool use_SSL = false;

  ACE_NEW_NORETURN (data_p,
                    typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF

  iterator = inherited::configuration_->metaInfo->begin ();
  for (;
       iterator != inherited::configuration_->metaInfo->end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY))
      break;
  ACE_ASSERT (iterator != inherited::configuration_->metaInfo->end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
  if (!HTTP_Tools::parseURL (*(*iterator).second->string,
                             host_name_string,
                             data_p->URI,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                ACE_TEXT (*(*iterator).second->string->c_str ())));
    goto error;
  } // end IF
  data_p->URI =
      BitTorrent_Tools::AnnounceURLToScrapeURL (data_p->URI);
  data_p->method = HTTP_Codes::HTTP_METHOD_GET;
  data_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER),
                                       HTTP_Tools::URLEncode (BitTorrent_Tools::MetaInfoToInfoHash (*inherited::configuration_->metaInfo))));
#if defined (HAVE_CONFIG_H)
  user_agent  = ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME);
  user_agent += ACE_TEXT_ALWAYS_CHAR ("/");
  user_agent += ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_VERSION);
#endif // HAVE_CONFIG_H
  if (!user_agent.empty ())
    data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_AGENT_STRING),
                                            user_agent));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                          host_name_string));
//                                            HTTP_Tools::IPAddress2HostName (tracker_address).c_str ()));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("*/*")));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_ENCODING_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("gzip;q=1.0, deflate, identity")));

  // *IMPORTANT NOTE*: fire-and-forget API (record_p)
  ACE_NEW_NORETURN (data_container_p,
                    typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T (data_p,
                                                                                        true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  data_p = NULL;

  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->trackerConnectionConfiguration);
  ACE_ASSERT (inherited::configuration_->trackerConnectionConfiguration->allocatorConfiguration);
allocate:
  message_p =
    static_cast<typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T*> (inherited::configuration_->trackerConnectionConfiguration->messageAllocator->malloc (sizeof (typename ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T)));
  // keep retrying ?
  if (!message_p &&
      !inherited::configuration_->trackerConnectionConfiguration->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate scrape message: \"%m\", returning\n")));
    data_container_p->decrease (); data_container_p = NULL;
    goto error;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (data_container_p)
  message_p->initialize (data_container_p,
                         message_p->sessionId (),
                         NULL);
  data_container_p = NULL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    ACE_ASSERT (inherited::state_.trackerConnectionId);
    tracker_connection_id = inherited::state_.trackerConnectionId;
  } // end lock scope
  iconnection_p =
      TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (tracker_connection_id);
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
  if (message_p)
    message_p->release ();
  else if (data_container_p)
    data_container_p->decrease ();
  else if (data_p)
    delete data_p;
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::trackerConnect (const ACE_INET_Addr& address_in)
#else
                     >::trackerConnect (const ACE_INET_Addr& address_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerConnect"));

  TrackerConnectorType connector (true);
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // step0: subscribe to notifications
  TRACKERMODULEHANDLERCONFIGURATIONITERATOR_T iterator;
  TrackerConnectionConfigurationType* configuration_p = NULL;
  TrackerUserDataType* user_data_p = NULL;
  typename TrackerStreamType::IDATA_NOTIFY_T* subscriber_p = NULL;
  bool clone_module = false;
  //bool delete_module = false;
  Stream_Module_t* module_p = NULL;
  TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (configuration_p,
                                                            user_data_p);
  ACE_ASSERT (configuration_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_p->streamConfiguration);

  // step1: set up configuration
  configuration_p->address = address_in;

  iterator =
      configuration_p->streamConfiguration->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_p->streamConfiguration->end ());
  subscriber_p = (*iterator).second.second.subscriber;
  ACE_ASSERT (!subscriber_p);
  (*iterator).second.second.subscriber = &trackerStreamHandler_;

  clone_module =
    configuration_p->streamConfiguration->configuration->cloneModule;
  //delete_module =
  //  configuration_p->streamConfiguration->configuration->deleteModule;
  module_p =
    configuration_p->streamConfiguration->configuration->module;
  ACE_ASSERT (!module_p);

  configuration_p->streamConfiguration->configuration->cloneModule =
    false;
  //configuration_p->streamConfiguration->configuration->deleteModule =
  //  false;
  configuration_p->streamConfiguration->configuration->module =
    trackerHandlerModule_;

  ACE_Time_Value deadline;
  ACE_Time_Value initialization_timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT_S,
                                         0);
  Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
  typename TrackerConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
      NULL;

  // step1: initialize connector
  typename TrackerConnectorType::ICONNECTOR_T* iconnector_p = &connector;
  typename TrackerConnectorType::ICONNECTION_T* iconnection_p = NULL;
  int result = -1;
  if (!iconnector_p->initialize (*inherited::configuration_->trackerConnectionConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
    goto error;
  } // end IF

  // step2: try to connect
  handle = iconnector_p->connect (address_in);
  if (handle == ACE_INVALID_HANDLE)
  { ACE_ASSERT (inherited::configuration_);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to tracker %s: \"%m\", returning\n"),
                ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    goto error;
  } // end IF
  if (inherited::isAsynch_)
  {
    deadline =
        (COMMON_TIME_NOW +
         ACE_Time_Value (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_S, 0));
    ACE_Time_Value delay (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_INTERVAL_S,
                          0);
    do
    {
      // *TODO*: this does not work
      iconnection_p =
          TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (address_in);
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
      TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
      TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (static_cast<Net_ConnectionId_t> (handle));
#endif // ACE_WIN32 || ACE_WIN64
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to tracker %s: \"%m\", returning\n"),
                ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
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

  iconnection_p->decrease (); iconnection_p = NULL;

error:
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->streamConfiguration);
  ACE_ASSERT (iterator != configuration_p->streamConfiguration->end ());
  (*iterator).second.second.subscriber = subscriber_p;
  configuration_p->streamConfiguration->configuration->cloneModule =
    clone_module;
  //configuration_p->streamConfiguration->configuration->deleteModule =
  //  delete_module;
  configuration_p->streamConfiguration->configuration->module =
    module_p;
  if (iconnection_p)
    iconnection_p->decrease ();
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::trackerConnect (Net_ConnectionId_t id_in)
#else
                     >::trackerConnect (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerConnect"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    if (inherited::state_.trackerConnectionId)
    {
      typename TrackerConnectorType::ICONNECTION_T* iconnection_p =
        TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (inherited::state_.trackerConnectionId);
      if (!iconnection_p)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve tracker connection handle (id was: %d), continuing\n"),
                    inherited::state_.trackerConnectionId));
      else
      {
        iconnection_p->close ();
        iconnection_p->decrease (); iconnection_p= NULL;
      } // end ELSE
    } // end IF
    inherited::state_.trackerConnectionId = id_in;
  } // end lock scope

  inherited::connect (id_in);

  ACE_ASSERT (inherited::configuration_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: new tracker connection (id: %d)\n"),
              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::trackerDisconnect (Net_ConnectionId_t id_in)
#else
                     >::trackerDisconnect (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerDisconnect"));

  inherited::disconnect (id_in);

  ACE_ASSERT (inherited::configuration_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: tracker connection closed (id was: %d)\n"),
              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in));

  bool notify_controller_b = false;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    if (inherited::state_.trackerConnectionId == id_in)
      inherited::state_.trackerConnectionId = 0;
    if (inherited::state_.connections.empty ())
      notify_controller_b = true;
  } // end lock scope
  if (notify_controller_b)
  { ACE_ASSERT (inherited::configuration_->controller);
    try {
      inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                     BITTORRENT_EVENT_CANCELLED,
                                                     ACE_TEXT_ALWAYS_CHAR (""));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
    }
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::trackerRedirect (Net_ConnectionId_t id_in,
#else
                     >::trackerRedirect (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                         const std::string& location_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerRedirect"));

  ACE_ASSERT (inherited::configuration_->controller);
  try {
    inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                   BITTORRENT_EVENT_TRACKER_REDIRECTED,
                                                   location_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
  }
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::trackerError (Net_ConnectionId_t id_in,
#else
                     >::trackerError (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                      const struct HTTP_Record& response_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerError"));

  ACE_ASSERT (inherited::configuration_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: tracker error (id was: %d): %s\n"),
              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in,
              ACE_TEXT (HTTP_Tools::dump (response_in).c_str ())));

  ACE_ASSERT (inherited::configuration_->controller);
  try {
    inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                   BITTORRENT_EVENT_CANCELLED,
                                                   ACE_TEXT_ALWAYS_CHAR (""));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
  }
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::notify (const Bencoding_Dictionary_t& record_in)
#else
                     >::notify (const Bencoding_Dictionary_t& record_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

//#if defined (_DEBUG)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s\n"),
//              ACE_TEXT (BitTorrent_Tools::Dictionary2String (record_in).c_str ())));
//#endif

  // *NOTE*: this could be the response to either a response or a 'scrape', the
  //         type can be deduced from the dictionary schema
  Bencoding_DictionaryIterator_t iterator = record_in.begin ();
  for (;
       iterator != record_in.end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_SCRAPE_RESPONSE_FILES_HEADER))
      break;
  if (iterator != record_in.end ())
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    if (inherited::state_.trackerScrapeResponse)
      Common_Parser_Bencoding_Tools::free (inherited::state_.trackerScrapeResponse);
    inherited::state_.trackerScrapeResponse =
        &const_cast<Bencoding_Dictionary_t&> (record_in);
    return;
  } // end IF

  // response is regular --> (try to) connect to all peers

  // retain tracker id for future notifications
  iterator = record_in.begin ();
    for (;
         iterator != record_in.end ();
         ++iterator)
      if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_TRACKERID_HEADER))
        break;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    if (iterator != record_in.end ())
    { ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
      if (inherited::state_.trackerId.empty ())
        inherited::state_.trackerId = *(*iterator).second->string;
      else if (unlikely (inherited::state_.trackerId != *(*iterator).second->string))
      { // update
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("tracked id changed (is: %s, was: %s), updating\n"),
                    ACE_TEXT ((*iterator).second->string->c_str ()),
                    ACE_TEXT (inherited::state_.trackerId.c_str ())));
        inherited::state_.trackerId = *(*iterator).second->string;
      } // end ELSE
    } // end IF
    if (inherited::state_.trackerRequestResponse)
      Common_Parser_Bencoding_Tools::free (inherited::state_.trackerRequestResponse);
    inherited::state_.trackerRequestResponse =
        &const_cast<Bencoding_Dictionary_t&> (record_in);
  } // end lock scope

  BitTorrent_PeerAddresses_t peer_addresses;
  int result = -1;
  ACE_INET_Addr inet_address;

  iterator = record_in.begin ();
  for (;
       iterator != record_in.end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_HEADER))
      break;
  ACE_ASSERT (iterator != record_in.end ());
  // can be 'dictionary' or 'binary' model
  if ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY)
  { // *TODO*
    ACE_ASSERT (false);
    ACE_NOTSUP;

    ACE_NOTREACHED (return;)
  } // end IF
  else
  { ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
    const char* char_p = (*iterator).second->string->c_str ();

    // *IMPORTANT NOTE*: the 'compact' 'peers' representation is assumed here
    //                   (see also: http://www.bittorrent.org/beps/bep_0023.html)
    for (unsigned int i = 0;
         i < ((*iterator).second->string->size () / 6);
         ++i)
    {
      result = inet_address.set (*reinterpret_cast<const u_short*> (char_p + 4),
                                 *reinterpret_cast<const ACE_UINT32*> (char_p),
                                 0, // already in network byte order
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
  ACE_ASSERT (inherited::configuration_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: connecting to %u peer(s)...\n"),
              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              peer_addresses.size ()));

  //std::uniform_int_distribution<int> random_distribution (1, peer_addresses.size ());
  //std::default_random_engine         random_engine;
  //std::function<int ()>              random_generator =
  //    std::bind (random_distribution, random_engine);
  //int random_number = random_generator ();
  //BitTorrent_PeerAddressesIterator_t iterator_2 = peer_addresses.begin ();
  //iterator_2 += random_number;
  //inet_address = *iterator_2;
  //peer_addresses.clear ();
  //peer_addresses.push_back (inet_address);

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
      static_cast<ACE_THR_FUNC> (::net_bittorrent_session_setup_function);
  void* arg_p = &thread_data;
  int group_id_i = -1;
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
    ACE_OS::memset (thread_name_p, 0, sizeof (char[BUFSIZ]));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (i + 1);
    buffer = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_SESSION_HANDLER_THREAD_NAME);
    buffer += ACE_TEXT_ALWAYS_CHAR (" #");
    buffer += converter.str ();
    ACE_OS::strcpy (thread_name_p, buffer.c_str ());
    thread_names_p[i] = thread_name_p;
  } // end FOR
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  group_id_i =
    thread_manager_p->spawn_n (thread_ids_p,                   // id(s)
                               thread_data.addresses->size (), // # threads
                               function_p,                     // function
                               arg_p,                          // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),            // flags
                               ACE_DEFAULT_THREAD_PRIORITY,    // priority
                               BITTORRENT_SESSION_HANDLER_THREAD_GROUP_ID, // group id
                               NULL,                           // stack(s)
                               NULL,                           // stack size(s)
                               thread_handles_p,               // handle(s)
                               NULL,                           // task
                               thread_names_p);                // name(s)
  if (group_id_i == -1)
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

  result = thread_manager_p->wait_grp (group_id_i);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", aborting\n"),
                group_id_i));
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::notify (Net_ConnectionId_t id_in,
#else
                     >::notify (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                           const struct BitTorrent_PeerHandShake& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%u: %s\n"),
              id_in,
              ACE_TEXT (BitTorrent_Tools::HandShakeToString (record_in).c_str ())));
#endif // _DEBUG

  // compare handshake peer id with tracker peer id
  ACE_ASSERT (inherited::state_.trackerRequestResponse);
  Bencoding_DictionaryIterator_t iterator =
      inherited::state_.trackerRequestResponse->begin ();
  for (;
       iterator != inherited::state_.trackerRequestResponse->end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_HEADER))
      break;
  ACE_ASSERT (iterator != inherited::state_.trackerRequestResponse->end ());
  if ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY)
  {
    Bencoding_DictionaryIterator_t iterator2 =
        (*iterator).second->dictionary->begin ();
    for (;
         iterator2 != (*iterator).second->dictionary->end ();
         ++iterator2)
      if (*(*iterator2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_PEERID_HEADER))
      { // *TODO*: compare IP addresses to determine corresponding peer_id
        break;
      } // end IF
    ACE_ASSERT (iterator2 != (*iterator).second->dictionary->end ());
    ACE_ASSERT ((*iterator2).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
    if (record_in.peer_id != *(*iterator2).second->string)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("peer-ids do not match: \"%s\" != \"%s\", continuing\n"),
                  ACE_TEXT (record_in.peer_id.c_str ()),
                  ACE_TEXT ((*iterator2).second->string->c_str ())));
    } // end IF
  } // end IF
  else
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%u: tracker response had binary \"peers\" model: cannot validate handshake peer_id (was: \"%s\"), continuing\n"),
                id_in,
                ACE_TEXT (record_in.peer_id.c_str ())));
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::notify (Net_ConnectionId_t id_in,
#else
                     >::notify (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                           const struct BitTorrent_PeerRecord& record_in,
                                           ACE_Message_Block* messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%u: %s\n"),
              id_in,
              ACE_TEXT (BitTorrent_Tools::RecordToString (record_in).c_str ())));
#endif // _DEBUG

  if (!record_in.length)
    return;
  switch (record_in.type)
  {
    case BITTORRENT_MESSAGETYPE_UNCHOKE:
    {
      BitTorrent_PeerStatusIterator_t iterator;
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
      iterator = inherited::state_.peerStatus.find (id_in);
      ACE_ASSERT (iterator != inherited::state_.peerStatus.end ());
      (*iterator).second.am_choking = false;
      requestNextPiece (id_in);
      break;
    }
    case BITTORRENT_MESSAGETYPE_HAVE:
    {
      BitTorrent_PeerPiecesIterator_t iterator;
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
      iterator = inherited::state_.peerPieces.find (id_in);
      ACE_ASSERT (iterator != inherited::state_.peerPieces.end ());
      if ((*iterator).second.empty ())
      { // protocol error: no bitfield message received yet ?
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%u: populating pieces bitfield...\n"),
                    id_in));
        Bencoding_DictionaryIterator_t iterator_2 =
            inherited::configuration_->metaInfo->begin ();
        for (;
             iterator_2 != inherited::configuration_->metaInfo->end ();
             ++iterator_2)
          if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_PIECES_KEY))
            break;
        ACE_ASSERT (iterator_2 != inherited::configuration_->metaInfo->end ());
        ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
        ACE_ASSERT (!((*iterator_2).second->string->size () % BITTORRENT_PRT_INFO_PIECE_HASH_SIZE));
        unsigned int pieces =
            (*iterator_2).second->string->size () / BITTORRENT_PRT_INFO_PIECE_HASH_SIZE;
        for (unsigned int i = 0;
             i < pieces;
             ++i)
          (*iterator).second.push_back (0);
      } // end IF
      unsigned int index = record_in.have / (sizeof (ACE_UINT8) * 8);
      unsigned int index_2 = record_in.have % (sizeof (ACE_UINT8) * 8);
      ACE_ASSERT (!((*iterator).second[index] & (0x80 >> index_2)));
      (*iterator).second[index] |= (0x80 >> index_2);

      // send 'interested' ?
      BitTorrent_PeerStatusIterator_t iterator_2;
      iterator_2 = inherited::state_.peerStatus.find (id_in);
      ACE_ASSERT (iterator_2 != inherited::state_.peerStatus.end ());
      if (!(*iterator_2).second.am_interested &&
          BitTorrent_Tools::hasMissingPiece (inherited::state_.pieces,
                                             (*iterator).second))
      {
        interested (id_in);
        (*iterator_2).second.am_interested = true;
      } // end IF
      break;
    }
    case BITTORRENT_MESSAGETYPE_BITFIELD:
    {
      BitTorrent_PeerPiecesIterator_t iterator;
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
      iterator = inherited::state_.peerPieces.find (id_in);
      ACE_ASSERT (iterator != inherited::state_.peerPieces.end ());
      ACE_ASSERT ((*iterator).second.empty ());
      (*iterator).second = record_in.bitfield;

      // send 'interested' ?
      BitTorrent_PeerStatusIterator_t iterator_2;
      iterator_2 = inherited::state_.peerStatus.find (id_in);
      ACE_ASSERT (iterator_2 != inherited::state_.peerStatus.end ());
      if (!(*iterator_2).second.am_interested &&
          BitTorrent_Tools::hasMissingPiece (inherited::state_.pieces,
                                             (*iterator).second))
      {
        interested (id_in);
        (*iterator_2).second.am_interested = true;
      } // end IF
      break;
    }
    case BITTORRENT_MESSAGETYPE_PIECE:
    { ACE_ASSERT (messageBlock_in);
      struct BitTorrent_Piece_Chunk chunk_s;
      chunk_s.data = messageBlock_in;
      chunk_s.offset = record_in.piece.begin;
      BitTorrent_PiecesIterator_t iterator;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        iterator = inherited::state_.pieces.begin ();
        std::advance (iterator, record_in.piece.index);
        (*iterator).chunks.push_back (chunk_s);
        if (unlikely (BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                                         (*iterator).chunks)))
        {
          if (!BitTorrent_Tools::savePiece (ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                                                                 ACE_DIRECTORY_SEPARATOR_CHAR)),
                                            record_in.piece.index,
                                            *iterator))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to BitTorrent_Tools::savePiece(%s,%u), continuing\n"),
                        ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                       ACE_DIRECTORY_SEPARATOR_CHAR),
                        record_in.piece.index));
            // --> download again ?! *TODO*: handle case where the chunks are still available
          } // end IF
          else
          {
            for (iterator = inherited::state_.pieces.begin ();
                 iterator != inherited::state_.pieces.end ();
                 ++iterator)
              if (!BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                                      (*iterator).chunks))
                goto request_next;
            // all complete !

            // assemble the file(s) from the downloaded piece(s)
            if (!BitTorrent_Tools::assembleFiles (ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                                                                       ACE_DIRECTORY_SEPARATOR_CHAR)),
                                                  *inherited::configuration_->metaInfo))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to BitTorrent_Tools::assembleFiles(), continuing\n"),
                          ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                         ACE_DIRECTORY_SEPARATOR_CHAR)));
              // --> download again ?!
              break;
            } // end IF
            goto notify;
          } // end ELSE IF
        } // end IF
      } // end lock scope
request_next:
      requestNextPiece (id_in);
      break;
notify:
      ACE_ASSERT (inherited::configuration_->controller);
      try {
        inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                       BITTORRENT_EVENT_COMPLETE,
                                                       ACE_TEXT_ALWAYS_CHAR (""));
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
      }
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::error (const struct BitTorrent_PeerRecord& record_in)
#else
                     >::error (const struct BitTorrent_PeerRecord& record_in)
#endif // GUI_SUPPORT
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::log (const struct BitTorrent_PeerRecord& record_in)
#else
                     >::log (const struct BitTorrent_PeerRecord& record_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::log"));

  std::string message_text = BitTorrent_Tools::RecordToString (record_in);
  message_text += '\n';
  log (std::string (), // --> server log
       message_text);
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::getConnectionAndMessage (Net_ConnectionId_t id_in,
#else
                     >::getConnectionAndMessage (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                                            ISTREAM_CONNECTION_T*& connection_out,
                                                            typename PeerStreamType::MESSAGE_T*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::getConnectionAndMessage"));

  // sanity check(s)
  ACE_ASSERT (id_in);
  ACE_ASSERT (!connection_out);
  ACE_ASSERT (!message_out);

  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  ICONNECTION_T* iconnection_p = NULL;

  ACE_NEW_NORETURN (data_p,
                    struct BitTorrent_PeerMessageData ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    return false;
  } // end IF

  // *IMPORTANT NOTE*: fire-and-forget API (data_p)
  ACE_NEW_NORETURN (data_container_p,
                    typename PeerStreamType::MESSAGE_T::DATA_T (data_p,
                                                                true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    goto error;
  } // end IF
  data_p = NULL;

  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration);
  ACE_ASSERT (inherited::configuration_->connectionConfiguration->allocatorConfiguration);

allocate:
  message_out =
    static_cast<typename PeerStreamType::MESSAGE_T*> (inherited::configuration_->connectionConfiguration->messageAllocator->malloc (inherited::configuration_->connectionConfiguration->allocatorConfiguration->defaultBufferSize));
  // keep retrying ?
  if (!message_out &&
      !inherited::configuration_->connectionConfiguration->messageAllocator->block ())
    goto allocate;
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate peer stream message: \"%m\", aborting\n")));
    goto error;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (data_container_p)
  message_out->initialize (data_container_p,
                           message_out->sessionId (),
                           NULL);
  data_container_p = NULL;

  iconnection_p =
      inherited::CONNECTION_MANAGER_SINGLETON_T::instance ()->get (id_in);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve connection handle (id was: %d), aborting\n"),
                id_in));
    goto error;
  } // end IF
  connection_out = dynamic_cast<ISTREAM_CONNECTION_T*> (iconnection_p);
  if (!connection_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), aborting\n"),
                iconnection_p));
    iconnection_p->decrease (); iconnection_p = NULL;
    goto error;
  } // end IF

  return true;

error:
  if (message_out)
  {
    message_out->release (); message_out = NULL;
  } // end IF
  else if (data_container_p)
    data_container_p->decrease ();
  else if (data_p)
    delete data_p;
  if (connection_out)
  {
    connection_out->decrease (); connection_out = NULL;
  } // end IF

  return false;
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
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
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
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::requestNextPiece (Net_ConnectionId_t id_in)
#else
                     >::requestNextPiece (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::requestNextPiece"));

  BitTorrent_PeerPiecesIterator_t iterator =
      inherited::state_.peerPieces.find (id_in);
  ACE_ASSERT (iterator != inherited::state_.peerPieces.end ());
  BitTorrent_PiecesIterator_t iterator_2 = inherited::state_.pieces.begin ();
  for (;
       iterator_2 != inherited::state_.pieces.end ();
       ++iterator_2)
  {
    if (!BitTorrent_Tools::isPieceComplete ((*iterator_2).length,
                                            (*iterator_2).chunks) &&
        BitTorrent_Tools::havePiece (std::distance (inherited::state_.pieces.begin (), iterator_2),
                                     (*iterator).second))
      break;
  } // end FOR
  ACE_ASSERT (iterator_2 != inherited::state_.pieces.end ());
  struct bittorrent_piece_chunks_less less_s;
  std::sort ((*iterator_2).chunks.begin (),
             (*iterator_2).chunks.end (),
             less_s);
  unsigned int offset = 0, length = 0;
  BitTorrent_PieceChunksIterator_t iterator_3 =
      (*iterator_2).chunks.begin ();
  for (;
       iterator_3 != (*iterator_2).chunks.end ();
       ++iterator_3)
  {
    if ((*iterator_3).offset != (offset + length))
      break;
    offset = (*iterator_3).offset;
    length = BitTorrent_Tools::chunkLength (*iterator_3);
  } // end FOR
  offset = offset + length;
  unsigned int missing_data =
      static_cast<unsigned int> (BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX);
  if (iterator_3 != (*iterator_2).chunks.end ())
    missing_data = (*iterator_3).offset - offset;
  length =
    std::min (missing_data,
              static_cast<unsigned int> (BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX));
  request (id_in,
           std::distance (inherited::state_.pieces.begin (), iterator_2),
           offset,
           length);
}
