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
#include "ace/INET_Addr.h"

#include "common_iinitialize.h"
#include "common_isignal.h"
#include "common_signalhandler.h"

#include "IRC_client_defines.h"
#include "IRC_client_network.h"

// forward declarations
struct IRC_Client_CursesState;

struct IRC_Client_SignalHandlerConfiguration
{
  inline IRC_Client_SignalHandlerConfiguration ()
   : connector (NULL)
   , cursesState (NULL)
   , peerAddress ()
  {};

  IRC_Client_IConnector_t* connector;
  IRC_Client_CursesState*  cursesState;
  ACE_INET_Addr            peerAddress;
};

class IRC_Client_SignalHandler
 : public Common_SignalHandler
 , public Common_IInitialize_T<IRC_Client_SignalHandlerConfiguration>
 , public Common_ISignal
{
 public:
  IRC_Client_SignalHandler (bool = IRC_CLIENT_DEF_CLIENT_USES_REACTOR, // use reactor ?
                            bool = IRC_CLIENT_SESSION_DEF_CURSES);     // use curses library ?
  virtual ~IRC_Client_SignalHandler ();

  // implement Common_IInitialize_T
  virtual bool initialize (const IRC_Client_SignalHandlerConfiguration&); // configuration

  // implement Common_ISignal
  virtual bool handleSignal (int); // signal

 private:
  typedef Common_SignalHandler inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_SignalHandler ());
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_SignalHandler (const IRC_Client_SignalHandler&));
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_SignalHandler& operator= (const IRC_Client_SignalHandler&));

  IRC_Client_SignalHandlerConfiguration* configuration_;
  bool                                   useCursesLibrary_;
  bool                                   useReactor_;
};

#endif
