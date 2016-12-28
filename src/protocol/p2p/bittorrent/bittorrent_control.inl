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

#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/OS_Memory.h>
#include <ace/Time_Value.h>

#include "net_defines.h"
#include "net_macros.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

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
                     SessionStateType>::BitTorrent_Control_T (SessionConfigurationType* configuration_in)
 : condition_ (lock_)
 , configuration_ (configuration_in)
 , lock_ ()
 , sessions_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::BitTorrent_Control_T"));

}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionInterfaceType,
                     SessionStateType>::~BitTorrent_Control_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::~BitTorrent_Control_T"));

  stop (true);
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
                  ACE_TEXT ("failed to retrieve session handle (metainfo filename was: \"%s\"), aborting\n"),
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
                     SessionStateType>::request (const std::string& metaInfoFileName_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::request"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->trackerSocketHandlerConfiguration);
  ACE_ASSERT (configuration_->trackerSocketHandlerConfiguration->messageAllocator);

  bool remove_session = false;
  typename SessionType::ISESSION_T* isession_p = NULL;
  SessionStateType* session_state_p = NULL;
  Bencoding_DictionaryIterator_t iterator;
  std::string interface_string, host_name_string;
  ACE_INET_Addr tracker_address, external_ip_address;
  int result = -1;
  typename SessionType::ITRACKER_CONNECTION_T* iconnection_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  struct HTTP_Record* record_p = NULL;
  std::ostringstream converter;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  std::string key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY);
  std::string user_agent;
  // *TODO*: support more link layer types
  int link_layers = (NET_LINKLAYER_802_3 | NET_LINKLAYER_PPP);

  // step1: parse metainfo
  ACE_ASSERT (configuration_->parserConfiguration);
  ACE_ASSERT (!configuration_->metaInfo);
  if (!BitTorrent_Tools::parseMetaInfoFile (*configuration_->parserConfiguration,
                                            metaInfoFileName_in,
                                            configuration_->metaInfo))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Tools::parseMetaInfoFile(\"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (BitTorrent_Tools::MetaInfo2String (*configuration_->metaInfo).c_str ())));
#endif

  // step2: create/initialize session
  if (configuration_->useReactor)
    ACE_NEW_NORETURN (isession_p,
                      SessionType ());
  else
    ACE_NEW_NORETURN (isession_p,
                      SessionAsynchType ());
  if (!isession_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  if (!isession_p->initialize (*configuration_))
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

  // step3: connect to the tracker
  ACE_NEW_NORETURN (record_p,
                    struct HTTP_Record ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF

//  iterator = configuration_->metaInfo->find (&key);
  iterator = configuration_->metaInfo->begin ();
  for (;
       iterator != configuration_->metaInfo->end ();
       ++iterator)
    if (*(*iterator).first == key) break;
  ACE_ASSERT (iterator != configuration_->metaInfo->end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
  if (!HTTP_Tools::parseURL (*(*iterator).second->string,
                             host_name_string,
//                             tracker_address,
                             record_p->URI))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                ACE_TEXT (*(*iterator).second->string->c_str ())));
    goto error;
  } // end IF
  result = tracker_address.set (host_name_string.c_str (),
                                AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"), aborting\n"),
                ACE_TEXT (host_name_string.c_str ())));
    goto error;
  } // end IF

  // step4: send request to the tracker
  record_p->method = HTTP_Codes::HTTP_METHOD_GET;
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER),
                                         HTTP_Tools::URLEncode (BitTorrent_Tools::MetaInfo2InfoHash (*configuration_->metaInfo))));
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PEER_ID_HEADER),
                                         HTTP_Tools::URLEncode (session_state_p->peerId)));
  converter << BITTORRENT_DEFAULT_PORT;
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PORT_HEADER),
                                         converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 0;
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_UPLOADED_HEADER),
                                         converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << BitTorrent_Tools::MetaInfo2Length (*configuration_->metaInfo);
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_LEFT_HEADER),
                                         converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 1;
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_COMPACT_HEADER),
                                         converter.str ()));
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_HEADER),
                                         ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_STARTED_STRING)));
  interface_string = Net_Common_Tools::getDefaultInterface (link_layers);
  if (!Net_Common_Tools::interface2ExternalIPAddress (interface_string,
                                                      external_ip_address))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2ExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (interface_string.c_str ())));
  else
    record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_IP_HEADER),
                                           Net_Common_Tools::IPAddress2String (external_ip_address).c_str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << BITTORRENT_DEFAULT_TRACKER_REQUEST_NUMWANT_PEERS;
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_NUMWANT_HEADER),
                                         converter.str ()));
  if (session_state_p->key.empty ())
    session_state_p->key = BitTorrent_Tools::generateKey ();
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_KEY_HEADER),
                                         session_state_p->key));
  record_p->form.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_TRACKERID_HEADER),
                                         session_state_p->trackerId));

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
                    typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T (record_p,
                                                                                                     true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  ACE_ASSERT (!record_p);
allocate:
  message_p =
    static_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T*> (configuration_->trackerSocketHandlerConfiguration->messageAllocator->malloc (configuration_->trackerSocketHandlerConfiguration->PDUSize));
  // keep retrying ?
  if (!message_p &&
      !configuration_->trackerSocketHandlerConfiguration->messageAllocator->block ())
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
                         NULL);
  ACE_ASSERT (!data_container_p);

  isession_p->trackerConnect (tracker_address);
  iconnection_p =
    configuration_->trackerConnectionManager->get (tracker_address);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to the tracker (was: %s), returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddress2String (tracker_address).c_str ())));
    goto error;
  } // end IF
  remove_session = false;
  istream_connection_p =
      dynamic_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T*> (iconnection_p);
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
  if (remove_session)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    SESSIONS_ITERATOR_T iterator = sessions_.find (metaInfoFileName_in);
    if (iterator != sessions_.end ())
      sessions_.erase (iterator);
  } // end IF
  if (iconnection_p)
    iconnection_p->decrease ();
  if (isession_p)
    delete isession_p;
  if (record_p)
    delete record_p;
  if (data_container_p)
    data_container_p->decrease ();
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
                     SessionStateType>::stop (bool waitForCompletion_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::stop"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

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

  if (waitForCompletion_in)
    wait ();

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    if (sessions_.size ())
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled %d session(s)\n"),
                  sessions_.size ()));

      sessions_.clear ();
    } // end IF
  } // end lock scope
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
                     SessionStateType>::wait ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::wait"));

  int result = -1;

  // synch access
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    while (!sessions_.empty ())
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting (session count: %u)...\n"),
                  sessions_.size ()));

      result = condition_.wait ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_CONDITION::wait(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope
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
                                                enum BitTorrent_Event event_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::notify"));

  int result = -1;

  switch (event_in)
  {
    case BITTORRENT_EVENT_CANCELLED:
    case BITTORRENT_EVENT_COMPLETE:
    {
      SESSIONS_ITERATOR_T iterator;

      // synch access
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

        iterator = sessions_.find (metaInfoFileName_in);
        ACE_ASSERT (iterator != sessions_.end ());
        sessions_.erase (iterator);

        // awaken any waiter(s)
        result = condition_.broadcast ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
      } // end lock scope
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown event (type was: %d), returning\n"),
                  event_in));
      break;
    }
  } // end SWITCH
}
