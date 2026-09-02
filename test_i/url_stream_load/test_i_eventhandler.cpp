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

#include "test_i_eventhandler.h"

#if defined (RAPIDJSON_SUPPORT)
#include "document.h"
#endif // RAPIDJSON_SUPPORT

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include <regex>

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_ui_common.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#include "stream_session_message_base.h"
#include "stream_tools.h"

#include "stream_html_tools.h"

#include "net_macros.h"

#include "test_i_url_stream_load_defines.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT

//////////////////////////////////////////

std::string
executeEjsChallenge (const std::string& challenge_in,
                     const std::string& token_in,
                     const std::string& baseJsPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("executeEjsChallenge"));

  std::string result;

  char* lib_root_p = ACE_OS::getenv (ACE_TEXT_ALWAYS_CHAR ("LIB_ROOT"));
  ACE_ASSERT (lib_root_p);
  std::string ytdlp_ejs_executable = lib_root_p;
  ytdlp_ejs_executable += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ytdlp_ejs_executable += ACE_TEXT_ALWAYS_CHAR ("ytdlp-ejs");
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ytdlp_ejs_executable += ACE_TEXT_ALWAYS_CHAR (".exe");
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (Common_File_Tools::isExecutable (ytdlp_ejs_executable));

  std::string command_string = ytdlp_ejs_executable +
                               ACE_TEXT_ALWAYS_CHAR (" --challenge ") + challenge_in +
                               ACE_TEXT_ALWAYS_CHAR (" --input \"") + token_in + ACE_TEXT_ALWAYS_CHAR ("\"") +
                               ACE_TEXT_ALWAYS_CHAR (" --player \"") + baseJsPath_in + ACE_TEXT_ALWAYS_CHAR ("\"");

  char buffer_a[128];
  FILE* file_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  file_p = _popen (command_string.c_str (), ACE_TEXT_ALWAYS_CHAR ("r"));
#else
  file_p =  popen (command_string.c_str (), ACE_TEXT_ALWAYS_CHAR ("r"));
#endif // ACE_WIN32 || ACE_WIN64
  if (!file_p)
    return result;
  while (fgets (buffer_a, sizeof (buffer_a), file_p) != NULL)
    result += buffer_a;
  if (!result.empty () && result.back () == '\n')
    result.pop_back ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  _pclose (file_p);
#else
  pclose (file_p);
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}

//////////////////////////////////////////

Test_I_EventHandler::Test_I_EventHandler (struct Test_I_URLStreamLoad_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::Test_I_EventHandler"));

}

