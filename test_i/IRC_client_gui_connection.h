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

#ifndef IRC_CLIENT_GUI_CONNECTION_H
#define IRC_CLIENT_GUI_CONNECTION_H

#include <map>
#include <string>

#include "ace/Global_Macros.h"

#include "gtk/gtk.h"

#include "common_iget.h"

#include "stream_common.h"

#include "IRC_client_gui_common.h"
#include "IRC_client_IRCmessage.h"
#include "IRC_client_stream_common.h"

// forward declaration(s)
struct Common_UI_GTKState;
class ACE_Thread_Mutex;
class IRC_Client_IIRCControl;
class IRC_Client_GUI_MessageHandler;

/**
  @author Erik Sohns <erik.sohns@web.de>
*/
class IRC_Client_GUI_Connection
 : public IRC_Client_INotify_t
 , public Common_IGet_T<IRC_Client_GTK_ConnectionCBData>
{
  friend class IRC_Client_GUI_MessageHandler;

 public:
  // *WARNING*: make sure the ctor/dtor calls are made either:
  // - by the main thread (servicing the gtk_main event loop)
  // - protected by gdk_threads_enter/gdk_threads_leave
  IRC_Client_GUI_Connection (Common_UI_GTKState*,           // GTK state handle
                             IRC_Client_IIRCControl*,       // controller handle
                             IRC_Client_GUI_Connections_t*, // connections handle
                             //const std::string&,            // (starting) nickname
                             const std::string&,            // (server tab) label
                             const std::string&,            // UI (glade) file directory
                             GtkNotebook*);                 // parent widget
  virtual~IRC_Client_GUI_Connection ();

  // implement IRC_Client_INotify_t
  virtual void start (const IRC_Client_StreamModuleConfiguration&);
  virtual void notify (const IRC_Client_IRCMessage&);
  virtual void end ();

  // implement Common_IGet_T
  virtual const IRC_Client_GTK_ConnectionCBData& get () const;

  // *NOTE*: a return value of -1 indicates non-existence
  guint exists (const std::string&); // channel/nick
  void channels (string_list_t&); // return value: list of active channels

  // *WARNING*: callers may need protection from:
  //            - the thread(s) servicing the UI (GTK) event loop
  //            - the event dispatch thread(s) (reactor/proactor)
  std::string getActiveID (); // *NOTE*: can be a channel/nick !
  IRC_Client_GUI_MessageHandler* getActiveHandler (bool = true); // locked access ?
  void createMessageHandler (const std::string&); // channel/nick
  void terminateMessageHandler (const std::string&); // channel/nick

  bool isInitialized_;

 private:
  typedef std::map<std::string,
                   IRC_Client_GUI_MessageHandler* > MESSAGE_HANDLERS_T;
  typedef MESSAGE_HANDLERS_T::iterator MESSAGE_HANDLERSITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection ());
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection (const IRC_Client_GUI_Connection&));
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection& operator= (const IRC_Client_GUI_Connection&));

  // helper methods
  void forward (const std::string&,  // channel/nick
                const std::string&); // message text
  void log (const std::string&);
  void log (const IRC_Client_IRCMessage&);
  void error (const IRC_Client_IRCMessage&);

  IRC_Client_GUI_MessageHandler* getHandler (const std::string&); // id (channel/nick)

  IRC_Client_GTK_ConnectionCBData CBData_;
  bool                            isFirstUsersMsg_;
  std::string                     UIFileDirectory_;

  ACE_Thread_Mutex                lock_;
  MESSAGE_HANDLERS_T              messageHandlers_;

  guint                           contextID_;
  GtkNotebook*                    parent_;
};

#endif
