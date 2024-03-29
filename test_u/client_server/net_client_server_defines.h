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

#ifndef NET_CLIENT_SERVER_DEFINES_H
#define NET_CLIENT_SERVER_DEFINES_H

#define TEST_U_STREAM_MODULE_HEADERPARSER_NAME          "HeaderParser"
#define TEST_U_STREAM_MODULE_PROTOCOLHANDLER_NAME       "ProtocolHandler"

// define behaviour
#define NET_CLIENT_DEFAULT_CONNECT_PROBABILITY          0.5F // 50%
#define NET_CLIENT_DEFAULT_ABORT_PROBABILITY            0.2F // 20%
#define NET_CLIENT_DEFAULT_DEFAULT_MODE                 Net_Client_TimeoutHandler::ACTION_NORMAL
 
#define NET_CLIENT_DEFAULT_MAX_NUM_OPEN_CONNECTIONS     0
#define NET_CLIENT_DEFAULT_SERVER_HOSTNAME              ACE_LOCALHOST
#define NET_CLIENT_DEFAULT_CONNECT_INTERVAL             0
#define NET_CLIENT_DEFAULT_PING_INTERVAL                0 // ms {0: OFF}
#define NET_CLIENT_DEFAULT_TEST_ALTERNATING_INTERVAL    500 // ms
#define NET_CLIENT_DEFAULT_TEST_NORMAL_INTERVAL         20 // ms
#define NET_CLIENT_DEFAULT_TEST_STRESS_INTERVAL         1 // ms

// *** UI ***
#define NET_CLIENT_UI_FILE                              "net_client.glade"
#define NET_SERVER_UI_FILE                              "net_server.glade"

// -----------------------------------------------------------------------------

#define NET_UI_GTK_EVENT_RESOLUTION                     200 // ms --> 5 FPS

// GTK progress/status bar
#define NET_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL          27 // ms (?)
#define NET_UI_GTK_STATUSBAR_CONTEXT_DESCRIPTION        "Network::main"

#define NET_UI_GTK_BUTTONBOX_ACTIONS_NAME               "vbuttonbox"
#define NET_UI_GTK_BUTTON_ABOUT_NAME                    "about"
#define NET_UI_GTK_BUTTON_CLOSEALL_NAME                 "close_all"
#define NET_UI_GTK_BUTTON_REPORT_NAME                   "report"
#define NET_UI_GTK_BUTTON_QUIT_NAME                     "quit"
#define NET_UI_GTK_DIALOG_ABOUT_NAME                    "dialog_about"
#define NET_UI_GTK_DIALOG_MAIN_NAME                     "dialog_main"
#define NET_UI_GTK_FRAME_PROTOCOL_NAME                  "frame_protocol"
#define NET_UI_GTK_PROGRESSBAR_NAME                     "progressbar"
#define NET_UI_GTK_RADIOBUTTON_SSL_NAME                 "radiobutton_ssl"
#define NET_UI_GTK_RADIOBUTTON_TCP_NAME                 "radiobutton_tcp"
#define NET_UI_GTK_RADIOBUTTON_UDP_NAME                 "radiobutton_udp"
#define NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME       "spinbutton_connections"
#define NET_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME          "spinbutton_messages"
#define NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME   "spinbutton_session_messages"
#define NET_UI_GTK_SPINBUTTON_PINGINTERVAL_NAME         "spinbutton_ping_interval"

// -----------------------------------------------------------------------------

#define NET_CLIENT_UI_GTK_BUTTON_CONNECT_NAME           "connect"
#define NET_CLIENT_UI_GTK_BUTTON_CLOSE_NAME             "close"
#define NET_CLIENT_UI_GTK_BUTTON_PING_NAME              "ping"
//#define NET_CLIENT_UI_GTK_BUTTON_TEST_LABEL_START       "T_est"
//#define NET_CLIENT_UI_GTK_BUTTON_TEST_LABEL_STOP        "S_top"
#define NET_CLIENT_UI_GTK_BUTTON_TEST_NAME              "test"
#define NET_CLIENT_UI_GTK_IMAGE_START_NAME              "image_start"
#define NET_CLIENT_UI_GTK_IMAGE_STOP_NAME               "image_stop"
#define NET_CLIENT_UI_GTK_RADIOBUTTON_NORMAL_NAME       "radiobutton_normal"
#define NET_CLIENT_UI_GTK_RADIOBUTTON_ALTERNATING_NAME  "radiobutton_alternating"
#define NET_CLIENT_UI_GTK_RADIOBUTTON_STRESS_NAME       "radiobutton_stress"
#define NET_CLIENT_UI_GTK_RADIOBUTTON_NORMAL_LABEL      "normal"
#define NET_CLIENT_UI_GTK_RADIOBUTTON_ALTERNATING_LABEL "alternating"
#define NET_CLIENT_UI_GTK_RADIOBUTTON_STRESS_LABEL      "stress"

// -----------------------------------------------------------------------------

#define NET_SERVER_UI_GTK_TOGGLEBUTTON_LISTEN_NAME      "listen"

#endif
