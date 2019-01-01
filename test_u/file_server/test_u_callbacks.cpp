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

#include "test_u_callbacks.h"

#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "ace/Synch.h"
#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "test_u_connection_manager_common.h"
#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_stream.h"

#include "file_server_common.h"
#include "file_server_defines.h"

gboolean
idle_finalize_ui_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_ui_cb"));

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_log_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct FileServer_UI_CBData* data_p =
    static_cast<struct FileServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  GtkTextIter text_iterator;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iterator);

  gchar* converted_text = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    // sanity check
    if (data_p->UIState->logStack.empty ())
      return G_SOURCE_CONTINUE;

    // step1: convert text
    for (Common_MessageStackConstIterator_t iterator_2 = data_p->UIState->logStack.begin ();
         iterator_2 != data_p->UIState->logStack.end ();
         ++iterator_2)
    {
      converted_text = Common_UI_GTK_Tools::localeToUTF8 (*iterator_2);
      if (!converted_text)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"), aborting\n"),
                    ACE_TEXT ((*iterator_2).c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF

        // step2: display text
      gtk_text_buffer_insert (buffer_p,
                              &text_iterator,
                              converted_text,
                              -1);

      // clean up
      g_free (converted_text);
    } // end FOR

    data_p->UIState->logStack.clear ();
  } // end lock scope

    // step3: scroll the view accordingly
    //  // move the iterator to the beginning of line, so it doesn't scroll
    //  // in horizontal direction
    //  gtk_text_iter_set_line_offset (&text_iterator, 0);

    //  // ...and place the mark at iter. The mark will stay there after insertion
    //  // because it has "right" gravity
    //  GtkTextMark* text_mark_p =
    //      gtk_text_buffer_get_mark (buffer_p,
    //                                ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SCROLLMARK_NAME));
    ////  gtk_text_buffer_move_mark (buffer_p,
    ////                             text_mark_p,
    ////                             &text_iterator);

    //  // scroll the mark onscreen
    //  gtk_text_view_scroll_mark_onscreen (view_p,
    //                                      text_mark_p);
  GtkAdjustment* adjustment_p =
    GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_ADJUSTMENT_NAME)));
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p));

  return G_SOURCE_CONTINUE;
}

