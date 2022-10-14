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

#include <regex>

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_string_tools.h"

#include "common_parser_m3u_defines.h"

#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_session_message_base.h"

#include "net_macros.h"

#include "test_i_defines.h"
#include "test_i_stream.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "test_i_web_tv_defines.h"

Test_I_EventHandler::Test_I_EventHandler (
#if defined (GUI_SUPPORT)
                                          struct Test_I_WebTV_UI_CBData* CBData_in
#endif // GUI_SUPPORT
                                         )
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
#else
 : sessionDataMap_ ()
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::Test_I_EventHandler"));

}

void
Test_I_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Test_I_WebTV_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<struct Test_I_WebTV_SessionData&> (sessionData_in)));

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                           idle_end_session_cb,
                                           CBData_,
                                           NULL);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);

  if (sessionId_in == CBData_->streamSessionId)
  {
    guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                             idle_end_session_2,
                                             CBData_,
                                             NULL);
    ACE_ASSERT (event_source_id);
    state_r.eventSourceIds.insert (event_source_id);
  } // end IF

  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
#endif // GTK_USE
#endif // GUI_SUPPORT

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
#endif // GUI_SUPPORT

  Test_I_MessageDataContainer& data_container_r =
    const_cast<Test_I_MessageDataContainer&> (message_in.getR ());
  struct Test_I_WebTV_MessageData& data_r =
    const_cast<struct Test_I_WebTV_MessageData&> (data_container_r.getR ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_ASSERT (data_r.M3UPlaylist);

  // sanity check(s)
  ACE_ASSERT (CBData_->channels);
  ACE_ASSERT (CBData_->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
      CBData_->channels->find (CBData_->currentChannel);
  ACE_ASSERT (channel_iterator != CBData_->channels->end ());

  // process video stream data
  struct Test_I_WebTV_Channel_Resolution resolution_s;
  std::istringstream converter;
  for (M3U_StreamInf_ElementsIterator_t iterator = data_r.M3UPlaylist->stream_inf_elements.begin ();
        iterator != data_r.M3UPlaylist->stream_inf_elements.end ();
        ++iterator)
  {
    for (M3U_KeyValuesIterator_t iterator_2 = (*iterator).keyValues.begin ();
          iterator_2 != (*iterator).keyValues.end ();
          ++iterator_2)
    {
      if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_EXTINFO_RESOLUTION_KEY_STRING)))
      {
        std::string regex_string =
            ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+)(?:[[:space:]xX]+)([[:digit:]]+)$");
        std::regex regex (regex_string);
        std::smatch match_results;
        if (unlikely(!std::regex_match ((*iterator_2).second,
                                        match_results,
                                        regex,
                                        std::regex_constants::match_default)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT("failed to parse M3U \"%s\" key (value was: ""\"%s\"), returning\n"),
                      ACE_TEXT (TEST_I_M3U_EXTINFO_RESOLUTION_KEY_STRING),
                      ACE_TEXT ((*iterator_2).second.c_str ())));
          return;
        } // end IF
        ACE_ASSERT (match_results.ready () && !match_results.empty ());
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[1].str ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        converter >> resolution_s.resolution.cx;
#else
        converter >> resolution_s.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[2].str ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        converter >> resolution_s.resolution.cy;
#else
        converter >> resolution_s.resolution.height;
#endif // ACE_WIN32 || ACE_WIN64
      } // end IF

      if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                            ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_EXTINFO_FRAMERATE_KEY_STRING)))
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str ((*iterator_2).second);
        converter >> resolution_s.frameRate;
      } // end IF
    } // end FOR
    resolution_s.URI = (*iterator).URL;
    (*channel_iterator).second.resolutions.push_back (resolution_s);
  } // end FOR

  // process audio stream data
  struct Test_I_WebTV_Channel_AudioChannel channel_s;
  bool is_channel_b = false;
  for (M3U_Media_ElementsIterator_t iterator = data_r.M3UPlaylist->media_elements.begin ();
       iterator != data_r.M3UPlaylist->media_elements.end ();
       ++iterator)
  {
    is_channel_b = true;
    for (M3U_KeyValuesIterator_t iterator_2 = (*iterator).keyValues.begin ();
          iterator_2 != (*iterator).keyValues.end ();
          ++iterator_2)
    {
      // filter audio channels
      if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                           ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_MEDIAINFO_TYPE_KEY_STRING)) &&
           ACE_OS::strcmp ((*iterator_2).second.c_str (),
                           ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_MEDIAINFO_TYPE_AUDIO_STRING)))
      {
        is_channel_b = false;
        break;
      } // end IF

      if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                           ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_MEDIAINFO_DEFAULT_KEY_STRING)))
        channel_s.default_ =
          !ACE_OS::strcmp ((*iterator_2).first.c_str (),
                           ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_MEDIAINFO_DEFAULT_YES_STRING));
      else if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                           ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_MEDIAINFO_NAME_KEY_STRING)))
        channel_s.description = (*iterator_2).second;
      else if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                                ACE_TEXT_ALWAYS_CHAR (TEST_I_M3U_MEDIAINFO_URI_KEY_STRING)))
        channel_s.URI = Common_String_Tools::uncomment ((*iterator_2).second);
    } // end FOR
    if (is_channel_b)
      (*channel_iterator).second.audioChannels.push_back (channel_s);
  } // end FOR

  guint event_source_id =
      g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                        idle_load_channel_configuration_cb,
                        CBData_,
                        NULL);
  if (unlikely (event_source_id == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_load_channel_configuration_cb): ""\"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
#if defined (GUI_SUPPORT)
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
#endif // GUI_SUPPORT

      event_e = COMMON_UI_EVENT_ABORT;
      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT; break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT; break;
    case STREAM_SESSION_MESSAGE_STEP:
      event_e = COMMON_UI_EVENT_STEP; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

//////////////////////////////////////////

Test_I_EventHandler_2::Test_I_EventHandler_2 (
#if defined (GUI_SUPPORT)
                                              struct Test_I_WebTV_UI_CBData* CBData_in
#endif // GUI_SUPPORT
                                             )
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
#else
 : sessionDataMap_ ()
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::Test_I_EventHandler_2"));

}

