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

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/OS_Memory.h"
#include "ace/Time_Value.h"

#include "common_parser_bencoding_tools.h"

#include "stream_dec_common.h"

#include "net_defines.h"
#include "net_macros.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "http_codes.h"
#include "http_common.h"
#include "http_tools.h"

#include "bittorrent_common.h"
#include "bittorrent_network.h"
#include "bittorrent_stream_common.h"
#include "bittorrent_tools.h"

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::BitTorrent_Control_T (SessionConfigurationType* sessionConfiguration_in)
 : inherited (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CONTROL_HANDLER_THREAD_NAME), // thread name
              BITTORRENT_CONTROL_HANDLER_THREAD_GROUP_ID,                    // group id
              1,                                                             // # thread(s)
              false,                                                         // auto-start ?
              NULL)                                                          // queue handle
 , condition_ (lock_)
 , lock_ ()
 , sessionConfiguration_ (sessionConfiguration_in)
 , sessions_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::BitTorrent_Control_T"));

}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::request (const std::string& metaInfoFileName_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::request"));

  // sanity check(s)
  ACE_ASSERT (sessionConfiguration_);

  bool remove_session = false;
  typename SessionType::ISESSION_T* isession_p = NULL;
  SessionStateType* session_state_p = NULL;
  Bencoding_DictionaryIterator_t iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier = GUID_NULL;
#else
  std::string interface_identifier;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string interface_identifier;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INET_Addr external_ip_address;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  std::ostringstream converter;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
      NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  std::string user_agent;
  ACE_INET_Addr host_address;
  std::string host_name_string;
  std::string URI_string;
  bool use_SSL = false;

  // step1: parse metainfo
  ACE_ASSERT (sessionConfiguration_->parserConfiguration);
  ACE_ASSERT (!sessionConfiguration_->metaInfo);
  if (unlikely (!BitTorrent_Tools::parseMetaInfoFile (*static_cast<struct Common_FlexBisonParserConfiguration*> (sessionConfiguration_->parserConfiguration),
                                                      metaInfoFileName_in,
                                                      sessionConfiguration_->metaInfo)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Tools::parseMetaInfoFile(\"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
    goto error;
  } // end IF

  // step2: create/initialize session
  if (sessionConfiguration_->dispatch == COMMON_EVENT_DISPATCH_REACTOR)
    ACE_NEW_NORETURN (isession_p,
                      SessionType ());
  else
    ACE_NEW_NORETURN (isession_p,
                      SessionAsynchType ());
  if (unlikely (!isession_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  if (unlikely (!isession_p->initialize (*sessionConfiguration_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize session, returning\n")));
    goto error;
  } // end IF
  session_state_p = &const_cast<SessionStateType&> (isession_p->state ());

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    sessions_.insert (std::make_pair (metaInfoFileName_in,
                                      isession_p));
  } // end lock scope
  remove_session = true;

  // step3: initialize session state
  iterator = sessionConfiguration_->metaInfo->begin ();
  for (;
       iterator != sessionConfiguration_->metaInfo->end ();
       ++iterator)
    if (*(*iterator).first == ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY))
      break;
  ACE_ASSERT (iterator != sessionConfiguration_->metaInfo->end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
  if (unlikely (!HTTP_Tools::parseURL (*(*iterator).second->string,
                                       host_address,
                                       host_name_string,
                                       URI_string,
                                       use_SSL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                ACE_TEXT (*(*iterator).second->string->c_str ())));
    goto error;
  } // end IF
  session_state_p->trackerAddress =
      Net_Common_Tools::stringToIPAddress (host_name_string,
                                           (use_SSL ? HTTPS_DEFAULT_SERVER_PORT
                                                    : HTTP_DEFAULT_SERVER_PORT));
  if (unlikely (session_state_p->trackerAddress.is_any ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::stringToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (host_name_string.c_str ())));
    goto error;
  } // end IF
  session_state_p->trackerBaseURI = URI_string;

  // step4: send request
  if (unlikely (!getTrackerConnectionAndMessage (isession_p,
                                                 istream_connection_p,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Control_T::getTrackerConnectionAndMessage(), aborting\n")));
    goto error;
  } // end IF
  data_container_p =
    &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
    &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T&> (data_container_p->getR ());
  data_p->method = HTTP_Codes::HTTP_METHOD_GET;
  data_p->URI = URI_string;
  data_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER),
                                                             HTTP_Tools::URLEncode (BitTorrent_Tools::MetaInfoToInfoHash (*sessionConfiguration_->metaInfo))));
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PEER_ID_HEADER),
                                                             HTTP_Tools::URLEncode (session_state_p->peerId)));
  converter << BITTORRENT_DEFAULT_PORT;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PORT_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 0;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_UPLOADED_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 0;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_DOWNLOADED_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter <<
      BitTorrent_Tools::MetaInfoToLength (*sessionConfiguration_->metaInfo);
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_LEFT_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 1;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_COMPACT_HEADER),
                                       converter.str ()));
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_HEADER),
                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_STARTED_STRING)));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  interface_identifier = Net_Common_Tools::getDefaultInterface_2 ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#else
  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#endif // _WIN32_WINNT_VISTA
