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

#ifndef TEST_I_CALLBACKS_H
#define TEST_I_CALLBACKS_H

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "guiddef.h"
#undef GetObject
#include "mfidl.h"
#else
#include "linux/videodev2.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "gtk/gtk.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_common.h"
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IAMStreamConfig;
#endif // ACE_WIN32 || ACE_WIN64

// helper functions
#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool load_capture_devices (enum Stream_MediaFramework_Type,
                           GtkListStore*);
#else
bool load_capture_devices (GtkListStore*);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool load_formats (IAMStreamConfig*, // device configuration handle
                   GtkListStore*);   // return value: supported media subtypes
//bool load_formats (IMFSourceReader*, // source reader handle
bool load_formats (IMFMediaSource*, // source handle
                   GtkListStore*);  // return value: supported media subtypes
bool load_resolutions (IAMStreamConfig*, // device configuration handle
                       REFGUID,          // media subtype
                       GtkListStore*);   // return value: supported resolutions
//bool load_resolutions (IMFSourceReader*,    // source reader handle
bool load_resolutions (IMFMediaSource*, // source handle
                       REFGUID,         // media subtype
                       GtkListStore*);  // return value: supported resolutions
bool load_rates (IAMStreamConfig*, // device configuration handle
                 REFGUID,          // media subtype
                 unsigned int,     // resolution (width)
                 GtkListStore*);   // return value: supported rates
//bool load_rates (IMFSourceReader*,    // source reader handle
bool load_rates (IMFMediaSource*, // source handle
                 REFGUID,         // media subtype
                 unsigned int,    // resolution (width)
                 GtkListStore*);  // return value: supported rates
#else
int dirent_selector (const dirent*);
int dirent_comparator (const dirent**,
                       const dirent**);
bool load_formats (int,            // (capture) device file descriptor
                   GtkListStore*); // return value: supported formats (fourcc)
bool load_resolutions (int,            // (capture) device file descriptor
                       __u32,          // format (fourcc)
                       GtkListStore*); // return value: supported resolutions
bool load_rates (int,            // (capture) device file descriptor
                 __u32,          // format (fourcc)
                 unsigned int,   // resolution (width)
                 unsigned int,   // resolution (height)
                 GtkListStore*); // return value: supported rates
#endif // ACE_WIN32 || ACE_WIN64

// thread functions
ACE_THR_FUNC_RETURN stream_processing_function (void*);

//------------------------------------------------------------------------------

// idle routines
gboolean idle_finalize_source_UI_cb (gpointer);
gboolean idle_initialize_source_UI_cb (gpointer);
gboolean idle_end_source_UI_cb (gpointer);
gboolean idle_update_progress_source_cb (gpointer);

/////////////////////////////////////////

gboolean idle_finalize_target_UI_cb (gpointer);
gboolean idle_initialize_target_UI_cb (gpointer);
gboolean idle_start_target_UI_cb (gpointer);
gboolean idle_end_target_UI_cb (gpointer);
gboolean idle_reset_target_UI_cb (gpointer);
gboolean idle_update_progress_target_cb (gpointer);

/////////////////////////////////////////

gboolean idle_update_info_display_cb (gpointer);
gboolean idle_update_log_display_cb (gpointer);
gboolean idle_update_video_display_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// callbacks
G_MODULE_EXPORT void action_reset_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_settings_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void combobox_source_changed_cb (GtkComboBox*, gpointer);
G_MODULE_EXPORT void combobox_format_changed_cb (GtkComboBox*, gpointer);
G_MODULE_EXPORT void combobox_resolution_changed_cb (GtkComboBox*, gpointer);
G_MODULE_EXPORT void combobox_rate_changed_cb (GtkComboBox*, gpointer);
#if GTK_CHECK_VERSION (3,0,0)
  G_MODULE_EXPORT void drawingarea_size_allocate_source_cb (GtkWidget*, GdkRectangle*, gpointer);
#else
  G_MODULE_EXPORT gboolean drawingarea_configure_event_source_cb (GtkWidget*, GdkEvent*, gpointer);
#endif
G_MODULE_EXPORT void filechooserbutton_source_cb (GtkFileChooserButton*, gpointer);
G_MODULE_EXPORT void toggleaction_stream_toggled_cb (GtkToggleAction*, gpointer);

/////////////////////////////////////////

G_MODULE_EXPORT void action_close_all_activate_cb (GtkAction*, gpointer);
#if GTK_CHECK_VERSION (3,0,0)
  G_MODULE_EXPORT void drawingarea_size_allocate_target_cb (GtkWidget*, GdkRectangle*, gpointer);
#else
  G_MODULE_EXPORT gboolean drawingarea_configure_event_target_cb (GtkWidget*, GdkEvent*, gpointer);
#endif
G_MODULE_EXPORT void filechooserbutton_target_cb (GtkFileChooserButton*, gpointer);
G_MODULE_EXPORT void filechooserdialog_target_cb (GtkFileChooser*, gpointer);
G_MODULE_EXPORT void filechooser_target_cb (GtkFileChooser*, gpointer);
G_MODULE_EXPORT void toggleaction_listen_activate_cb (GtkToggleAction*, gpointer);

/////////////////////////////////////////

G_MODULE_EXPORT void action_report_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_draw_cb (GtkWidget*, cairo_t*, gpointer);
//G_MODULE_EXPORT void radiobutton_protocol_toggled_cb (GtkToggleButton*, gpointer);
//G_MODULE_EXPORT void spinbutton_port_value_changed_cb (GtkWidget*, gpointer);

/////////////////////////////////////////

G_MODULE_EXPORT gint button_clear_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_quit_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void textview_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
