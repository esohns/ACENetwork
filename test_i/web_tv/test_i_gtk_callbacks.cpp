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

#include "test_i_gtk_callbacks.h"

#include <iomanip>
#include <limits>
#include <regex>
#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "iphlpapi.h"
#else
#include "netinet/ether.h"
#include "ifaddrs.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "gdk/gdkkeysyms.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_file_tools.h"

#include "common_timer_manager.h"
#include "common_timer_manager_common.h"

#include "common_ui_tools.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "stream_vis_defines.h"
#include "stream_vis_iresize.h"

#include "net_macros.h"

#include "net_client_common_tools.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_connection_stream.h"
#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_stream.h"
#include "test_i_web_tv_common.h"
#include "test_i_web_tv_defines.h"

// initialize statics
static bool un_toggling_play = false;

/////////////////////////////////////////

gboolean
idle_end_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // stop progress reporting
//  GtkSpinner* spinner_p =
//    GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
//                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
//  ACE_ASSERT (spinner_p);
//  gtk_spinner_stop (spinner_p);
//  gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), FALSE);

  //ACE_ASSERT (data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    if (!g_source_remove (data_p->progressData.eventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  data_p->progressData.eventSourceId));
    state_r.eventSourceIds.erase (data_p->progressData.eventSourceId);
    data_p->progressData.eventSourceId = 0;
  } // end lock scope

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progressbar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), FALSE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_session_2 (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_2"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);

  // stop progress reporting
//  GtkSpinner* spinner_p =
//    GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
//                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
//  ACE_ASSERT (spinner_p);
//  gtk_spinner_stop (spinner_p);
//  gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), FALSE);

  //ACE_ASSERT (data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    if (!g_source_remove (data_p->progressData.eventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  data_p->progressData.eventSourceId));
    state_r.eventSourceIds.erase (data_p->progressData.eventSourceId);
    data_p->progressData.eventSourceId = 0;

    if (data_p->videoUpdateEventSourceId &&
        !g_source_remove (data_p->videoUpdateEventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  data_p->videoUpdateEventSourceId));
    state_r.eventSourceIds.erase (data_p->videoUpdateEventSourceId);
    data_p->videoUpdateEventSourceId = 0;
  } // end lock scope

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progressbar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), FALSE);

  //if (gtk_toggle_button_get_active (toggle_button_p))
  //{
    gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                          GTK_STOCK_MEDIA_PLAY);

  //  un_toggling_play = true;
  //  gtk_toggle_button_toggled (toggle_button_p);
  //} // end IF

  // step1: update widgets
  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CHANNEL_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
  frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  if (data_p->nextChannel != -1)
  {
#if GTK_CHECK_VERSION (2,30,0)
    struct _GValue value = G_VALUE_INIT;
#else
    struct _GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
    g_value_init (&value, G_TYPE_UINT);
    g_value_set_uint (&value,
                      data_p->nextChannel);
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME)));
    ACE_ASSERT (combo_box_p);
    Common_UI_GTK_Tools::selectValue (combo_box_p,
                                      value,
                                      1);
    g_value_unset (&value);
  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_session_3 (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_3"));

  ACE_UNUSED_ARG (userData_in);

  // sanity check(s)
  //struct Test_I_WebTV_UI_CBData* data_p =
  //    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  //ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p, FALSE);

  return G_SOURCE_REMOVE;
}

void
load_audio_channels (GtkListStore* listStore_in,
                     const Test_I_WebTV_Channel_AudioChannels_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_audio_channels"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  guint index_i = 0;
  for (Test_I_WebTV_Channel_AudioChannelsConstIterator_t iterator_2 = channels_in.begin ();
       iterator_2 != channels_in.end ();
       ++iterator_2, ++index_i)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                       0, (*iterator_2).description.c_str (),
                       1, index_i,
                       -1);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("loaded audio channel \"%s\"\n"),
                ACE_TEXT ((*iterator_2).description.c_str ())));
  } // end FOR
}

void
load_adapters (GtkListStore* listStore_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_adapters"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  Common_UI_DisplayAdapters_t adapters_a =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Common_UI_Tools::getAdapters (true); // exclude 'mirroring' devices
#else
    Common_UI_Tools::getAdapters ();
#endif // ACE_WIN32 || ACE_WIN64
  for (Common_UI_DisplayAdaptersIterator_t iterator_2 = adapters_a.begin ();
       iterator_2 != adapters_a.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).description.c_str (),
                        1, (*iterator_2).device.c_str (),
                        -1);
  } // end FOR
}

void
load_displays (GtkListStore* listStore_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_displays"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  Common_UI_DisplayDevices_t displays_a = Common_UI_Tools::getDisplays ();
  for (Common_UI_DisplayDevicesIterator_t iterator_2 = displays_a.begin ();
       iterator_2 != displays_a.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, (*iterator_2).description.c_str (),
                        1, (*iterator_2).device.c_str (),
                        -1);
  } // end FOR
}

void
load_resolutions (GtkListStore* listStore_in,
                  const Test_I_WebTV_Channel_Resolutions_t& resolutions_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_resolutions"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  std::ostringstream converter;
  GtkTreeIter iterator;
  for (Test_I_WebTV_Channel_ResolutionsConstIterator_t iterator_2 = resolutions_in.begin ();
       iterator_2 != resolutions_in.end ();
       ++iterator_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    converter << (*iterator_2).resolution.cx;
#else
    converter << (*iterator_2).resolution.width;
#endif // ACE_WIN32 || ACE_WIN64
    converter << ACE_TEXT_ALWAYS_CHAR (" x ");
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    converter << (*iterator_2).resolution.cy;
#else
    converter << (*iterator_2).resolution.height;
#endif // ACE_WIN32 || ACE_WIN64
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                       0, converter.str ().c_str (),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       1, (*iterator_2).resolution.cx,
                       2, (*iterator_2).resolution.cy,
#else
                       1, (*iterator_2).resolution.width,
                       2, (*iterator_2).resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                       -1);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("loaded resolution %ux%u\n"),
                (*iterator_2).resolution.cx,
                (*iterator_2).resolution.cy));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("loaded resolution %ux%u\n"),
                (*iterator_2).resolution.width,
                (*iterator_2).resolution.height));
#endif // ACE_WIN32 || ACE_WIN64
  } // end FOR
}

gboolean
idle_load_channel_configuration_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_load_channel_configuration_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
      data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // close connection
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  ACE_ASSERT (data_p->videoHandle);
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->videoHandle));
#else
      iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->videoHandle));
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_p)
  {
    iconnection_p->abort ();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF
  data_p->videoHandle = ACE_INVALID_HANDLE;

  switch (data_p->currentChannel)
  {
    case 14: // Parlamentsfernsehen
    case 15: // ANIXE
    {
      for (Test_I_WebTV_Channel_ResolutionsIterator_t iterator_2 = (*channel_iterator).second.resolutions.begin ();
           iterator_2 != (*channel_iterator).second.resolutions.end ();
           ++iterator_2)
      { ACE_ASSERT (!(*iterator_2).frameRate);
        (*iterator_2).frameRate = 25;
      } // end FOR
      break;
    }
    case 18: // QVC
    case 25: // Channel NewsAsia
    {
      struct Test_I_WebTV_Channel_Resolution resolution_s;
      resolution_s.frameRate = 25;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      resolution_s.resolution.cx = 960; // *NOTE*: got this info from VLC; where does it get it from ?
      resolution_s.resolution.cy = 540;
#else
      resolution_s.resolution.width = 960; // *NOTE*: got this info from VLC; where does it get it from ?
      resolution_s.resolution.height = 540;
#endif // ACE_WIN32 || ACE_WIN64
      (*channel_iterator).second.resolutions.push_back (resolution_s);
      break;
    }
    default:
      break;
  } // end SWITCH

  // update configuration
  GtkListStore* liststore_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_AUDIOCHANNEL_NAME)));
  ACE_ASSERT (liststore_p);
  load_audio_channels (liststore_p,
                       (*channel_iterator).second.audioChannels);
  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_AUDIOCHANNEL_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_combo_box_set_active (combo_box_p, 0);
  liststore_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (liststore_p);
  load_resolutions (liststore_p,
                    (*channel_iterator).second.resolutions);
  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_combo_box_set_active (combo_box_p, 0);

  //GtkButton* button_p =
  //  GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                      ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_LOAD_NAME)));
  //ACE_ASSERT (button_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), TRUE);

  // auto-play ?
  if (data_p->nextChannel != -1)
  { ACE_ASSERT (data_p->nextChannel == static_cast<int> (data_p->currentChannel));
    guint event_source_id = g_idle_add (idle_start_session_2,
                                        userData_in);
    if (unlikely (event_source_id == 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_start_session_2): ""\"%m\", returning\n")));
      return G_SOURCE_REMOVE;
    } // end IF
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
      state_r.eventSourceIds.insert (event_source_id);
    } // end lock scope
  
    data_p->nextChannel = -1;
  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  state_r.eventSourceIds.clear ();

  gtk_main_quit ();

  // one-shot action
  return G_SOURCE_REMOVE;
}