void
Test_I_EventHandler_2::start (Stream_SessionId_t sessionId_in,
                              const Test_I_WebTV_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<Test_I_WebTV_SessionData&> (sessionData_in)));

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler_2::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                           idle_end_session_cb,
                                           CBData_,
                                           NULL);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);

  if (sessionId_in == CBData_->streamSessionId)
  {
    guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                             idle_end_session_2,
                                             CBData_,
                                             NULL);
    ACE_ASSERT (event_source_id);
    state_r.eventSourceIds.insert (event_source_id);
  } // end IF

  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
#endif // GTK_USE
#endif // GUI_SUPPORT

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  { ACE_GUARD(ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push(COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT

  Test_I_MessageDataContainer& data_container_r =
    const_cast<Test_I_MessageDataContainer&> (message_in.getR ());
  struct Test_I_WebTV_MessageData& data_r =
    const_cast<struct Test_I_WebTV_MessageData&> (data_container_r.getR ());
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());
  Net_ConnectionId_t connection_id = ACE_INVALID_HANDLE;
  //ACE_ASSERT ((*iterator).second->connection);
  if (likely ((*iterator).second->connection))
    connection_id = (*iterator).second->connection->id ();

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_ASSERT (data_r.M3UPlaylist);

  // sanity check(s)
  ACE_ASSERT (CBData_->channels);
  ACE_ASSERT (CBData_->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
      CBData_->channels->find (CBData_->currentChannel);
  ACE_ASSERT (channel_iterator != CBData_->channels->end ());

  // process stream data

  // process audio/video playlist data
  int UTC_offset_i = 0;
  unsigned int seconds_i = 0;
  bool is_audio_b = ((connection_id != ACE_INVALID_HANDLE) &&
                     (connection_id == CBData_->audioHandle));
  struct Test_I_WebTV_ChannelSegment* segment_p =
    (is_audio_b ? &(*channel_iterator).second.audioSegment
                : &(*channel_iterator).second.videoSegment);
  if (!data_r.M3UPlaylist->ext_inf_elements.empty ())
  {
    M3U_ExtInf_ElementsIterator_t iterator =
        data_r.M3UPlaylist->ext_inf_elements.begin ();
    for (M3U_KeyValuesIterator_t iterator_2 = (*iterator).keyValues.begin ();
         iterator_2 != (*iterator).keyValues.end ();
         ++iterator_2)
      if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                           ACE_TEXT_ALWAYS_CHAR (COMMON_PARSER_M3U_EXT_X_PROGRAM_DATE_TIME)))
      {
        segment_p->start =
            Common_Timer_Tools::ISO8601ToTimestamp ((*iterator_2).second,
                                                    UTC_offset_i);
        break;
      } // end IF
    // *WARNING*: lengths might not be uniform
    segment_p->length = (*iterator).Length;
    for (;
         iterator != data_r.M3UPlaylist->ext_inf_elements.end ();
         ++iterator)
    {
      seconds_i += (*iterator).Length;
      segment_p->URLs.push_back ((*iterator).URL);
    } // end FOR
  } // end IF
  else
  {
    M3U_StreamInf_ElementsIterator_t iterator =
        data_r.M3UPlaylist->stream_inf_elements.begin ();
    for (M3U_KeyValuesIterator_t iterator_2 = (*iterator).keyValues.begin ();
         iterator_2 != (*iterator).keyValues.end ();
         ++iterator_2)
      if (!ACE_OS::strcmp ((*iterator_2).first.c_str (),
                           ACE_TEXT_ALWAYS_CHAR (COMMON_PARSER_M3U_EXT_X_PROGRAM_DATE_TIME)))
      {
        segment_p->start =
            Common_Timer_Tools::ISO8601ToTimestamp ((*iterator_2).second,
                                                    UTC_offset_i);
        break;
      } // end IF
    // *WARNING*: lengths might not be uniform
    segment_p->length = (*iterator).Length;
    for (;
         iterator != data_r.M3UPlaylist->stream_inf_elements.end ();
         ++iterator)
    {
      seconds_i += (*iterator).Length;
      segment_p->URLs.push_back ((*iterator).URL);
    } // end FOR
  } // end ELSE
  segment_p->end = segment_p->start;
  segment_p->end += static_cast<time_t> (seconds_i);
  // convert times
  if (UTC_offset_i)
  {
    segment_p->start =
        Common_Timer_Tools::localToUTC (segment_p->start,
                                        UTC_offset_i,
                                        true);
    segment_p->end =
        Common_Timer_Tools::localToUTC (segment_p->end,
                                        UTC_offset_i,
                                        true);
  } // end IF
  segment_p->start =
      Common_Timer_Tools::UTCToLocal (segment_p->start);
  segment_p->end =
      Common_Timer_Tools::UTCToLocal (segment_p->end);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received %s segment data (%u second(s): start %#T, end: %#T\n"),
              (is_audio_b ? ACE_TEXT ("audio") : ACE_TEXT ("video")),
              seconds_i,
              &segment_p->start,
              &segment_p->end));

  // keep the most recent 5% entries
  // *TODO*: remove this altogether
  ACE_ASSERT (!segment_p->URLs.empty ());
  Test_I_WebTV_ChannelSegmentURLsIterator_t iterator_3 =
    segment_p->URLs.begin ();
  ACE_UINT32 number_to_erase_i = segment_p->URLs.size () - 2;
  //  ((segment_p->URLs.size () <= 10) ? 0
  //                                   : (static_cast<float> (segment_p->URLs.size ()) * 0.95));
  if (number_to_erase_i)
  {
    number_to_erase_i =
      std::min (static_cast<size_t> (number_to_erase_i), segment_p->URLs.size () - 3);
    std::advance (iterator_3, number_to_erase_i);
    segment_p->URLs.erase (segment_p->URLs.begin (),
                           iterator_3);
  } // end IF
  ACE_ASSERT (segment_p->URLs.size () >= 2);

  guint event_source_id = g_idle_add (idle_notify_segment_data_cb,
                                      CBData_);
  if (unlikely (event_source_id == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_notify_segment_data_cb): ""\"%m\", returning\n")));
    return;
  } // end IF
  { ACE_GUARD(ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventSourceIds.insert(event_source_id);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      event_e = COMMON_UI_EVENT_FINISHED; break;
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT; break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT; break;
    case STREAM_SESSION_MESSAGE_RESIZE:
      event_e = COMMON_UI_EVENT_RESIZE; break;
    case STREAM_SESSION_MESSAGE_STEP:
      event_e = COMMON_UI_EVENT_STEP; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if ((*iterator).second->lock)
      {
        result = (*iterator).second->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

//////////////////////////////////////////

Test_I_EventHandler_3::Test_I_EventHandler_3 (
#if defined (GUI_SUPPORT)
                                              struct Test_I_WebTV_UI_CBData* CBData_in
#endif // GUI_SUPPORT
                                             )
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionDataMap_ ()
#else
 : sessionDataMap_ ()
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_3::Test_I_EventHandler_3"));

}

void
Test_I_EventHandler_3::start (Stream_SessionId_t sessionId_in,
                              const Test_I_WebTV_SessionData_3& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_3::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator == sessionDataMap_.end ());

  sessionDataMap_.insert (std::make_pair (sessionId_in,
                                          &const_cast<Test_I_WebTV_SessionData_3&> (sessionData_in)));

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  CBData_->progressData.transferred = 0;
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler_3::notify (Stream_SessionId_t sessionId_in,
                               const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_3::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler_3::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_3::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
//  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  //ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);

  guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                           idle_end_session_cb,
                                           CBData_,
                                           NULL);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_session_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);

  if (sessionId_in == CBData_->streamSessionId)
  {
    guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                             idle_end_session_2,
                                             CBData_,
                                             NULL);
    ACE_ASSERT (event_source_id);
    state_r.eventSourceIds.insert (event_source_id);
  } // end IF

  state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
#endif // GTK_USE
#endif // GUI_SUPPORT

  //sessionDataMap_.erase (iterator);
}

void
Test_I_EventHandler_3::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_3::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  CBData_->progressData.statistic.bytes += message_in.total_length ();
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler_3::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_SessionMessage_3& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_3::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_ITERATOR_T iterator = sessionDataMap_.find (sessionId_in);
  ACE_ASSERT (iterator != sessionDataMap_.end ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
#endif // GUI_SUPPORT

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      event_e = COMMON_UI_EVENT_FINISHED; break;
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT; break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT; break;
    case STREAM_SESSION_MESSAGE_RESIZE:
      event_e = COMMON_UI_EVENT_RESIZE; break;
    case STREAM_SESSION_MESSAGE_STEP:
    {
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                               idle_segment_download_complete_cb,
                                               CBData_,
                                               NULL);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_segment_download_complete_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
#endif // GUI_SUPPORT

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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      CBData_->progressData.statistic = (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
#endif // GUI_SUPPORT
}
