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

#ifndef TEST_I_WEBTV_DEFINES_H
#define TEST_I_WEBTV_DEFINES_H

// ini
#define TEST_I_WEBTV_CNF_CHANNELS_SECTION_HEADER          "channels"
#define TEST_I_WEBTV_CNF_CHANNEL_NUMBER_KEY               "number"
#define TEST_I_WEBTV_CNF_CHANNEL_NAME_KEY                 "name"
#define TEST_I_WEBTV_CNF_CHANNEL_INDEX_POSITIONS_KEY      "index_positions"
#define TEST_I_WEBTV_CNF_CHANNEL_MAX_INDEX_KEY            "max_index"
#define TEST_I_WEBTV_CNF_CHANNEL_URL_KEY                  "URL"

#define TEST_I_WEBTV_CNF_INI_FILE                         "web_tv.ini"

#define TEST_I_WEBTV_DEFAULT_TARGET_FILE                  "output.avi"

// *NOTE*: compute and add new segments when this value is reached
#define TEST_I_WEBTV_DEFAULT_SEGMENT_LIST_LWM             10
// *NOTE*: preload, NOT download
#define TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS    100

#define TEST_I_WEBTV_DEFAULT_BUFFER_SIZE                  131072 // bytes

#define TEST_I_WEBTV_NUMBER_OF_DISPATCH_THREADS_REACTOR   1
#define TEST_I_WEBTV_NUMBER_OF_DISPATCH_THREADS_PROACTOR  3

// M3U
#define TEST_I_M3U_EXTINFO_FRAMERATE_KEY_STRING           "FRAME-RATE"
#define TEST_I_M3U_EXTINFO_RESOLUTION_KEY_STRING          "RESOLUTION"
#define TEST_I_M3U_EXTINFO_URI_KEY_STRING                 "URI"

// keys
#define TEST_I_M3U_MEDIAINFO_DEFAULT_KEY_STRING           "DEFAULT"
#define TEST_I_M3U_MEDIAINFO_NAME_KEY_STRING              "NAME"
#define TEST_I_M3U_MEDIAINFO_TYPE_KEY_STRING              "TYPE"
#define TEST_I_M3U_MEDIAINFO_URI_KEY_STRING               "URI"
// values
#define TEST_I_M3U_MEDIAINFO_DEFAULT_YES_STRING           "YES"
#define TEST_I_M3U_MEDIAINFO_TYPE_AUDIO_STRING            "AUDIO"

//----------------------------------------

#if defined (GTK_SUPPORT)
#define TEST_I_UI_GTK_BOX_DISPLAY_NAME                    "box_display"
#define TEST_I_UI_GTK_BOX_SAVE_NAME                       "box_save"
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                   "button_about"
#define TEST_I_UI_GTK_BUTTON_LOAD_NAME                    "button_load"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                    "button_quit"
#define TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME               "combobox_adapter"
#define TEST_I_UI_GTK_COMBOBOX_AUDIOCHANNEL_NAME          "combobox_audio_channel"
#define TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME               "combobox_channel"
#define TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME               "combobox_display"
#define TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME            "combobox_resolution"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                   "aboutdialog"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                    "dialog_main"
#define TEST_I_UI_GTK_DRAWINGAREA_NAME                    "drawingarea"
#define TEST_I_UI_GTK_DRAWINGAREA_FULLSCREEN_NAME         "drawingarea_fullscreen"
#define TEST_I_UI_GTK_ENTRY_SAVE_NAME                     "entry_save"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME         "filechooserbutton_save"
#define TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME            "frame_configuration"
#define TEST_I_UI_GTK_FRAME_CHANNEL_NAME                  "frame_channel"
#define TEST_I_UI_GTK_LISTSTORE_ADAPTER_NAME              "liststore_adapter"
#define TEST_I_UI_GTK_LISTSTORE_AUDIOCHANNEL_NAME         "liststore_audio_channel"
#define TEST_I_UI_GTK_LISTSTORE_CHANNEL_NAME              "liststore_channel"
#define TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME              "liststore_display"
#define TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME           "liststore_resolution"
#define TEST_I_UI_GTK_PROGRESSBAR_NAME                    "progressbar"
#define TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME         "spinbutton_connections"
#define TEST_I_UI_GTK_SPINBUTTON_BYTES_NAME               "spinbutton_bytes"
#define TEST_I_UI_GTK_SPINBUTTON_DATA_NAME                "spinbutton_data"
#define TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME             "spinbutton_session"
#define TEST_I_UI_GTK_STATUSBAR_NAME                      "statusbar"
#define TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME           "togglebutton_display"
#define TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME        "togglebutton_fullscreen"
#define TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME              "togglebutton_play"
#define TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME              "togglebutton_save"
#define TEST_I_UI_GTK_WINDOW_FULLSCREEN_NAME              "window_fullscreen"
#endif // GTK_SUPPORT

#endif