gboolean
idle_session_start_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_start_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct FileServer_UI_CBData* data_p =
    static_cast<struct FileServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);

  ACE_OS::memset (&data_p->progressData.statistic, 0, sizeof (Stream_Statistic));

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct FileServer_UI_CBData* data_p =
    static_cast<struct FileServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (data_p->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_CONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_spin (spin_button_p,
                                GTK_SPIN_STEP_FORWARD,
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;

          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_DISCONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_spin (spin_button_p,
                                GTK_SPIN_STEP_BACKWARD,
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;

          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
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
    while (!data_p->UIState->eventStack.is_empty ())
    {
      result = data_p->UIState->eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

gboolean
idle_initialize_ui_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_ui_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct FileServer_UI_CBData* data_p =
    static_cast<struct FileServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon = gtk_image_new_from_file (path.c_str());
  //  ACE_ASSERT (image_icon);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window_set_title(,
  //                     caption.c_str ());

  //  GtkWidget* about_dialog_p =
  //    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
  //                                        ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_DIALOG_ABOUT_NAME)));
  //  ACE_ASSERT (about_dialog_p);

  // step2: initialize info view
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<gdouble> (data_p->configuration->listenerConfiguration.connectionConfiguration->socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ()));

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_FILECHOOSERBUTTON_FILE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  //GError* error_p = NULL;
  //GFile* file_p = NULL;
  gchar* filename_p = NULL;
  FileServer_StreamConfiguration_t::ITERATOR_T iterator_2 =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->streamConfiguration.end ());
  if (!(*iterator_2).second.second.fileName.empty ())
  {
    // *NOTE*: gtk does not complain if the file doesn't exist, but the button
    //         will display "(None)"
    //file_p =
    //  g_file_new_for_path (data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ());
    //ACE_ASSERT (file_p);
    //ACE_ASSERT (g_file_query_exists (file_p, NULL));

    //std::string file_uri =
    //  ACE_TEXT_ALWAYS_CHAR ("file://") +
    //  data_p->configuration->moduleHandlerConfiguration.targetFileName;
    //if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
    //                                              file_uri.c_str ()))
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        (*iterator_2).second.second.fileName.c_str ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT ((*iterator_2).second.second.fileName.c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF

    //if (!gtk_file_chooser_select_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
    //                                   file_p,
    //                                   &error_p))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to gtk_file_chooser_select_file(\"%s\"): \"%s\", aborting\n"),
    //              ACE_TEXT (data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ()),
    //              ACE_TEXT (error_p->message)));

    //  // clean up
    //  g_error_free (error_p);
    //  g_object_unref (file_p);

    //  return G_SOURCE_REMOVE;
    //} // end IF
    //g_object_unref (file_p);
  } // end IF
  else
  {
    //file_p =
    //  g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
    //ACE_ASSERT (file_p);

    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        Common_File_Tools::getTempDirectory ().c_str ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF
    //g_object_unref (file_p);
  } // end ELSE

  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri += (*iterator_2).second.second.fileName;
  filename_p = Common_UI_GTK_Tools::localeToUTF8 (default_folder_uri);
  ACE_ASSERT (filename_p);
  gboolean result_2 =
    gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                             filename_p);
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (default_folder_uri.c_str ())));

    g_free (filename_p);

    return G_SOURCE_REMOVE;
  } // end IF
  g_free (filename_p);
  filename_p = NULL;

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  // step3: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  //  GtkTextBuffer* buffer_p =
  ////      gtk_text_buffer_new (NULL); // text tag table --> create new
  ////      gtk_text_view_set_buffer (view_p, buffer_p);
  //      gtk_text_view_get_buffer (view_p);
  //  ACE_ASSERT (buffer_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (TEST_U_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (FILE_SERVER_GTK_PANGO_LOG_FONT_DESCRIPTION)));
    return G_SOURCE_REMOVE;
  } // end IF
  // apply font
  GtkRcStyle* rc_style_p = gtk_rc_style_new ();
  if (!rc_style_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_rc_style_new(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  rc_style_p->font_desc = font_description_p;
  GdkColor base_colour, text_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags>(GTK_RC_BASE |
                            GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  // step4: initialize updates
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    guint event_source_id = g_timeout_add_seconds (1,
                                                   idle_update_log_display_cb,
                                                   userData_in);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
      // schedule asynchronous updates of the info view
    event_source_id =
        g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
                       idle_update_info_display_cb,
                       userData_in);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

    // step5: disable some functions ?
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            data_p->allowUserRuntimeStatistic);

  // step6: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6a: connect default signals
  gulong result =
    g_signal_connect (dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      &dialog_p);
  ACE_ASSERT (result);

  // step6b: connect custom signals
  //GObject* object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_LISTEN_NAME));
  //ACE_ASSERT (object_p);
  //result =
  //  g_signal_connect (object_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                    G_CALLBACK (togglebutton_listen_toggled_cb),
  //                    userData_in);
  //ACE_ASSERT (result);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //    ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_CLOSEALL_NAME));
  //ACE_ASSERT (object_p);
  //result =
  //  g_signal_connect (object_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("clicked"),
  //                    G_CALLBACK (button_close_all_clicked_cb),
  //                    userData_in);
  //ACE_ASSERT (result);

  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_REPORT_NAME));
  //ACE_ASSERT (object_p);
  //result =
  //  g_signal_connect (object_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("clicked"),
  //                    G_CALLBACK (button_report_clicked_cb),
  //                    userData_in);
  //ACE_ASSERT (result);

  // -------------------------------------

  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //    ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_CLEAR_NAME));
  //ACE_ASSERT (object_p);
  //result =
  //  g_signal_connect (object_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("clicked"),
  //                    G_CALLBACK (button_clear_clicked_cb),
  //                    userData_in);
  //ACE_ASSERT (result);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_ABOUT_NAME));
  //ACE_ASSERT (object_p);
  //result = g_signal_connect (object_p,
  //                           ACE_TEXT_ALWAYS_CHAR ("clicked"),
  //                           G_CALLBACK (button_about_clicked_cb),
  //                           userData_in);
  //ACE_ASSERT (result);
  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_QUIT_NAME));
  //ACE_ASSERT (object_p);
  //result =
  //  g_signal_connect (object_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("clicked"),
  //                    G_CALLBACK (button_quit_clicked_cb),
  //                    userData_in);
  //ACE_ASSERT (result);

  //   // step7: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step8: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step9: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    event_source_id = g_timeout_add_seconds (1,
                                             idle_update_log_display_cb,
                                             data_p);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
    // schedule asynchronous updates of the info view
    event_source_id =
        g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
                       idle_update_info_display_cb,
                       data_p);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step10: activate some widgets
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_BUTTON_LISTEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                true);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct Test_U_GTK_ProgressData* data_p =
    static_cast<struct Test_U_GTK_ProgressData*> (userData_in);
  ACE_ASSERT (data_p->state);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->state->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  // step1: done ?
  bool done = false;
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gint number_of_connections = gtk_spin_button_get_value_as_int (spin_button_p);
  if (number_of_connections == 0)
  {
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);
    done = true;
  } // end IF

  // step2: update progress bar text
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;
  float speed = data_p->statistic.bytesPerSecond;
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
    result = ACE_OS::sprintf (buffer, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : ACE_TEXT_ALWAYS_CHAR (buffer)));
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule ?
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
G_MODULE_EXPORT void
button_close_all_clicked_cb (GtkButton* button_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_close_all_clicked_cb"));

  ACE_UNUSED_ARG (button_in);
  ACE_UNUSED_ARG (userData_in);

  FILESERVER_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
} // button_close_all_clicked_cb

