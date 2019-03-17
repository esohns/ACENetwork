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

#ifndef IRC_SESSION_H
#define IRC_SESSION_H

#include <string>

#include "ace/Asynch_Connector.h"
#include "ace/config-macros.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "stream_common.h"

#include "net_iconnection.h"

#include "irc_record.h"

// forward declarations
class ACE_Message_Block;

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType, // IRC_IControl_T
          typename NotificationType, // Common_INotify_T
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename ModuleHandlerConfigurationIteratorType,
          typename StateType, // ui state (curses/gtk/...) *TODO*: to be removed
          ///////////////////////////////
          // *TODO*: remove these ASAP
          typename ConnectionConfigurationType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType> // *NOTE*: needs to inherit from ACE_FILE_IO
class IRC_Session_T
 : public ConnectionType
 //, virtual public Net_ISession_T<ACE_INET_Addr,
 //                                Net_SocketConfiguration,
 //                                IRC_Client_Configuration,
 //                                Stream_Statistic,
 //                                IRC_Client_Stream,
 //                                IRC_Client_ConnectionState>
 , public NotificationType
{
 friend class ACE_Connector<IRC_Session_T<ConnectionType,
                                          SessionDataType,
//                                          ControllerType,
                                          NotificationType,
                                          ConfigurationType,
                                          MessageType,
                                          SessionMessageType,
                                          ModuleHandlerConfigurationIteratorType,
                                          StateType,
                                          ConnectionConfigurationType,
                                          ConnectionManagerType,
                                          InputHandlerType,
                                          InputHandlerConfigurationType,
                                          LogOutputType>,
                            ACE_SOCK_CONNECTOR>;
 friend class ACE_Asynch_Connector<IRC_Session_T<ConnectionType,
                                                 SessionDataType,
//                                                 ControllerType,
                                                 NotificationType,
                                                 ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType,
                                                 ModuleHandlerConfigurationIteratorType,
                                                 StateType,
                                                 ConnectionConfigurationType,
                                                 ConnectionManagerType,
                                                 InputHandlerType,
                                                 InputHandlerConfigurationType,
                                                 LogOutputType> >;

 public:
 // *NOTE*: if there is no default ctor, this will not compile
 inline IRC_Session_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
 IRC_Session_T (bool); // managed ?
  virtual ~IRC_Session_T ();

  // implement Net_ISession_T
  //virtual const IRC_SessionState& state () const;

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const SessionDataType&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const MessageType&);
  virtual void notify (Stream_SessionId_t,
                       const SessionMessageType&);

  // override some task-based members
  // *TODO*: make these private
  virtual int open (void* = NULL); // arg
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)

 private:
  typedef ConnectionType inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Session_T (const IRC_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Session_T& operator= (const IRC_Session_T&))

  void error (const IRC_Record&);
  void log (const IRC_Record&);
  void log (const std::string&,  // channel (empty ? server log : channel)
            const std::string&); // text

  bool              close_;
  InputHandlerType* inputHandler_;
  bool              logToFile_;
  LogOutputType     output_;
  bool              shutDownOnEnd_;
  StateType*        UIState_;
};

// include template definition
#include "irc_session.inl"

#endif