void
Test_I_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Test_I_URLStreamLoad_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (_DEBUG)
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());
#endif // _DEBUG
  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_URLStreamLoad_SessionData&> (sessionData_in)));

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
//  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);

  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const enum Stream_SessionMessageType& sessionEvent_in,
                             bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add (idle_end_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
  Test_I_MessageDataContainer& data_container_r =
    const_cast<Test_I_MessageDataContainer&> (message_in.getR ());
  struct Test_I_URLStreamLoad_MessageData& data_r =
    const_cast<struct Test_I_URLStreamLoad_MessageData&> (data_container_r.getR ());
  Stream_HTML_XPathNameSpaces_t namespaces_a;
  xmlXPathObject* xpath_object_p = NULL;
  std::string URL_string;

  CBData_->progressData.transferred += message_in.total_length ();
#if defined (GTK_USE)
  guint event_source_id;
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE

  // scrape "initial response"
  std::string html_string =
    Stream_Tools::toString (&const_cast<Test_I_Message&> (message_in));

  std::regex ytInitialPlayerResponseRegex (R"(ytInitialPlayerResponse\s*=\s*(\{.*?\});)");
  std::smatch match;
  if (!std::regex_search (html_string,
                          match,
                          ytInitialPlayerResponseRegex,
                          std::regex_constants::match_default))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("could not find streaming manifest object in HTML, returning\n")));
    return;
  } // end IF
  ACE_ASSERT (match.ready () && !match.empty ());
  ACE_ASSERT (match[1].matched);
  std::string json_string = match[1].str ();

  rapidjson::Document player_response;
  if (player_response.Parse (json_string.c_str ()).HasParseError ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to parse JSON: %d, returning\n"),
                player_response.GetParseError ()));
    return;
  } // end IF
  if (!player_response.HasMember (ACE_TEXT_ALWAYS_CHAR ("streamingData")))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("JSON does not contain 'streamingData' member, returning\n")));
    return;
  } // end IF
  const rapidjson::Value& streaming_data_value_r =
    player_response[ACE_TEXT_ALWAYS_CHAR ("streamingData")];
  if (!streaming_data_value_r.HasMember (ACE_TEXT_ALWAYS_CHAR ("adaptiveFormats")))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("JSON does not contain 'adaptiveFormats' member, returning\n")));
    return;
  } // end IF
  const rapidjson::Value& adaptive_formats_r =
    streaming_data_value_r[ACE_TEXT_ALWAYS_CHAR ("adaptiveFormats")];
  if (!adaptive_formats_r.IsArray () || adaptive_formats_r.Size () == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("JSON 'adaptiveFormats' member is not a valid array or is empty, returning\n")));
    return;
  } // end IF
  const rapidjson::Value& target_format_r = adaptive_formats_r[0];
  // retrieve throttling strings (n-parameter matches)
  if (target_format_r.HasMember (ACE_TEXT_ALWAYS_CHAR ("n")))
  {
    const rapidjson::Value& n_r = target_format_r[ACE_TEXT_ALWAYS_CHAR ("n")];
    ACE_ASSERT (n_r.IsString ());
    CBData_->encryptedNParameter = n_r.GetString ();
  } // end IF

  // retrieve cipher extraction block
  if (target_format_r.HasMember (ACE_TEXT_ALWAYS_CHAR ("signatureCipher")))
  {
    const rapidjson::Value& signature_cipher_r =
      target_format_r[ACE_TEXT_ALWAYS_CHAR ("signatureCipher")];
    ACE_ASSERT (signature_cipher_r.IsString ());
    std::string signature_cipher_string = signature_cipher_r.GetString ();
    std::regex pair_regex (ACE_TEXT_ALWAYS_CHAR ("([^&=]+)=([^&]*)"));
    std::sregex_iterator words_begin =
      std::sregex_iterator (signature_cipher_string.begin (),
                            signature_cipher_string.end (), pair_regex);
    std::sregex_iterator words_end;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i)
    {
      std::smatch m = *i;
      std::string key = m[1].str ();
      std::string value = m[2].str ();
      std::string unescaped_value = HTTP_Tools::URLDecode (value);
      if (key == ACE_TEXT_ALWAYS_CHAR ("url"))
        CBData_->baseFormatURL = unescaped_value;
      else if (key == ACE_TEXT_ALWAYS_CHAR ("s"))
        CBData_->encryptedSignature = unescaped_value;
      else if (key == ACE_TEXT_ALWAYS_CHAR ("sp"))
        CBData_->signatureParameter = unescaped_value;
    } // end FOR
  } // end IF
  else if (target_format_r.HasMember (ACE_TEXT_ALWAYS_CHAR ("url")))
  {
    const rapidjson::Value& url_r =
      target_format_r[ACE_TEXT_ALWAYS_CHAR ("url")];
    ACE_ASSERT (url_r.IsString ());
    CBData_->baseFormatURL = url_r.GetString ();
  } // end ELSE IF

  std::string path_string = Common_File_Tools::getTempDirectory ();
  path_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path_string +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_BASE_JS_FILENAME_STRING);
  std::string clear_signature_string;
  std::string clear_n_parameter_string;
  std::stringstream final_url_string;
  if (!Common_File_Tools::isReadable (path_string))
    goto continue_2;

  if (!CBData_->encryptedSignature.empty ())
    clear_signature_string = executeEjsChallenge (ACE_TEXT_ALWAYS_CHAR ("sig"),
                                                  CBData_->encryptedSignature,
                                                  path_string);
  if (!CBData_->encryptedNParameter.empty ())
    clear_n_parameter_string = executeEjsChallenge (ACE_TEXT_ALWAYS_CHAR ("n"),
                                                    CBData_->encryptedNParameter,
                                                    path_string);
  final_url_string << CBData_->baseFormatURL;
  if (!clear_signature_string.empty ())
    final_url_string << ACE_TEXT_ALWAYS_CHAR ("&") << CBData_->signatureParameter << ACE_TEXT_ALWAYS_CHAR ("=") << clear_signature_string;
  if (!clear_n_parameter_string.empty ())
    final_url_string << ACE_TEXT_ALWAYS_CHAR ("&n=") << clear_n_parameter_string;

  CBData_->URL = final_url_string.str ();

#if defined (GTK_USE)
  event_source_id = g_idle_add (idle_connect_to_peer_cb,
                                CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_connect_to_peer_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

  goto continue_;

continue_2:
  if (!data_r.document)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to parse HTML, returning\n")));
    return;
  } // end IF

  xpath_object_p =
    Stream_HTML_Tools::query (data_r.document,
                              namespaces_a,
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_XPATH_QUERY_STRING));
  if (!xpath_object_p                    ||
      !xpath_object_p->nodesetval        ||
      !xpath_object_p->nodesetval->nodeNr)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no xpath data, continuing\n")));
    goto continue_;
  } // end IF
  URL_string = (char*)xpath_object_p->nodesetval->nodeTab[0]->content;

  xmlXPathFreeObject (xpath_object_p); xpath_object_p = NULL;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("loading \"%s\"...\n"),
              ACE_TEXT (URL_string.c_str ())));

  CBData_->URL = URL_string;
