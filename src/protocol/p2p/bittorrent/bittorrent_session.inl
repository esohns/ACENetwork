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

#include <algorithm>
#include <functional>
#include <random>
#include <vector>

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_parser_common.h"

#include "common_timer_manager_common.h"

#include "net_defines.h"
#include "net_macros.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_client_common_tools.h"
#include "net_client_defines.h"

#include "http_tools.h"

#include "bittorrent_defines.h"
#include "bittorrent_network.h"
#include "bittorrent_tools.h"

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::BitTorrent_Session_T ()
#else
                     >::BitTorrent_Session_T ()
#endif // GUI_SUPPORT
 : inherited ()
 , inherited2 (this,  // handler
               false) // one-shot ?
 , logToFile_ (BITTORRENT_DEFAULT_SESSION_LOG)
 , peerHandlerModule_ (NULL)
 , trackerHandlerModule_ (NULL)
 , timerId_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::BitTorrent_Session_T"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    inherited::state_.peerId = BitTorrent_Tools::generatePeerId ();
  } // end lock scope
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  if (timerId_ != -1)
  {
    int result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerId_,
                                                                NULL);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel interval timer (id: %d): \"%m\", continuing\n"),
                  timerId_));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled interval timer (id: %d)\n"),
                  timerId_));
  } // end IF

  if (peerHandlerModule_)
    delete peerHandlerModule_;
  if (trackerHandlerModule_)
    delete trackerHandlerModule_;
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::handle (const void* ACT_in)
#else
                     >::handle (const void* ACT_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::handle"));

  ACE_UNUSED_ARG (ACT_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->controller);

  try {
    inherited::configuration_->controller->notifyTracker (inherited::configuration_->metaInfoFileName,
                                                          BITTORRENT_EVENT_TRACKER_REREQUEST);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::notifyTracker(), continuing\n")));
  }
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
bool
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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
  ACE_ASSERT (configuration_in.parserConfiguration);
  ACE_ASSERT (configuration_in.metaInfo);
//  ACE_ASSERT (configuration_in.peerModuleHandlerConfiguration);
//  ACE_ASSERT (configuration_in.peerStreamConfiguration);
//  ACE_ASSERT (configuration_in.trackerConnectionConfiguration);
//  ACE_ASSERT (configuration_in.trackerModuleHandlerConfiguration);
//  ACE_ASSERT (configuration_in.trackerStreamConfiguration);

  // *TODO*: remove type inferences
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, false);
    inherited::state_.metaInfo = configuration_in.metaInfo;
    // initialize piece data
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
      ACE_OS::memcpy (piece_s.hash,
                      piece_hashes.data (),
                      BITTORRENT_PRT_INFO_PIECE_HASH_SIZE);
      piece_hashes.erase (0, BITTORRENT_PRT_INFO_PIECE_HASH_SIZE);
      piece_s.length =
          (piece_hashes.empty () ? total_length % static_cast<unsigned int> ((*iterator_3).second->integer)
                                 : static_cast<unsigned int> ((*iterator_3).second->integer));
      piece_s.length =
        (piece_s.length ? piece_s.length
                        : static_cast<unsigned int> ((*iterator_3).second->integer));
      inherited::state_.pieces.push_back (piece_s);
    } // end WHILE

    // load existing pieces (to memory ?)
    if (!BitTorrent_Tools::loadPieces (configuration_in.metaInfoFileName,
                                       inherited::state_.pieces,
                                       false)) // don't waste memory
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to BitTorrent_Tools::loadPieces(\"%s\"), aborting\n"),
                  ACE_TEXT (configuration_in.metaInfoFileName.c_str ())));
      return false;
    } // end IF
    // notify existing pieces to event subscriber
    if (configuration_in.subscriber)
    {
      unsigned int index_i = 0;
      for (BitTorrent_PiecesIterator_t iterator = inherited::state_.pieces.begin ();
           iterator != inherited::state_.pieces.end ();
           ++iterator, ++index_i)
        if (!(*iterator).chunks.empty ())
          configuration_in.subscriber->pieceComplete (index_i);
    } // end IF

    ACE_ASSERT (!inherited::state_.peerStreamHandler);
#if defined (GUI_SUPPORT)
    ACE_NEW_NORETURN (inherited::state_.peerStreamHandler,
                      PeerStreamHandlerType (this, static_cast<CBDataType*> (configuration_in.CBData)));
#else
    ACE_NEW_NORETURN (inherited::state_.peerStreamHandler,
                      PeerStreamHandlerType (this));
#endif // GUI_SUPPORT
    ACE_ASSERT (inherited::state_.peerStreamHandler);

    ACE_NEW_NORETURN (peerHandlerModule_,
                      PEER_MESSAGEHANDLER_MODULE_T (NULL,
                                                    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_HANDLER_MODULE_NAME)));
    if (!peerHandlerModule_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      return false;
    } // end IF

    ACE_ASSERT (!inherited::state_.trackerStreamHandler);
#if defined (GUI_SUPPORT)
    ACE_NEW_NORETURN (inherited::state_.trackerStreamHandler,
                      TrackerStreamHandlerType (this, static_cast<CBDataType*> (configuration_in.CBData)));
#else
    ACE_NEW_NORETURN (inherited::state_.trackerStreamHandler,
                      TrackerStreamHandlerType (this));
