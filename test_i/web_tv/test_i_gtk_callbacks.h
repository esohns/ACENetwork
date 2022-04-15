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

//// thread functions
//ACE_THR_FUNC_RETURN stream_processing_function (void*);

//------------------------------------------------------------------------------

// idle routines
gboolean idle_end_session_cb (gpointer);
gboolean idle_load_channel_configuration_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);
gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_start_session_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);
gboolean idle_update_info_display_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// callbacks
G_MODULE_EXPORT gint button_load_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void togglebutton_play_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void combobox_channel_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_resolution_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void togglebutton_save_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void filechooserbutton_output_directory_set_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void togglebutton_display_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void combobox_display_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void togglebutton_fullscreen_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void drawingarea_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
G_MODULE_EXPORT gint button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_quit_clicked_cb (GtkWidget*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
