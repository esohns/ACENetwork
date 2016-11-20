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

//// scripts
//#define BITTORRENT_CLIENT_GUI_GTK_SCRIPTS_DIRECTORY "scripts"

// phonebook
#define BITTORRENT_CLIENT_GUI_DEF_FILE_PHONEBOOK                        "bittorrent_client_phonebook.ini"

// glade/GTKBuilder definitions
// files
#define BITTORRENT_CLIENT_GUI_GTK_UI_CHANNEL_FILE                       "bittorrent_client_channel.glade"
#define BITTORRENT_CLIENT_GUI_GTK_UI_CONNECTION_FILE                    "bittorrent_client_connection.glade"
#define BITTORRENT_CLIENT_GUI_GTK_UI_MAIN_FILE                          "bittorrent_client_main.glade"

// GTK .rc files
#define BITTORRENT_CLIENT_GUI_GTK_UI_RC_FILE                            "IRC_client.rc"

// GTK widget names
#define BITTORRENT_CLIENT_GUI_GTK_ACTION_AWAY                           "action_away"
#define BITTORRENT_CLIENT_GUI_GTK_ACTION_BAN                            "action_ban"
#define BITTORRENT_CLIENT_GUI_GTK_ACTION_INFO                           "action_info"
#define BITTORRENT_CLIENT_GUI_GTK_ACTION_INVITE                         "action_invite"
#define BITTORRENT_CLIENT_GUI_GTK_ACTION_KICK                           "action_kick"
#define BITTORRENT_CLIENT_GUI_GTK_ACTION_MESSAGE                        "action_msg"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_ABOUT                          "button_about"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_CHANNELS_REFRESH               "button_page_connection_channels_refresh"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_CONNECT                        "button_connect"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_DISCONNECT                     "button_close"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_NICK_ACCEPT                    "button_page_connection_nick_accept"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_USERS_REFRESH                  "button_page_connection_users_refresh"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_JOIN                           "button_page_connection_channels_join"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_PART                           "button_close"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_SEND                           "button_send"
#define BITTORRENT_CLIENT_GUI_GTK_BUTTON_QUIT                           "button_quit"
#define BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CHANNELS                     "combobox_page_connection_channels"
#define BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_SERVERS                      "combobox_servers"
#define BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_USERS                        "combobox_page_connection_users"
#define BITTORRENT_CLIENT_GUI_GTK_DIALOG_CHANNEL_ENTRY                  "dialog_page_channel_entry"
#define BITTORRENT_CLIENT_GUI_GTK_DIALOG_CONNECTION_ENTRY               "dialog_page_connection_entry"
#define BITTORRENT_CLIENT_GUI_GTK_DIALOG_MAIN_ABOUT                     "dialog_main_about"
#define BITTORRENT_CLIENT_GUI_GTK_DIALOG_MAIN_ENTRY                     "dialog_main_entry"
#define BITTORRENT_CLIENT_GUI_GTK_DIALOG_MAIN_MESSAGE                   "dialog_main_message"
#define BITTORRENT_CLIENT_GUI_GTK_ENTRY_CHANNEL                         "entry_dialog_page_channel_entry"
#define BITTORRENT_CLIENT_GUI_GTK_ENTRY_CONNECTION                      "entry_dialog_page_connection_entry"
#define BITTORRENT_CLIENT_GUI_GTK_ENTRY_CONNECTION_CHANNEL              "entry_page_connection_channels"
#define BITTORRENT_CLIENT_GUI_GTK_ENTRY_MAIN                            "entry_dialog_entry_main"
#define BITTORRENT_CLIENT_GUI_GTK_ENTRY_NICK                            "entry_page_connection_nick"
#define BITTORRENT_CLIENT_GUI_GTK_ENTRY_SEND                            "entry_send"
#define BITTORRENT_CLIENT_GUI_GTK_EVENTBOX_TOPIC                        "eventbox_page_channel_topic"
#define BITTORRENT_CLIENT_GUI_GTK_FRAME_CHANNELMODE                     "frame_page_channel_mode"
#define BITTORRENT_CLIENT_GUI_GTK_HBOX_CHANNELMODE                      "hbox_page_channel_mode"
#define BITTORRENT_CLIENT_GUI_GTK_HBOX_CHANNEL_TAB                      "hbox_tab_channel"
#define BITTORRENT_CLIENT_GUI_GTK_HBOX_CONNECTION_TAB                   "hbox_tab_connection"
#define BITTORRENT_CLIENT_GUI_GTK_HBOX_NICK_CHANNEL                     "hbox_page_connection_nick_channel"
#define BITTORRENT_CLIENT_GUI_GTK_HBOX_SEND                             "hbox_send"
#define BITTORRENT_CLIENT_GUI_GTK_HBOX_USERMODE                         "hbox_page_connection_usermode"
#define BITTORRENT_CLIENT_GUI_GTK_LABEL_CHANNEL_TAB                     "label_tab_channel"
#define BITTORRENT_CLIENT_GUI_GTK_LABEL_CONNECTION_TAB                  "label_tab_connection"
#define BITTORRENT_CLIENT_GUI_GTK_LABEL_NICK                            "label_page_connection_nick"
#define BITTORRENT_CLIENT_GUI_GTK_LABEL_TOPIC                           "label_page_channel_topic"
#define BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_CHANNEL                     "liststore_page_channel"
#define BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_CHANNELS                    "liststore_page_connection_channels"
#define BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_USERS                       "liststore_page_connection_users"
#define BITTORRENT_CLIENT_GUI_GTK_MENU_CHANNEL                          "menu_page_channel"
#define BITTORRENT_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE                   "menu_page_channel_invite"
#define BITTORRENT_CLIENT_GUI_GTK_MENUITEM_BAN                          "menuitem_ban"
#define BITTORRENT_CLIENT_GUI_GTK_MENUITEM_INVITE                       "menuitem_invite"
#define BITTORRENT_CLIENT_GUI_GTK_MENUITEM_KICK                         "menuitem_kick"
#define BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS                     "notebook_page_connection_channels"
#define BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS                  "notebook_connections"
#define BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR                           "progressbar"
#define BITTORRENT_CLIENT_GUI_GTK_SCROLLEDWINDOW_CONNECTION             "scrolledwindow_page_connection_channels"
#define BITTORRENT_CLIENT_GUI_GTK_STATUSBAR                             "statusbar"
#define BITTORRENT_CLIENT_GUI_GTK_TEXTVIEW_CHANNEL                      "textview_page_channel"
#define BITTORRENT_CLIENT_GUI_GTK_TEXTVIEW_CONNECTION                   "textview_page_connection_channels"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_ANONYMOUS    "togglebutton_anonymous"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BAN          "togglebutton_ban"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BLOCKFOREIGN "togglebutton_blockforeign"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_INVITEONLY   "togglebutton_inviteonly"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_KEY          "togglebutton_key"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_MODERATED    "togglebutton_moderated"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_OPERATOR     "togglebutton_operator"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_PRIVATE      "togglebutton_private"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_QUIET        "togglebutton_quiet"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_REOP         "togglebutton_reop"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_RESTRICTOPIC "togglebutton_restricttopic"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_SECRET       "togglebutton_secret"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_USERLIMIT    "togglebutton_userlimit"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_VOICE        "togglebutton_voice"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_TOOLS_1                  "togglebutton_tool_1"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY            "togglebutton_away"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE       "togglebutton_invisible"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_LOCALOPERATOR   "togglebutton_localoperator"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES         "togglebutton_notices"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_OPERATOR        "togglebutton_operator"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_RESTRICTED      "togglebutton_restricted"
#define BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS         "togglebutton_wallops"
#define BITTORRENT_CLIENT_GUI_GTK_TREESTORE_SERVERS                     "treestore_servers"
#define BITTORRENT_CLIENT_GUI_GTK_TREEVIEW_CHANNEL                      "treeview_page_channel"
#define BITTORRENT_CLIENT_GUI_GTK_VBOX_CHANNEL                          "vbox_page_channel"
#define BITTORRENT_CLIENT_GUI_GTK_VBOX_CONNECTION                       "vbox_page_connection"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_CHANNEL                        "window_page_channel"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_CONNECTION                     "window_page_connection"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_MAIN                           "window_main"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_TAB_CHANNEL                    "window_tab_channel"
#define BITTORRENT_CLIENT_GUI_GTK_WINDOW_TAB_CONNECTION                 "window_tab_connection"

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
