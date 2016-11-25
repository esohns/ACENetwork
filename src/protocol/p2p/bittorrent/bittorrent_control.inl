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
          typename SessionStateType>
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionStateType>::BitTorrent_Control_T (SessionConfigurationType* configuration_in)
 : configuration_ (configuration_in)
 , lock_ ()
 , sessions_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::BitTorrent_Control_T"));

}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionStateType>
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionStateType>::~BitTorrent_Control_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::~BitTorrent_Control_T"));

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

  for (SESSIONS_ITERATOR_T iterator = sessions_.begin ();
       iterator != sessions_.end ();
       ++iterator)
  {
    (*iterator).second->close (true); // wait
    delete (*iterator).second;
  } // end FOR
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionStateType>
typename SessionType::ISESSION_T*
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionStateType>::get (const std::string& metaInfoFileName_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::get"));

  SESSIONS_ITERATOR_T iterator;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, NULL);

    iterator = sessions_.find (metaInfoFileName_in);
    if (iterator == sessions_.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to retrieve session handle (filename was: \"%s\"), aborting\n"),
                  ACE_TEXT (metaInfoFileName_in.c_str ())));
      return NULL;
    } // end IF
  } // end lock scope

  return (*iterator).second;
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionStateType>::download (const std::string& metaInfoFileName_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::download"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->trackerSocketHandlerConfiguration);
  ACE_ASSERT (configuration_->trackerSocketHandlerConfiguration->messageAllocator);

  typename SessionType::ISESSION_T* isession_p = NULL;
  SessionStateType* session_state_p = NULL;
  Bencoding_DictionaryIterator_t iterator;
  std::string URI_string;
  ACE_INET_Addr tracker_address, external_ip_address;
  ACE_Time_Value deadline;
  ACE_Time_Value one_second (1, 0);
  int result = -1;
  typename SessionType::ICONNECTION_T* iconnection_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  struct HTTP_Record* record_p = NULL;
  std::ostringstream converter;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;
  std::string key = ACE_TEXT_ALWAYS_CHAR (BITTORRENT_METAINFO_ANNOUNCE_KEY);

  // step1: parse metainfo
  ACE_ASSERT (!configuration_->metaInfo);
  if (!BitTorrent_Tools::parseMetaInfoFile (metaInfoFileName_in,
                                            configuration_->metaInfo,
                                            configuration_->traceScanning,
                                            configuration_->traceParsing))
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

  // step3: connect to the tracker
  iterator = configuration_->metaInfo->find (&key);
  ACE_ASSERT (iterator != configuration_->metaInfo->end ());
  ACE_ASSERT ((*iterator).second->type == Bencoding_Element::BENCODING_TYPE_STRING);
  if (!HTTP_Tools::parseURL (*(*iterator).second->string,
                             tracker_address,
                             record_p->URI))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                ACE_TEXT (*(*iterator).second->string->c_str ())));
    goto error;
  } // end IF

  isession_p->trackerConnect (tracker_address);
  // *TODO*: find a better way to do this
  deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT, 0));
  do
  {
    result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &one_second));

    // *TODO*: this does not work...
    iconnection_p = configuration_->connectionManager->get (tracker_address);
    if (iconnection_p)
      break; // done
  } while (COMMON_TIME_NOW < deadline);
  if (!iconnection_p)
  {
    // debug info
    result = tracker_address.addr_to_string (buffer,
                                             sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to the tracker (\"%s\"), aborting\n"),
                buffer));

    goto error;
  } // end IF
  istream_connection_p =
      dynamic_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T*> (iconnection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    goto error;
  } // end IF

  // step4: send request to the tracker

  record_p->method = HTTP_Codes::HTTP_METHOD_GET;
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_INFO_HASH_HEADER),
                                           HTTP_Tools::URLEncode (BitTorrent_Tools::MetaInfo2InfoHash (*configuration_->metaInfo))));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PEER_ID_HEADER),
                                           HTTP_Tools::URLEncode (BitTorrent_Tools::generatePeerId ())));
  converter << BITTORRENT_DEFAULT_PORT;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_PORT_HEADER),
                                           converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 0;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_UPLOADED_HEADER),
                                           converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << BitTorrent_Tools::MetaInfo2Length (*configuration_->metaInfo);
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_LEFT_HEADER),
                                           converter.str ()));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 1;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_COMPACT_HEADER),
                                           converter.str ()));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_HEADER),
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_EVENT_STARTED_STRING)));
  if (!Net_Common_Tools::interface2ExternalIPAddress (ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET),
                                                      external_ip_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2ExternalIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (NET_INTERFACE_DEFAULT_ETHERNET)));
    goto error;
  } // end IF
  result = external_ip_address.addr_to_string (buffer,
                                               sizeof (buffer));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", returning\n")));
    goto error;
  } // end IF
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_IP_HEADER),
                                           ACE_TEXT_ALWAYS_CHAR (buffer)));
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << 0;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_NUMWANT_HEADER),
                                           converter.str ()));
  if (session_state_p->key.empty ())
    session_state_p->key = BitTorrent_Tools::generateKey ();
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_KEY_HEADER),
                                           session_state_p->key));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_TRACKER_REQUEST_TRACKERID_HEADER),
                                           session_state_p->trackerId));

  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  ACE_NEW_NORETURN (data_container_p,
//                    Test_U_MessageData_t (message_data_p,
                    typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T (record_p,
                                                                                                     true));
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
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
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));

    // clean up
    data_container_p->decrease ();

    goto error;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  message_p->initialize (data_container_p,
                         NULL);

  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  message_block_p = message_p;
  try {
    istream_connection_p->send (message_block_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_IStreamConnection_T::send(), returning\n")));
    goto error;
  }

  // step5: clean up
  iconnection_p->decrease ();

  return;

error:
  if (iconnection_p)
    iconnection_p->decrease ();
  if (isession_p)
    delete isession_p;
}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionStateType>::stop (bool waitForCompletion_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::stop"));

}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionStateType>::wait ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::wait"));

}

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionStateType>
void
BitTorrent_Control_T<SessionAsynchType,
                     SessionType,
                     SessionConfigurationType,
                     SessionStateType>::notify (const std::string& metaInfoFileName_in,
                                                enum BitTorrent_Event event_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Control_T::notify"));

}