void
load_channels (GtkListStore* listStore_in,
               const Test_I_WebTV_ChannelConfigurations_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_channels"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  for (Test_I_WebTV_ChannelConfigurationsConstIterator_t iterator_2 = channels_in.begin ();
       iterator_2 != channels_in.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                       0, (*iterator_2).second.name.c_str (),
                       1, (*iterator_2).first,
                       -1);
  } // end FOR
}

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T iterator_4b =
    data_p->configuration->streamConfiguration_4b.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_4b != data_p->configuration->streamConfiguration_4b.end ());

  // step1: initialize dialog window(s)
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BYTES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT64>::max ());

  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_CHANNEL_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  load_channels (list_store_p,
                 *data_p->channels);

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_AUDIOCHANNEL_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);
  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_AUDIOCHANNEL_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);
  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);

  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                !(*iterator_4b).second.second->display.device.empty ());

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_ADAPTER_NAME)));
  ACE_ASSERT (list_store_p);
  load_adapters (list_store_p);

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
  load_displays (list_store_p);

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);

  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  std::string target_filename_string = (*iterator_4b).second.second->targetFileName;
  gtk_toggle_button_set_active (toggle_button_p,
                                !target_filename_string.empty ());
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SAVE_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      ACE_TEXT (Common_File_Tools::basename (target_filename_string, false).c_str ()));
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  if (target_filename_string.empty ())
  {
    target_filename_string = Common_File_Tools::getTempDirectory ();
    target_filename_string += ACE_DIRECTORY_SEPARATOR_STR;
    target_filename_string +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_DEFAULT_TARGET_FILE);
  } // end IF
  if (!gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                            ACE_TEXT (Common_File_Tools::directory (target_filename_string).c_str ())))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (target_filename_string.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progressbar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progressbar_p,
                                   1.0 / static_cast<double> (width));

  GtkStatusbar* statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);
  guint context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_CONTEXT_DATA));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_DATA,
                                             context_id));
  context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_CONTEXT_INFORMATION));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                             context_id));

  // step5: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    //// schedule asynchronous updates of the log view
    //event_source_id = g_timeout_add_seconds (1,
    //                                         idle_update_log_display_cb,
    //                                         data_p);
    //if (event_source_id > 0)
    //  data_p->eventSourceIds.insert (event_source_id);
    //else
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
    //  return G_SOURCE_REMOVE;
    //} // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                     idle_update_info_display_cb,
                     data_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?
  toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), FALSE);

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6a: connect custom signals
  GtkDialog* dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  gulong result = g_signal_connect (G_OBJECT (dialog_p),
                                    ACE_TEXT_ALWAYS_CHAR ("destroy"),
                                    G_CALLBACK (button_quit_clicked_cb),
                                    data_p);
//                   G_CALLBACK(gtk_widget_destroyed),
//                   &main_dialog_p,
  ACE_ASSERT (result);
//  result = g_signal_connect (G_OBJECT (dialog_p),
//                    ACE_TEXT_ALWAYS_CHAR ("delete-event"),
//                    G_CALLBACK (delete_event_cb),
//                    NULL);
  result = g_signal_connect (dialog_p,
                             ACE_TEXT_ALWAYS_CHAR ("destroy"),
                             G_CALLBACK (gtk_widget_destroyed),
                             NULL);
  ACE_ASSERT (result);

  dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (dialog_p);
  result = g_signal_connect_swapped (G_OBJECT (dialog_p),
                                     ACE_TEXT_ALWAYS_CHAR ("response"),
                                     G_CALLBACK (gtk_widget_hide),
                                     dialog_p);
  ACE_ASSERT (result);

  //-------------------------------------

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  GtkDrawingArea* drawing_area_p =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  (*iterator_4b).second.second->window =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT ((*iterator_4b).second.second->window);

  // select some widgets
#if GTK_CHECK_VERSION (2,30,0)
  struct _GValue value = G_VALUE_INIT;
#else
  struct _GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  if (data_p->currentChannel)
  {
    data_p->nextChannel = data_p->currentChannel; // auto-play
    combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME)));
    ACE_ASSERT (combo_box_p);
    g_value_init (&value, G_TYPE_UINT);
    g_value_set_uint (&value,
                      data_p->currentChannel);
    Common_UI_GTK_Tools::selectValue (combo_box_p,
                                      value,
                                      1);
    g_value_unset (&value);
  } // end IF

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
#if GTK_CHECK_VERSION (2,30,0)
  value = G_VALUE_INIT;
#else
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value,
                      (*iterator_4b).second.second->display.device.c_str ());
  Common_UI_GTK_Tools::selectValue (combo_box_p,
                                    value,
                                    1);
  g_value_unset (&value);

  return G_SOURCE_REMOVE;
}

void
add_segment_URIs (unsigned int program_in,
                  const std::string& lastURI_in,
                  Test_I_WebTV_ChannelSegmentURLs_t& URIs_out,
                  unsigned int maxIndex_in,
                  unsigned int indexPositions_in)
{
  NETWORK_TRACE (ACE_TEXT ("::add_segment_URIs"));

  std::string URI_string_head, URI_string_head_2, URI_string_tail;
  std::string regex_string;
  std::regex regex;
  std::smatch match_results;
  std::stringstream converter;
  bool URI_has_path = false;
  std::string::size_type position_i;

  switch (program_in)
  {
    case 5:  // ARTE
    case 8:  // MDR
    case 28: // NASA TV
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = lastURI_in.rfind ('/', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);
      URI_string_tail =
          lastURI_in.substr (position_i + 1, std::string::npos);

      position_i = lastURI_in.find ('/', 0);
      ACE_ASSERT (position_i != std::string::npos);
      position_i = lastURI_in.find ('/', position_i + 1);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = lastURI_in;
      URI_string_head.erase (position_i + 1, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (lastURI_in.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^(master)(_)([[:alnum:]]+)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (lastURI_in.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str ());
      URI_string_tail = match_results[6].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 7: // HR
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = lastURI_in.rfind ('/', std::string::npos);
      URI_string_tail =
          lastURI_in.substr (position_i + 1, std::string::npos);
      URI_string_head = lastURI_in;
      position_i = lastURI_in.find ('/', 0);
      position_i = lastURI_in.find ('/', position_i + 1);
      URI_string_head.erase (position_i, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (lastURI_in.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^-]+)(-)([^-]+)(-)([[:digit:]]+)(_)([[:digit:]]+)(.ts|.aac)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (lastURI_in.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> date_i;

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[7].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () + // date
                           match_results[6].str ());
      URI_string_tail = match_results[8].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        URI_string += '/';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 10: // RBB
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = lastURI_in.rfind ('/', std::string::npos);
      URI_string_tail =
          lastURI_in.substr (position_i + 1, std::string::npos);
      URI_string_head = lastURI_in;
      position_i = lastURI_in.find ('/', 0);
      position_i = lastURI_in.find ('/', position_i + 1);
      URI_string_head.erase (position_i, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (lastURI_in.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^-]+)(-)([^-]+)(-)([^-]+)(-)([^-]+)(-)([[:digit:]]+)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (lastURI_in.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[9].str ());
      converter >> date_i;

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[11].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () +
                           match_results[6].str () +
                           match_results[7].str () +
                           match_results[8].str () +
                           match_results[9].str () + // date
                           match_results[10].str ());
      URI_string_tail = match_results[12].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        URI_string += '/';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 13: // WDR
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = lastURI_in.rfind ('/', std::string::npos);
      URI_string_tail =
          lastURI_in.substr (position_i + 1, std::string::npos);
      URI_string_head = lastURI_in;
      position_i = lastURI_in.find ('/', 0);
      URI_string_head.erase (position_i, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (lastURI_in.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^_]+)(_)([[:digit:]]+)(_)([[:digit:]]+)(.ts|.aac)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (lastURI_in.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[3].str ());
      converter >> date_i;

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () + // date
                           match_results[4].str ());
      URI_string_tail = match_results[6].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        URI_string += '/';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 18: // QVC
    {
      ACE_UINT32 index_i = 0, date_i = 0;

      position_i = lastURI_in.rfind ('_', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = lastURI_in;
      URI_string_head.erase (position_i + 1, std::string::npos);

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+)(_)([^_]+)(_)([^_]+)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (lastURI_in,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (lastURI_in.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[7].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () +
                           match_results[6].str ());
      URI_string_tail = match_results[8].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << ++index_i;
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 25: // Channel NewsAsia
    {
      ACE_UINT32 index_i = 0, date_i = 0;

      position_i = lastURI_in.rfind ('_', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = lastURI_in;
      URI_string_head.erase (position_i + 1, std::string::npos);

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^_]+)(_)([^_]+)(_)([[:digit:]]+)(.*)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (lastURI_in,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (lastURI_in.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str ());
      URI_string_tail = match_results[6].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << ++index_i;
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    default:
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = lastURI_in.rfind ('/', std::string::npos);
      if (position_i != std::string::npos)
      {
        URI_has_path = true;
        URI_string_tail =
            lastURI_in.substr (position_i + 1, std::string::npos);
        URI_string_head = lastURI_in;
        position_i = lastURI_in.find ('/', 0);
        URI_string_head.erase (position_i, std::string::npos);

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (lastURI_in.substr (position_i + 1, indexPositions_in));
        converter >> index_2;

        regex_string =
            ACE_TEXT_ALWAYS_CHAR ("^([^_]+)(_)([[:alnum:]]+)(_)([[:digit:]]+)(_)([[:digit:]]+)(_)([[:digit:]]+)(.ts|.aac)$");
        regex.assign (regex_string);
        if (unlikely(!std::regex_match (URI_string_tail,
                                        match_results,
                                        regex,
                                        std::regex_constants::match_default)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                      ACE_TEXT (lastURI_in.c_str ())));
          return;
        } // end IF
        ACE_ASSERT (match_results.ready () && !match_results.empty ());

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[7].str ());
        converter >> date_i;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[9].str ());
        converter >> index_i;
        URI_string_head_2 = (match_results[1].str () +
                             match_results[2].str () +
                             match_results[3].str () +
                             match_results[4].str () +
                             match_results[5].str () +
                             match_results[6].str () +
                             match_results[7].str () + // date
                             match_results[8].str ());
        URI_string_tail = match_results[10].str ();
      } // end IF
      else
      {
        std::string regex_string =
            ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+)(.ts|.aac)$");
        regex.assign (regex_string);
        if (unlikely(!std::regex_match (lastURI_in,
                                        match_results,
                                        regex,
                                        std::regex_constants::match_default)))
        {
          ACE_DEBUG ((LM_ERROR,
                     ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                     ACE_TEXT (lastURI_in.c_str ())));
          return;
        } // end IF
        ACE_ASSERT (match_results.ready () && !match_results.empty ());
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[1].str ());
        converter >> index_i;
        URI_string_tail = match_results[2].str ();
      } // end ELSE

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        if (URI_has_path)
        {
          URI_string += '/';
          converter.str (ACE_TEXT_ALWAYS_CHAR (""));
          converter.clear ();
          converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
          URI_string += converter.str ();
          URI_string += '/';
          URI_string += URI_string_head_2;
        } // end IF

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
  } // end SWITCH
}