#endif // GUI_SUPPORT
    ACE_ASSERT (inherited::state_.trackerStreamHandler);

    ACE_NEW_NORETURN (trackerHandlerModule_,
                      TRACKER_MESSAGEHANDLER_MODULE_T (NULL,
                                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_HANDLER_MODULE_NAME)));
    if (!trackerHandlerModule_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end lock scope

  if (!inherited::state_.trackerStreamHandler->initialize (*static_cast<struct Common_FlexBisonParserConfiguration*> (configuration_in.parserConfiguration),
                                                           configuration_in.trackerConnectionConfiguration->messageAllocator))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_TrackerStreamHandler_T::initialize(), aborting\n")));
    return false;
  } // end IF

  return inherited::initialize (configuration_in);
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  PeerConnectionConfigurationType* configuration_p = NULL;
  PeerUserDataType* user_data_p = NULL;
  typename PeerStreamType::CONFIGURATION_T::ITERATOR_T iterator;

  inherited::CONNECTION_MANAGER_SINGLETON_T::instance ()->get (configuration_p,
                                                               user_data_p);

  // sanity check(s)
  ACE_ASSERT (configuration_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_p->streamConfiguration);
  ACE_ASSERT (configuration_p->streamConfiguration->configuration_);

  // step1: set up configuration
  configuration_p->socketConfiguration.address = address_in;

  iterator =
      configuration_p->streamConfiguration->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_p->streamConfiguration->end ());
  (*iterator).second.second->subscriber = inherited::state_.peerStreamHandler;

  configuration_p->streamConfiguration->configuration_->cloneModule =
    false;
  configuration_p->streamConfiguration->configuration_->module =
    peerHandlerModule_;

  // step2: (try to) connect
  inherited::connect (address_in);
};

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  inherited::connect (id_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: new peer connection (id: %d)\n"),
              ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in));

  struct BitTorrent_PeerState peer_state_s;
  struct BitTorrent_PeerHandShake* record_p = NULL;
  struct BitTorrent_PeerRecord* record_2 = NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  typename PeerStreamType::MESSAGE_T* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ISTREAM_CONNECTION_T* istream_connection_p = NULL;
  std::vector<unsigned int> piece_indexes_a;
  bool send_bitfield_b = false;
  unsigned int index_i = 0, offset_i = 0;
  ACE_UINT32 peer_address_i = connectionIdToPeerAddress (id_in);
  bool populate_pieces_bitfield_b = false;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    BitTorrent_PeerStateIterator_t iterator =
      inherited::state_.peerState.find (peer_address_i);
    if (iterator == inherited::state_.peerState.end ())
    {
      std::pair<BitTorrent_PeerStateIterator_t, bool> result_s =
        inherited::state_.peerState.insert (std::make_pair (peer_address_i, peer_state_s));
      ACE_ASSERT (result_s.second);
      iterator = result_s.first;
    } // end IF
    ACE_ASSERT (iterator != inherited::state_.peerState.end ());
    (*iterator).second.connections.push_back (id_in);
  
    if ((*iterator).second.pieces.empty ())
      populate_pieces_bitfield_b = true;
  } // end lock scope
  if (unlikely (populate_pieces_bitfield_b))
    populatePeerPiecesBitfield (id_in);

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
    ACE_DEBUG ((LM_ERROR,
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

  // step2: send bitfield immediately ?
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    piece_indexes_a =
      BitTorrent_Tools::getPieceIndexes (inherited::state_.pieces,
                                         false);
    send_bitfield_b =
      !piece_indexes_a.empty () &&
      inherited::configuration_->sendBitfieldAfterHandshake;
  } // end lock scope
  if (!send_bitfield_b)
    goto continue_;

  ACE_NEW_NORETURN (record_2,
                    struct BitTorrent_PeerRecord ());
  if (!record_2)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  record_2->type = BITTORRENT_MESSAGETYPE_BITFIELD;
  record_2->bitfield.insert (record_2->bitfield.begin (),
                             inherited::state_.pieces.size (),
                             0);
  for (std::vector<unsigned int>::const_iterator iterator = piece_indexes_a.begin ();
       iterator != piece_indexes_a.end ();
       ++iterator)
  {
    index_i = (*iterator) / (sizeof (ACE_UINT8) * 8);
    offset_i = (*iterator) % (sizeof (ACE_UINT8) * 8);
    record_2->bitfield[index_i] |= (0x80 >> offset_i);
  } // end FOR

  if (!getConnectionAndMessage (id_in,
                                istream_connection_p,
                                message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Session_T::getConnectionAndMessage, returning\n")));
    goto error;
  } // end IF
  ACE_ASSERT (istream_connection_p); ACE_ASSERT (message_p);
  data_container_p =
      &const_cast<typename PeerStreamType::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
      &const_cast<struct BitTorrent_PeerMessageData&> (data_container_p->getR ());
  data_p->peerRecord = record_2;
  record_2 = NULL;

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

continue_:
  // clean up
  istream_connection_p->decrease ();

  return;

error:
  if (record_p)
    delete record_p;
  if (record_2)
    delete record_2;
  if (message_p)
    message_p->release ();
  if (istream_connection_p)
    istream_connection_p->decrease ();
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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
              ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in));

  ACE_UINT32 peer_address_i = connectionIdToPeerAddress (id_in);

  bool notify_controller_b = false;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    BitTorrent_PeerStateIterator_t iterator =
      inherited::state_.peerState.find (peer_address_i);
    ACE_ASSERT (iterator != inherited::state_.peerState.end ());
    std::vector<Net_ConnectionId_t>::iterator iterator_2 =
      std::find ((*iterator).second.connections.begin (), (*iterator).second.connections.end (),
                 id_in);
    ACE_ASSERT (iterator_2 != (*iterator).second.connections.end ());
    (*iterator).second.connections.erase (iterator_2);
    if ((*iterator).second.connections.empty ())
    { // the last connection to this peer has closed; erase peer state
      inherited::state_.peerState.erase (iterator);
    } // end IF

    if (unlikely (inherited::state_.connections.empty ()))
      notify_controller_b = true;
  } // end lock scope
  if (notify_controller_b)
  { ACE_ASSERT (inherited::configuration_->controller);
    try {
      inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                     BITTORRENT_EVENT_NO_MORE_PEERS,
                                                     ACE_TEXT_ALWAYS_CHAR (""));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
    }
  } // end IF
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::choke (Net_ConnectionId_t id_in,
#else
                     >::choke (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                          bool choke_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::choke"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

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
  record_p->type = (choke_in ? BITTORRENT_MESSAGETYPE_CHOKE
                             : BITTORRENT_MESSAGETYPE_UNCHOKE);

  if (!getConnectionAndMessage (id_in,
                                istream_connection_p,
                                message_p))
  {
    ACE_DEBUG ((LM_ERROR,
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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::interested (Net_ConnectionId_t id_in,
#else
                     >::interested (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                               bool interested_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::interested"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

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
  record_p->type = (interested_in ? BITTORRENT_MESSAGETYPE_INTERESTED
                                  : BITTORRENT_MESSAGETYPE_NOT_INTERESTED);

  if (!getConnectionAndMessage (id_in,
                                istream_connection_p,
                                message_p))
  {
    ACE_DEBUG ((LM_ERROR,
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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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
    ACE_DEBUG ((LM_ERROR,
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

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: requested chunk (piece %u, begin %u, length %u) from peer (id was: %d)...\n"),
  //            ACE::basename (inherited::configuration_->metaInfoFileName.c_str (), ACE_DIRECTORY_SEPARATOR_CHAR),
  //            index_in,
  //            begin_in,
  //            length_in,
  //            id_in));

  return;

error:
  if (record_p)
    delete record_p;
  if (message_p)
    message_p->release ();
  if (istream_connection_p)
    istream_connection_p->decrease ();
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::piece (Net_ConnectionId_t id_in,
#else
                     >::piece (Net_ConnectionId_t id_in,
#endif // GUI_SUPPORT
                                          unsigned int index_in,
                                          unsigned int begin_in,
                                          unsigned int length_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::piece"));

  // sanity check(s)
  ACE_ASSERT (id_in);
  ACE_ASSERT (length_in);

  struct BitTorrent_PeerRecord* record_p = NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  typename PeerStreamType::MESSAGE_T* message_p = NULL;
  ISTREAM_CONNECTION_T* istream_connection_p = NULL;
  unsigned int missing_bytes = length_in, offset = 0;
  BitTorrent_PiecesIterator_t iterator;
  ACE_Message_Block* message_block_p = NULL, *message_block_2 = NULL, *message_block_3 = NULL;
  BitTorrent_PieceChunksConstIterator_t iterator_2;

  ACE_NEW_NORETURN (record_p,
                    struct BitTorrent_PeerRecord ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  record_p->type = BITTORRENT_MESSAGETYPE_PIECE;
  record_p->piece.index = index_in;
  record_p->piece.begin = begin_in;

  if (!getConnectionAndMessage (id_in,
                                istream_connection_p,
                                message_p))
  {
    ACE_DEBUG ((LM_ERROR,
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

  // attach chunk
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    iterator = inherited::state_.pieces.begin ();
    std::advance (iterator, index_in);
    // sanity check(s)
    if ((*iterator).onDisk)
    { // *TODO*: flush chunks where the corresponding piece has the 'onDisk'
      //         flag set
      if (!BitTorrent_Tools::loadPiece (inherited::configuration_->metaInfoFileName,
                                        index_in,
                                        *iterator))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to BitTorrent_Tools::loadPiece (%s,%u), returning\n"),
                    ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
                                   ACE_DIRECTORY_SEPARATOR_CHAR),
                    index_in));
        goto error;
      } // end IF
    } // end IF
    else if (unlikely (!BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                                           (*iterator).chunks)))
    { // *TODO*: try to satisfy request even if piece is incomplete ?
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("requested piece (index was: %u) is incomplete, returning\n"),
                  index_in));
      goto error;
    } // end IF
    iterator_2 = (*iterator).chunks.begin ();
    while ((*iterator_2).offset < begin_in)
      ++iterator_2;
    if ((*iterator_2).offset > begin_in)
      --iterator_2;
    ACE_ASSERT (((*iterator_2).offset <= begin_in) && ((*iterator_2).offset + (*iterator_2).data->total_length () > begin_in));
    message_block_p = (*iterator_2).data->duplicate ();
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, returning\n")));
      goto error;
    } // end IF
    offset = begin_in - (*iterator_2).offset;
    ACE_ASSERT (offset < message_block_p->length ());
    message_block_p->rd_ptr (offset);
    message_block_2 = message_block_p;
    while (missing_bytes)
    {
      if (message_block_2->length () >= missing_bytes)
      {
        message_block_2->length (missing_bytes);
        break;
      } // end IF
      missing_bytes -= message_block_2->length ();
      if (message_block_2->cont ())
      {
        message_block_2 = message_block_2->cont ();
        continue;
      } // end IF
      std::advance (iterator_2, 1);
      ACE_ASSERT (iterator_2 != (*iterator).chunks.end ()); // requested too much data !
      message_block_3 = (*iterator_2).data->duplicate ();
      if (!message_block_3)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory, returning\n")));
        message_block_p->release (); message_block_p = NULL;
        goto error;
      } // end IF
      message_block_2->cont (message_block_3);
      message_block_2 = message_block_3;
    } // end WHILE
    if (message_block_2->cont ())
    {
      message_block_2->cont ()->release (); message_block_2->cont (NULL);
    } // end IF
    ACE_ASSERT (message_block_p->total_length () == length_in);
    message_p->cont (message_block_p);
  } // end lock scope

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

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: sent chunk (piece %u, begin %u, length %u) to peer (id was: %d)...\n"),
  //            ACE::basename (inherited::configuration_->metaInfoFileName.c_str (),
  //                           ACE_DIRECTORY_SEPARATOR_CHAR),
  //            index_in,
  //            begin_in,
  //            length_in,
  //            id_in));

  return;

error:
  if (record_p)
    delete record_p;
  if (message_p)
    message_p->release ();
  if (istream_connection_p)
    istream_connection_p->decrease ();
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::have (unsigned int index_in)
#else
                     >::have (unsigned int index_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::have"));

  struct BitTorrent_PeerRecord* record_p = NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  typename PeerStreamType::MESSAGE_T* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ISTREAM_CONNECTION_T* istream_connection_p = NULL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    for (Net_ConnectionIdsIterator_t iterator = inherited::state_.connections.begin ();
         iterator != inherited::state_.connections.end ();
         ++iterator)
    {
      if (*iterator == inherited::state_.trackerConnectionId)
        continue;

      ACE_NEW_NORETURN (record_p,
                        struct BitTorrent_PeerRecord ());
      if (!record_p)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory, returning\n")));
        goto error;
      } // end IF
      record_p->type = BITTORRENT_MESSAGETYPE_HAVE;
      record_p->have = index_in;

      if (!getConnectionAndMessage (*iterator,
                                    istream_connection_p,
                                    message_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to BitTorrent_Session_T::getConnectionAndMessage, continuing\n")));
        continue;
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
      istream_connection_p->decrease (); istream_connection_p = NULL;
    } // end FOR
  } // end lock scope

  return;

error:
  if (record_p)
    delete record_p;
  if (message_p)
    message_p->release ();
  if (istream_connection_p)
    istream_connection_p->decrease ();
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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
  ACE_INET_Addr host_address;
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
                             host_address,
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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  TrackerConnectionConfigurationType* configuration_p = NULL;
  TrackerUserDataType* user_data_p = NULL;
  typename TrackerStreamType::CONFIGURATION_T::ITERATOR_T iterator;
  TrackerConnectorType connector (true);
  TrackerUserDataType user_data_s;
  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;

  TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (configuration_p,
                                                            user_data_p);

  // sanity check(s)
  ACE_ASSERT (configuration_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_p->streamConfiguration);
  ACE_ASSERT (configuration_p->streamConfiguration->configuration_);

  // step1: set up configuration
  configuration_p->socketConfiguration.address = address_in;

  iterator =
    configuration_p->streamConfiguration->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_p->streamConfiguration->end ());
  (*iterator).second.second->subscriber =
    inherited::state_.trackerStreamHandler;

  configuration_p->streamConfiguration->configuration_->cloneModule =
    false;
  configuration_p->streamConfiguration->configuration_->module =
    trackerHandlerModule_;

  // step2: try to connect
  handle_h =
    Net_Client_Common_Tools::connect (connector,
                                      *static_cast<TrackerConnectionConfigurationType*> (inherited::configuration_->trackerConnectionConfiguration),
                                      user_data_s,
                                      address_in,
                                      true,
                                      true);
  if (handle_h == ACE_INVALID_HANDLE)
  { ACE_ASSERT (inherited::configuration_);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to connect to tracker %s: \"%m\", returning\n"),
                ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ())));
    goto error;
  } // end IF
//  iconnection_p =
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (reinterpret_cast<Net_ConnectionId_t> (handle_h));
//#else
//      TRACKER_CONNECTION_MANAGER_SINGLETON_T::instance ()->get (static_cast<Net_ConnectionId_t> (handle_h));
//#endif // ACE_WIN32 || ACE_WIN64
//  if (unlikely (!iconnection_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to connect to tracker %s: \"%m\", returning\n"),
//                ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
//                               ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
//    goto error;
//  } // end IF
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: connected to tracker %s: %u\n"),
//              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
//                             ACE_DIRECTORY_SEPARATOR_CHAR),
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in, false, false).c_str ()),
//              iconnection_p->id ()));

  //iconnection_p->decrease (); iconnection_p = NULL;

error:
  //if (iconnection_p)
  //  iconnection_p->decrease ();
  ;
};

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

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
        iconnection_p->abort ();
        iconnection_p->decrease (); iconnection_p= NULL;
      } // end ELSE
    } // end IF
    inherited::state_.trackerConnectionId = id_in;
  } // end lock scope

  inherited::connect (id_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: new tracker connection (id: %d)\n"),
              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in));
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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
  enum BitTorrent_Event event_e = BITTORRENT_EVENT_INVALID;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    if (inherited::state_.trackerConnectionId == id_in)
      inherited::state_.trackerConnectionId = 0;

    if (inherited::state_.aborted)
    {
      event_e = BITTORRENT_EVENT_CANCELLED;
      notify_controller_b = true;
    } // end IF
    else if (inherited::state_.connections.empty ())
    {
      event_e = BITTORRENT_EVENT_NO_MORE_PEERS;
      notify_controller_b = true;
    } // end IF
  } // end lock scope

  if (notify_controller_b)
  { ACE_ASSERT (inherited::configuration_->controller);
    try {
      inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                     event_e,
                                                     ACE_TEXT_ALWAYS_CHAR (""));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
    }
  } // end IF

  //if (timerId_ != -1)
  //{
  //  int result =
  //    COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerId_,
  //                                                              NULL);
  //  if (unlikely (result == -1))
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to cancel interval timer (id: %d): \"%m\", continuing\n"),
  //                timerId_));
  //  else
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("cancelled interval timer (id: %d)\n"),
  //                timerId_));
  //  timerId_ = -1;
  //} // end IF
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: tracker error (id was: %d): %s\n"),
              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              id_in,
              ACE_TEXT (HTTP_Tools::dump (response_in).c_str ())));

  // notify torrent cancelled to event subscriber
  if (inherited::configuration_->subscriber)
    inherited::configuration_->subscriber->complete (true);

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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s\n"),
//              ACE_TEXT (BitTorrent_Tools::DictionaryToString (record_in).c_str ())));

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

  ACE_Time_Value rerequest_interval;

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

    Bencoding_DictionaryIterator_t iterator =
      inherited::state_.trackerRequestResponse->begin ();
    for (;
         iterator != inherited::state_.trackerRequestResponse->end ();
         ++iterator)
      if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_INTERVAL_HEADER))
        break;
    ACE_ASSERT (iterator != inherited::state_.trackerRequestResponse->end ());
    ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
    rerequest_interval.sec ((*iterator).second->integer);
  } // end lock scope

  // start rerequest interval timer
  if (unlikely (timerId_ != -1))
  {
    int result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerId_,
                                                                NULL);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel interval timer (id: %d): \"%m\", continuing\n"),
                  timerId_));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled interval timer (id: %d)\n"),
                  timerId_));
    timerId_ = -1;
  } // end IF
  ACE_ASSERT (timerId_ == -1);
  timerId_ =
    COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (this,
                                                                NULL,
                                                                rerequest_interval,
                                                                rerequest_interval);
  if (unlikely (timerId_ == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Timer_Manager_T::schedule_timer (%#T), continuing\n"),
                &rerequest_interval));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("scheduled interval timer (id: %d) for interval: %#T\n"),
                timerId_,
                &rerequest_interval));

  // connect to peers
  BitTorrent_PeerAddresses_t peer_addresses_a;
  int result = -1;
  ACE_INET_Addr inet_address;

  iterator = record_in.begin ();
  for (;
       iterator != record_in.end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_HEADER))
      break;
  if (iterator == record_in.end ())
  {
    iterator = record_in.begin ();
    for (;
         iterator != record_in.end ();
         ++iterator)
      if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_FAILURE_REASON_HEADER))
        break;
    ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve peers: \"%s\", returning\n"),
                ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ((*iterator).second->string->c_str ())));
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
      inherited::state_.aborted = true;
    } // end lock scope
    return;
  } // end IF

  // can be 'list' or 'binary' model
  switch ((*iterator).second->type)
  {
    case Bencoding_Element::BENCODING_TYPE_STRING:
    {
      const char* char_p = (*iterator).second->string->c_str ();

      // *IMPORTANT NOTE*: the 'compact' 'peers' representation is assumed here
      //                   (see also: http://www.bittorrent.org/beps/bep_0023.html)
      for (unsigned int i = 0;
           i < ((*iterator).second->string->size () / 6);
           ++i)
      {
        result = inet_address.set (*reinterpret_cast<const u_short*> (char_p + 4),
                                   *reinterpret_cast<const ACE_UINT32*> (char_p),
                                   0,  // already in network byte order
                                   0); // do not map to ipv6
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
          return;
        } // end IF
        peer_addresses_a.push_back (inet_address);
        char_p += 6;
      } // end FOR

      break;
    }
    case Bencoding_Element::BENCODING_TYPE_LIST:
    {
      u_short port_number_i;
      int result;
      for (Bencoding_ListIterator_t iterator_2 = (*iterator).second->list->begin ();
           iterator_2 != (*iterator).second->list->end ();
           ++iterator_2)
      { ACE_ASSERT ((*iterator_2)->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
        Bencoding_DictionaryIterator_t iterator_3;
        for (iterator_3 = (*iterator_2)->dictionary->begin ();
             iterator_3 != (*iterator_2)->dictionary->end ();
             ++iterator_3)
          if (*(*iterator_3).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_PORT_HEADER))
          { ACE_ASSERT ((*iterator_3).second->type == Bencoding_Element::BENCODING_TYPE_INTEGER);
            port_number_i = (*iterator_3).second->integer;
            break;
          } // end IF
        ACE_ASSERT (port_number_i);
        for (iterator_3 = (*iterator_2)->dictionary->begin ();
             iterator_3 != (*iterator_2)->dictionary->end ();
             ++iterator_3)
          if (*(*iterator_3).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_RESPONSE_PEERS_IP_HEADER))
          { ACE_ASSERT ((*iterator_3).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
            std::string ip_address_string = *(*iterator_3).second->string;
            int address_family_i = AF_INET;
            if (Net_Common_Tools::matchIPv6Address (ip_address_string))
            {
              ip_address_string = '[' + ip_address_string;
              ip_address_string.push_back (']');
              address_family_i = AF_INET6;
            } // end IF
            ip_address_string.push_back (':');
            std::ostringstream converter;
            converter << port_number_i;
            ip_address_string += converter.str ();
            result = inet_address.set (ip_address_string.c_str (),
                                       address_family_i);
            if (unlikely (result == -1))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", continuing\n"),
                          ACE_TEXT (ip_address_string.c_str ())));
              break;
            } // end IF
            peer_addresses_a.push_back (inet_address);

            break;
          } // end IF
      } // end FOR

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown type (was: %d), returning\n"),
                  (*iterator).second->type));
      return;
    }
  } // end SWITCH

  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!peer_addresses_a.size ()))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: tracker sent no peers, returning\n"),
                ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                               ACE_DIRECTORY_SEPARATOR_CHAR)));
    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: connecting to %u peer(s)...\n"),
              ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                             ACE_DIRECTORY_SEPARATOR_CHAR),
              peer_addresses_a.size ()));

  struct BitTorrent_SessionInitiationThreadData* thread_data_p = NULL;
  ACE_NEW_NORETURN (thread_data_p,
                    struct BitTorrent_SessionInitiationThreadData ());
  ACE_ASSERT (thread_data_p);
  thread_data_p->addresses = peer_addresses_a;
  thread_data_p->lock = &(inherited::lock_);
  thread_data_p->session = this;
  ACE_Thread_Manager* thread_manager_p = NULL;
  const char** thread_names_p = NULL;
  char* thread_name_p = NULL;
  std::string buffer;
  std::ostringstream converter;
  int group_id_i = -1;
  size_t number_of_threads_i = peer_addresses_a.size ();
  ACE_ASSERT (number_of_threads_i);
  ACE_thread_t* thread_ids_p = NULL;
  ACE_hthread_t* thread_handles_p = NULL;
  // *TODO*: use ACE_NEW_MALLOC_ARRAY (as soon as the NORETURN variant becomes
  //         available)
  ACE_NEW_NORETURN (thread_ids_p,
                    ACE_thread_t[number_of_threads_i]);
  if (!thread_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory(%u), aborting\n"),
                (sizeof (ACE_thread_t) * number_of_threads_i)));
    goto error;
  } // end IF
  ACE_NEW_NORETURN (thread_handles_p,
                    ACE_hthread_t[number_of_threads_i]);
  if (!thread_handles_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory(%u), aborting\n"),
                (sizeof (ACE_hthread_t) * number_of_threads_i)));
    goto error;
  } // end IF
