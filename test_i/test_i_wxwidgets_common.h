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

#ifndef TEST_I_WXWIDGETS_COMMON_H
#define TEST_I_WXWIDGETS_COMMON_H

#include "ace/OS.h"
//#include "ace/Synch_Traits.h"

#include "common_ui_wxwidgets_common.h"

#include "test_i_common.h"

// forward declarations
struct Test_I_Configuration;

struct Test_I_wxWidgets_ProgressData
 : Common_UI_wxWidgets_ProgressData
{
  Test_I_wxWidgets_ProgressData ()
   : Common_UI_wxWidgets_ProgressData ()
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (Test_I_Statistic_t));
  }

  Test_I_Statistic_t statistic;
};

struct Test_I_wxWidgets_CBData
 : Test_I_UI_CBData
{
  Test_I_wxWidgets_CBData ()
   : Test_I_UI_CBData ()
   , progressData ()
   , UIState (NULL)
  {
    progressData.state = UIState;
  }

  struct Test_I_wxWidgets_ProgressData progressData;
  struct Common_UI_wxWidgets_State*    UIState;
};

struct Test_I_wxWidgets_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_wxWidgets_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_wxWidgets_CBData* CBData;
};

#endif