gboolean
idle_segment_download_complete_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_segment_download_complete_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
    data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  ACE_ASSERT (!(*channel_iterator).second.videoSegment.URLs.empty ());
  ACE_ASSERT (data_p->videoTimeoutHandler->lock_);
  std::string current_URL;
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_3a =
      data_p->configuration->streamConfiguration_3a.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator_3a != data_p->configuration->streamConfiguration_3a.end ());

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, *data_p->videoTimeoutHandler->lock_, G_SOURCE_REMOVE);
    if ((data_p->currentAudioStream == -1) ||
        (*stream_iterator_3a).second.second->URL.empty ())
      goto continue_;
    if ((*channel_iterator).second.audioSegment.URLs.size () <= TEST_I_WEBTV_DEFAULT_SEGMENT_LIST_LWM)
    {
      current_URL = (*channel_iterator).second.audioSegment.URLs.back ();
      add_segment_URIs (data_p->currentChannel,
                        current_URL,
                        (*channel_iterator).second.audioSegment.URLs,
                        (*channel_iterator).second.maxIndex,
                        (*channel_iterator).second.indexPositions);
    } // end IF

continue_:
    if ((*channel_iterator).second.videoSegment.URLs.size () <= TEST_I_WEBTV_DEFAULT_SEGMENT_LIST_LWM)
    {
      current_URL = (*channel_iterator).second.videoSegment.URLs.back ();
      add_segment_URIs (data_p->currentChannel,
                        current_URL,
                        (*channel_iterator).second.videoSegment.URLs,
                        (*channel_iterator).second.maxIndex,
                        (*channel_iterator).second.indexPositions);
    } // end ELSE IF
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_video_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_video_display_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  GtkDrawingArea* drawing_area_2 =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_FULLSCREEN_NAME)));
  ACE_ASSERT (drawing_area_2);

  drawing_area_p =
      (gtk_toggle_button_get_active (toggle_button_p) ? drawing_area_2
                                                      : drawing_area_p);
  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)),
                              NULL,
                              false);

  return G_SOURCE_CONTINUE;
}

gboolean
idle_notify_segment_data_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_notify_segment_data_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
      data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_3a =
      data_p->configuration->streamConfiguration_3a.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator_3a != data_p->configuration->streamConfiguration_3a.end ());

  if ((*channel_iterator).second.videoSegment.URLs.empty () ||
      (((data_p->currentAudioStream != -1) && (*channel_iterator).second.audioSegment.URLs.empty ()) &&
       ((data_p->currentAudioStream != -1) && !(*stream_iterator_3a).second.second->URL.empty ()))) // e.g. RBB
    return G_SOURCE_REMOVE; // wait for audio/video segment data

  // close connection(s)
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  ACE_ASSERT (data_p->audioHandle);
  ACE_ASSERT (data_p->videoHandle);

  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->audioHandle));
#else
      iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->audioHandle));
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_p)
  {
    iconnection_p->abort ();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF
  data_p->audioHandle = ACE_INVALID_HANDLE;

  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->videoHandle));
