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

#ifndef BITTORRENT_CLIENT_GUI_DEFINES_H
#define BITTORRENT_CLIENT_GUI_DEFINES_H

// glade/GTKBuilder definitions
// files
#define BITTORRENT_CLIENT_GUI_GTK_UI_MAIN_FILE                          "bittorrent_client_main.glade"
#define BITTORRENT_CLIENT_GUI_GTK_UI_SESSION_FILE                       "bittorrent_client_session.glade"

// GTK .rc files
#define BITTORRENT_CLIENT_GUI_GTK_UI_RC_FILE                            "bittorrent_client.rc"

// GTK widget names
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_ABOUT                          "button_about"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_CONNECT                        "button_connect"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_CONNECTION_CLOSE               "button_page_session_connection_close"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_DISCONNECT                     "button_close"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_TAB_SESSION_CLOSE              "button_tab_session_close"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_QUIT                           "button_quit"
#define BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CONNECTIONS                  "combobox_page_session_connections"
#define BITTORRENT_CLIENT_GUI_GTK_DIALOG_MAIN_ABOUT                     "dialog_main_about"
#define BITTORRENT_CLIENT_GUI_GTK_DIALOG_MAIN_MESSAGE                   "dialog_main_message"
#define BITTORRENT_CLIENT_GUI_GTK_FILECHOOSERBUTTON_DESTINATION         "filechooserbutton_destination"
#define BITTORRENT_CLIENT_GUI_GTK_FILECHOOSERBUTTON_TORRENT             "filechooserbutton_torrent"
#define BITTORRENT_CLIENT_GUI_GTK_FRAME_CHANNELMODE                     "frame_page_channel_mode"
#define BITTORRENT_CLIENT_GUI_GTK_HBOX_TAB_SESSION                      "hbox_tab_session"
#define BITTORRENT_CLIENT_GUI_GTK_LABEL_TAB_SESSION                     "label_tab_session"
#define BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_CONNECTIONS                 "liststore_page_connections"
#define BITTORRENT_CLIENT_GUI_GTK_MENU_CHANNEL                          "menu_page_channel"
#define BITTORRENT_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE                   "menu_page_channel_invite"
#define BITTORRENT_CLIENT_GUI_GTK_MENUITEM_BAN                          "menuitem_ban"
#define BITTORRENT_CLIENT_GUI_GTK_MENUITEM_INVITE                       "menuitem_invite"
#define BITTORRENT_CLIENT_GUI_GTK_MENUITEM_KICK                         "menuitem_kick"
#define BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_SESSIONS                     "notebook_sessions"
#define BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR                           "progressbar"
#define BITTORRENT_CLIENT_GUI_GTK_SCROLLEDWINDOW_SESSION                "scrolledwindow_page_session_log"
#define BITTORRENT_CLIENT_GUI_GTK_STATUSBAR                             "statusbar"
#define BITTORRENT_CLIENT_GUI_GTK_TEXTVIEW_SESSION                      "textview_page_session_log"
#define BITTORRENT_CLIENT_GUI_GTK_VBOX_SESSION                          "vbox_page_session"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_MAIN                           "window_main"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_SESSION                        "window_page_session"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_TAB_SESSION                    "window_tab_session"

// GTK cursor
// *TODO*: use GDK_BLANK_CURSOR instead ?
#define BITTORRENT_CLIENT_GUI_GTK_CURSOR_BUSY                           GDK_WATCH

// GTK progress/status-bar
#define BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR_UPDATE_INTERVAL           27 // ms (?)
#define BITTORRENT_CLIENT_GUI_GTK_STATUSBAR_CONTEXT_DESCRIPTION         "BitTorrent client::main"

// GTK log refresh rate
#define BITTORRENT_CLIENT_GUI_GTK_LOG_REFRESH_RATE                      2 // 1/s

#define BITTORRENT_CLIENT_GUI_GTK_LABEL_DEF_LOG_TEXT                    "server log"
#define BITTORRENT_CLIENT_GUI_GTK_LABEL_DEF_TOPIC_TEXT                  "no topic"

// dialog titles
#define BITTORRENT_CLIENT_GUI_DIALOG_ENTRY_TITLE_AWAY                   "please provide an away message..."
#define BITTORRENT_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_PASSWORD          "please provide a key..."
#define BITTORRENT_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_VOICE             "please specify a nickname..."
#define BITTORRENT_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_BAN               "set a ban mask..."
#define BITTORRENT_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_USERLIMIT         "set the channel user limit..."
#define BITTORRENT_CLIENT_GUI_DIALOG_ENTRY_TITLE_NICK                   "please provide a nickname..."
#define BITTORRENT_CLIENT_GUI_DIALOG_ENTRY_TITLE_TOPIC                  "set the channel topic..."

// message dialog text
#define BITTORRENT_CLIENT_GUI_MESSAGEDIALOG_TEXT_NICKNAMETAKEN          "the nickname you supplied is in use, please try again"

//#define BITTORRENT_CLIENT_GUI_DEF_SERVER_LABEL_TEXT  "no server"
//#define BITTORRENT_CLIENT_GUI_DEF_CHANNEL_LABEL_TEXT "no channel"

#endif
