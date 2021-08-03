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

#ifndef TEST_U_DHCP_CLIENT_DEFINES_H
#define TEST_U_DHCP_CLIENT_DEFINES_H

#define TEST_U_DEFAULT_GTK_RC_FILE                        "resources.rc"
#define TEST_U_DEFAULT_GLADE_FILE                         "test_u.glade"
#define TEST_U_DEFAULT_OUTPUT_FILE                        "output.txt"
#define TEST_U_THREAD_NAME                                "stream processor"

//#define TEST_U_DEFAULT_BUFFER_SIZE                        DHCP_BUFFER_SIZE

//---------------------------------------

#define TEST_U_UI_GTKEVENT_RESOLUTION                     200 // ms --> 5 FPS
#define TEST_U_UI_GTK_ACTION_DISCOVER_NAME                "action_discover"
#define TEST_U_UI_GTK_ACTION_INFORM_NAME                  "action_inform"
#define TEST_U_UI_GTK_ACTION_REQUEST_NAME                 "action_request"
#define TEST_U_UI_GTK_ACTION_RELEASE_NAME                 "action_release"
#define TEST_U_UI_GTK_ACTION_REPORT_NAME                  "action_report"
#define TEST_U_UI_GTK_ADJUSTMENT_NAME                     "scrolledwindow_vadjustment"
#define TEST_U_UI_GTK_BUTTONBOX_ACTIONS_NAME              "vbuttonbox"
#define TEST_U_UI_GTK_BUTTON_ABOUT_NAME                   "about"
#define TEST_U_UI_GTK_BUTTON_CLEAR_NAME                   "clear"
//#define TEST_U_UI_GTK_BUTTON_REPORT_NAME                  "report"
//#define TEST_U_UI_GTK_BUTTON_SEND_NAME                    "send"
#define TEST_U_UI_GTK_BUTTON_QUIT_NAME                    "quit"
#define TEST_U_UI_GTK_CHECKBUTTON_ASYNCH_NAME             "checkbutton_asynch"
#define TEST_U_UI_GTK_CHECKBUTTON_BROADCAST_NAME          "checkbutton_broadcast"
#define TEST_U_UI_GTK_CHECKBUTTON_REQUEST_NAME            "checkbutton_request"
#define TEST_U_UI_GTK_CHECKBUTTON_LOOPBACK_NAME           "checkbutton_loopback"
#define TEST_U_UI_GTK_COMBOBOX_INTERFACE_NAME             "combobox_interface"
#define TEST_U_UI_GTK_DIALOG_ABOUT_NAME                   "dialog_about"
#define TEST_U_UI_GTK_DIALOG_MAIN_NAME                    "dialog_main"
#define TEST_U_UI_GTK_FRAME_CONFIGURATION_NAME            "frame_configuration"
#define TEST_U_UI_GTK_HBOX_OPTIONS_NAME                   "hbox_options"
#define TEST_U_UI_GTK_IMAGE_CONNECT_NAME                  "image_connect"
#define TEST_U_UI_GTK_IMAGE_DISCONNECT_NAME               "image_disconnect"
#define TEST_U_UI_GTK_LISTSTORE_INTERFACE_NAME            "liststore_interface"
#define TEST_U_UI_GTK_PANGO_LOG_FONT_DESCRIPTION          "Monospace 8"
#define TEST_U_UI_GTK_PANGO_LOG_COLOR_BASE                "#FFFFFF" // white
#define TEST_U_UI_GTK_PANGO_LOG_COLOR_TEXT                "#000000" // black
#define TEST_U_UI_GTK_PROGRESSBAR_NAME                    "progressbar"
#define TEST_U_UI_GTK_SCROLLEDWINDOW_NAME                 "scrolledwindow"
//#define TEST_U_UI_GTK_SCROLLMARK_NAME                 "insert"
#define TEST_U_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME          "spinbutton_buffersize"
#define TEST_U_UI_GTK_SPINBUTTON_DATA_NAME                "spinbutton_data"
#define TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME        "spinbutton_data_messages"
#define TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME     "spinbutton_session_messages"
#define TEST_U_UI_GTK_SPINBUTTON_PORT_NAME                "spinbutton_port"
#define TEST_U_UI_GTK_STATUSBAR_NAME                      "statusbar"
//#define TEST_U_UI_GTK_TABLE_OPTIONS_NAME                  "table_options"
#define TEST_U_UI_GTK_TOGGLEACTION_LISTEN_NAME            "toggleaction_listen"
#define TEST_U_UI_GTK_TOGGLEBUTTON_LISTEN_NAME            "listen"
#define TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTEN_STRING    "Listen"
#define TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTENING_STRING "Listening"
#define TEST_U_UI_GTK_TEXTVIEW_NAME                       "textview"

// GTK progress/status bar
#define TEST_U_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL         27 // ms (?)
#define TEST_U_UI_GTK_STATUSBAR_CONTEXT_DESCRIPTION       "Stream::main"

//---------------------------------------

// send DHCPREQUEST on DHCPOFFER ?
#define TEST_U_DEFAULT_DHCP_SEND_REQUEST_ON_OFFER         false

#endif