#else
      iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->videoHandle));
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_p)
  {
    iconnection_p->abort ();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF
  data_p->videoHandle = ACE_INVALID_HANDLE;

  // got both A/V segment data; schedule session start
  guint event_source_id = g_idle_add (idle_start_session_cb,
                                      userData_in);
  if (unlikely (event_source_id == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_session_cb): ""\"%m\", returning\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_start_session_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_ConnectionManager_3_t::INTERFACE_T* iconnection_manager_2 =
      TEST_I_CONNECTIONMANAGER_SINGLETON_3::instance ();
  ACE_ASSERT (iconnection_manager_2);
  Net_ConnectionConfigurationsIterator_t iterator_3a =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("3a"));
  ACE_ASSERT (iterator_3a != data_p->configuration->connectionConfigurations.end ());
  Net_ConnectionConfigurationsIterator_t iterator_3b =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("3b"));
  ACE_ASSERT (iterator_3b != data_p->configuration->connectionConfigurations.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_3a =
      data_p->configuration->streamConfiguration_3a.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator_3a != data_p->configuration->streamConfiguration_3a.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_marshal_3a =
      data_p->configuration->streamConfiguration_3a.find (ACE_TEXT_ALWAYS_CHAR ("Marshal"));
  ACE_ASSERT (stream_iterator_marshal_3a != data_p->configuration->streamConfiguration_3a.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_3b =
      data_p->configuration->streamConfiguration_3b.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator_3b != data_p->configuration->streamConfiguration_3b.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_marshal_3b =
      data_p->configuration->streamConfiguration_3b.find (ACE_TEXT_ALWAYS_CHAR ("Marshal"));
  ACE_ASSERT (stream_iterator_marshal_3b != data_p->configuration->streamConfiguration_3b.end ());
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
      data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  ACE_ASSERT (!(*channel_iterator).second.videoSegment.URLs.empty ());
  ACE_ASSERT (data_p->audioTimeoutHandler);
  ACE_ASSERT (data_p->videoTimeoutHandler);

  (*stream_iterator_3a).second.second->parserConfiguration->messageQueue = NULL;
  (*stream_iterator_marshal_3a).second.second->parserConfiguration->messageQueue = NULL;
  (*stream_iterator_3b).second.second->parserConfiguration->messageQueue = NULL;
  (*stream_iterator_marshal_3b).second.second->parserConfiguration->messageQueue = NULL;

  Test_I_TCPConnector_3_t connector (true);
#if defined (SSL_SUPPORT)
  Test_I_SSLConnector_3_t ssl_connector (true);
#endif // SSL_SUPPORT
  Test_I_AsynchTCPConnector_3_t asynch_connector (true);
  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string, URI_string_2, URL_string, URL_string_2;
  bool use_SSL = false;
  struct Net_UserData user_data_s;
  //ACE_ASSERT (((data_p->currentAudioStream == -1) || !(*channel_iterator).second.audioSegment.URLs.empty ()) &&
  //            !(*channel_iterator).second.videoSegment.URLs.empty ());

  std::string current_URL_1, current_URL_2;
  bool is_URI_b, is_URI_2;

  if ((data_p->currentAudioStream == -1) ||
      (*stream_iterator_3a).second.second->URL.empty ())
    goto continue_;
  current_URL_1 =
      (*channel_iterator).second.audioSegment.URLs.front ();
  (*channel_iterator).second.audioSegment.URLs.pop_front ();
  is_URI_b = HTTP_Tools::URLIsURI (current_URL_1);
  if (is_URI_b)
    URL_string = (*stream_iterator_3a).second.second->URL;
  else
    URL_string = current_URL_1;
  if (!HTTP_Tools::parseURL (URL_string,
                             host_address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (URL_string.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.address =
      host_address;
  static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.hostname =
      hostname_string;
  if (is_URI_b)
  {
    size_t position = URI_string.find_last_of ('/', std::string::npos);
    ACE_ASSERT (position != std::string::npos);
    URI_string.erase (position + 1, std::string::npos);
    URI_string_2 = URI_string;
    URI_string_2 += current_URL_1;
    URI_string = URI_string_2;
  } // end IF

continue_:
  current_URL_2 =
      (*channel_iterator).second.videoSegment.URLs.front ();
  (*channel_iterator).second.videoSegment.URLs.pop_front ();
  is_URI_2 = HTTP_Tools::URLIsURI (current_URL_2);
  if (is_URI_2)
    URL_string_2 = (*stream_iterator_3b).second.second->URL;
  else
    URL_string_2 = current_URL_2;
  if (!HTTP_Tools::parseURL (URL_string_2,
                             host_address,
                             hostname_string,
                             URI_string_2,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (URL_string_2.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.address =
      host_address;
  static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.hostname =
      hostname_string;
  if (is_URI_2)
  {
    size_t position = URI_string_2.find_last_of ('/', std::string::npos);
    ACE_ASSERT (position != std::string::npos);
    URI_string_2.erase (position + 1, std::string::npos);
    URI_string = URI_string_2;
    URI_string += current_URL_2;
    URI_string_2 = URI_string;
  } // end IF

  // connect to peers
  if ((data_p->currentAudioStream == -1) ||
      (*stream_iterator_3a).second.second->URL.empty ())
    goto continue_2;
  if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      data_p->audioHandle =
          Net_Client_Common_Tools::connect (ssl_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    else
#endif // SSL_SUPPORT
      data_p->audioHandle =
          Net_Client_Common_Tools::connect (connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    data_p->audioHandle =
        Net_Client_Common_Tools::connect (asynch_connector,
                                          *static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second),
                                          user_data_s,
                                          static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.address,
                                          false, // wait ?
                                          true); // peer address ?
  } // end ELSE
  if (data_p->audioHandle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.address).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: opened socket to %s\n"),
              data_p->audioHandle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: opened socket to %s\n"),
              data_p->audioHandle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

continue_2:
  if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      data_p->videoHandle =
          Net_Client_Common_Tools::connect (ssl_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    else
#endif // SSL_SUPPORT
      data_p->videoHandle =
          Net_Client_Common_Tools::connect (connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    data_p->videoHandle =
        Net_Client_Common_Tools::connect (asynch_connector,
                                          *static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second),
                                          user_data_s,
                                          static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.address,
                                          false, // wait ?
                                          true); // peer address ?
  } // end ELSE
  if (data_p->videoHandle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.address).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: opened socket to %s\n"),
              data_p->videoHandle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: opened socket to %s\n"),
              data_p->videoHandle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  const Stream_Module_t* module_p =
      data_p->AVStream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING));
  ACE_ASSERT (module_p);
  data_p->dispatch =
      dynamic_cast<Common_IDispatch*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (data_p->dispatch);
  Common_ISetP_T<GdkWindow>* isetp_p =
      dynamic_cast<Common_ISetP_T<GdkWindow>*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (isetp_p);
  GtkDrawingArea* drawing_area_p =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  //isetp_p->setP (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)));

  //ACE_ASSERT (!data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    data_p->progressData.eventSourceId =
    //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
    //                 idle_update_progress_cb,
    //                 &data_p->progressData,
    //                 NULL);
    g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                    COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                    idle_update_progress_cb,
                    &data_p->progressData);//,
//                       NULL);
    if (data_p->progressData.eventSourceId > 0)
      state_r.eventSourceIds.insert (data_p->progressData.eventSourceId);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end IF
  } // end lock scope

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    data_p->videoUpdateEventSourceId =
    g_timeout_add (COMMON_UI_REFRESH_DEFAULT_VIDEO_MS, // ms (?)
                   idle_update_video_display_cb,
                   userData_in);
    ACE_ASSERT (data_p->videoUpdateEventSourceId > 0);
    state_r.eventSourceIds.insert (data_p->videoUpdateEventSourceId);
  } // end lock scope

  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  //ACE_ASSERT (gtk_toggle_button_get_active (toggle_button_p));
  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                          GTK_STOCK_MEDIA_STOP);
    un_toggling_play = true;
    gtk_toggle_button_toggled (toggle_button_p);
  } // end IF

  // schedule timers
  if ((data_p->currentAudioStream == -1) ||
      (*stream_iterator_3a).second.second->URL.empty ())
    goto continue_3;
  data_p->audioTimeoutHandler->initialize (data_p->audioHandle,
                                           &(*channel_iterator).second.audioSegment,
                                           (*stream_iterator_3a).second.second->URL,
                                           static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->allocatorConfiguration,
                                           static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3a).second)->messageAllocator);
  data_p->audioTimeoutHandler->interval_.sec ((*channel_iterator).second.audioSegment.length);
  // start first download now
  data_p->audioTimeoutHandler->handle (NULL);
  // schedule regular downloads in-between buffers
  data_p->audioTimeoutHandler->timerId_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (data_p->audioTimeoutHandler,
                                                                  NULL,
                                                                  ACE_Time_Value ((*channel_iterator).second.audioSegment.length / 2, 0),
                                                                  data_p->audioTimeoutHandler->interval_);
  ACE_ASSERT (data_p->audioTimeoutHandler->timerId_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("scheduled audio segment download interval timer (id: %d, interval: %#T)\n"),
              data_p->audioTimeoutHandler->timerId_,
              &data_p->audioTimeoutHandler->interval_));

continue_3:
  data_p->videoTimeoutHandler->initialize (data_p->videoHandle,
                                           &(*channel_iterator).second.videoSegment,
                                           (*stream_iterator_3b).second.second->URL,
                                           static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->allocatorConfiguration,
                                           static_cast<Test_I_WebTV_ConnectionConfiguration_3_t*> ((*iterator_3b).second)->messageAllocator);
  data_p->videoTimeoutHandler->interval_.sec ((*channel_iterator).second.videoSegment.length);
  // start first download now
  data_p->videoTimeoutHandler->handle (NULL);
  // schedule regular downloads in-between buffers
  data_p->videoTimeoutHandler->timerId_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (data_p->videoTimeoutHandler,
                                                                  NULL,
                                                                  ACE_Time_Value ((*channel_iterator).second.videoSegment.length / 2, 0),
                                                                  data_p->videoTimeoutHandler->interval_);
  ACE_ASSERT (data_p->videoTimeoutHandler->timerId_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("scheduled video segment download interval timer (id: %d, interval: %#T)\n"),
              data_p->videoTimeoutHandler->timerId_,
              &data_p->videoTimeoutHandler->interval_));

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_session_2 (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_start_session_2"));

  ACE_UNUSED_ARG (userData_in);

  // sanity check(s)
  //struct Test_I_WebTV_UI_CBData* data_p =
  //  static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  //ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p, TRUE);

  return G_SOURCE_REMOVE;
}

