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

#ifndef IRC_CLIENT_GUI_MESSAGEHANDLER_H
#define IRC_CLIENT_GUI_MESSAGEHANDLER_H

#include <deque>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch.h"

#include "gtk/gtk.h"

#include "IRC_client_gui_common.h"

// forward declaration(s)
struct Common_UI_GTKState;
class IRC_Client_GUI_Connection;
class IRC_Client_IIRCControl;

class IRC_Client_GUI_MessageHandler
{
 public:
  // ctor for default handler (== server log)
  // *WARNING*: ctors/dtor need gdk_threads_enter/gdk_threads_leave protection
  // (or call from gtk_main context...)
  IRC_Client_GUI_MessageHandler (Common_UI_GTKState*, // GTK state handle
                                 GtkTextView*);       // text view handle
  // ctor for regular channel handler
  IRC_Client_GUI_MessageHandler (Common_UI_GTKState*,        // GTK state handle
                                 IRC_Client_GUI_Connection*, // connection handle
                                 IRC_Client_IIRCControl*,    // controller handle
                                 const std::string&,         // identifier (channel/nick)
                                 const std::string&,         // UI (glade) file directory
                                 GtkNotebook*);              // parent widget handle
  virtual ~IRC_Client_GUI_MessageHandler ();

  bool isServerLog () const;

  // *WARNING*: to be called from gtk_main context ONLY (trigger with g_idle_add())
  // DO NOT USE DO NOT USE DO NOT USE DO NOT USE DO NOT USE DO NOT USE DO NOT USE !
  // [cannot make this a private member :-(]
  void update ();

//   const std::string getChannel() const;

  // *WARNING*: any method below this point needs to be either:
  // - protected by gdk_threads_enter/gdk_threads_leave
  // - called from gtk_main context

  // display (local) text
  void queueForDisplay (const std::string&);

  // returns the toplevel widget of the channel page tab child
  // *NOTE*: the server log handler page doesn't have dynamic (!) children...
  GtkWidget* getTopLevelPageChild ();

  void setTopic (const std::string&);
  void setModes (const std::string&,  // mode string [i.e. "+|-x"]
                 const std::string&); // parameter, if any [i.e. <limit>,<user>,<ban mask>,...]

  // users
  void user (const std::string&,   // nick
             bool,                 // away ?
             bool,                 // IRC operator ?
             bool,                 // channel operator ?
             bool,                 // voiced ?
             unsigned int,         // hop count
             const std::string&);  // real name
  void endUsers ();

  // channel members
  void add (const std::string&);
  void remove (const std::string&);
  void members (const string_list_t&);
  void endMembers ();
  void updateNick (const std::string&); // previous nickname

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler ());
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler (const IRC_Client_GUI_MessageHandler&));
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler& operator=(const IRC_Client_GUI_MessageHandler&));

  // helper methods
  void clearMembers ();
  void updateModeButtons ();

  IRC_Client_GTK_HandlerCBData CBData_;

  std::deque<std::string>      displayQueue_;
  ACE_Thread_Mutex             lock_;

  std::string                  builderLabel_;
  guint                        eventSourceID_;
  bool                         isFirstMemberListMsg_;
  GtkNotebook*                 parent_;
  GtkTextView*                 view_;
};

#endif