#else
  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!Net_Common_Tools::interfaceToExternalIPAddress (interface_identifier,
                                                                 external_ip_address)))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
  else
    data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_IP_HEADER),
                                         Net_Common_Tools::IPAddressToString (external_ip_address).c_str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << BITTORRENT_DEFAULT_TRACKER_REQUEST_NUMWANT_PEERS;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_NUMWANT_HEADER),
                                       converter.str ()));
  if (session_state_p->key.empty ())
    session_state_p->key = BitTorrent_Tools::generateKey ();
  //data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_KEY_HEADER),
  //                                     session_state_p->key));
  //data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_TRACKERID_HEADER),
  //                                     session_state_p->trackerId));
#if defined (HAVE_CONFIG_H)
  user_agent  = ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME);
  user_agent += ACE_TEXT_ALWAYS_CHAR ("/");
  user_agent += ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_VERSION);
#endif // HAVE_CONFIG_H
  if (!user_agent.empty ())
    data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_AGENT_STRING),
                                            user_agent));
  if (Net_Common_Tools::matchIPAddress (host_name_string))
  {
    std::string host_name_string_2;
    Net_Common_Tools::getAddress (host_name_string_2,
                                  host_name_string);
    host_name_string = (host_name_string_2.empty () ? host_name_string
                                                    : host_name_string_2);
  } // end IF
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                          host_name_string));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("*/*")));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_ENCODING_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("gzip;q=1.0, deflate, identity")));

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
  if (remove_session)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    SESSIONS_ITERATOR_T iterator = sessions_.find (metaInfoFileName_in);
    if (iterator != sessions_.end ())
      sessions_.erase (iterator);
  } // end IF
  bool stop_b = false;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    if (sessions_.empty ())
      stop_b = true;
  } // end lock scope
  if (stop_b)
    stop (false,
          true);
  if (istream_connection_p)
    istream_connection_p->decrease ();
  if (isession_p)
    delete isession_p;
  if (message_p)
    message_p->release ();
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
SessionInterfaceType*
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::get (const std::string& metaInfoFileName_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::get"));

  SESSIONS_ITERATOR_T iterator;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, NULL);
    iterator = sessions_.find (metaInfoFileName_in);
    if (iterator == sessions_.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to retrieve session handle (metainfo file was: \"%s\"), aborting\n"),
                  ACE_TEXT (metaInfoFileName_in.c_str ())));
      return NULL;
    } // end IF
  } // end lock scope

  return (*iterator).second;
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::notifyTracker (const std::string& metaInfoFileName_in,
                                                       enum BitTorrent_Event event_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::notifyTracker"));

  // sanity check(s)
  ACE_ASSERT (sessionConfiguration_);
  ACE_ASSERT (sessionConfiguration_->metaInfo);

  SESSIONS_ITERATOR_T iterator;
  std::string tracker_base_uri;
  unsigned int uploaded_bytes_i = 0, downloaded_bytes_i = 0, left_bytes_i = 0;
  std::string key_string, peer_id_string, tracker_id_string;
  SessionStateType* session_state_p = NULL;
  Bencoding_DictionaryIterator_t iterator_2;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier = GUID_NULL;
#else
  std::string interface_identifier;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string interface_identifier;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INET_Addr external_ip_address;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  std::ostringstream converter;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
      NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  std::string user_agent;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    iterator = sessions_.find (metaInfoFileName_in);
    if (unlikely (iterator == sessions_.end ()))
    { // *NOTE*: possible cause: tracker error
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to retrieve session handle, returning\n")));
      goto error;
    } // end IF
    session_state_p =
        &const_cast<SessionStateType&> ((*iterator).second->state ());
    tracker_base_uri = session_state_p->trackerBaseURI;
    downloaded_bytes_i = BitTorrent_Tools::receivedBytes (session_state_p->pieces);
    left_bytes_i =
      (BitTorrent_Tools::MetaInfoToLength (*sessionConfiguration_->metaInfo) - downloaded_bytes_i);
    key_string = session_state_p->key;
    peer_id_string = session_state_p->peerId;
    tracker_id_string = session_state_p->trackerId;
    if (!getTrackerConnectionAndMessage ((*iterator).second,
                                         istream_connection_p,
                                         message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to BitTorrent_Control_T::getTrackerConnectionAndMessage(), returning\n")));
      goto error;
    } // end IF
  } // end lock scope
  data_container_p =
    &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
    &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T&> (data_container_p->getR ());
  data_p->method = HTTP_Codes::HTTP_METHOD_GET;
  data_p->URI = tracker_base_uri;
  data_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER),
                                                             HTTP_Tools::URLEncode (BitTorrent_Tools::MetaInfoToInfoHash (*sessionConfiguration_->metaInfo))));
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PEER_ID_HEADER),
                                                             HTTP_Tools::URLEncode (peer_id_string)));
  converter << BITTORRENT_DEFAULT_PORT;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PORT_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << uploaded_bytes_i; // *TODO*
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_UPLOADED_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << downloaded_bytes_i;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_DOWNLOADED_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << left_bytes_i;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_LEFT_HEADER),
                                       converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 1;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_COMPACT_HEADER),
                                       converter.str ()));
  if (event_in != BITTORRENT_EVENT_NO_MORE_PEERS)
    data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_HEADER),
                                         ((event_in == BITTORRENT_EVENT_CANCELLED) ? ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_STOPPED_STRING)
                                                                                   : ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_COMPLETED_STRING))));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  interface_identifier = Net_Common_Tools::getDefaultInterface_2 ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#else
  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#endif // _WIN32_WINNT_VISTA