#if GTK_CHECK_VERSION (3,0,0)
gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  ACE_ASSERT (context_in);
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  if (!data_p->dispatch)
    return FALSE; // propagate further

  try {
    data_p->dispatch->dispatch (context_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  gtk_widget_queue_draw (widget_in);

  return FALSE; // propagate further
}
#else
gboolean
drawingarea_expose_event_cb (GtkWidget* widget_in,
                             GdkEvent* event_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_expose_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  if (!data_p->dispatch)
    return FALSE;

  // sanity check(s)
  cairo_t* context_p =
    gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget_in)));
  if (unlikely (!context_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
    return FALSE;
  } // end IF

  try {
    data_p->dispatch->dispatch (context_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  cairo_destroy (context_p); context_p = NULL;

  return FALSE; // propagate
} // drawingarea_expose_event_cb
#endif // GTK_CHECK_VERSION(3,0,0)

gboolean
drawing_area_resize_end (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawing_area_resize_end"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  GtkDrawingArea* drawing_area_p =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                                (gtk_toggle_button_get_active (toggle_button_p) ? ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_FULLSCREEN_NAME)
                                                                                                : ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME))));
  ACE_ASSERT (drawing_area_p);

  GtkAllocation allocation_s;
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation_s);
  ACE_DEBUG ((LM_DEBUG,
             ACE_TEXT ("window resized to %dx%d\n"),
             allocation_s.width, allocation_s.height));

  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T iterator_4b =
      data_p->configuration->streamConfiguration_4b.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_4b != data_p->configuration->streamConfiguration_4b.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*iterator_4b).second.second->outputFormat.video.resolution.cy =
    allocation_s.height;
  (*iterator_4b).second.second->outputFormat.video.resolution.cx =
    allocation_s.width;
#else
  (*iterator_4b).second.second->outputFormat.video.resolution.height =
      allocation_s.height;
  (*iterator_4b).second.second->outputFormat.video.resolution.width =
      allocation_s.width;
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (data_p->AVStream);
  if (!data_p->AVStream->isRunning ())
    return FALSE;

  // *NOTE*: two things need doing:
  //         - drop inbound frames until the 'resize' session message is through
  //         - enqueue a 'resize' session message

  // step1:
  const Stream_Module_t* module_p =
    data_p->AVStream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING));
  ACE_ASSERT (module_p);
  Stream_Visualization_IResize* iresize_p =
      dynamic_cast<Stream_Visualization_IResize*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (iresize_p);
  try {
    iresize_p->resizing ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("caught exception in Stream_Visualization_IResize::resizing(), aborting\n")));
    return FALSE;
  }

  // step2
  data_p->AVStream->notify (STREAM_SESSION_MESSAGE_RESIZE,
                          false, // recurse upstream ?
                          true); // expedite ?

  return FALSE;
} // drawing_area_resize_end

void
drawingarea_size_allocate_cb (GtkWidget* widget_in,
                              GdkRectangle* allocation_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (allocation_in);

  static gint timer_id = 0;
  if (timer_id == 0)
  {
    timer_id = g_timeout_add (300, drawing_area_resize_end, userData_in);
    return;
  } // end IF
  g_source_remove (timer_id);
  timer_id = g_timeout_add (300, drawing_area_resize_end, userData_in);
} // drawingarea_size_allocate_cb

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_ProgressData* data_p =
    static_cast<struct Test_I_WebTV_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
  int result = -1;
  float speed = 0.0F;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_CONTINUE);
    speed = data_p->statistic.bytesPerSecond;
  } // end lock scope
  std::string magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  if (speed)
  {
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("kbyte(s)/s");
    } // end IF
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("mbyte(s)/s");
    } // end IF
    result = ACE_OS::sprintf (buffer_a, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (buffer_a));
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_CONNECT:
        case COMMON_UI_EVENT_DISCONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_spin (spin_button_p,
                                ((*event_p == COMMON_UI_EVENT_CONNECT) ? GTK_SPIN_STEP_FORWARD
                                                                       : GTK_SPIN_STEP_BACKWARD),
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BYTES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BYTES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.bytes));

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        case COMMON_UI_EVENT_ABORT:
        case COMMON_UI_EVENT_STEP:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        case COMMON_UI_EVENT_RESIZE:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *event_p));
          break;
        }
      } // end SWITCH
      ACE_UNUSED_ARG (is_session_message);
      gtk_spin_button_spin (spin_button_p,
                            GTK_SPIN_STEP_FORWARD,
                            1.0);
      event_p = NULL;
    } // end FOR

    // clean up
    while (!state_r.eventStack.is_empty ())
    {
      result = state_r.eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

/////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
togglebutton_play_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_play_toggled_cb"));

  if (un_toggling_play)
  {
    un_toggling_play = false;
    return;
  } // end IF

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_3_t::INTERFACE_T* iconnection_manager_2 =
      TEST_I_CONNECTIONMANAGER_SINGLETON_3::instance ();
  ACE_ASSERT (iconnection_manager_2);

//  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
//  bool success = false;

  if (gtk_toggle_button_get_active (toggleButton_in))
  {
    // --> download segment playlist

    // step1: update widgets
    gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                          GTK_STOCK_MEDIA_STOP);
    GtkFrame* frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CHANNEL_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);
    frame_p =
        GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

    // step2: update configuration
    std::string URI_string, URI_string_2;
    GtkToggleButton* toggle_button_p = NULL;
    GtkEntry* entry_p = NULL;
    GtkFileChooserButton* file_chooser_button_p = NULL;
    Net_ConnectionConfigurationsIterator_t iterator_2a =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("2a"));
    ACE_ASSERT (iterator_2a != data_p->configuration->connectionConfigurations.end ());
    Net_ConnectionConfigurationsIterator_t iterator_2b =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("2b"));
    ACE_ASSERT (iterator_2b != data_p->configuration->connectionConfigurations.end ());
    Test_I_WebTV_StreamConfiguration_t::ITERATOR_T stream_iterator_2a =
      data_p->configuration->streamConfiguration_2a.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (stream_iterator_2a != data_p->configuration->streamConfiguration_2a.end ());
    Test_I_WebTV_StreamConfiguration_t::ITERATOR_T stream_iterator_marshal_2a =
      data_p->configuration->streamConfiguration_2a.find (ACE_TEXT_ALWAYS_CHAR ("Marshal"));
    ACE_ASSERT (stream_iterator_marshal_2a != data_p->configuration->streamConfiguration_2a.end ());
    Test_I_WebTV_StreamConfiguration_t::ITERATOR_T stream_iterator_2b =
      data_p->configuration->streamConfiguration_2b.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (stream_iterator_2b != data_p->configuration->streamConfiguration_2b.end ());
    Test_I_WebTV_StreamConfiguration_t::ITERATOR_T stream_iterator_marshal_2b =
      data_p->configuration->streamConfiguration_2b.find (ACE_TEXT_ALWAYS_CHAR ("Marshal"));
    ACE_ASSERT (stream_iterator_marshal_2b != data_p->configuration->streamConfiguration_2b.end ());
    Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_3a =
        data_p->configuration->streamConfiguration_3a.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (stream_iterator_3a != data_p->configuration->streamConfiguration_3a.end ());
    Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_3b =
        data_p->configuration->streamConfiguration_3b.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (stream_iterator_3b != data_p->configuration->streamConfiguration_3b.end ());
    Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T stream_iterator_4b =
        data_p->configuration->streamConfiguration_4b.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (stream_iterator_4b != data_p->configuration->streamConfiguration_4b.end ());
    ACE_ASSERT ((*stream_iterator_4b).second.second->delayConfiguration);
    Test_I_TCPConnector_t connector (true);
#if defined (SSL_SUPPORT)
    Test_I_SSLConnector_t ssl_connector (true);
#endif // SSL_SUPPORT
    Test_I_AsynchTCPConnector_t asynch_connector (true);
//    GtkSpinner* spinner_p = NULL;
    GtkProgressBar* progressbar_p = NULL;
    GtkTreeIter iterator_7;
#if GTK_CHECK_VERSION (2,30,0)
    struct _GValue value = G_VALUE_INIT;
#else
    struct _GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
    Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator;
    ACE_INET_Addr host_address;
    std::string hostname_string;
    bool use_SSL = false;
    struct Net_UserData user_data_s;

    data_p->configuration->parserConfiguration.messageQueue = NULL;
    (*stream_iterator_2a).second.second->parserConfiguration->messageQueue = NULL;
    (*stream_iterator_marshal_2a).second.second->parserConfiguration->messageQueue = NULL;
    (*stream_iterator_2b).second.second->parserConfiguration->messageQueue = NULL;
    (*stream_iterator_marshal_2b).second.second->parserConfiguration->messageQueue = NULL;

    // retrieve stream URLs
    GtkComboBox* combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
    ACE_ASSERT (combo_box_p);
    if (!gtk_combo_box_get_active_iter (combo_box_p,
                                        &iterator_7))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
      return;
    } // end IF
    GtkListStore* list_store_p =
        GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
    ACE_ASSERT (list_store_p);
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_7,
                              1,
                              &value);
    ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.cx =
        g_value_get_uint (&value);
#else
    data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.width =
        g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
    g_value_unset (&value);
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_7,
                              2, &value);
    ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.cy =
        g_value_get_uint (&value);
