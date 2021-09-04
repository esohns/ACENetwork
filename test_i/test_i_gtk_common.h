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

#ifndef TEST_I_GTK_COMMON_H
#define TEST_I_GTK_COMMON_H

#include "gtk/gtk.h"

#include "common_ui_gtk_common.h"
#if defined (GTKGL_SUPPORT)
#include "common_ui_gtk_gl_common.h"
#endif // GTKGL_SUPPORT

//#include "test_i_configuration.h"
#include "test_i_common.h"

struct Test_I_GTK_Configuration
 : Test_I_Configuration
{
  Test_I_GTK_Configuration ()
   : Test_I_Configuration ()
   , GTKConfiguration ()
  {}

  Common_UI_GTK_Configuration_t GTKConfiguration;
};

//////////////////////////////////////////

struct Test_I_GTK_ProgressData
 : Common_UI_GTK_ProgressData
{
  Test_I_GTK_ProgressData ()
   : Common_UI_GTK_ProgressData ()
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (Test_I_Statistic_t));
  }

  Test_I_Statistic_t statistic;
};

struct Test_I_GTK_CBData
 : Test_I_UI_CBData
{
  Test_I_GTK_CBData ()
   : Test_I_UI_CBData ()
   , configuration (NULL)
//   , cursorType (GDK_LAST_CURSOR)
   , progressData ()
   , progressEventSourceId (0)
   , UIState (NULL)
  {
    progressData.state = UIState;
  }

  struct Test_I_Configuration*   configuration;
//  GdkCursorType                      cursorType;
  struct Test_I_GTK_ProgressData progressData;
  guint                          progressEventSourceId;
  Common_UI_GTK_State_t*         UIState;
};

struct Test_I_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
   , eventSourceId (0)
  {}

  struct Test_I_GTK_CBData* CBData;
  guint                     eventSourceId;
};

#endif