//  ACE_OS::memset (thread_handles_p, 0, sizeof (thread_handles_p));
  // *TODO*: use ACE_NEW_MALLOC_ARRAY (as soon as the NORETURN variant becomes
  //         available)
  ACE_NEW_NORETURN (thread_names_p,
                    const char*[number_of_threads_i]);
  if (!thread_names_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory(%u), aborting\n"),
                (sizeof (char*) * number_of_threads_i)));
    goto error;
  } // end IF
  ACE_OS::memset (thread_names_p, 0, sizeof (thread_names_p));
  for (unsigned int i = 0; i < number_of_threads_i; i++)
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_OS::strcpy (thread_name_p, buffer.c_str ());
#else
    ACE_ASSERT (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH <= BUFSIZ);
    ACE_OS::strncpy (thread_name_p,
                     buffer.c_str (),
                     std::min (static_cast<size_t> (COMMON_THREAD_PTHREAD_NAME_MAX_LENGTH - 1), static_cast<size_t> (ACE_OS::strlen (buffer.c_str ()))));
#endif // ACE_WIN32 || ACE_WIN64
    thread_names_p[i] = thread_name_p;
  } // end FOR
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  group_id_i =
    thread_manager_p->spawn_n (thread_ids_p,                               // id(s)
                               number_of_threads_i,                        // # threads
                               static_cast<ACE_THR_FUNC> (::net_bittorrent_session_setup_function), // function
                               thread_data_p,                              // argument
                               (THR_NEW_LWP      |
                                //THR_JOINABLE     |
                                THR_DETACHED     |
                                THR_INHERIT_SCHED),                        // flags
                               ACE_DEFAULT_THREAD_PRIORITY,                // priority
                               BITTORRENT_SESSION_HANDLER_THREAD_GROUP_ID, // group id
                               NULL,                                       // stack(s)
                               NULL,                                       // stack size(s)
                               thread_handles_p,                           // handle(s)
                               NULL,                                       // task
                               thread_names_p);                            // name(s)
  if (unlikely (group_id_i == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn_n(%u): \"%m\", aborting\n"),
                number_of_threads_i));
    goto error;
  } // end IF

  // clean up
  if (thread_ids_p)
    delete [] thread_ids_p;
  delete [] thread_handles_p;
  for (unsigned int i = 0; i < number_of_threads_i; i++)
    delete [] thread_names_p[i];
  delete [] thread_names_p;

  //result = thread_manager_p->wait_grp (group_id_i);
  //if (result == -1)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", aborting\n"),
  //              group_id_i));
  //  goto error;
  //} // end IF

  return;