#else
    data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.height =
        g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
    g_value_unset (&value);
    ACE_ASSERT (data_p->channels);
    ACE_ASSERT (data_p->currentChannel);
    channel_iterator = data_p->channels->find (data_p->currentChannel);
    ACE_ASSERT (channel_iterator != data_p->channels->end ());

    combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_AUDIOCHANNEL_NAME)));
    ACE_ASSERT (combo_box_p);
    if (!gtk_combo_box_get_active_iter (combo_box_p,
                                        &iterator_7))
    { // most likely cause, joint a/v stream (e.g. ARTE de/fr)
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
      data_p->currentAudioStream = -1;
      goto continue_;
    } // end IF
    list_store_p =
        GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_AUDIOCHANNEL_NAME)));
    ACE_ASSERT (list_store_p);
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_7,
                              1,
                              &value);
    ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
    data_p->currentAudioStream = g_value_get_uint (&value);
    g_value_unset (&value);

continue_:
    // compute URI/URL for video stream
    for (Test_I_WebTV_Channel_ResolutionsConstIterator_t iterator_8 = (*channel_iterator).second.resolutions.begin ();
         iterator_8 != (*channel_iterator).second.resolutions.end ();
         ++iterator_8)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (((*iterator_8).resolution.cx == data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.cx) &&
          ((*iterator_8).resolution.cy == data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.cy))
#else
      if (((*iterator_8).resolution.width == data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.width) &&
          ((*iterator_8).resolution.height == data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.resolution.height))
#endif // ACE_WIN32 || ACE_WIN64
      { ACE_ASSERT ((*iterator_8).frameRate);
        (*stream_iterator_4b).second.second->delayConfiguration->interval.msec (static_cast<long> (1000 / (*iterator_8).frameRate));
        (*stream_iterator_4b).second.second->delayConfiguration->averageTokensPerInterval =
            1;
        URI_string = (*iterator_8).URI;
        break;
      } // end IF
    //ACE_ASSERT (!URI_string.empty ()); // *NOTE* might be empty (e.g. QVC)
    if (!HTTP_Tools::parseURL ((*channel_iterator).second.mainURL,
                               host_address,
                               hostname_string,
                               URI_string_2,
                               use_SSL))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                 ACE_TEXT ((*channel_iterator).second.mainURL.c_str ())));
      return;
    } // end IF
    bool is_URI_b =
      (URI_string.empty () ? false : HTTP_Tools::URLIsURI (URI_string));
    if (is_URI_b)
    {
      (*stream_iterator_2b).second.second->URL = ACE_TEXT_ALWAYS_CHAR ("http");
      (*stream_iterator_2b).second.second->URL +=
          (use_SSL ? ACE_TEXT_ALWAYS_CHAR ("s") : ACE_TEXT_ALWAYS_CHAR (""));
      (*stream_iterator_2b).second.second->URL += ACE_TEXT_ALWAYS_CHAR ("://");
      (*stream_iterator_2b).second.second->URL += hostname_string;
      size_t position = URI_string_2.find_last_of ('/', std::string::npos);
      ACE_ASSERT (position != std::string::npos);
      URI_string_2.erase (position + 1, std::string::npos);
      (*stream_iterator_2b).second.second->URL += URI_string_2;
      (*stream_iterator_2b).second.second->URL += URI_string;
    } // end IF
    else
      (*stream_iterator_2b).second.second->URL =
        (!URI_string.empty () ? URI_string : (*channel_iterator).second.mainURL);
    static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.address =
        host_address;
    static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.hostname =
        hostname_string;
    //static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address =
    //  host_address;
    //static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.hostname =
    //  hostname_string;
    (*stream_iterator_3b).second.second->URL = (*stream_iterator_2b).second.second->URL;

    // compute URI/URL for audio stream
    if (data_p->currentAudioStream == -1)
      goto continue_2;
    URI_string =
      (*channel_iterator).second.audioChannels[data_p->currentAudioStream].URI;
    //ACE_ASSERT (!URI_string.empty ()); // e.g. RBB (default audio)
    is_URI_b =
      (URI_string.empty () ? false : HTTP_Tools::URLIsURI (URI_string));
    if (is_URI_b)
    {
      (*stream_iterator_2a).second.second->URL = ACE_TEXT_ALWAYS_CHAR ("http");
      (*stream_iterator_2a).second.second->URL +=
          (use_SSL ? ACE_TEXT_ALWAYS_CHAR ("s") : ACE_TEXT_ALWAYS_CHAR (""));
      (*stream_iterator_2a).second.second->URL += ACE_TEXT_ALWAYS_CHAR ("://");
      (*stream_iterator_2a).second.second->URL += hostname_string;
      (*stream_iterator_2a).second.second->URL += URI_string_2;
      (*stream_iterator_2a).second.second->URL += URI_string;
    } // end IF
    else
      (*stream_iterator_2a).second.second->URL = URI_string;
    static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address =
        host_address;
    static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.hostname =
        hostname_string;
    //static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address =
    //  host_address;
    //static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.hostname =
    //  hostname_string;
    (*stream_iterator_3a).second.second->URL = (*stream_iterator_2a).second.second->URL;

continue_2:
    // save to file ?
    toggle_button_p =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
    ACE_ASSERT (toggle_button_p);
    if (!gtk_toggle_button_get_active (toggle_button_p))
    {
      (*stream_iterator_4b).second.second->targetFileName.clear ();
      goto continue_3;
    } // end IF
    entry_p =
        GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_SAVE_NAME)));
    ACE_ASSERT (entry_p);
    file_chooser_button_p =
        GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
    ACE_ASSERT (file_chooser_button_p);
    (*stream_iterator_4b).second.second->targetFileName =
      gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p));
    (*stream_iterator_4b).second.second->targetFileName +=
      ACE_DIRECTORY_SEPARATOR_STR;
    (*stream_iterator_4b).second.second->targetFileName +=
      gtk_entry_get_text (entry_p);

continue_3:
    // step2: start processing stream
    if (unlikely (!data_p->AudioStream->initialize (data_p->configuration->streamConfiguration_4a)))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to Test_I_Stream::initialize(), returning\n")));
      return;
    } // end IF
    data_p->AudioStream->start ();
    if (unlikely (!data_p->AVStream->initialize (data_p->configuration->streamConfiguration_4b)))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to Test_I_Stream::initialize(), returning\n")));
      return;
    } // end IF
    data_p->AVStream->start ();
    data_p->streamSessionId = data_p->AVStream->getR_2 ().getR ().sessionId;

    // step3: connect to peers
    if ((data_p->currentAudioStream == -1) ||
        (*stream_iterator_3a).second.second->URL.empty ())
      goto continue_4;
    if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
    {
#if defined (SSL_SUPPORT)
      if (use_SSL)
        data_p->audioHandle =
            Net_Client_Common_Tools::connect (ssl_connector,
                                              *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second),
                                              user_data_s,
                                              static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address,
                                              false, // wait ?
                                              true); // peer address ?
      else
#endif // SSL_SUPPORT
        data_p->audioHandle =
            Net_Client_Common_Tools::connect (connector,
                                              *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second),
                                              user_data_s,
                                              static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address,
                                              false, // wait ?
                                              true); // peer address ?
    } // end IF
    else
    {
#if defined (SSL_SUPPORT)
      // *TODO*: add SSL support to the proactor framework
      ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
      data_p->audioHandle =
          Net_Client_Common_Tools::connect (asynch_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    } // end ELSE
    if (data_p->audioHandle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address).c_str ())));
      goto error;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: opened socket to %s\n"),
                data_p->audioHandle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address).c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: opened socket to %s\n"),
                data_p->audioHandle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2a).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

continue_4:
    if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
    {
#if defined (SSL_SUPPORT)
      if (use_SSL)
        data_p->videoHandle =
            Net_Client_Common_Tools::connect (ssl_connector,
                                              *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second),
                                              user_data_s,
                                              static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.address,
                                              false, // wait ?
                                              true); // peer address ?
      else
#endif // SSL_SUPPORT
        data_p->videoHandle =
            Net_Client_Common_Tools::connect (connector,
                                              *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second),
                                              user_data_s,
                                              static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.address,
                                              false, // wait ?
                                              true); // peer address ?
    } // end IF
    else
    {
#if defined (SSL_SUPPORT)
      // *TODO*: add SSL support to the proactor framework
      ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
      data_p->videoHandle =
          Net_Client_Common_Tools::connect (asynch_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    } // end ELSE
    if (data_p->videoHandle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.address).c_str ())));
      goto error;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: opened socket to %s\n"),
                data_p->videoHandle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.address).c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: opened socket to %s\n"),
                data_p->videoHandle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2b).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

    // step3: start progress reporting
