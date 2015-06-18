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

#include "net_client_connector_common.h"

struct IRC_Client_SignalHandlerConfiguration
{
//  long                     actionTimerId;
  Net_Client_IConnector_t* connector;
  ACE_INET_Addr            peerAddress;
};

class IRC_Client_SignalHandler
 : public Common_SignalHandler
 , public Common_IInitialize_T<IRC_Client_SignalHandlerConfiguration>
 , public Common_ISignal
{
 public:
  IRC_Client_SignalHandler (bool = true); // use reactor ?
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
  bool                                   useReactor_;
};

#endif
