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

#ifndef TEST_U_SIGNALHANDLER_H
#define TEST_U_SIGNALHANDLER_H

#include "ace/Global_Macros.h"

#include "common.h"
#include "common_signalhandler.h"

#include "wlan_monitor_common.h"

class Test_U_SignalHandler
 : public Common_SignalHandler_T<struct WLANMonitor_SignalHandlerConfiguration>
{
 public:
  Test_U_SignalHandler (struct WLANMonitor_GTK_CBData*); // GTK state
  inline virtual ~Test_U_SignalHandler () {}

  // implement Common_ISignal
  virtual void handle (const struct Common_Signal&); // signal information

 private:
  typedef Common_SignalHandler_T<struct WLANMonitor_SignalHandlerConfiguration> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_SignalHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_SignalHandler (const Test_U_SignalHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_SignalHandler& operator= (const Test_U_SignalHandler&))

  struct WLANMonitor_GTK_CBData* CBData_;
};

#endif