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

#ifndef FILESERVER_DEFINES_H
#define FILESERVER_DEFINES_H

 // *** trace log ***
#define FILE_SERVER_LOG_FILENAME_PREFIX                   "file_server"

#define FILE_SERVER_DEFAULT_FILE                          "test.mp4"

 // stream
#define FILE_SERVER_DEFAULT_MESSAGE_DATA_BUFFER_SIZE      65536 // bytes
#define FILE_SERVER_DEFAULT_MPEG_TS_PROGRAM_NUMBER        1
#define FILE_SERVER_DEFAULT_MPEG_TS_STREAM_TYPE           27 // H264

//////////////////////////////////////////

#define FILE_SERVER_UI_FILE                               "file_server.glade"

// glade/GTKBuilder definitions
#define FILE_SERVER_GTK_ABOUTDIALOG_NAME                   "aboutdialog"
#define FILE_SERVER_GTK_ADJUSTMENT_NAME                    "scrolledwindow_vadjustment"
#define FILE_SERVER_GTK_BUTTON_ABOUT_NAME                  "about"
#define FILE_SERVER_GTK_BUTTON_CLEAR_NAME                  "clear"
#define FILE_SERVER_GTK_BUTTON_CLOSEALL_NAME               "close_all"
#define FILE_SERVER_GTK_BUTTON_LISTEN_NAME                 "listen"
#define FILE_SERVER_GTK_BUTTON_REPORT_NAME                 "report"
#define FILE_SERVER_GTK_BUTTON_QUIT_NAME                   "quit"
#define FILE_SERVER_GTK_BUTTONBOX_ACTIONS_NAME             "vbuttonbox"
#define FILE_SERVER_GTK_DIALOG_MAIN_NAME                   "dialog_main"
#define FILE_SERVER_GTK_FILECHOOSERBUTTON_FILE_NAME        "filechooserbutton_file"
#define FILE_SERVER_GTK_FRAME_CONFIGURATION_NAME           "frame_configuration"
#define FILE_SERVER_GTK_PROGRESSBAR_NAME                   "progressbar"
#define FILE_SERVER_GTK_SCROLLMARK_NAME                    "insert"
#define FILE_SERVER_GTK_SPINBUTTON_DATA_NAME               "spinbutton_data"
#define FILE_SERVER_GTK_SPINBUTTON_NUMCONNECTIONS_NAME     "spinbutton_connections"
#define FILE_SERVER_GTK_SPINBUTTON_NUMMESSAGES_NAME        "spinbutton_messages"
#define FILE_SERVER_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME "spinbutton_session_messages"
#define FILE_SERVER_GTK_SPINBUTTON_PORT_NAME               "spinbutton_port"
#define FILE_SERVER_GTK_TEXTVIEW_NAME                      "textview_log"

// pango
#define FILE_SERVER_GTK_PANGO_LOG_FONT_DESCRIPTION         "Monospace 8"
#define FILE_SERVER_GTK_PANGO_LOG_COLOR_BASE               "#FFFFFF" // white
#define FILE_SERVER_GTK_PANGO_LOG_COLOR_TEXT               "#000000" // green

// -----------------------------------------------------------------------------

//#define NET_CLIENT_DEF_SERVER_CONNECT_INTERVAL          0
//#define NET_CLIENT_DEF_SERVER_PING_INTERVAL             0 // ms {0: OFF}
//#define NET_CLIENT_DEF_SERVER_TEST_INTERVAL             20 // ms
//#define NET_CLIENT_DEF_SERVER_STRESS_INTERVAL           1 // ms

// GTK progress/status bar
#define FILE_SERVER_GTK_STATUSBAR_CONTEXT_DESCRIPTION     "ACENetwork_Test_U::main"

//// define behaviour
//#define NET_CLIENT_U_TEST_CONNECT_PROBABILITY           0.5F // 50%
//#define NET_CLIENT_U_TEST_ABORT_PROBABILITY             0.2F // 20%
//#define NET_CLIENT_U_TEST_DEFAULT_MODE                  Net_Client_TimeoutHandler::ACTION_NORMAL

#endif
