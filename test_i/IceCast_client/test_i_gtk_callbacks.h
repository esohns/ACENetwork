/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef TEST_I_GTK_CALLBACKS_H
#define TEST_I_GTK_CALLBACKS_H

#include "gtk/gtk.h"

#include <string>

//------------------------------------------------------------------------------

ACE_UINT8 URLToChannelId (const std::string&);
std::string channelIdToURL (ACE_UINT8);

//------------------------------------------------------------------------------

// idle routines
gboolean idle_end_session_cb (gpointer);
gboolean idle_load_segment_cb (gpointer);
gboolean idle_stream_uris_received_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);
gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_reset_UI_cb (gpointer);
gboolean idle_start_session_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);
gboolean idle_update_info_display_cb (gpointer);
gboolean idle_update_display_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// callbacks
G_MODULE_EXPORT gboolean dialog_main_key_press_event_cb (GtkWidget*, GdkEventKey*, gpointer);
G_MODULE_EXPORT void togglebutton_connect_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void button_cut_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_scrape_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void combobox_streams_changed_cb (GtkComboBox*, gpointer);
G_MODULE_EXPORT void entry_url_activate_cb (GtkEntry*, gpointer);
G_MODULE_EXPORT void scale_volume_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void scale_beat_sensitivity_value_changed_cb (GtkRange*, gpointer);
  //G_MODULE_EXPORT void filechooserbutton_output_file_set_cb (GtkWidget*, gpointer);
//G_MODULE_EXPORT void spinbutton_buffer_value_changed_cb (GtkWidget*, gpointer);
//G_MODULE_EXPORT void checkbutton_save_toggled_cb (GtkCheckButton*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_query_tooltip_cb (GtkWidget*, gint, gint, gboolean, GtkTooltip*, gpointer);
G_MODULE_EXPORT void drawingarea_realize_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void drawingarea_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
#if GTK_CHECK_VERSION (3,0,0)
G_MODULE_EXPORT gboolean drawingarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_draw_cb (GtkWidget*, cairo_t*, gpointer);
#else
G_MODULE_EXPORT gboolean drawingarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_projectm_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#endif // GTK_CHECK_VERSION (3,0,0)
G_MODULE_EXPORT gint button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_quit_clicked_cb (GtkWidget*, gpointer);

// OpenGL
G_MODULE_EXPORT gboolean glarea_key_press_cb (GtkWidget*, GdkEventKey, gpointer);
G_MODULE_EXPORT gboolean glarea_clicked_cb (GtkWidget*, GdkEventButton, gpointer);
G_MODULE_EXPORT void glarea_realize_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void glarea_unrealize_cb (GtkWidget*, gpointer);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
G_MODULE_EXPORT GdkGLContext* glarea_create_context_cb (GtkGLArea*, gpointer);
G_MODULE_EXPORT gboolean glarea_render_cb (GtkGLArea*, GdkGLContext*, gpointer);
G_MODULE_EXPORT void glarea_resize_cb (GtkGLArea*, gint, gint, gpointer);
#else
#if defined (GTKGLAREA_SUPPORT)
G_MODULE_EXPORT void glarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean glarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#else
G_MODULE_EXPORT void glarea_size_allocate_event_cb (GtkWidget*, GdkRectangle*, gpointer);
G_MODULE_EXPORT gboolean glarea_draw_cb (GtkWidget*, cairo_t*, gpointer);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
G_MODULE_EXPORT void glarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean glarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#else
G_MODULE_EXPORT void glarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean glarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
