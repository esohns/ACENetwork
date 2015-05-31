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

#ifndef IRC_CLIENT_GUI_CALLBACKS_H
#define IRC_CLIENT_GUI_CALLBACKS_H

#include "gtk/gtk.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void is_entry_sensitive (GtkCellLayout*,
                         GtkCellRenderer*,
                         GtkTreeModel*,
                         GtkTreeIter*,
                         gpointer);
gboolean update_display (gpointer);

//------------------------------------------------------------------------------
G_MODULE_EXPORT gboolean idle_initialize_UI_cb (gpointer);
G_MODULE_EXPORT gboolean idle_finalize_UI_cb (gpointer);
// main
G_MODULE_EXPORT void connect_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void main_send_entry_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gboolean send_entry_kb_focused_cb (GtkWidget*, GdkEventFocus*, gpointer);
G_MODULE_EXPORT void send_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint quit_activated_cb (GtkWidget*, GdkEvent*, gpointer);

// IRC_Client_GUI_Connection
G_MODULE_EXPORT void disconnect_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gboolean nick_entry_kb_focused_cb (GtkWidget*, GdkEventFocus*, gpointer);
G_MODULE_EXPORT void change_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void usersbox_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void refresh_users_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gboolean channel_entry_kb_focused_cb (GtkWidget*, GdkEventFocus*, gpointer);
G_MODULE_EXPORT void join_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void channelbox_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void refresh_channels_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void user_mode_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void switch_channel_cb (GtkNotebook*, GtkNotebookPage*, guint, gpointer);
G_MODULE_EXPORT void action_away_cb (GtkAction*, gpointer);

// IRC_Client_GUI_MessageHandler
G_MODULE_EXPORT void channel_mode_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void topic_clicked_cb (GtkWidget*, GdkEventButton*, gpointer);
G_MODULE_EXPORT void part_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gboolean members_clicked_cb (GtkWidget*, GdkEventButton*, gpointer);
G_MODULE_EXPORT void action_msg_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_invite_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_info_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_kick_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_ban_cb (GtkAction*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
