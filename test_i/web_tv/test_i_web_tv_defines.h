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

// M3U
#define TEST_I_M3U_EXTINFO_RESOLUTION_HEIGHT_STRING       "height"
#define TEST_I_M3U_EXTINFO_RESOLUTION_WIDTH_STRING        "width"

//----------------------------------------

#if defined (GTK_SUPPORT)
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                   "button_about"
#define TEST_I_UI_GTK_BUTTON_CUT_NAME                     "button_load"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                    "button_quit"
#define TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME               "combobox_channel"
#define TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME               "combobox_display"
#define TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME            "combobox_resolution"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                   "dialog_about"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                    "dialog_main"
#define TEST_I_UI_GTK_DRAWINGAREA_NAME                    "drawingarea"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME         "filechooserbutton_save"
#define TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME            "frame_configuration"
#define TEST_I_UI_GTK_FRAME_CHANNEL_NAME                  "frame_channel"
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
#endif // GTK_SUPPORT

#endif
