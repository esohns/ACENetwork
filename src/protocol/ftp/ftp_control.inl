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

#include "ace/Log_Msg.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename DataMessageType>
FTP_Control_T<DataMessageType>::FTP_Control_T ()
 : currentDataConnection_ (ACE_INVALID_HANDLE)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::FTP_Control_T"));

}

template <typename DataMessageType>
void
FTP_Control_T<DataMessageType>::cwd (const std::string& path_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::cwd"));
}

template <typename DataMessageType>
void
FTP_Control_T<DataMessageType>::list ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::list"));

  // sanity check(s)
  ACE_ASSERT (sessionConfiguration_);

  typename SessionType::ITRACKER_STREAM_CONNECTION_T* istream_connection_p = NULL;
  std::ostringstream converter;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T* message_p =
      NULL;
  ACE_Message_Block* message_block_p = NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T* data_p =
      NULL;
  typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T* data_container_p =
      NULL;

  // step4: send request
  if (unlikely (!getTrackerConnectionAndMessage (isession_p,
                                                 istream_connection_p,
                                                 message_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to FTP_Control_T::getTrackerConnectionAndMessage(), aborting\n")));
    goto error;
  } // end IF
  data_container_p =
    &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T&> (message_p->getR ());
  data_p =
    &const_cast<typename SessionType::ITRACKER_STREAM_CONNECTION_T::STREAM_T::MESSAGE_T::DATA_T::DATA_T&> (data_container_p->getR ());
  data_p->method = HTTP_Codes::HTTP_METHOD_GET;

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
  if (isession_p)
    delete isession_p;
  if (message_p)
    message_p->release ();
}

template <typename DataMessageType>
bool
FTP_Control_T<DataMessageType>::getDataConnectionAndMessage (ACE_HANDLE& connection_out,
                                                             DataMessageType*& message_out)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Control_T::getDataConnectionAndMessage"));

  // sanity check(s)
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