error:
  if (thread_ids_p)
    delete [] thread_ids_p;
  if (thread_handles_p)
    delete [] thread_handles_p;
  if (thread_names_p)
    for (unsigned int j = 0; j < number_of_threads_i; j++)
      delete [] thread_names_p[j];
  if (thread_names_p)
    delete [] thread_names_p;
  if (thread_data_p)
    delete thread_data_p;
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  // compare handshake peer id with tracker peer id
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
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
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: tracker response had binary \"peers\" model: cannot validate handshake peer_id (was: \"%s\"), continuing\n"),
                  ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                 ACE_DIRECTORY_SEPARATOR_CHAR),
                  ACE_TEXT (record_in.peer_id.c_str ())));
  } // end lock scope
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  if (inherited::configuration_->subscriber)
  {
    std::string record_string = BitTorrent_Tools::RecordToString (record_in);
    inherited::configuration_->subscriber->log (record_string);
  }
  // ACE_DEBUG ((LM_DEBUG,
  //             ACE_TEXT ("%u: %s\n"),
  //             id_in,
  //             ACE_TEXT (record_string.c_str ())));

  ACE_UINT32 peer_address_i = connectionIdToPeerAddress (id_in);

  if (!record_in.length)
    return;
  switch (record_in.type)
  {
    case BITTORRENT_MESSAGETYPE_CHOKE:
    {
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        (*iterator).second.status.am_choking = true;
      } // end lock scope
      break;
    }
    case BITTORRENT_MESSAGETYPE_UNCHOKE:
    {
      bool is_not_first_connection_to_peer_b = false;
      bool have_peer_bitfield_b = false;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        (*iterator).second.status.am_choking = false;

        is_not_first_connection_to_peer_b =
          (*iterator).second.connections.size () > 1;
        have_peer_bitfield_b =
          !(*iterator).second.pieces.empty ();
      } // end lock scope

      // request a piece ?
      if (is_not_first_connection_to_peer_b ||
          have_peer_bitfield_b)
      {
        bool requesting_piece_b = requestNextPiece (id_in);
        if (requesting_piece_b)
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
          BitTorrent_PeerStateIterator_t iterator =
            inherited::state_.peerState.find (peer_address_i);
          ACE_ASSERT (iterator != inherited::state_.peerState.end ());
          ++(*iterator).second.status.requesting_piece;
        } // end IF && lock scope
      } // end IF

      break;
    }
    case BITTORRENT_MESSAGETYPE_INTERESTED:
    {
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        (*iterator).second.status.peer_interested = true;
        (*iterator).second.status.peer_choking = false;
      } // end lock scope
      choke (id_in,
             false);

      break;
    }
    case BITTORRENT_MESSAGETYPE_NOT_INTERESTED:
    {
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        (*iterator).second.status.peer_interested = false;
      } // end lock scope
      break;
    }
    case BITTORRENT_MESSAGETYPE_HAVE:
    {
      bool send_interested_b = false;
      bool has_missing_piece_b = false;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        unsigned int index = record_in.have / (sizeof (ACE_UINT8) * 8);
        unsigned int index_2 = record_in.have % (sizeof (ACE_UINT8) * 8);
        // *IMPORTANT NOTE*: the peer may 'repeat' itself, so cannot assert here
        //ACE_ASSERT (!((*iterator).second.pieces[index] & (0x80 >> index_2)));
        (*iterator).second.pieces[index] |= (0x80 >> index_2);

        // send 'interested' ?
        has_missing_piece_b = BitTorrent_Tools::hasMissingPiece (inherited::state_.pieces,
                                                                 (*iterator).second.pieces);
        if (!(*iterator).second.status.am_interested &&
            has_missing_piece_b)
        {
          (*iterator).second.status.am_interested = true;
          send_interested_b = true;
        } // end IF
      } // end lock scope
      if (send_interested_b)
        interested (id_in,
                    true);

      // request piece ?
      bool am_choking_b = false;
      bool request_piece_b = false;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        am_choking_b = (*iterator).second.status.am_choking;
        request_piece_b =
          !am_choking_b                               &&
          !(*iterator).second.status.requesting_piece &&
          has_missing_piece_b;
      } // end lock scope
      if (request_piece_b)
      {
        bool requesting_piece_b = requestNextPiece (id_in);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
          BitTorrent_PeerStateIterator_t iterator =
            inherited::state_.peerState.find (peer_address_i);
          ACE_ASSERT (iterator != inherited::state_.peerState.end ());
          if (requesting_piece_b)
            (*iterator).second.status.requesting_piece++;
        } // end lock scope
      } // end IF
      else if (!am_choking_b &&
               has_missing_piece_b) // *TODO*: ...that is not already being requested...
      { // open another connection to the peer ?
        unsigned int number_of_connections_to_this_peer_i = 0;
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
          BitTorrent_PeerStateIterator_t iterator =
            inherited::state_.peerState.find (peer_address_i);
          ACE_ASSERT (iterator != inherited::state_.peerState.end ());
          number_of_connections_to_this_peer_i =
            (*iterator).second.connections.size ();
        } // end lock scope
        if (inherited::configuration_->allowMultipleConnectionsPerPeer && 
            (number_of_connections_to_this_peer_i < BITTORRENT_DEFAULT_MAX_CONNECTIONS_PER_PEER))
        {
          ICONNECTION_T* iconnection_p =
            inherited::CONNECTION_MANAGER_SINGLETON_T::instance ()->get (id_in);
          if (unlikely (!iconnection_p))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to retrieve connection handle (id was: %d), continuing\n"),
                        id_in));
            break;
          } // end IF

          ACE_HANDLE handle_h;
          ACE_INET_Addr local_address, peer_address;
          iconnection_p->info (handle_h,
                               local_address,
                               peer_address);

          // clean up
          iconnection_p->decrease (); iconnection_p = NULL;

          connect (peer_address);
        } // end IF
      } // end ELSE IF

      break;
    }
    case BITTORRENT_MESSAGETYPE_BITFIELD:
    {
      bool send_interested_b = false;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        //ACE_ASSERT ((*iterator).second.empty ());
        (*iterator).second.pieces = record_in.bitfield;

        // send 'interested' ?
        if (!(*iterator).second.status.am_interested &&
            BitTorrent_Tools::hasMissingPiece (inherited::state_.pieces,
                                               (*iterator).second.pieces))
        {
          (*iterator).second.status.am_interested = true;
          send_interested_b = true;
        } // end IF
      } // end lock scope
      if (send_interested_b)
        interested (id_in,
                    true);

      break;
    }
    case BITTORRENT_MESSAGETYPE_REQUEST:
    {
      bool send_piece_b = true;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        send_piece_b = !(*iterator).second.status.peer_choking;
      } // end lock scope
      if (send_piece_b)
        piece (id_in,
               record_in.request.index,
               record_in.request.begin,
               record_in.request.length);

      break;
    }
    case BITTORRENT_MESSAGETYPE_PIECE:
    { ACE_ASSERT (messageBlock_in);
      struct BitTorrent_Piece_Chunk chunk_s;
      chunk_s.offset = record_in.piece.begin;
      bool send_have_b = false;
      bool request_next_piece_b = true;
      bool notify_completion_b = false;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PiecesIterator_t iterator =
          inherited::state_.pieces.begin ();
        std::advance (iterator, record_in.piece.index);
        // sanity check(s): piece already complete ?
        if (unlikely ((*iterator).onDisk ||
                      BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                                         (*iterator).chunks)))
          goto continue_;
        chunk_s.data = messageBlock_in->duplicate ();
        if (unlikely (!chunk_s.data))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to BitTorrent_Message_T::duplicate(), continuing\n")));
          goto continue_;
        } // end IF
        (*iterator).chunks.push_back (chunk_s);
        BitTorrent_Tools::sanitizeChunks ((*iterator).chunks);
        if (unlikely (BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                                         (*iterator).chunks)))
        {
          if (!BitTorrent_Tools::validatePieceHash (*iterator))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to BitTorrent_Tools::validatePieceHash(%s,%u), retrying\n"),
                        ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                       ACE_DIRECTORY_SEPARATOR_CHAR),
                        record_in.piece.index));
            // --> download again
            for (BitTorrent_PieceChunksConstIterator_t iterator_2 = (*iterator).chunks.begin ();
                 iterator_2 != (*iterator).chunks.end ();
                 ++iterator_2)
              (*iterator_2).data->release ();
            (*iterator).chunks.clear ();
            goto continue_;
          } // end IF
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("%s: piece %u complete...\n"),
          //            ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
          //                           ACE_DIRECTORY_SEPARATOR_CHAR),
          //            record_in.piece.index));

          send_have_b = true;
          if (!BitTorrent_Tools::savePiece (ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                                                                 ACE_DIRECTORY_SEPARATOR_CHAR)),
                                            inherited::state_.pieces.size (),
                                            record_in.piece.index,
                                            *iterator))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to BitTorrent_Tools::savePiece(%s,%u), continuing\n"),
                        ACE::basename (ACE_TEXT (inherited::configuration_->metaInfoFileName.c_str ()),
                                       ACE_DIRECTORY_SEPARATOR_CHAR),
                        record_in.piece.index));
            ACE_ASSERT (false); // *TODO*
            break;
          } // end IF

          // notify piece complete to event subscriber
          if (inherited::configuration_->subscriber)
            inherited::configuration_->subscriber->pieceComplete (record_in.piece.index);

          for (iterator = inherited::state_.pieces.begin ();
               iterator != inherited::state_.pieces.end ();
               ++iterator)
            if (!(*iterator).onDisk &&
                !BitTorrent_Tools::isPieceComplete ((*iterator).length,
                                                    (*iterator).chunks))
              goto continue_;
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
            ACE_ASSERT (false); // *TODO*
            break;
          } // end IF
          notify_completion_b = true;
        } // end IF
