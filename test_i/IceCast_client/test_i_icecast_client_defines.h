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

#ifndef TEST_I_ICECAST_CLIENT_DEFINES_H
#define TEST_I_ICECAST_CLIENT_DEFINES_H

#include "net_defines.h"

#define TEST_I_ICECAST_CLIENT_DEFAULT_OUTPUT_FILE           "output.wav"
//#define TEST_I_ICECAST_CLIENT_DEFAULT_URL                  "http://streaming.fueralle.org:8000/bermudafunk_high.m3u"
//#define TEST_I_ICECAST_CLIENT_DEFAULT_URL                   "http://streaming.fueralle.org:8000/bermudafunk.ogg.m3u"
//#define TEST_I_ICECAST_CLIENT_DEFAULT_URL                   "http://xfer.hirschmilch.de:8000/chillout.opus.m3u"
#define TEST_I_ICECAST_CLIENT_DEFAULT_URL                   "https://rdst.win:59000/dos.webm.m3u"
#define TEST_I_ICECAST_CLIENT_DEFAULT_WINDOW_TITLE          "IceCast client"

#define TEST_I_ICECAST_CLIENT_DEFAULT_BUFFER_SIZE           NET_STREAM_DEFAULT_MESSAGE_DATA_BUFFER_SIZE

// HTTP headers
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_SERVER_PREFIX       "Icecast"
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_AUDIO_INFO          "ice-audio-info"
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_AUDIO_INFO_CHANNELS "ice-channels"
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_AUDIO_INFO_HZ       "ice-samplerate"
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_ICY_METADATA        "Icy-Metadata"
#define TEST_I_ICECAST_CLIENT_DEFAULT_USER_AGENT_SPOOF            "VLC/3.0.16"

// stream URL
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_STREAM_OGG_SUFFIX  ".ogg"
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_STREAM_OPUS_SUFFIX ".opus"
#define TEST_I_ICECAST_CLIENT_DEFAULT_ICECAST_STREAM_WEBM_SUFFIX ".webm"

//---------------------------------------

#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PRESET_DURATION_D             30.0
#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PRESET_TRANSITION_DURATION_D  3.0
// *NOTE*: relative to ${LIB_ROOT}
#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTURES_DIRECTORY            "presets-cream-of-the-crop-textures"
//#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTURES_DIRECTORY            "presets-milkdrop-texture-pack"
#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PRESETS_DIRECTORY             "presets-cream-of-the-crop"
//#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PRESETS_DIRECTORY             "presets-milkdrop-original"

// Milkdrop default
//#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PER_VERTEX_MESH_RESOLUTION_X  48
//#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PER_VERTEX_MESH_RESOLUTION_Y  32
#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PER_VERTEX_MESH_RESOLUTION_X  220
#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PER_VERTEX_MESH_RESOLUTION_Y  125

// Milkdrop default
//#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTEL_OFFSET_X               0.5f
//#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTEL_OFFSET_Y               0.5f
#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTEL_OFFSET_X               0.0f
#define TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTEL_OFFSET_Y               0.0f

//---------------------------------------

#define TEST_I_UI_GTK_BOX_DISPLAY_NAME                      "box_display"
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                     "button_about"
#define TEST_I_UI_GTK_BUTTON_CUT_NAME                       "button_cut"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                      "button_quit"
#define TEST_I_UI_GTK_CHECKBUTTON_ASYNCH_NAME               "checkbutton_asynch"
#define TEST_I_UI_GTK_CHECKBUTTON_LOOPBACK_NAME             "checkbutton_loopback"
#define TEST_I_UI_GTK_CHECKBUTTON_SAVE_NAME                 "checkbutton_save"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                     "dialog_about"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                      "dialog_main"
#define TEST_I_UI_GTK_DRAWINGAREA_NAME                      "drawingarea_analyzer"
#define TEST_I_UI_GTK_DRAWINGAREA_PROJECTM_NAME             "drawingarea_projectm"
#define TEST_I_UI_GTK_ENTRY_URL_NAME                        "entry_url"
#define TEST_I_UI_GTK_SCALE_VOLUME_NAME                     "scale_volume"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME           "filechooserbutton_save"
#define TEST_I_UI_GTK_PROGRESSBAR_NAME                      "progressbar"
#define TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME            "spinbutton_buffersize"
#define TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME           "spinbutton_connections"
#define TEST_I_UI_GTK_SPINBUTTON_DATA_NAME                  "spinbutton_data"
#define TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME          "spinbutton_data_messages"
#define TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME       "spinbutton_session_messages"
#define TEST_I_UI_GTK_SPINNER_NAME                          "spinner"
#define TEST_I_UI_GTK_STATUSBAR_NAME                        "statusbar"
#define TEST_I_UI_GTK_TOGGLEBUTTON_CONNECT_NAME             "togglebutton_connect"
#define TEST_I_UI_GTK_VBOX_CONFIGURATION_NAME               "vbox_configuration"

#endif
