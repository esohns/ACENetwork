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

#ifndef IRC_CLIENT_GUI_DEFINES_H
#define IRC_CLIENT_GUI_DEFINES_H

//// scripts
//#define IRC_CLIENT_GUI_GTK_SCRIPTS_DIRECTORY "scripts"

// phonebook
#define IRC_CLIENT_GUI_DEF_FILE_PHONEBOOK "IRC_client_phonebook.ini"

// glade/GTKBuilder definitions
#define IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY  "etc"
// files
#define IRC_CLIENT_GUI_GTK_UI_CHANNEL_FILE    "IRC_client_channel.glade"
#define IRC_CLIENT_GUI_GTK_UI_CONNECTION_FILE "IRC_client_connection.glade"
#define IRC_CLIENT_GUI_GTK_UI_MAIN_FILE       "IRC_client_main.glade"

// GTK .rc files
#define IRC_CLIENT_GUI_GTK_UI_RC_FILE "IRC_client.rc"

// GTK widget names
#define IRC_CLIENT_GUI_GTK_ACTION_AWAY                           "action_away"
#define IRC_CLIENT_GUI_GTK_ACTION_BAN                            "action_ban"
#define IRC_CLIENT_GUI_GTK_ACTION_INFO                           "action_info"
#define IRC_CLIENT_GUI_GTK_ACTION_INVITE                         "action_invite"
#define IRC_CLIENT_GUI_GTK_ACTION_KICK                           "action_kick"
#define IRC_CLIENT_GUI_GTK_ACTION_MESSAGE                        "action_msg"
#define IRC_CLIENT_GUI_GTK_BUTTON_ABOUT                          "button_about"
#define IRC_CLIENT_GUI_GTK_BUTTON_CHANNELS_REFRESH               "button_page_connection_channels_refresh"
#define IRC_CLIENT_GUI_GTK_BUTTON_CONNECT                        "button_connect"
#define IRC_CLIENT_GUI_GTK_BUTTON_DISCONNECT                     "button_close"
#define IRC_CLIENT_GUI_GTK_BUTTON_NICK_ACCEPT                    "button_page_connection_nick_accept"
#define IRC_CLIENT_GUI_GTK_BUTTON_USERS_REFRESH                  "button_page_connection_users_refresh"
#define IRC_CLIENT_GUI_GTK_BUTTON_JOIN                           "button_page_connection_channels_join"
#define IRC_CLIENT_GUI_GTK_BUTTON_PART                           "button_close"
#define IRC_CLIENT_GUI_GTK_BUTTON_SEND                           "button_send"
#define IRC_CLIENT_GUI_GTK_BUTTON_QUIT                           "button_quit"
#define IRC_CLIENT_GUI_GTK_COMBOBOX_CHANNELS                     "combobox_page_connection_channels"
#define IRC_CLIENT_GUI_GTK_COMBOBOX_SERVERS                      "combobox_servers"
#define IRC_CLIENT_GUI_GTK_COMBOBOX_USERS                        "combobox_page_connection_users"
#define IRC_CLIENT_GUI_GTK_DIALOG_CHANNEL_ENTRY                  "dialog_page_channel_entry"
#define IRC_CLIENT_GUI_GTK_DIALOG_CONNECTION_ENTRY               "dialog_page_connection_entry"
#define IRC_CLIENT_GUI_GTK_DIALOG_MAIN_ABOUT                     "dialog_main_about"
#define IRC_CLIENT_GUI_GTK_DIALOG_MAIN_ENTRY                     "dialog_main_entry"
#define IRC_CLIENT_GUI_GTK_DIALOG_MAIN_MESSAGE                   "dialog_main_message"
#define IRC_CLIENT_GUI_GTK_ENTRY_CHANNEL                         "entry_dialog_page_channel_entry"
#define IRC_CLIENT_GUI_GTK_ENTRY_CONNECTION                      "entry_dialog_page_connection_entry"
#define IRC_CLIENT_GUI_GTK_ENTRY_CONNECTION_CHANNEL              "entry_page_connection_channels"
#define IRC_CLIENT_GUI_GTK_ENTRY_MAIN                            "entry_dialog_entry_main"
#define IRC_CLIENT_GUI_GTK_ENTRY_NICK                            "entry_page_connection_nick"
#define IRC_CLIENT_GUI_GTK_ENTRY_SEND                            "entry_send"
#define IRC_CLIENT_GUI_GTK_EVENTBOX_TOPIC                        "eventbox_page_channel_topic"
#define IRC_CLIENT_GUI_GTK_FRAME_CHANNELMODE                     "frame_page_channel_mode"
#define IRC_CLIENT_GUI_GTK_HBOX_CHANNELMODE                      "hbox_page_channel_mode"
#define IRC_CLIENT_GUI_GTK_HBOX_CHANNEL_TAB                      "hbox_tab_channel"
#define IRC_CLIENT_GUI_GTK_HBOX_CONNECTION_TAB                   "hbox_tab_connection"
#define IRC_CLIENT_GUI_GTK_HBOX_NICK_CHANNEL                     "hbox_page_connection_nick_channel"
#define IRC_CLIENT_GUI_GTK_HBOX_SEND                             "hbox_send"
#define IRC_CLIENT_GUI_GTK_HBOX_USERMODE                         "hbox_page_connection_usermode"
#define IRC_CLIENT_GUI_GTK_LABEL_CHANNEL_TAB                     "label_tab_channel"
#define IRC_CLIENT_GUI_GTK_LABEL_CONNECTION_TAB                  "label_tab_connection"
#define IRC_CLIENT_GUI_GTK_LABEL_NICK                            "label_page_connection_nick"
#define IRC_CLIENT_GUI_GTK_LABEL_TOPIC                           "label_page_channel_topic"
#define IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNEL                     "liststore_page_channel"
#define IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNELS                    "liststore_page_connection_channels"
#define IRC_CLIENT_GUI_GTK_LISTSTORE_USERS                       "liststore_page_connection_users"
#define IRC_CLIENT_GUI_GTK_MENU_CHANNEL                          "menu_page_channel"
#define IRC_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE                   "menu_page_channel_invite"
#define IRC_CLIENT_GUI_GTK_MENUITEM_BAN                          "menuitem_ban"
#define IRC_CLIENT_GUI_GTK_MENUITEM_INVITE                       "menuitem_invite"
#define IRC_CLIENT_GUI_GTK_MENUITEM_KICK                         "menuitem_kick"
#define IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS                     "notebook_page_connection_channels"
#define IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS                  "notebook_connections"
#define IRC_CLIENT_GUI_GTK_PROGRESSBAR                           "progressbar"
#define IRC_CLIENT_GUI_GTK_SCROLLEDWINDOW_CONNECTION             "scrolledwindow_page_connection_channels"
#define IRC_CLIENT_GUI_GTK_STATUSBAR                             "statusbar"
#define IRC_CLIENT_GUI_GTK_TEXTVIEW_CHANNEL                      "textview_page_channel"
#define IRC_CLIENT_GUI_GTK_TEXTVIEW_CONNECTION                   "textview_page_connection_channels"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_ANONYMOUS    "togglebutton_anonymous"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BAN          "togglebutton_ban"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BLOCKFOREIGN "togglebutton_blockforeign"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_INVITEONLY   "togglebutton_inviteonly"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_KEY          "togglebutton_key"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_MODERATED    "togglebutton_moderated"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_OPERATOR     "togglebutton_operator"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_PRIVATE      "togglebutton_private"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_QUIET        "togglebutton_quiet"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_REOP         "togglebutton_reop"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_RESTRICTOPIC "togglebutton_restricttopic"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_SECRET       "togglebutton_secret"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_USERLIMIT    "togglebutton_userlimit"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_VOICE        "togglebutton_voice"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_TOOLS_1                  "togglebutton_tool_1"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY            "togglebutton_away"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE       "togglebutton_invisible"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_LOCALOPERATOR   "togglebutton_localoperator"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES         "togglebutton_notices"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_OPERATOR        "togglebutton_operator"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_RESTRICTED      "togglebutton_restricted"
#define IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS         "togglebutton_wallops"
#define IRC_CLIENT_GUI_GTK_TREESTORE_SERVERS                     "treestore_servers"
#define IRC_CLIENT_GUI_GTK_TREEVIEW_CHANNEL                      "treeview_page_channel"
#define IRC_CLIENT_GUI_GTK_VBOX_CHANNEL                          "vbox_page_channel"
#define IRC_CLIENT_GUI_GTK_VBOX_CONNECTION                       "vbox_page_connection"
#define IRC_CLIENT_GUI_GTK_WINDOW_CHANNEL                        "window_page_channel"
#define IRC_CLIENT_GUI_GTK_WINDOW_CONNECTION                     "window_page_connection"
#define IRC_CLIENT_GUI_GTK_WINDOW_MAIN                           "window_main"
#define IRC_CLIENT_GUI_GTK_WINDOW_TAB_CHANNEL                    "window_tab_channel"
#define IRC_CLIENT_GUI_GTK_WINDOW_TAB_CONNECTION                 "window_tab_connection"

