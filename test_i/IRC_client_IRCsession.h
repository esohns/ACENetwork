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

#ifndef IRC_CLIENT_IRCSESSION_H
#define IRC_CLIENT_IRCSESSION_H

#include <string>

#include "ace/Asynch_Connector.h"
#include "ace/config-macros.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"

#include "IRC_client_common.h"
#include "IRC_client_inputhandler.h"
#include "IRC_client_IRCmessage.h"
#include "IRC_client_network.h"
//#include "IRC_client_statemachine_registration.h"
#include "IRC_client_stream_common.h"

// forward declarations
class ACE_Message_Block;
class IRC_Client_IIRCControl;

template <typename ConnectionType>
class IRC_Client_IRCSession_T
 : public ConnectionType
 , public IRC_Client_INotify_t
{
 friend class ACE_Connector<IRC_Client_IRCSession_T<ConnectionType>,
                            ACE_SOCK_CONNECTOR>;
 friend class ACE_Asynch_Connector<IRC_Client_IRCSession_T<ConnectionType> >;

 public:
  IRC_Client_IRCSession_T (IRC_Client_IConnection_Manager_t* = NULL, // connection manager handle
                           unsigned int = 0);                        // statistics collecting interval (second(s))
                                                                     // 0 --> DON'T collect statistics
  virtual ~IRC_Client_IRCSession_T ();

  // implement IRC_Client_INotify_t
  virtual void start (const IRC_Client_StreamModuleConfiguration&);
  virtual void notify (const IRC_Client_IRCMessage&);
  virtual void end ();

 private:
  typedef ConnectionType inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_IRCSession_T (const IRC_Client_IRCSession_T&));
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_IRCSession_T& operator= (const IRC_Client_IRCSession_T&));

  // helper methods
  virtual int open (void*); // arg
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)

  void error (const IRC_Client_IRCMessage&);
  void log (const IRC_Client_IRCMessage&);
  void log (const std::string&);

  bool                     close_;
  IRC_Client_IIRCControl*  controller_;
  IRC_Client_InputHandler* inputHandler_;
  IRC_Client_IOStream_t    output_;
  IRC_Client_SessionState  state_;
};

// include template implementation
#include "IRC_client_IRCsession.inl"

#endif