continue_:
        BitTorrent_PeerStateIterator_t iterator_2;
        iterator_2 = inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator_2 != inherited::state_.peerState.end ());
        if ((*iterator_2).second.status.am_choking || notify_completion_b)
          request_next_piece_b = false;
      } // end lock scope
      if (unlikely (send_have_b))
        have (record_in.piece.index);

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        BitTorrent_PeerStateIterator_t iterator =
          inherited::state_.peerState.find (peer_address_i);
        ACE_ASSERT (iterator != inherited::state_.peerState.end ());
        --(*iterator).second.status.requesting_piece;
      } // end lock scope

      if (likely (request_next_piece_b))
      {
        bool requesting_piece_b = requestNextPiece (id_in);
        if (likely (requesting_piece_b))
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
          BitTorrent_PeerStateIterator_t iterator =
            inherited::state_.peerState.find (peer_address_i);
          ACE_ASSERT (iterator != inherited::state_.peerState.end ());
          ++(*iterator).second.status.requesting_piece;
        } // end IF && lock scope
      } // end IF

      if (unlikely (notify_completion_b))
      {
        // notify torrent complete to event subscriber
        if (inherited::configuration_->subscriber)
          inherited::configuration_->subscriber->complete (false);

        ACE_ASSERT (inherited::configuration_->controller);
        try {
          inherited::configuration_->controller->notify (inherited::configuration_->metaInfoFileName,
                                                         BITTORRENT_EVENT_COMPLETE,
                                                         ACE_TEXT_ALWAYS_CHAR (""));
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
        }
      } // end IF
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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
bool
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  const typename ISTREAM_CONNECTION_T::STREAM_T* stream_p =
    NULL;
  const typename ISTREAM_CONNECTION_T::STREAM_T::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  const typename ISTREAM_CONNECTION_T::STREAM_T::SESSION_DATA_T* session_data_p =
    NULL;
  struct BitTorrent_PeerMessageData* data_p = NULL;
  typename PeerStreamType::MESSAGE_T::DATA_T* data_container_p = NULL;
  ICONNECTION_T* iconnection_p = NULL;

  if (message_out) // only want connection
    goto continue_;

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

  // sanity check(s)
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

