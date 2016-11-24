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

#ifndef BITTORRENT_CLIENT_GUI_CALLBACKS_H
#define BITTORRENT_CLIENT_GUI_CALLBACKS_H

#include <ace/config-macros.h>

#include <gtk/gtk.h>

ACE_THR_FUNC_RETURN session_setup_function (void*);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// idle routines
gboolean idle_add_session_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);
gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_remove_session_cb (gpointer);
gboolean idle_update_display_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);

//------------------------------------------------------------------------------

// callbacks
// main
G_MODULE_EXPORT void button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_connect_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_disconnect_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_quit_clicked_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT void switch_session_cb (GtkNotebook*, GtkWidget*, guint, gpointer);

// BitTorrent_Client_GUI_Session
G_MODULE_EXPORT void button_session_close_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_connection_close_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_connections_changed_cb (GtkWidget*, gpointer);

// BitTorrent_Client_GUI_Connection

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
