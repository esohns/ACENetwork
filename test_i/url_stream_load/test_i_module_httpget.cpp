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

#include "net_macros.h"

Test_I_Module_HTTPGet::Test_I_Module_HTTPGet (ISTREAM_T* stream_in)
 : inherited (stream_in)
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
  ACE_INET_Addr host_address;
  std::string uri_string, host_name_string;
  bool close_connection_b = true;
  std::string host_name_string_2;
  std::string uri_string_2;
  bool use_SSL = false, use_SSL_2 = false;
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

      if (data_r.M3UPlaylist)
      { ACE_ASSERT (!data_r.M3UPlaylist->stream_inf_elements.empty ());
        const struct M3U_StreamInf_Element& element_r =
            data_r.M3UPlaylist->stream_inf_elements.front ();
        bool is_basename_b = Common_File_Tools::isBasename (element_r.URL);
        std::string URL_string = element_r.URL;

        // sanity check(s)
        ACE_ASSERT (!element_r.URL.empty ());

        if (!Common_String_Tools::endswith (element_r.URL,
                                            ACE_TEXT_ALWAYS_CHAR ("m3u")) &&
            !Common_String_Tools::endswith (element_r.URL,
                                            ACE_TEXT_ALWAYS_CHAR ("m3u8")))
          goto continue_2;

        // send request ?
        // *IMPORTANT NOTE*: only auto-effectuate same-server/protocol redirects
        if (!is_basename_b &&
            !HTTP_Tools::parseURL (element_r.URL,
                                   host_address,
                                   host_name_string,
                                   uri_string,
                                   use_SSL))
        {
          ACE_DEBUG ((LM_ERROR,
                     ACE_TEXT ("%s: failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                     inherited::mod_->name (),
                     ACE_TEXT (element_r.URL.c_str ())));
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
        if (is_basename_b)
        {
          host_name_string = host_name_string_2;
          use_SSL = use_SSL_2;

          URL_string = ACE_TEXT_ALWAYS_CHAR ("http");
          URL_string +=
            (use_SSL ? ACE_TEXT_ALWAYS_CHAR ("s") : ACE_TEXT_ALWAYS_CHAR (""));
          URL_string += ACE_TEXT_ALWAYS_CHAR ("://");
          URL_string += host_name_string;
          size_t position = uri_string_2.find_last_of ('/', std::string::npos);
          ACE_ASSERT (position != std::string::npos);
          uri_string_2.erase (position + 1, std::string::npos);
          URL_string += uri_string_2;
          URL_string += element_r.URL;
        } // end IF
        if (likely ((host_name_string != host_name_string_2) ||
                    (use_SSL != use_SSL_2)))
        { // *TODO*
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: URL (was: \"%s\") redirects to a different host, and/or requires a HTTP(S) connection, cannot proceed\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->URL.c_str ())));
          passMessageDownstream_out = false;
          goto error;
        } // end IF

        inherited::receivedBytes_ = 0;
        close_connection_b = false;
        passMessageDownstream_out = false;

        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("retrieving \"%s\"...\n"),
                    ACE_TEXT (URL_string.c_str ())));
        if (!inherited::send (URL_string,
                              inherited::configuration_->HTTPHeaders,
                              inherited::configuration_->HTTPForm))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (URL_string.c_str ())));
          goto error;
        } // end IF

        goto continue_;
      } // end IF

      // got all data ? --> close connection ?
continue_2:
      iterator =
          data_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
      if (iterator != data_r.headers.end ())
      {
        std::istringstream converter ((*iterator).second);
        unsigned int content_length = 0;
        converter >> content_length;
        ACE_ASSERT (session_data_r.lock);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          if (inherited::configuration_->closeAfterReception &&
              (content_length == receivedBytes_)             &&
              session_data_r.connection                      &&
              close_connection_b)
          {
            ACE_DEBUG ((LM_DEBUG,
                       ACE_TEXT ("%s: received all content, closing connection\n"),
                       inherited::mod_->name ()));
            ACE_ASSERT (session_data_r.connection);
            session_data_r.connection->close ();
          } // end IF
        } // end lock scope
      } // end IF
      else
        ACE_DEBUG ((LM_WARNING,
                   ACE_TEXT ("%s: missing \"%s\" HTTP header, continuing\n"),
                   inherited::mod_->name (),
                   ACE_TEXT (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
continue_:
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
      ACE_DEBUG ((LM_DEBUG,
                 ACE_TEXT ("%s: \"%s\" has been redirected to \"%s\" (status was: %d)\n"),
                 inherited::mod_->name (),
                 ACE_TEXT (inherited::configuration_->URL.c_str ()),
                 ACE_TEXT ((*iterator).second.c_str ()),
                 data_r.status));

      // step2: send request ?
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
      { // *TODO*
        ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("%s: URL (was: \"%s\") redirects to a different host, and/or requires a HTTP(S) connection, cannot proceed\n"),
                   inherited::mod_->name (),
                   ACE_TEXT (inherited::configuration_->URL.c_str ())));

        passMessageDownstream_out = false;

        goto error;
      } // end IF

      if (!inherited::send ((*iterator).second,
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