#if defined (GTK_USE)
  event_source_id = g_idle_add (idle_load_player_base_cb,
                                CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_load_player_base_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

continue_:
  CBData_->progressData.statistic.bytes += message_in.total_length ();
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
#if defined (GTK_USE)
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

      guint event_source_id = g_idle_add (idle_end_session_cb,
                                          CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_ABORT;
      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    {
      event_e = COMMON_UI_EVENT_STEP;
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
}

//////////////////////////////////////////

Test_I_EventHandler_1b::Test_I_EventHandler_1b (struct Test_I_URLStreamLoad_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::Test_I_EventHandler_1b"));

}

void
Test_I_EventHandler_1b::start (Stream_SessionId_t sessionId_in,
                               const struct Test_I_URLStreamLoad_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (_DEBUG)
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());
#endif // _DEBUG
  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_URLStreamLoad_SessionData&> (sessionData_in)));

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

//#if defined (GTK_USE)
////  CBData_->progressData.transferred = 0;
//  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
//
//  guint event_source_id = g_idle_add (idle_start_session_cb,
//                                      CBData_);
//  if (event_source_id == 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
//    return;
//  } // end IF
//  state_r.eventSourceIds.insert (event_source_id);
//#endif // GTK_USE
}

void
Test_I_EventHandler_1b::notify (Stream_SessionId_t sessionId_in,
                                const enum Stream_SessionMessageType& sessionEvent_in,
                                bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler_1b::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

//#if defined (GTK_USE)
//  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
//
//  guint event_source_id = g_idle_add (idle_end_session_cb,
//                                      CBData_);
//  if (event_source_id == 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
//    return;
//  } // end IF
//  state_r.eventSourceIds.insert (event_source_id);
//#endif // GTK_USE

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler_1b::notify (Stream_SessionId_t sessionId_in,
                                const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

  CBData_->progressData.transferred += message_in.total_length ();
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE

  std::string js_string =
    Stream_Tools::toString (&const_cast<Test_I_Message&> (message_in));
  std::string path_string = Common_File_Tools::getTempDirectory ();
  path_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path_string +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_BASE_JS_FILENAME_STRING);
  Common_File_Tools::store (path_string,
                            reinterpret_cast<const uint8_t*> (js_string.c_str ()), js_string.size (),
                            false);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("saved yt player base \"%s\"...\n"),
              ACE_TEXT (path_string.c_str ())));

  std::string clear_signature_string;
  std::string clear_n_parameter_string;
  if (!CBData_->encryptedSignature.empty ())
    clear_signature_string = executeEjsChallenge (ACE_TEXT_ALWAYS_CHAR ("sig"),
                                                  CBData_->encryptedSignature,
                                                  path_string);
  if (!CBData_->encryptedNParameter.empty ())
    clear_n_parameter_string = executeEjsChallenge (ACE_TEXT_ALWAYS_CHAR ("n"),
                                                    CBData_->encryptedNParameter,
                                                    path_string);
  std::stringstream final_url_string;
  final_url_string << CBData_->baseFormatURL;
  if (!clear_signature_string.empty ())
    final_url_string << ACE_TEXT_ALWAYS_CHAR ("&") << CBData_->signatureParameter << ACE_TEXT_ALWAYS_CHAR ("=") << clear_signature_string;
  if (!clear_n_parameter_string.empty ())
    final_url_string << ACE_TEXT_ALWAYS_CHAR ("&n=") << clear_n_parameter_string;

  CBData_->URL = final_url_string.str ();

#if defined (GTK_USE)
  guint event_source_id = g_idle_add (idle_connect_to_peer_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_connect_to_peer_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

continue_:
  CBData_->progressData.statistic.bytes += message_in.total_length ();
}

void
Test_I_EventHandler_1b::notify (Stream_SessionId_t sessionId_in,
                                const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_1b::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
#if defined (GTK_USE)
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

      guint event_source_id = g_idle_add (idle_end_session_cb,
                                          CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_ABORT;
      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    {
      event_e = COMMON_UI_EVENT_STEP;
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
}

//////////////////////////////////////////

Test_I_EventHandler_2::Test_I_EventHandler_2 (struct Test_I_URLStreamLoad_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::Test_I_EventHandler_2"));

}

void
Test_I_EventHandler_2::start (Stream_SessionId_t sessionId_in,
                              const struct Test_I_URLStreamLoad_SessionData_2& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_URLStreamLoad_SessionData_2&> (sessionData_in)));

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
//  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);

  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const enum Stream_SessionMessageType& sessionEvent_in,
                               bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler_2::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add (idle_end_session_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

  CBData_->progressData.transferred += message_in.total_length ();
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE

  Test_I_MessageDataContainer& data_container_r =
    const_cast<Test_I_MessageDataContainer&> (message_in.getR ());
  struct Test_I_URLStreamLoad_MessageData& data_r =
    const_cast<struct Test_I_URLStreamLoad_MessageData&> (data_container_r.getR ());

continue_:
  CBData_->progressData.statistic.bytes += message_in.total_length ();
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_SessionMessage_2& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      event_e = COMMON_UI_EVENT_FINISHED;
      break;
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
      event_e = COMMON_UI_EVENT_STEP;
      break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
}
