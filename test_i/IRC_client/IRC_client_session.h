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

#ifndef IRC_CLIENT_SESSION_H
#define IRC_CLIENT_SESSION_H

#include <string>

#include "ace/config-macros.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "stream_common.h"

#include "net_iconnection.h"

#include "irc_record.h"
#include "irc_session.h"

#include "IRC_client_inputhandler.h"
#include "IRC_client_network.h"

// forward declarations
class ACE_Message_Block;
class IRC_Client_SessionMessage;
typedef Stream_ISessionDataNotify_T<struct IRC_Client_SessionData,
                                    enum Stream_SessionMessageType,
                                    IRC_Message,
                                    IRC_Client_SessionMessage> IRC_Client_ISessionNotify_t;

template <typename ConnectionType,
          typename UIStateType>
class IRC_Client_Session_T
 : public IRC_Session_T<ConnectionType,
                        struct IRC_Client_SessionData,
                        IRC_Client_ISessionNotify_t,
                        IRC_Client_ConnectionConfiguration,
                        IRC_Message,
                        IRC_Client_SessionMessage,
                        IRC_Client_StreamConfiguration_t::ITERATOR_T,
#if defined (GUI_SUPPORT)
                        UIStateType,
#endif // GUI_SUPPORT
                        IRC_Client_ConnectionConfiguration,
                        IRC_Client_Connection_Manager_t,
                        IRC_Client_InputHandler,
                        struct IRC_Client_InputHandlerConfiguration>
{
 friend class ACE_Connector<IRC_Client_Session_T<ConnectionType,
                                                 UIStateType>,
                            ACE_SOCK_CONNECTOR>;
 friend class ACE_Asynch_Connector<IRC_Client_Session_T<ConnectionType,
                                                        UIStateType> >;

 typedef IRC_Session_T<ConnectionType,
                       struct IRC_Client_SessionData,
                       IRC_Client_ISessionNotify_t,
                       IRC_Client_ConnectionConfiguration,
                       IRC_Message,
                       IRC_Client_SessionMessage,
                       IRC_Client_StreamConfiguration_t::ITERATOR_T,
#if defined (GUI_SUPPORT)
                       UIStateType,
#endif // GUI_SUPPORT
                       IRC_Client_ConnectionConfiguration,
                       IRC_Client_Connection_Manager_t,
                       IRC_Client_InputHandler,
                       struct IRC_Client_InputHandlerConfiguration> inherited;

 public:
  // *NOTE*: if there is no default ctor, this will not compile
  inline IRC_Client_Session_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  IRC_Client_Session_T (bool); // managed ?
  inline virtual ~IRC_Client_Session_T () {}

  // override Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const struct IRC_Client_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const IRC_Message&);
  virtual void notify (Stream_SessionId_t,
                       const IRC_Client_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Session_T (const IRC_Client_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Session_T& operator= (const IRC_Client_Session_T&))

  void log (const std::string&,  // channel (empty ? server log : channel)
            const std::string&); // text
};

// include template definition
#include "IRC_client_session.inl"

#endif
