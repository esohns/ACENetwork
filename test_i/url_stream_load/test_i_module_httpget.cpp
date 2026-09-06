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

#include "test_i_module_httpget.h"

#include "ace/Log_Msg.h"

#include "common_string_tools.h"

#include "net_macros.h"

#include "http_tools.h"

#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT

Test_I_Module_HTTPGet::Test_I_Module_HTTPGet (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , handle_ (ACE_INVALID_HANDLE)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTTPGet::Test_I_Module_HTTPGet"));

}

void
Test_I_Module_HTTPGet::handleDataMessage (Test_I_Message*& message_inout,
                                          bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTTPGet::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  HTTP_HeadersIterator_t iterator;
  struct Test_I_URLStreamLoad_SessionData& session_data_r =
    const_cast<struct Test_I_URLStreamLoad_SessionData&> (inherited::sessionData_->getR ());

  ACE_ASSERT (message_inout->isInitialized ());
  const Test_I_MessageDataContainer& data_container_r =
    message_inout->getR ();
  struct Test_I_URLStreamLoad_MessageData& data_r =
    const_cast<struct Test_I_URLStreamLoad_MessageData&> (data_container_r.getR ());

  switch (data_r.status)
  {
    case HTTP_Codes::HTTP_STATUS_OK:
    {
      inherited::receivedBytes_ += message_inout->total_length ();

      // got all data ? --> close connection ?
      iterator =
        data_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
      if (iterator != data_r.headers.end ())
      {
        std::istringstream converter ((*iterator).second);
        ACE_UINT64 content_length = 0;
        converter >> content_length;
        ACE_ASSERT (session_data_r.lock);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          if (inherited::configuration_->closeAfterReception         &&
              (content_length && (content_length == receivedBytes_)) &&
              session_data_r.connection)
          {
            ACE_DEBUG ((LM_DEBUG,
                       ACE_TEXT ("%s: received all content, aborting connection\n"),
                       inherited::mod_->name ()));
            ACE_ASSERT (session_data_r.connection);
            session_data_r.connection->abort ();
          } // end IF
        } // end lock scope
      } // end IF
      else // most likely: chunked transfer
        ACE_DEBUG ((LM_WARNING,
                   ACE_TEXT ("%s: missing \"%s\" HTTP header, continuing\n"),
                   inherited::mod_->name (),
                   ACE_TEXT (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
      break; // done
    }
    case HTTP_Codes::HTTP_STATUS_MULTIPLECHOICES:
    case HTTP_Codes::HTTP_STATUS_MOVEDPERMANENTLY:
    case HTTP_Codes::HTTP_STATUS_MOVEDTEMPORARILY:
    case HTTP_Codes::HTTP_STATUS_NOTMODIFIED:
    case HTTP_Codes::HTTP_STATUS_USEPROXY:
    case HTTP_Codes::HTTP_STATUS_SWITCHPROXY:
    case HTTP_Codes::HTTP_STATUS_TEMPORARYREDIRECT:
    case HTTP_Codes::HTTP_STATUS_PERMANENTREDIRECT:
    {
      // step1: redirected --> extract location
      iterator =
        data_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_LOCATION_STRING));
      if (iterator == data_r.headers.end ())
      {
        ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("%s: missing \"%s\" HTTP header, aborting\n"),
                   inherited::mod_->name (),
                   ACE_TEXT (HTTP_PRT_HEADER_LOCATION_STRING)));
        goto error;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //           ACE_TEXT ("%s: \"%s\" has been redirected to \"%s\" (status was: %d)\n"),
      //           inherited::mod_->name (),
      //           ACE_TEXT (inherited::configuration_->URL.c_str ()),
      //           ACE_TEXT ((*iterator).second.c_str ()),
      //           data_r.status));

      // step2: send request ?
      ACE_INET_Addr host_address;
      std::string host_name_string, host_name_string_2, uri_string, uri_string_2;
      bool use_SSL = false, use_SSL_2 = false;
      // *IMPORTANT NOTE*: only auto-effectuate same-server/protocol redirects
      if (!HTTP_Tools::parseURL ((*iterator).second,
                                 host_address,
                                 host_name_string,
                                 uri_string,
                                 use_SSL))
      {
        ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("%s: failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                   inherited::mod_->name (),
                   ACE_TEXT ((*iterator).second.c_str ())));
        goto error;
      } // end IF
      if (!HTTP_Tools::parseURL (inherited::configuration_->URL,
                                 host_address,
                                 host_name_string_2,
                                 uri_string_2,
                                 use_SSL_2))
      {
        ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("%s: failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                   inherited::mod_->name (),
                   ACE_TEXT ((*iterator).second.c_str ())));
        goto error;
      } // end IF
      if (likely ((host_name_string != host_name_string_2) ||
                  (use_SSL != use_SSL_2)))
      {
        //ACE_DEBUG ((LM_WARNING,
        //           ACE_TEXT ("%s: \"%s\" redirects to a different host (was: \"%s\"), and/or requires a HTTP(S) connection, continuing\n"),
        //           inherited::mod_->name (),
        //           ACE_TEXT (inherited::configuration_->URL.c_str ()),
        //           ACE_TEXT ((*iterator).second.c_str ())));

#if defined (GTK_USE)
        struct Test_I_URLStreamLoad_Redirect_CBData* cb_data_p = NULL;
        ACE_NEW_NORETURN (cb_data_p,
                          struct Test_I_URLStreamLoad_Redirect_CBData);
        ACE_ASSERT (cb_data_p);
        cb_data_p->CBData = inherited::configuration_->CBData;
        cb_data_p->handle = handle_;
        cb_data_p->URL = (*iterator).second;

        guint event_source_id = g_idle_add (idle_handle_redirect_cb,
                                            cb_data_p);
        if (event_source_id == 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to g_idle_add(idle_handle_redirect_cb): \"%m\", aborting\n")));
          goto error;
        } // end IF
        inherited::configuration_->CBData->UIState->eventSourceIds.insert (event_source_id);