// GTK cursor
// *TODO*: use GDK_BLANK_CURSOR instead ?
#define IRC_CLIENT_GUI_GTK_CURSOR_BUSY GDK_WATCH

// GTK progress/status-bar
#define IRC_CLIENT_GUI_GTK_PROGRESSBAR_UPDATE_INTERVAL   27 // ms (?)
#define IRC_CLIENT_GUI_GTK_STATUSBAR_CONTEXT_DESCRIPTION "IRC client::main"

// GTK log refresh rate
#define IRC_CLIENT_GUI_GTK_LOG_REFRESH_RATE 2 // 1/s

#define IRC_CLIENT_GUI_GTK_LABEL_DEF_LOG_TEXT   "server log"
#define IRC_CLIENT_GUI_GTK_LABEL_DEF_TOPIC_TEXT "no topic"

// dialog titles
#define IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_AWAY           "please provide an away message..."
#define IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_PASSWORD  "please provide a key..."
#define IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_VOICE     "please specify a nickname..."
#define IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_BAN       "set a ban mask..."
#define IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_USERLIMIT "set the channel user limit..."
#define IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_NICK           "please provide a nickname..."
#define IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_TOPIC          "set the channel topic..."

// message dialog text
#define IRC_CLIENT_GUI_MESSAGEDIALOG_TEXT_NICKNAMETAKEN "the nickname you supplied is in use, please try again"

//#define IRC_CLIENT_GUI_DEF_SERVER_LABEL_TEXT  "no server"
//#define IRC_CLIENT_GUI_DEF_CHANNEL_LABEL_TEXT "no channel"

#endif
