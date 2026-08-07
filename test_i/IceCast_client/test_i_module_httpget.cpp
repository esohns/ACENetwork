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

#include <string>
#include <sstream>
#include <vector>

#include "ace/Log_Msg.h"

#include "common_string_tools.h"

#include "net_macros.h"

#include "http_defines.h"

#include "test_i_icecast_client_defines.h"

Test_I_HTTPGet::Test_I_HTTPGet (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , handleBody_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPGet::Test_I_HTTPGet"));

}

void
Test_I_HTTPGet::handleDataMessage (Test_I_Message*& message_inout,
                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPGet::handleDataMessage"));

  if (handleBody_)
    return;

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
  struct Test_I_IceCastClient_SessionData& session_data_r =
    const_cast<struct Test_I_IceCastClient_SessionData&> (inherited::sessionData_->getR ());

  if (unlikely (!message_inout->isInitialized ()))
    return; // assume it's part of the body
  const Test_I_MessageDataContainer& data_container_r =
    message_inout->getR ();
  struct Test_I_IceCastClient_MessageData& data_r =
    const_cast<struct Test_I_IceCastClient_MessageData&> (data_container_r.getR ());

  switch (data_r.status)
  {
    case HTTP_Codes::HTTP_STATUS_OK:
    { handleBody_ = true;

      inherited::receivedBytes_ += message_inout->total_length ();

      if (!data_r.M3UPlaylist.stream_inf_elements.empty ())
      {
        const struct M3U_StreamInf_Element& element_r =
          data_r.M3UPlaylist.stream_inf_elements.front ();
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
                              HTTP_Codes::HTTP_METHOD_GET,
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
              (content_length <= inherited::receivedBytes_)  &&
              session_data_r.connection                      &&
              close_connection_b)
          {
            ACE_DEBUG ((LM_DEBUG,
                       ACE_TEXT ("%s: received all content, aborting connection\n"),
                       inherited::mod_->name ()));
            ACE_ASSERT (session_data_r.connection);
            session_data_r.connection->abort ();
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

//////////////////////////////////////////

Test_I_HTTPGet_2::Test_I_HTTPGet_2 (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , handleBody_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPGet_2::Test_I_HTTPGet_2"));

}

void
Test_I_HTTPGet_2::handleDataMessage (Test_I_Message*& message_inout,
                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPGet_2::handleDataMessage"));

  if (handleBody_)
    return;

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
  struct Test_I_IceCastClient_SessionData_2& session_data_r =
    const_cast<struct Test_I_IceCastClient_SessionData_2&> (inherited::sessionData_->getR ());

  if (unlikely (!message_inout->isInitialized ()))
    return; // assume it's part of the body
  const Test_I_MessageDataContainer& data_container_r =
    message_inout->getR ();
  struct Test_I_IceCastClient_MessageData& data_r =
    const_cast<struct Test_I_IceCastClient_MessageData&> (data_container_r.getR ());

  switch (data_r.status)
  {
    case HTTP_Codes::HTTP_STATUS_OK:
    { handleBody_ = true;

      inherited::receivedBytes_ += message_inout->total_length ();

      HTTP_HeadersConstIterator_t iterator =
        data_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_SERVER_STRING)));
      if (iterator != data_r.headers.end () &&
          Common_String_Tools::startswith ((*iterator).second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_SERVER_PREFIX)))
      { // --> "Icecast" server
        // retrieve format
        iterator =
          data_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_TYPE_STRING)));
        ACE_ASSERT (iterator != data_r.headers.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
        struct _AMMediaType media_type_s;
        ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
        struct tWAVEFORMATEX waveformatex_s;
        ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
        waveformatex_s.nChannels = 2;
        waveformatex_s.nSamplesPerSec = 44100;
#else
        struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
        if ((*iterator).second == ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_MIMETYPE_VIDEO_WEBM_STRING))
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          waveformatex_s.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
          waveformatex_s.wBitsPerSample = 32;
          waveformatex_s.nSamplesPerSec = 48000;
#else
          media_type_s.format = SND_PCM_FORMAT_FLOAT;
#endif // ACE_WIN32 || ACE_WIN64
        } // end IF
        else if ((*iterator).second == ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_MIMETYPE_APPLICATION_OGG_STRING))
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          waveformatex_s.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
          waveformatex_s.wBitsPerSample = 32;
#else
          media_type_s.format = SND_PCM_FORMAT_FLOAT;
#endif // ACE_WIN32 || ACE_WIN64
        } // end ELSE IF
        else if ((*iterator).second == ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_MIMETYPE_AUDIO_MPEG_STRING))
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          waveformatex_s.wFormatTag = WAVE_FORMAT_PCM;
          waveformatex_s.wBitsPerSample = 16;
#else
          media_type_s.format = SND_PCM_FORMAT_S16;
