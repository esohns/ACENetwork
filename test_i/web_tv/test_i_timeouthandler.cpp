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
#include "stdafx.h"

#include "test_i_timeouthandler.h"

#include "ace/Log_Msg.h"

#include "common_configuration.h"

#include "common_timer_manager_common.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "http_tools.h"

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_connection_stream.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_web_tv_common.h"

Test_I_TimeoutHandler::Test_I_TimeoutHandler ()
 : inherited (this,  // dispatch interface
              false) // invoke only once ?
 , interval_ (ACE_Time_Value::zero)
 , timerId_ (0)
 , lock_ (NULL)
 , handle_ (ACE_INVALID_HANDLE)
 , segment_ (NULL)
 , segmentURL_ ()
 , allocatorConfiguration_ (NULL)
 , messageAllocator_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_TimeoutHandler::Test_I_TimeoutHandler"));

}

void
Test_I_TimeoutHandler::initialize (ACE_HANDLE handle_in,
                                   struct Test_I_WebTV_ChannelSegment* segment_in,
                                   const std::string& segmentURL_in,
                                   struct Common_AllocatorConfiguration* allocatorConfiguration_in,
                                   Stream_IAllocator* messageAllocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_TimeoutHandler::initialize"));

  handle_ = handle_in;
  segment_ = segment_in;
  segmentURL_ = segmentURL_in;
  allocatorConfiguration_ = allocatorConfiguration_in;
  messageAllocator_ = messageAllocator_in;

  // sanity check(s)
  ACE_ASSERT (handle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (segment_);
  ACE_ASSERT (!segmentURL_.empty ());
  ACE_ASSERT (allocatorConfiguration_);
  ACE_ASSERT (messageAllocator_);
}

void
Test_I_TimeoutHandler::handle (const void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_TimeoutHandler::handle"));

  ACE_UNUSED_ARG (arg_in);

  // sanity check(s)
  std::string current_URL;
  if (unlikely (segment_->URLs.size () == 1))
    return; // '1' because at least one URL is required to compute subsequent URLs
  ACE_ASSERT (lock_);
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, *lock_);
    current_URL = segment_->URLs.front ();
    segment_->URLs.pop_front ();
  } // end lock scope
  bool is_URI_b = HTTP_Tools::URLIsURI (current_URL);
  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string, URI_string_2, URL_string;
  bool use_SSL = false;
  typename Test_I_ConnectionManager_3_t::INTERFACE_T* iconnection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON_3::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_3_t::ICONNECTION_T* iconnection_p = NULL;
  Test_I_IStreamConnection_3* istream_connection_p = NULL;
  HTTP_Form_t HTTP_form;
  HTTP_Headers_t HTTP_headers;
  struct HTTP_Record* HTTP_record_p = NULL;
  Test_I_Message::DATA_T* message_data_p = NULL;
  Test_I_Message* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;

  if (is_URI_b)
    URL_string = segmentURL_;
  else
    URL_string = current_URL;
  if (!HTTP_Tools::parseURL (URL_string,
                             host_address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (URL_string.c_str ())));
    return;
  } // end IF

  if (is_URI_b)
  {
    size_t position = URI_string.find_last_of ('/', std::string::npos);
    ACE_ASSERT (position != std::string::npos);
    URI_string.erase (position + 1, std::string::npos);
    URI_string_2 = URI_string;
    URI_string_2 += current_URL;
    URI_string = URI_string_2;
  } // end IF

  // send HTTP request
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle_));
#else
      iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (handle_));
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!iconnection_p))
    return;
  istream_connection_p =
      dynamic_cast<Test_I_IStreamConnection_3*> (iconnection_p);
  ACE_ASSERT (istream_connection_p);
  const Test_I_IStreamConnection_3::STREAM_T& stream_r =
      istream_connection_p->stream ();

  ACE_NEW_NORETURN (HTTP_record_p,
                    struct HTTP_Record ());
  if (unlikely (!HTTP_record_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    iconnection_p->decrease (); iconnection_p = NULL;
    return;
  } // end IF
  HTTP_record_p->form = HTTP_form;
  HTTP_headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                       hostname_string));
  HTTP_record_p->headers = HTTP_headers;
  HTTP_record_p->method =
      (HTTP_form.empty () ? HTTP_Codes::HTTP_METHOD_GET
                          : HTTP_Codes::HTTP_METHOD_POST);
  HTTP_record_p->URI = URI_string;
  HTTP_record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

  ACE_NEW_NORETURN (message_data_p,
                    Test_I_Message::DATA_T ());
  if (unlikely (!message_data_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    delete HTTP_record_p; HTTP_record_p = NULL;
    iconnection_p->decrease (); iconnection_p = NULL;
    return;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (HTTP_record_p)
  message_data_p->setPR (HTTP_record_p);

allocate:
  message_p =
      static_cast<Test_I_Message*> (messageAllocator_->malloc (allocatorConfiguration_->defaultBufferSize));
  // keep retrying ?
  if (!message_p &&
      !messageAllocator_->block ())
    goto allocate;
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
    delete message_data_p; message_data_p = NULL;
    iconnection_p->decrease (); iconnection_p = NULL;
    return;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  message_p->initialize (message_data_p,
                         stream_r.getR_2 ().getR ().sessionId,
                         NULL);

  message_block_p = message_p;
  istream_connection_p->send (message_block_p);
  message_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
             ACE_TEXT ("requesting: \"%s\"\n"),
             ACE_TEXT (URI_string.c_str ())));

  // clean up
  iconnection_p->decrease (); iconnection_p = NULL;
}
