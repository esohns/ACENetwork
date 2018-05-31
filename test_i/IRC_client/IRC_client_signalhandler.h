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

#ifndef IRC_CLIENT_SIGNALHANDLER_H
#define IRC_CLIENT_SIGNALHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_signalhandler.h"

#include "IRC_client_common.h"
#include "IRC_client_defines.h"

class IRC_Client_SignalHandler
 : public Common_SignalHandler_T<struct IRC_Client_SignalHandlerConfiguration>
{
  typedef Common_SignalHandler_T<struct IRC_Client_SignalHandlerConfiguration> inherited;

 public:
  IRC_Client_SignalHandler (enum Common_SignalDispatchType,    // dispatch mode
                            ACE_SYNCH_RECURSIVE_MUTEX*,        // lock handle
                            //////////////
                            bool = IRC_CLIENT_SESSION_USE_CURSES); // use curses library ?
  inline virtual ~IRC_Client_SignalHandler () {}

  // implement Common_ISignal
  virtual void handle (const struct Common_Signal&); // signal

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_SignalHandler ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_SignalHandler (const IRC_Client_SignalHandler&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_SignalHandler& operator= (const IRC_Client_SignalHandler&))

  bool useCursesLibrary_;
};

#endif