continue_:
  if (connection_out) // only want message
    goto continue_2;

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

continue_2:
  stream_p = &connection_out->stream ();
  session_data_container_p = &stream_p->getR_2 ();
  session_data_p = &session_data_container_p->getR ();
  ACE_ASSERT (session_data_p->sessionId);

  // *IMPORTANT NOTE*: fire-and-forget API (data_container_p)
  message_out->initialize (data_container_p,
                           session_data_p->sessionId,
                           NULL);
  ACE_ASSERT (!data_container_p);

  return true;

error:
  if (message_out)
  {
    message_out->release (); message_out = NULL;
  } // end IF
  if (data_container_p)
    data_container_p->decrease ();
  if (data_p)
    delete data_p;
  if (connection_out)
  {
    connection_out->decrease (); connection_out = NULL;
  } // end IF

  return false;
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
bool
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
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

  ACE_UINT32 peer_address_i = connectionIdToPeerAddress (id_in);

  unsigned int index = 0, offset = 0, length = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, false);
    BitTorrent_PeerStateIterator_t iterator =
      inherited::state_.peerState.find (peer_address_i);
    ACE_ASSERT (iterator != inherited::state_.peerState.end ());
    if (!BitTorrent_Tools::hasMissingPiece (inherited::state_.pieces,
                                            (*iterator).second.pieces))
    {
      // --> send 'not interested'
      (*iterator).second.status.am_interested = false;
      goto not_interested;
    } // end IF

    // get random missing piece# that the peer has
    ACE_ASSERT (!inherited::state_.pieces.empty ());
    std::vector<unsigned int> missing_pieces_a =
      BitTorrent_Tools::getPieceIndexes (inherited::state_.pieces, true);
    std::vector<unsigned int> doesnt_have_a;
    for (unsigned int i = 0;
         i < inherited::state_.pieces.size ();
         ++i)
      if (!BitTorrent_Tools::havePiece (i, (*iterator).second.pieces))
        doesnt_have_a.push_back (i);
    struct net_bittorrent_piece_index_remove_predicate predicate_s;
    missing_pieces_a.erase (std::remove_if (missing_pieces_a.begin (), missing_pieces_a.end (),
                                            std::bind (predicate_s,
                                                       std::placeholders::_1,
                                                       doesnt_have_a)),
                            missing_pieces_a.end ());
    ACE_ASSERT (!missing_pieces_a.empty ());
    BitTorrent_PiecesIterator_t iterator_2;
    iterator_2 = inherited::state_.pieces.begin ();
    std::advance (iterator_2,
                  missing_pieces_a[Common_Tools::getRandomNumber (static_cast<size_t> (0),
                                                                  missing_pieces_a.size () - 1)]);
    index = std::distance (inherited::state_.pieces.begin (), iterator_2);