#endif // GTK_USE

        break;
      } // end IF

      if (!inherited::send ((*iterator).second,
                            HTTP_Codes::HTTP_METHOD_GET,
                            inherited::configuration_->HTTPHeaders,
                            inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                   inherited::mod_->name (),
                   ACE_TEXT ((*iterator).second.c_str ())));
        goto error;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: invalid HTTP response (status was: %d): \"%s\", aborting\n"),
                 inherited::mod_->name (),
                 data_r.status,
                 ACE_TEXT (data_r.reason.c_str ())));
      goto error;
    }
  } // end SWITCH

  goto continue_3;

error:
  this->notify (STREAM_SESSION_MESSAGE_ABORT);

continue_3:
  if (!passMessageDownstream_out)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF
}

void
Test_I_Module_HTTPGet::handleSessionMessage (Test_I_SessionMessage*& message_inout,
                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTTPGet::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  struct Test_I_URLStreamLoad_SessionData& session_data_r =
    const_cast<struct Test_I_URLStreamLoad_SessionData&> (inherited::sessionData_->getR ());
  if (session_data_r.connection)
    handle_ = static_cast<ACE_HANDLE> (session_data_r.connection->id ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      // send HTTP request ?
      if (inherited::configuration_->waitForConnect)
        break;
      if (!send (inherited::configuration_->URL,
                 HTTP_Codes::HTTP_METHOD_GET,
                 inherited::configuration_->HTTPHeaders,
                 inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->URL.c_str ())));
        goto error;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: started HTTP request for \"%s\"\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (inherited::configuration_->URL.c_str ())));
      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      // send HTTP request ?
      if (!inherited::configuration_->waitForConnect)
        break;
      if (!send (inherited::configuration_->URL,
                 HTTP_Codes::HTTP_METHOD_GET,
                 inherited::configuration_->HTTPHeaders,
                 inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->URL.c_str ())));
        goto error_2;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: started HTTP request for \"%s\"\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (inherited::configuration_->URL.c_str ())));
      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    default:
      break;
  } // end SWITCH
}
