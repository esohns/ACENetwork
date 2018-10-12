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

#ifndef TEST_U_GTK_COMMON_H
#define TEST_U_GTK_COMMON_H

#include "gtk/gtk.h"

#include "common_ui_gtk_common.h"
//#if defined (GTKGL_SUPPORT)
//#include "common_ui_gtk_gl_common.h"
//#endif // GTKGL_SUPPORT

#include "test_u_common.h"

struct Test_U_GTK_ProgressData
 : Common_UI_GTK_ProgressData
{
  Test_U_GTK_ProgressData ()
   : Common_UI_GTK_ProgressData ()
   , size (0)
   , transferred (0)
   , statistic ()
  {}

  size_t             size; // bytes
  size_t             transferred; // bytes
  Test_U_Statistic_t statistic;
};

struct Test_U_GTK_CBData
 : Test_U_UI_CBData
{
  Test_U_GTK_CBData ()
   : Test_U_UI_CBData ()
   , allowUserRuntimeStatistic (true)
   , progressData ()
   , UIState (NULL)
  {}

  bool                           allowUserRuntimeStatistic;
  struct Test_U_GTK_ProgressData progressData;
  Common_UI_GTK_State_t*         UIState;
};

struct Test_U_GTK_ThreadData
{
  Test_U_GTK_ThreadData ()
   : CBData (NULL)
   , eventSourceId (0)
  {}

  struct Test_U_GTK_CBData* CBData;
  guint                     eventSourceId;
};

#endif