#undef NET_BITTORRENT_GET_CHUNKS_ORDERED
#if defined (NET_BITTORRENT_GET_CHUNKS_ORDERED)
    // *IMPORTANT NOTE*: this gets the first one missing
    BitTorrent_PieceChunksIterator_t iterator_3 = (*iterator_2).chunks.begin ();
    for (;
         iterator_3 != (*iterator_2).chunks.end ();
         ++iterator_3)
    {
      if ((*iterator_3).offset != (offset + length))
        break; // there's a 'hole'
      offset = (*iterator_3).offset;
      length = BitTorrent_Tools::chunkLength (*iterator_3);
    } // end FOR
    offset = offset + length;
    unsigned int missing_data =
        static_cast<unsigned int> (BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX);
    if (iterator_3 != (*iterator_2).chunks.end ())
    { // there's a 'hole': fill it
      missing_data = (*iterator_3).offset - offset;
      length =
        std::min (missing_data,
                  static_cast<unsigned int> (BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX));
    } // end IF
    else if (!(*iterator_2).chunks.empty ())
    { // there are no 'holes'; there's data missing at the end though
      missing_data =
        (*iterator_2).length - BitTorrent_Tools::chunksLength ((*iterator_2).chunks);
      ACE_ASSERT (missing_data > 0);
      length =
        std::min (missing_data,
                  static_cast<unsigned int> (BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX));
    } // end ELSE IF
    else // no chunks yet; get as much as possible
      length = missing_data;