G_MODULE_EXPORT void
togglebutton_listen_toggled_cb (GtkToggleButton* toggleButton_in,
                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_listen_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct FileServer_UI_CBData* data_p =
    static_cast<struct FileServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->listener);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
  GtkFrame* frame_p = NULL;
  if (is_active)
  {
    // update configuration
    GtkSpinButton* spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_SPINBUTTON_PORT_NAME)));
    ACE_ASSERT (spin_button_p);
    data_p->configuration->listenerConfiguration.connectionConfiguration->socketHandlerConfiguration.socketConfiguration_2.address.set_port_number (static_cast<u_short> (gtk_spin_button_get_value_as_int (spin_button_p)),
                                                                                                                                                    1);

    GtkFileChooserButton* file_chooser_button_p =
      GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_FILECHOOSERBUTTON_FILE_NAME)));
    ACE_ASSERT (file_chooser_button_p);
    GFile* file_p =
      gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_button_p));
    if (!file_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_get_file(): \"%m\", returning\n")));
      return;
    } // end IF
    char* filename_p = g_file_get_path (file_p);
    if (!filename_p)
    {

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));

      // clean up
      g_object_unref (file_p);

      return;
    } // end IF
    g_object_unref (file_p);
    FileServer_StreamConfiguration_t::ITERATOR_T iterator_2 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_2 != data_p->configuration->streamConfiguration.end ());
    (*iterator_2).second.second.fileName =
        Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
    g_free (filename_p);

    if (!data_p->configuration->listener->initialize (data_p->configuration->listenerConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_U_Server_IListener::initialize(): \"%m\", returning\n")));
      return;
    } // end IF

    // update widgets
    frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

    ACE_thread_t thread_id = 0;
    try {
      data_p->configuration->listener->start (thread_id);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Test_U_Server_IListener::start(): \"%m\", aborting\n")));
      goto error;
    } // end catch
    ACE_UNUSED_ARG (thread_id);
  } // end IF
  else
  {
    frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

    try {
      data_p->configuration->listener->stop ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_Server_IListener::stop(): \"%m\", aborting\n")));
      goto error;
    } // end catch
  } // end IF

  return;

error:
  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), is_active);
} // togglebutton_listen_toggled_cb

G_MODULE_EXPORT void
button_report_clicked_cb (GtkButton* button_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_report_clicked_cb"));

  ACE_UNUSED_ARG (button_in);
  ACE_UNUSED_ARG (userData_in);

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
  int signal = 0;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  signal = SIGUSR1;
#else
  signal = SIGBREAK;
#endif
  if (ACE_OS::raise (signal) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));
}

G_MODULE_EXPORT void
button_clear_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  FileServer_UI_CBData* data_p =
    static_cast<FileServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p =
//    gtk_text_buffer_new (NULL); // text tag table --> create new
    gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_set_text (buffer_p,
                            ACE_TEXT_ALWAYS_CHAR (""), 0);
}

G_MODULE_EXPORT void
button_about_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  FileServer_UI_CBData* data_p =
    static_cast<FileServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_GTK_ABOUTDIALOG_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (FILE_SERVER_GTK_ABOUTDIALOG_NAME)));
    return;
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
} // button_about_clicked_cb

G_MODULE_EXPORT void
button_quit_clicked_cb (GtkButton* button_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (button_in);
  ACE_UNUSED_ARG (userData_in);

  int result = -1;

  //FileServer_UI_CBData* data_p = static_cast<FileServer_UI_CBData*> (userData_in);
  //// sanity check(s)
  //ACE_ASSERT (data_p);

  //// step1: remove event sources
  //{
  //  ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->UIState->lock);

  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->UIState->eventSourceIds.begin ();
  //       iterator != data_p->UIState->eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->UIState->eventSourceIds.clear ();
  //} // end lock scope

  // step2: initiate shutdown sequence
  result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance()->stop (false,  // wait ?
                                                     false); // N/A
} // button_quit_clicked_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
