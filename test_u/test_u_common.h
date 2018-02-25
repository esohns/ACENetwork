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

#ifndef TEST_U_COMMON_H
#define TEST_U_COMMON_H

#include "common_configuration.h"
#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "net_common.h"

typedef Net_Statistic_t Test_U_Statistic_t;
typedef Common_IStatistic_T<Test_U_Statistic_t> Test_U_IStatistic_t;
typedef Common_StatisticHandler_T<Test_U_Statistic_t> Test_U_StatisticHandler_t;

struct Test_U_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_U_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
  {};
};

//////////////////////////////////////////

struct Test_U_UserData
 : Net_UserData
{
  Test_U_UserData ()
   : Net_UserData ()
  {};
};

struct Test_U_Configuration
{
  Test_U_Configuration ()
   : signalHandlerConfiguration ()
   , userData ()
  {};

  struct Test_U_SignalHandlerConfiguration signalHandlerConfiguration;

  struct Test_U_UserData                   userData;
};

#endif