#else
    // *IMPORTANT NOTE*: this gets random chunks
    std::vector<struct BitTorrent_MessagePayload_Request> missing_chunks_a =
      BitTorrent_Tools::getMissingChunks ((*iterator_2).length,
                                          (*iterator_2).chunks);
    ACE_ASSERT (!missing_chunks_a.empty ());
    std::vector<struct BitTorrent_MessagePayload_Request>::const_iterator iterator_3 =
      missing_chunks_a.begin ();
    std::advance (iterator_3,
                  Common_Tools::getRandomNumber (static_cast<size_t> (0),
                                                 missing_chunks_a.size () - 1));
    offset = (*iterator_3).begin;
    length = (*iterator_3).length;
//#else
//    ACE_ASSERT (false);
//    ACE_NOTSUP;
//    ACE_NOTREACHED (return;)
#endif // NET_BITTORRENT_GET_CHUNKS_ORDERED | NET_BITTORRENT_GET_CHUNKS_RANDOM
  } // end lock scope
  request (id_in,
           index,
           offset,
           length);
  return true;

not_interested:
  interested (id_in,
              false);

  return false;
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::populatePeerPiecesBitfield (Net_ConnectionId_t id_in)
#else
                     >::populatePeerPiecesBitfield (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::populatePeerPiecesBitfield"));

  ACE_UINT32 peer_address_i = connectionIdToPeerAddress (id_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    BitTorrent_PeerStateIterator_t iterator =
      inherited::state_.peerState.find (peer_address_i);
    ACE_ASSERT (iterator != inherited::state_.peerState.end ());
    ACE_ASSERT ((*iterator).second.pieces.empty ());
    Bencoding_DictionaryIterator_t iterator_2 =
        inherited::configuration_->metaInfo->begin ();
    for (;
          iterator_2 != inherited::configuration_->metaInfo->end ();
          ++iterator_2)
      if (*(*iterator_2).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_KEY))
        break;
    ACE_ASSERT (iterator_2 != inherited::configuration_->metaInfo->end ());
    ACE_ASSERT ((*iterator_2).second->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
    Bencoding_DictionaryIterator_t iterator_3 =
      (*iterator_2).second->dictionary->begin ();
    for (; iterator_3 != (*iterator_2).second->dictionary->end ();
          ++iterator_3)
      if (*(*iterator_3).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_INFO_PIECES_KEY))
        break;
    ACE_ASSERT (iterator_3 != (*iterator_2).second->dictionary->end ());
    ACE_ASSERT ((*iterator_3).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
    ACE_ASSERT (!((*iterator_3).second->string->size () % BITTORRENT_PRT_INFO_PIECE_HASH_SIZE));
    unsigned int pieces =
        (*iterator_3).second->string->size () / BITTORRENT_PRT_INFO_PIECE_HASH_SIZE;
    unsigned int indices_i = pieces / (sizeof (ACE_UINT8) * 8);
    unsigned int indices_2 = pieces % (sizeof (ACE_UINT8) * 8);
    (*iterator).second.pieces.resize ((indices_2 ? indices_i + 1 : indices_i), 0);
  } // end lock scope
}

template <typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerStreamHandlerType,
          typename TrackerStreamHandlerType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerStreamUserDataType,
          typename TrackerStreamUserDataType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType> // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
ACE_UINT32
BitTorrent_Session_T<PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerStreamHandlerType,
                     TrackerStreamHandlerType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerStreamUserDataType,
                     TrackerStreamUserDataType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType
#if defined (GUI_SUPPORT)
                     ,CBDataType>::connectionIdToPeerAddress (Net_ConnectionId_t id_in)
#else
                     >::connectionIdToPeerAddress (Net_ConnectionId_t id_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::connectionIdToPeerAddress"));

  // initialize return value(s)
  ACE_UINT32 result = 0;

  // sanity check(s)
  ACE_ASSERT (id_in);

  ICONNECTION_T* iconnection_p =
    inherited::CONNECTION_MANAGER_SINGLETON_T::instance ()->get (id_in);
  if (unlikely (!iconnection_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve connection handle (id was: %d), aborting\n"),
                id_in));
    return 0;
  } // end IF

  ACE_HANDLE handle_h;
  ACE_INET_Addr local_address, peer_address;
  iconnection_p->info (handle_h,
                       local_address,
                       peer_address);

  // clean up
  iconnection_p->decrease (); iconnection_p = NULL;

  if (peer_address.get_type () == PF_INET)
    result = peer_address.get_ip_address ();
  if (!result)
  { ACE_ASSERT (peer_address.get_type () == PF_INET6);
    sockaddr_in6* sockaddr_p = (sockaddr_in6*)peer_address.get_addr ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    uint8_t* data_p = static_cast<uint8_t*> (sockaddr_p->sin6_addr.u.Byte);
#elif defined (ACE_LINUX)
    uint8_t* data_p =
      static_cast<uint8_t*> (sockaddr_p->sin6_addr.__in6_u.__u6_addr8);
#else
    ACE_ASSERT (false); // *TODO*
#endif // ACE_WIN32 || ACE_WIN64
    //data_p += 128 / 8 - 32 / 8;
    //ACE_OS::memcpy (&result, data_p, sizeof (ACE_UINT32));
    result = data_p[0] | (data_p[5] << 8) | (data_p[9] << 16) | (data_p[13] << 24);
  } // end IF
  ACE_ASSERT (result);

  return result;
}
