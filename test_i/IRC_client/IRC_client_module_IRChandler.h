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

#ifndef IRC_CLIENT_MODULE_IRCHANDLER_H
#define IRC_CLIENT_MODULE_IRCHANDLER_H

#include <list>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

#include "irc_statemachine_registration.h"

#include "IRC_client_common.h"
#include "IRC_client_configuration.h"
#include "IRC_client_sessionmessage.h"
#include "IRC_client_stream_common.h"

// forward declaration(s)
class ACE_Time_Value;
class Stream_IAllocator;
class IRC_Message;
class IRC_Record;

class IRC_Client_Module_IRCHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct IRC_Client_ModuleHandlerConfiguration,
                                         ACE_Message_Block,
                                         IRC_Message,
                                         IRC_Client_SessionMessage,
                                         Stream_SessionId_t,
                                         struct IRC_Client_SessionData,
                                         struct IRC_Client_UserData>
 , public IRC_StateMachine_Registration
 , public IRC_IControl
{
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct IRC_Client_ModuleHandlerConfiguration,
                                         ACE_Message_Block,
                                         IRC_Message,
                                         IRC_Client_SessionMessage,
                                         Stream_SessionId_t,
                                         struct IRC_Client_SessionData,
                                         struct IRC_Client_UserData> inherited;
  typedef IRC_StateMachine_Registration inherited2;

 public:
  IRC_Client_Module_IRCHandler (ISTREAM_T*); // stream handle
  inline virtual ~IRC_Client_Module_IRCHandler () {}

  // implement (part of) Common_IStateMachine_T
  virtual bool wait (const ACE_Time_Value* = NULL);

  virtual bool initialize (const struct IRC_Client_ModuleHandlerConfiguration&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (IRC_Message*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (IRC_Client_SessionMessage*&, // session message handle
                                     bool&);                      // return value: pass message downstream ?

  // implement IRC_IControl
//  virtual void subscribe (IRC_Client_IStreamNotify_t*); // new subscriber
//  virtual void unsubscribe (IRC_Client_IStreamNotify_t*); // existing subscriber
  virtual bool registerc (const struct IRC_LoginOptions&); // login details
  virtual void nick (const std::string&); // nick
  virtual void quit (const std::string&); // reason
  virtual void join (const string_list_t&,  // channel(s)
                     const string_list_t&); // key(s)
  virtual void part (const string_list_t&); // channel(s)
  virtual void mode (const std::string&,    // nick/channel
                     char,                  // user/channel mode
                     bool,                  // enable ?
                     const string_list_t&); // any parameters
  virtual void topic (const std::string&,  // channel
                      const std::string&); // topic
  virtual void names (const string_list_t&); // channel(s)
  virtual void list (const string_list_t&); // channel(s)
  virtual void invite (const std::string&,  // nick
                       const std::string&); // channel
  virtual void kick (const std::string&,  // channel
                     const std::string&,  // nick
                     const std::string&); // comment
  virtual void send (const string_list_t&, // receiver(s) [nick/channel]
                     const std::string&);  // message
  virtual void who (const std::string&, // name
                    bool);              // query ops only ?
  virtual void whois (const std::string&,    // server
                      const string_list_t&); // nickmask(s)
  virtual void whowas (const std::string&,  // nick
                       unsigned int,        // count
                       const std::string&); // server
  virtual void away (const std::string&); // message
  virtual void users (const std::string&); // server
  virtual void userhost (const string_list_t&); // nicknames

  // override Common_IDumpState
  virtual void dump_state () const;

  //// override (part of) Stream_IModuleHandler_T
  //virtual bool initialize (const IRC_Client_ModuleHandlerConfiguration&);

  // override Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Module_IRCHandler ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Module_IRCHandler (const IRC_Client_Module_IRCHandler&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Module_IRCHandler& operator= (const IRC_Client_Module_IRCHandler&))

  // implement (part of) Common_IStateMachine_T
  virtual void onChange (enum IRC_RegistrationStateType); // new state

  // helper methods
  IRC_Record* allocateMessage (IRC_CommandType_t); // command

  // *NOTE*: "fire-and-forget" - the argument is consumed
  void sendMessage (IRC_Record*&); // command handle

  // *NOTE*: obviously, there is a delay between connection establishment and
  //         reception of the welcome NOTICE; let clients wait for it so they
  //         can start registering connections in accordance with the IRC
  //         protocol (*TODO*: reference)
  ACE_SYNCH_MUTEX     conditionLock_;
  ACE_SYNCH_CONDITION condition_;
  bool                connectionIsAlive_;
  bool                initialRegistration_;
  bool                receivedInitialNotice_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct IRC_Client_SessionData,                // session data type
                              enum Stream_SessionMessageType,               // session event type
                              struct IRC_Client_ModuleHandlerConfiguration, // module handler configuration type
                              IRC_Client_IStreamNotify_t,                   // stream notification interface type
                              IRC_Client_Module_IRCHandler);                // writer type

#endif