//    spinner_p =
//      GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
//                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
//    ACE_ASSERT (spinner_p);
//    gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), TRUE);
//    gtk_spinner_start (spinner_p);
    progressbar_p =
        GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), TRUE);

    //ACE_ASSERT (!data_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      data_p->progressData.eventSourceId =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &data_p->progressData,
        //                 NULL);
        g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                       COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                       idle_update_progress_cb,
                       &data_p->progressData);//,
//                       NULL);
      if (data_p->progressData.eventSourceId > 0)
        state_r.eventSourceIds.insert (data_p->progressData.eventSourceId);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", aborting\n")));
        goto error;
      } // end IF
    } // end lock scope

    return;
  } // end IF

  // --> disconnect

  if (likely (data_p->audioTimeoutHandler->timerId_))
  {
    int result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (data_p->audioTimeoutHandler->timerId_,
                                                                NULL);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel interval timer (id: %d): \"%m\", continuing\n"),
                  data_p->audioTimeoutHandler->timerId_));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled interval timer (id: %d)\n"),
                  data_p->audioTimeoutHandler->timerId_));
    data_p->audioTimeoutHandler->timerId_ = 0;
  } // end IF
  if (likely (data_p->videoTimeoutHandler->timerId_))
  {
    int result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (data_p->videoTimeoutHandler->timerId_,
                                                                NULL);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel interval timer (id: %d): \"%m\", continuing\n"),
                  data_p->videoTimeoutHandler->timerId_));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("cancelled interval timer (id: %d)\n"),
                  data_p->videoTimeoutHandler->timerId_));
    data_p->videoTimeoutHandler->timerId_ = 0;
  } // end IF

  iconnection_manager_p->abort ();
  iconnection_manager_2->abort ();
  data_p->audioHandle = ACE_INVALID_HANDLE;
  data_p->videoHandle = ACE_INVALID_HANDLE;

  data_p->dispatch = NULL;
  data_p->AudioStream->stop (false, // wait for completion ?
                             false, // recurse ?
                             true); // high priority
  data_p->AVStream->stop (false, // wait for completion ?
                          false, // recurse ?
                          true); // high priority

  return;

error:
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_MEDIA_PLAY);
//  box_p =
//    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
//                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_VBOX_CONFIGURATION_NAME)));
//  ACE_ASSERT (box_p);
//  gtk_widget_set_sensitive (GTK_WIDGET (box_p), TRUE);

  un_toggling_play = true;
  gtk_toggle_button_set_active (toggleButton_in, FALSE);
} // toggle_button_connect_toggled_cb

gint
button_load_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_load_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator;
  ACE_ASSERT (data_p->channels);
  if (!data_p->currentChannel) // no channel selected
    return G_SOURCE_REMOVE;
  channel_iterator = data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  Test_I_WebTV_StreamConfiguration_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
  Test_I_WebTV_StreamConfiguration_t::ITERATOR_T iterator_4 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR ("Marshal"));
  ACE_ASSERT (iterator_4 != data_p->configuration->streamConfiguration.end ());

  data_p->configuration->parserConfiguration.messageQueue = NULL;
  (*iterator_4).second.second->parserConfiguration->messageQueue = NULL;

  gtk_widget_set_sensitive (widget_in, FALSE);

  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string, URL_string;
  bool use_SSL = false;
  if (!HTTP_Tools::parseURL ((*channel_iterator).second.mainURL,
                             host_address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
               ACE_TEXT ((*channel_iterator).second.mainURL.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  (*iterator_3).second.second->URL = (*channel_iterator).second.mainURL;
  Net_ConnectionConfigurationsIterator_t iterator_2 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address =
      host_address;
  static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.hostname =
      hostname_string;

  // select connector
  Test_I_TCPConnector_t connector (true);
#if defined (SSL_SUPPORT)
  Test_I_SSLConnector_t ssl_connector (true);
#endif // SSL_SUPPORT
  Test_I_AsynchTCPConnector_t asynch_connector (true);
  struct Net_UserData user_data_s;
//  ACE_ASSERT (!data_p->handle)
  if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      data_p->videoHandle =
          Net_Client_Common_Tools::connect (ssl_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    else
#endif // SSL_SUPPORT
      data_p->videoHandle =
          Net_Client_Common_Tools::connect (connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    data_p->videoHandle =
        Net_Client_Common_Tools::connect (asynch_connector,
                                          *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                          user_data_s,
                                          static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                          false, // wait ?
                                          true); // peer address ?
  } // end ELSE
//    iconnection_p =
//        iconnection_manager_p->get (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
//                                    true);
  if (data_p->videoHandle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to connect to %s, aborting\n"),
               ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: opened TCP socket to %s\n"),
              data_p->videoHandle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: opened TCP socket to %s\n"),
              data_p->videoHandle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (!data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, FALSE);
    data_p->progressData.eventSourceId =
                                         //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                                         //                 idle_update_progress_cb,
                                         //                 &data_p->progressData,
                                         //                 NULL);
        g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                       COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                       idle_update_progress_cb,
                       &data_p->progressData);//,
               //                       NULL);
    ACE_ASSERT (data_p->progressData.eventSourceId > 0);
    state_r.eventSourceIds.insert (data_p->progressData.eventSourceId);
  } // end lock scope

  return FALSE;
} // button_load_clicked_cb

void
combobox_channel_changed_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_channel_changed_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTreeIter iterator_2;
  gboolean result = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                                   &iterator_2);
  ACE_ASSERT (result);
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_CHANNEL_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION (2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                           &iterator_2,
                           1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  data_p->currentChannel = g_value_get_uint (&value);
  g_value_unset (&value);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_LOAD_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), FALSE);

  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_AUDIOCHANNEL_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_list_store_clear (list_store_p);
  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_list_store_clear (list_store_p);
  data_p->currentAudioStream = 0;
  data_p->currentVideoStream = 0;

  // auto-load ?
  if (data_p->nextChannel != -1)
  { ACE_ASSERT (data_p->nextChannel == static_cast<int> (data_p->currentChannel));
    gtk_button_clicked (button_p);
  } // end IF
} // combobox_channel_changed_cb

void
combobox_resolution_changed_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T iterator_4b =
      data_p->configuration->streamConfiguration_4b.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_4b != data_p->configuration->streamConfiguration_4b.end ());
  ACE_ASSERT ((*iterator_4b).second.second->delayConfiguration);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator;
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  channel_iterator = data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());

  GtkTreeIter iterator_2;
  gboolean result = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                                   &iterator_2);
  if (result == FALSE)
    return; // changed channel ?
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION (2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  Common_Image_Resolution_t resolution_s;
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  resolution_s.cx = g_value_get_uint (&value);
#else
  resolution_s.width = g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  resolution_s.cy = g_value_get_uint (&value);
#else
  resolution_s.height = g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  data_p->currentVideoStream = resolution_s.cx;
#else
  data_p->currentVideoStream = resolution_s.width;
#endif // ACE_WIN32 || ACE_WIN64

  // set corresponding video framerate
  for (Test_I_WebTV_Channel_ResolutionsConstIterator_t iterator_3 = (*channel_iterator).second.resolutions.begin ();
       iterator_3 != (*channel_iterator).second.resolutions.end ();
       ++iterator_3)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((*iterator_3).resolution.cx == resolution_s.cx)
#else
    if ((*iterator_3).resolution.width == resolution_s.width)
#endif // ACE_WIN32 || ACE_WIN64
    {
      (*iterator_4b).second.second->delayConfiguration->averageTokensPerInterval =
          (*iterator_3).frameRate;
      data_p->configuration->streamConfiguration_4b.configuration_->mediaType.video.frameRate.num =
          (*iterator_3).frameRate;
      break;
    } // end IF
} // combobox_resolution_changed_cb

void
combobox_display_changed_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_display_changed_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T iterator_4b =
    data_p->configuration->streamConfiguration_4b.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_4b != data_p->configuration->streamConfiguration_4b.end ());

  GtkTreeIter iterator_2;
  gboolean result = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                                   &iterator_2);
  ACE_ASSERT (result);
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION (2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  (*iterator_4b).second.second->display =
    Common_UI_Tools::getDisplay (g_value_get_string (&value));
  g_value_unset (&value);

  // select corresponding adapter
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME)));
  ACE_ASSERT (combo_box_p);
  struct Common_UI_DisplayAdapter display_adapter_s =
    Common_UI_Tools::getAdapter ((*iterator_4b).second.second->display);
  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value,
                      display_adapter_s.description.c_str ());
  Common_UI_GTK_Tools::selectValue (combo_box_p,
                                    value,
                                    0);
}

