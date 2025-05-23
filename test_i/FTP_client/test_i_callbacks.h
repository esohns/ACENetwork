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

#include "gtk/gtk.h"

bool load_network_interfaces (GtkListStore*);

//------------------------------------------------------------------------------

// idle routines
gboolean idle_response_received_cb (gpointer);
gboolean idle_list_received_cb (gpointer);
gboolean idle_data_received_cb (gpointer);
gboolean idle_login_complete_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);

/////////////////////////////////////////

gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);

gboolean idle_update_info_display_cb (gpointer);
gboolean idle_update_log_display_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// callbacks
G_MODULE_EXPORT void action_connect_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void combobox_interface_changed_cb (GtkComboBox*, gpointer);
G_MODULE_EXPORT void spinbutton_server_port_value_changed_cb (GtkSpinButton*, gpointer);
G_MODULE_EXPORT void toggleaction_listen_toggled_cb (GtkToggleAction*, gpointer);
G_MODULE_EXPORT void togglebutton_mode_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void treeview_selection_directories_changed_cb (GtkTreeSelection*, gpointer);
G_MODULE_EXPORT void treeview_selection_files_changed_cb (GtkTreeSelection*, gpointer);

/////////////////////////////////////////

G_MODULE_EXPORT void button_clear_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_about_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_quit_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void textview_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