#endif // ACE_WIN32 || ACE_WIN64
        } // end ELSE IF
        else
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: invalid/unknown MIME Type (was: \"%s\"), continuing\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT ((*iterator).second.c_str ())));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          waveformatex_s.wFormatTag = WAVE_FORMAT_PCM; // assumed
#else
          media_type_s.format = SND_PCM_FORMAT_S16; // assumed
#endif // ACE_WIN32 || ACE_WIN64
        } // end ELSE

        // retrieve #channels,Hz
        iterator =
          data_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_AUDIO_INFO)));
        if (iterator == data_r.headers.end ())
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: no \"%s\" header: cannot update stream format, returning\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_AUDIO_INFO)));
          break;
        } // end IF
        std::stringstream values_string ((*iterator).second);
        std::string value_string;
        std::vector<std::string> values_a;
        while (std::getline (values_string, value_string, ';'))
          values_a.push_back (value_string);
        size_t position;
        std::istringstream converter;
        for (std::vector<std::string>::const_iterator iterator_2 = values_a.begin ();
             iterator_2 != values_a.end ();
             ++iterator_2)
        {
          if (Common_String_Tools::startswith (*iterator_2,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_AUDIO_INFO_HZ)))
          {
            position = (*iterator_2).rfind ('=', std::string::npos);
            ACE_ASSERT (position != std::string::npos);
            value_string = (*iterator_2).substr (position + 1, std::string::npos);
            converter.str (value_string);
            converter.clear ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            converter >> waveformatex_s.nSamplesPerSec;
#else
            converter >> media_type_s.rate;
#endif // ACE_WIN32 || ACE_WIN64
          } // end IF
          else if (Common_String_Tools::startswith (*iterator_2,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_AUDIO_INFO_CHANNELS)))
          {
            position = (*iterator_2).rfind ('=', std::string::npos);
            ACE_ASSERT (position != std::string::npos);
            value_string = (*iterator_2).substr (position + 1, std::string::npos);
            converter.str (value_string);
            converter.clear ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            converter >> waveformatex_s.nChannels;
#else
            converter >> media_type_s.channels;
#endif // ACE_WIN32 || ACE_WIN64
          } // end ELSE IF
        } // end FOR

#if defined (ACE_WIN32) || defined (ACE_WIN64)
        waveformatex_s.nBlockAlign =
          (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
        waveformatex_s.nAvgBytesPerSec =
          (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
        if (!Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (waveformatex_s,
                                                                       media_type_s))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx(), returning\n"),
                      ACE_TEXT (inherited::mod_->name ())));
          return;
        } // end IF
#endif // ACE_WIN32 || ACE_WIN64
 
        // --> update session data and notify stream
        session_data_r.formats.push_back (media_type_s);
        // *TODO*: this won't work as intended, because the stream layout has already been configured and the wrong decoder module might be used
        this->notify (STREAM_SESSION_MESSAGE_FORMAT,
                      true); // expedite
      } // end IF
      else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: not an icecast server, continuing\n"),
                    ACE_TEXT (inherited::mod_->name ())));

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
Test_I_HTTPGet_2::handleSessionMessage (Test_I_SessionMessage_2*& message_inout,
                                        bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_HTTPGet_2::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      // send HTTP request ?
      if (inherited::configuration_->waitForConnect)
        break;

      HTTP_Headers_t headers_a = inherited::configuration_->HTTPHeaders;
      if (Common_String_Tools::endswith (inherited::configuration_->URL,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_STREAM_WEBM_SUFFIX)))
      {
        HTTP_HeadersConstIterator_t iterator = headers_a.find (HTTP_PRT_HEADER_AGENT_STRING);
        if (iterator == headers_a.end ())
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: adding \"%s\" header (value: \"%s\") to HTTP request...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (HTTP_PRT_HEADER_AGENT_STRING),
                      ACE_TEXT (TEST_I_ICECAST_CLIENT_DEFAULT_USER_AGENT_SPOOF)));
          headers_a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_AGENT_STRING),
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_USER_AGENT_SPOOF)));
        } // end IF
        iterator = headers_a.find (HTTP_PRT_HEADER_ACCEPT_STRING);
        if (iterator == headers_a.end ())
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: adding \"%s\" header (value: \"%s\") to HTTP request...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (HTTP_PRT_HEADER_ACCEPT_STRING),
                      ACE_TEXT ("*/*")));
          headers_a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_AGENT_STRING),
                                            ACE_TEXT_ALWAYS_CHAR ("*/*")));
        } // end IF
        iterator = headers_a.find (HTTP_PRT_HEADER_HOST_STRING);
        if (iterator == headers_a.end ())
        {
          std::string hostname_string =
            Net_Common_Tools::URLToHostName (inherited::configuration_->URL,
                                             false,  // return hostname
                                             false); // do not return port#
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: adding \"%s\" header (value: \"%s\") to HTTP request...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (HTTP_PRT_HEADER_HOST_STRING),
                      ACE_TEXT (hostname_string.c_str ())));
          headers_a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                            hostname_string));
        } // end IF
        iterator = headers_a.find (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_ICY_METADATA);
        if (iterator == headers_a.end ())
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: adding \"%s\" header (value: \"%s\") to HTTP request...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_ICY_METADATA),
                      ACE_TEXT ("0")));
          headers_a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                            ACE_TEXT_ALWAYS_CHAR ("0")));
        } // end IF
      } // end IF

      if (!inherited::send (inherited::configuration_->URL,
                            HTTP_Codes::HTTP_METHOD_GET,
                            headers_a,
                            inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->URL.c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started HTTP request for \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->URL.c_str ())));
      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
//    case STREAM_SESSION_MESSAGE_CONNECT:
//    {
//      // sanity check(s)
//      ACE_ASSERT (inherited::configuration_);
//
//      // send HTTP request ?
//      if (!inherited::configuration_->waitForConnect)
//        break;
//      if (!send (inherited::configuration_->URL,
//                 HTTP_Codes::HTTP_METHOD_GET,
//                 inherited::configuration_->HTTPHeaders,
//                 inherited::configuration_->HTTPForm))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (inherited::configuration_->URL.c_str ())));
//        goto error_2;
//      } // end IF
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: started HTTP request for \"%s\"\n"),
//                  inherited::mod_->name (),
//                  ACE_TEXT (inherited::configuration_->URL.c_str ())));
//      break;
//
//error_2:
//      this->notify (STREAM_SESSION_MESSAGE_ABORT);
//
//      break;
//    }
    default:
      break;
  } // end SWITCH
}