void
togglebutton_display_toggled_cb (GtkToggleButton* toggleButton_in,
                                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_display_toggled_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_DISPLAY_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
} // togglebutton_display_toggled_cb

void
togglebutton_fullscreen_toggled_cb (GtkToggleButton* toggleButton_in,
                                    gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_fullscreen_toggled_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_WebTV_StreamConfiguration_3_t::ITERATOR_T iterator_4b =
      data_p->configuration->streamConfiguration_4b.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_4b != data_p->configuration->streamConfiguration_4b.end ());
  ACE_ASSERT (data_p->AVStream);
  if (!data_p->AVStream->isRunning ())
    return;

  GtkDrawingArea* drawing_area_p =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  GtkDrawingArea* drawing_area_2 =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_FULLSCREEN_NAME)));
  ACE_ASSERT (drawing_area_2);

  GtkWindow* window_p =
      GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (window_p);
  GtkWindow* window_2 =
      GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_WINDOW_FULLSCREEN_NAME)));
  ACE_ASSERT (window_2);

  const Stream_Module_t* module_p = NULL;
  module_p =
    data_p->AVStream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("%s: failed to Stream_IStream::find(\"Display\"), returning\n"),
               ACE_TEXT (data_p->AVStream->name ().c_str ())));
    return;
  } // end IF

  if (gtk_toggle_button_get_active (toggleButton_in))
  {
    gtk_widget_show (GTK_WIDGET (window_2));
    gtk_window_maximize (window_2);
    //  gtk_window_fullscreen (window_2);

    gtk_window_iconify (window_p);

    (*iterator_4b).second.second->window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_2));
  } // end IF
  else
  {
    //  gtk_window_unfullscreen (window_2);
    gtk_window_unmaximize (window_2);
    gtk_widget_hide (GTK_WIDGET (window_2));

    gtk_window_deiconify (window_p);

    (*iterator_4b).second.second->window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));

    g_signal_emit_by_name (G_OBJECT (drawing_area_p),
                           ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                           userData_in);

    // drop frames until resize signal AND session message have been processed
    Stream_Visualization_IResize* iresize_p =
        dynamic_cast<Stream_Visualization_IResize*> (const_cast<Stream_Module_t*> (module_p)->writer ());
    ACE_ASSERT (iresize_p);
    try {
      iresize_p->resizing ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("caught exception in Stream_Visualization_IResize::resizing(), returning\n")));
      return;
    }
  } // end ELSE
  ACE_ASSERT ((*iterator_4b).second.second->window);

  Common_UI_IFullscreen* ifullscreen_p =
      dynamic_cast<Common_UI_IFullscreen*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!ifullscreen_p)
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("%s:%s: failed to dynamic_cast<Common_UI_IFullscreen*>(0x%@), returning\n"),
               ACE_TEXT (data_p->AVStream->name ().c_str ()),
               module_p->name (),
               const_cast<Stream_Module_t*> (module_p)->writer ()));
    return;
  } // end IF
  try {
    ifullscreen_p->toggle ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("caught exception in Common_UI_IFullscreen::toggle(), returning\n")));
    return;
  }
} // togglebutton_fullscreen_toggled_cb

void
togglebutton_save_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_save_toggled_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_SAVE_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
} // togglebutton_save_toggled_cb

gboolean
key_cb (GtkWidget* widget_in,
        GdkEventKey* eventKey_in,
        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::key_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (eventKey_in);
  struct Test_I_WebTV_UI_CBData* data_p =
      reinterpret_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  switch (eventKey_in->keyval)
  {
#if GTK_CHECK_VERSION (3,0,0)
    case GDK_KEY_Escape:
    case GDK_KEY_f:
    case GDK_KEY_F:
#else
    case GDK_Escape:
    case GDK_f:
    case GDK_F:
#endif // GTK_CHECK_VERSION(3,0,0)
    {
      bool is_active_b = false;
      GtkToggleButton* toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
      ACE_ASSERT (toggle_button_p);
      is_active_b = gtk_toggle_button_get_active (toggle_button_p);

     // sanity check(s)
#if GTK_CHECK_VERSION(3,0,0)
      if ((eventKey_in->keyval == GDK_KEY_Escape) &&
#else
      if ((eventKey_in->keyval == GDK_Escape) &&
#endif // GTK_CHECK_VERSION(3,0,0)
          !is_active_b)
        break; // <-- not in fullscreen mode, nothing to do

      gtk_toggle_button_set_active (toggle_button_p,
                                   !is_active_b);

      break;
    }
#if GTK_CHECK_VERSION (3,0,0)
    case GDK_KEY_1:
    case GDK_KEY_2:
    case GDK_KEY_3:
    case GDK_KEY_4:
    case GDK_KEY_5:
    case GDK_KEY_6:
    case GDK_KEY_7:
    case GDK_KEY_8:
    case GDK_KEY_9:
    case GDK_KEY_0:
#else
    case GDK_1:
    case GDK_2:
    case GDK_3:
    case GDK_4:
    case GDK_5:
    case GDK_6:
    case GDK_7:
    case GDK_8:
    case GDK_9:
    case GDK_0:
#endif // GTK_CHECK_VERSION (3,0,0)
    {
      // sanity check(s)
      if (data_p->nextChannel != -1)
        return TRUE; // done (do not propagate further)
#if GTK_CHECK_VERSION (3,0,0)
      if (eventKey_in->keyval == GDK_KEY_0)
        data_p->nextChannel = 10;
      else
        data_p->nextChannel = eventKey_in->keyval - GDK_KEY_0;
#else
      if (eventKey_in->keyval == GDK_0)
        data_p->nextChannel = 10;
      else
        data_p->nextChannel = eventKey_in->keyval - GDK_0;
#endif // GTK_CHECK_VERSION (3,0,0)
      // sanity check(s)
      if (data_p->nextChannel == static_cast<int> (data_p->currentChannel))
      {
        data_p->nextChannel = -1; // reset
        return TRUE; // done (do not propagate further)
      } // end IF

      GtkToggleButton* toggle_button_p =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
      ACE_ASSERT (toggle_button_p);
      if (gtk_toggle_button_get_active (toggle_button_p))
      {
        guint event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT, // same as timeout !
                                                 idle_end_session_3,
                                                 userData_in,
                                                 NULL);
        if (unlikely (event_source_id == 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to g_idle_add_full(idle_end_session_3): ""\"%m\", returning\n")));
          return G_SOURCE_REMOVE;
        } // end IF
        { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, TRUE);
          state_r.eventSourceIds.insert (event_source_id);
        } // end lock scope
        return TRUE; // done (do not propagate further)
      } // end IF

#if GTK_CHECK_VERSION (2,30,0)
      struct _GValue value = G_VALUE_INIT;
#else
      struct _GValue value;
      ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
      g_value_init (&value, G_TYPE_UINT);
      g_value_set_uint (&value,
                        data_p->nextChannel);
      GtkComboBox* combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME)));
      ACE_ASSERT (combo_box_p);
      Common_UI_GTK_Tools::selectValue (combo_box_p,
                                        value,
                                        1);
      g_value_unset (&value);

      break;
    }
    default:
      return FALSE; // propagate
  } // end SWITCH

  return TRUE; // done (do not propagate further)
}

gboolean
dialog_main_key_press_event_cb (GtkWidget* widget_in,
                                GdkEventKey* eventKey_in,
                                gpointer userData_in)
{
  return key_cb (widget_in,
                 eventKey_in,
                 userData_in);
}

gboolean
window_fullscreen_key_press_event_cb (GtkWidget* widget_in,
                                      GdkEventKey* eventKey_in,
                                      gpointer userData_in)
{
  return key_cb (widget_in,
                 eventKey_in,
                 userData_in);
}

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
    return TRUE; // propagate
  } // end IF

  // run dialog
  gint result = gtk_dialog_run (about_dialog);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  gtk_widget_hide (GTK_WIDGET (about_dialog));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // step1: remove all event sources
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, state_r.lock, FALSE);
    for (Common_UI_GTK_EventSourceIdsIterator_t iterator = state_r.eventSourceIds.begin ();
         iterator != state_r.eventSourceIds.end ();
         iterator++)
      if (!g_source_remove (*iterator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    *iterator));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("removed event source (id: %u)\n"),
                    *iterator));
    state_r.eventSourceIds.clear ();
  } // end lock scope

  // step2: initiate shutdown sequence
  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int signal = SIGINT;
#else
  int signal = SIGQUIT;
#endif // ACE_WIN32 || ACE_WIN64
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  return FALSE;
} // button_quit_clicked_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