#else
  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#endif // ACE_WIN32 || ACE_WIN64
  if (!Net_Common_Tools::interfaceToExternalIPAddress (interface_identifier,
                                                       external_ip_address))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
  else
    data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_IP_HEADER),
                                         Net_Common_Tools::IPAddressToString (external_ip_address).c_str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << BITTORRENT_DEFAULT_TRACKER_REQUEST_NUMWANT_PEERS;
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_NUMWANT_HEADER),
                                       converter.str ()));
  //ACE_ASSERT (!key_string.empty ());
  //record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_KEY_HEADER),
  //                                       key_string));
  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_TRACKERID_HEADER),
                                       tracker_id_string));
#if defined (HAVE_CONFIG_H)
  user_agent  = ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME);
  user_agent += ACE_TEXT_ALWAYS_CHAR ("/");
  user_agent += ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_VERSION);
#endif // HAVE_CONFIG_H
  if (!user_agent.empty ())
    data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_AGENT_STRING),
                                            user_agent));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                          Net_Common_Tools::IPAddressToString (session_state_p->trackerAddress,
                                                                               true,
                                                                               true)));
//                                            HTTP_Tools::IPAddress2HostName (tracker_address).c_str ()));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("*/*")));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_ENCODING_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("gzip;q=1.0, deflate, identity")));

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
  if (istream_connection_p)
    istream_connection_p->decrease ();
  if (message_p)
    message_p->release ();
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::stop (bool waitForCompletion_in,
                                              bool highPriority_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::stop"));

  unsigned int sessions_i = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    sessions_i = sessions_.size ();
    for (SESSIONS_ITERATOR_T iterator = sessions_.begin ();
         iterator != sessions_.end ();
         ++iterator)
    {
      try {
        (*iterator).second->close (false); // wait ?
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_ISession_T::close(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // stop worker thread
  inherited::stop (false,
                   highPriority_in);

  if (waitForCompletion_in)
    wait ();

  if (sessions_i)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("cancelled %d session(s)\n"),
                sessions_i));
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::wait (bool waitForQueue_in) const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::wait"));

  int result = -1;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    while (!sessions_.empty ())
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting (session count: %u)...\n"),
                  sessions_.size ()));
      result = condition_.wait ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition_Thread_Mutex::wait(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  inherited::wait (waitForQueue_in);
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::handle (struct BitTorrent_Control_Event*& event_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::handle"));

  // sanity check(s)
  ACE_ASSERT (event_inout);

  int result = -1;

  switch (event_inout->type)
  {
    case BITTORRENT_EVENT_CANCELLED:
    case BITTORRENT_EVENT_COMPLETE:
    {
      notifyTracker (event_inout->metaInfoFileName,
                     static_cast<enum BitTorrent_Event> (event_inout->type));

      SESSIONS_ITERATOR_T iterator;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
        iterator = sessions_.find (event_inout->metaInfoFileName);
        ACE_ASSERT (iterator != sessions_.end ());

        // close tracker connection
        Net_ConnectionId_t tracker_connection_id =
            (*iterator).second->trackerConnectionId ();
        typedef typename SessionType::TRACKER_CONNECTION_MANAGER_SINGLETON_T TRACKER_CONNECTION_MANAGER_SINGLETON_2;
        typename SessionType::ITRACKER_CONNECTION_T* iconnection_p =
            TRACKER_CONNECTION_MANAGER_SINGLETON_2::instance ()->get (tracker_connection_id);
        if (!iconnection_p)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve tracker connection (id was: %u) handle, continuing\n"),
                      tracker_connection_id));
        else
        {
          iconnection_p->abort ();
          iconnection_p->decrease (); iconnection_p = NULL;
        } // end IF

        // close all session connections
        (*iterator).second->close (true); // wait ?

        sessions_.erase (iterator);
        if (!sessions_.empty ())
          break;

        // awaken any waiter(s)
        result = condition_.broadcast ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));

        stop (false,
              true);
      } // end lock scope
      break;
    }
    case BITTORRENT_EVENT_NO_MORE_PEERS:
    {
      notifyTracker (event_inout->metaInfoFileName,
                     static_cast<enum BitTorrent_Event> (event_inout->type));
      break;
    }
    case BITTORRENT_EVENT_TRACKER_REDIRECTED:
    { ACE_ASSERT (!event_inout->data.empty ());
      SESSIONS_ITERATOR_T iterator;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
        iterator = sessions_.find (event_inout->metaInfoFileName);
        ACE_ASSERT (iterator != sessions_.end ());

        // close tracker connection
        Net_ConnectionId_t tracker_connection_id =
            (*iterator).second->trackerConnectionId ();
        typedef typename SessionType::TRACKER_CONNECTION_MANAGER_SINGLETON_T TRACKER_CONNECTION_MANAGER_SINGLETON_2;
        typename SessionType::ITRACKER_CONNECTION_T* iconnection_p =
            TRACKER_CONNECTION_MANAGER_SINGLETON_2::instance ()->get (tracker_connection_id);
        if (!iconnection_p)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve tracker connection (id was: %u) handle, continuing\n"),
                      tracker_connection_id));
        else
        {
          iconnection_p->abort ();
          iconnection_p->decrease (); iconnection_p = NULL;
        } // end IF
        (*iterator).second->wait ();

        requestRedirected ((*iterator).second,
                           event_inout->data);
      } // end lock scope
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown event (type was: %d), returning\n"),
                  event_inout->type));
      break;
    }
  } // end SWITCH

  delete event_inout; event_inout = NULL;
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::notify (const std::string& metaInfoFileName_in,
                                                enum BitTorrent_Event event_in,
                                                const std::string& optionalData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::notify"));

  struct BitTorrent_Control_Event* event_p = NULL;
  ACE_NEW_NORETURN (event_p,
                    struct BitTorrent_Control_Event ());
  ACE_ASSERT (event_p);
  event_p->type = event_in;
  event_p->metaInfoFileName = metaInfoFileName_in;
  event_p->data = optionalData_in;

  int result = inherited::put (event_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Task_Ex_T::put(): \"%m\", returning\n")));
    delete event_p; event_p = NULL;
  } // end IF
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
bool
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::getTrackerConnectionAndMessage (SessionInterfaceType* session_in,
                                                                        typename SessionType::ITRACKER_STREAM_CONNECTION_T*& connection_out,
                                                                        typename SessionType::TRACKER_MESSAGE_T*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::getTrackerConnectionAndMessage"));

  // sanity check(s)
  ACE_ASSERT (session_in);
  ACE_ASSERT (!connection_out);
  ACE_ASSERT (!message_out);

  const typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T* stream_p =
    NULL;
  const typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  const typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::SESSION_DATA_T* session_data_p =
    NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
    NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
    NULL;
  Net_ConnectionId_t tracker_connection_id = 0;
  typename SessionType::ITRACKER_CONNECTION_T* iconnection_p = NULL;
  unsigned int buffer_size_i = 0;

  ACE_NEW_NORETURN (data_p,
                    typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF

  // *IMPORTANT NOTE*: fire-and-forget API (data_p)
  ACE_NEW_NORETURN (data_container_p,
                    typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T (data_p,
                                                                                                     true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  ACE_ASSERT (!data_p);

  ACE_ASSERT (sessionConfiguration_);
  ACE_ASSERT (sessionConfiguration_->trackerConnectionConfiguration);
  ACE_ASSERT (sessionConfiguration_->trackerConnectionConfiguration->allocatorConfiguration);
  buffer_size_i =
      sessionConfiguration_->trackerConnectionConfiguration->allocatorConfiguration->defaultBufferSize +
      sessionConfiguration_->trackerConnectionConfiguration->allocatorConfiguration->paddingBytes;

allocate:
  message_out =
    static_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T*> (sessionConfiguration_->trackerConnectionConfiguration->messageAllocator->malloc (buffer_size_i));
  // keep retrying ?
  if (!message_out &&
      !sessionConfiguration_->trackerConnectionConfiguration->messageAllocator->block ())
    goto allocate;
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate request message: \"%m\", returning\n")));
    data_container_p->decrease (); data_container_p = NULL;
    goto error;
  } // end IF

  // reuse tracker connection(s)
  tracker_connection_id = session_in->trackerConnectionId ();
  typedef typename SessionType::TRACKER_CONNECTION_MANAGER_SINGLETON_T TRACKER_CONNECTION_MANAGER_SINGLETON_2;
  if (!tracker_connection_id)
  {
    ACE_INET_Addr tracker_address = session_in->trackerAddress ();
    session_in->trackerConnect (tracker_address);
    iconnection_p =
      TRACKER_CONNECTION_MANAGER_SINGLETON_2::instance ()->get (tracker_address);
  } // end IF
  else
    iconnection_p =
      TRACKER_CONNECTION_MANAGER_SINGLETON_2::instance ()->get (tracker_connection_id);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve tracker connection handle, returning\n")));
    goto error;
  } // end IF
  connection_out =
    dynamic_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T*> (iconnection_p);
  if (!connection_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    iconnection_p->decrease (); iconnection_p = NULL;
    goto error;
  } // end IF

  stream_p = &connection_out->stream ();
  session_data_container_p = &stream_p->getR_2 ();
  session_data_p = &session_data_container_p->getR ();

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

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::requestRedirected (SessionInterfaceType* session_in,
                                                           const std::string& redirectedURL_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::requestRedirected"));

  // sanity check(s)
  ACE_ASSERT (sessionConfiguration_);
  ACE_ASSERT (session_in);

  ACE_INET_Addr host_address;
  std::string host_name_string, URI_string;
  bool use_SSL = false;
  SessionStateType* session_state_p =
    &const_cast<SessionStateType&> (session_in->state ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier = GUID_NULL;
#else
  std::string interface_identifier;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string interface_identifier;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INET_Addr external_ip_address;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  std::ostringstream converter;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
      NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  std::string user_agent;

  // step1: update tracker address / URI
  if (!HTTP_Tools::parseURL (redirectedURL_in,
                             host_address,
                             host_name_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (redirectedURL_in.c_str ())));
    return;
  } // end IF
  session_state_p->trackerAddress =
      Net_Common_Tools::stringToIPAddress (host_name_string,
                                           (use_SSL ? HTTPS_DEFAULT_SERVER_PORT
                                                    : HTTP_DEFAULT_SERVER_PORT));
  session_state_p->trackerBaseURI = HTTP_Tools::stripURI (URI_string);

  // step2: send request
  if (!getTrackerConnectionAndMessage (session_in,
                                       istream_connection_p,
                                       message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Control_T::getTrackerConnectionAndMessage(), returning\n")));
    return;
  } // end IF
  data_container_p =
      &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
      &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T&> (data_container_p->getR ());
  data_p->method = HTTP_Codes::HTTP_METHOD_GET;
  data_p->URI = URI_string;
  data_p->version = HTTP_Codes::HTTP_VERSION_1_1;
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER),
//                                                             HTTP_Tools::URLEncode (BitTorrent_Tools::MetaInfoToInfoHash (*configuration_->metaInfo))));
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PEER_ID_HEADER),
//                                                             HTTP_Tools::URLEncode (session_state_p->peerId)));
//  converter << BITTORRENT_DEFAULT_PORT;
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PORT_HEADER),
//                                       converter.str ()));
//  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//  converter.clear ();
//  converter << 0;
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_UPLOADED_HEADER),
//                                       converter.str ()));
//  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//  converter.clear ();
//  converter << 0;
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_DOWNLOADED_HEADER),
//                                       converter.str ()));
//  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//  converter.clear ();
//  converter << BitTorrent_Tools::MetaInfoToLength (*configuration_->metaInfo);
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_LEFT_HEADER),
//                                       converter.str ()));
//  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//  converter.clear ();
//  converter << 1;
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_COMPACT_HEADER),
//                                       converter.str ()));
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_HEADER),
//                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_STARTED_STRING)));
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//  interface_identifier = Net_Common_Tools::getDefaultInterface_2 ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
//#else
//  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
//#endif // _WIN32_WINNT_VISTA
//#else
//  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
//#endif // ACE_WIN32 || ACE_WIN64
//  if (!Net_Common_Tools::interfaceToExternalIPAddress (interface_identifier,
//                                                       external_ip_address))
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
//                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
//                ACE_TEXT (interface_identifier.c_str ())));
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
//                ACE_TEXT (interface_identifier.c_str ())));
//#endif // ACE_WIN32 || ACE_WIN64
//  else
//    data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_IP_HEADER),
//                                         Net_Common_Tools::IPAddressToString (external_ip_address).c_str ()));
//  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//  converter.clear ();
//  converter << BITTORRENT_DEFAULT_TRACKER_REQUEST_NUMWANT_PEERS;
//  data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_NUMWANT_HEADER),
//                                       converter.str ()));
//  if (session_state_p->key.empty ())
//    session_state_p->key = BitTorrent_Tools::generateKey ();
  //data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_KEY_HEADER),
  //                                     session_state_p->key));
  //data_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_TRACKERID_HEADER),
  //                                     session_state_p->trackerId));
#if defined (HAVE_CONFIG_H)
  user_agent  = ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME);
  user_agent += ACE_TEXT_ALWAYS_CHAR ("/");
  user_agent += ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_VERSION);
#endif // HAVE_CONFIG_H
  if (!user_agent.empty ())
    data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_AGENT_STRING),
                                            user_agent));
  if (Net_Common_Tools::matchIPAddress (host_name_string))
  {
    std::string host_name_string_2;
    Net_Common_Tools::getAddress (host_name_string_2,
                                  host_name_string);
    host_name_string = (host_name_string_2.empty () ? host_name_string
                                                    : host_name_string_2);
  } // end IF
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                          host_name_string));
//                                            HTTP_Tools::IPAddress2HostName (tracker_address).c_str ()));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("*/*")));
  data_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ACCEPT_ENCODING_STRING),
                                          ACE_TEXT_ALWAYS_CHAR ("gzip;q=1.0, deflate, identity")));

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
  if (istream_connection_p)
    istream_connection_p->decrease ();
  if (message_p)
    message_p->release ();
}
