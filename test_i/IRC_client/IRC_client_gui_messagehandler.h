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

#ifndef IRC_Client_GUI_MessageHandler_T_H
#define IRC_Client_GUI_MessageHandler_T_H

#include <string>

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"

#include "common_ui_common.h"

#include "IRC_client_common.h"
#include "IRC_client_stream_common.h"

#include "IRC_client_gui_common.h"

// forward declaration(s)
class IRC_Client_GUI_IConnection;

class IRC_Client_GUI_IMessageHandler
 : public Common_IGetR_T<struct IRC_Client_UI_HandlerCBData>
{
 public:
  inline IRC_Client_GUI_IMessageHandler () {}
  inline virtual ~IRC_Client_GUI_IMessageHandler () {}

  virtual bool isServerLog () const = 0;

  virtual void update () = 0;

  // display (local) text
  virtual void queueForDisplay (const std::string&) = 0;

  virtual void setTopic (const std::string&) = 0;
  virtual void setModes (const std::string&, // mode string [i.e. "+|-x"]
                         const std::string&, // parameter, if any [i.e. <limit>,<user>,<ban mask>,...]
                         bool = true) = 0;   // locked access ?

  // users
  //virtual void user (const std::string&,         // nick
  //                   bool,                       // away ?
  //                   bool,                       // IRC operator ?
  //                   bool,                       // channel operator ?
  //                   bool,                       // voiced ?
  //                   unsigned int,               // hop count
  //                   const std::string&) = NULL; // real name
  //virtual void endUsers () = NULL;

  // channel members
  virtual void add (const std::string&, // nickname
                    bool = true) = 0;   // locked access ?
  virtual void remove (const std::string&, // nickname
                       bool = true) = 0;   // locked access ?
  virtual void members (const string_list_t&, // channel members
                        bool = true) = 0;     // locked access ?
  virtual void endMembers (bool = true) = 0; // locked access ?
  virtual void update (const std::string&) = 0; // previous nickname

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_IMessageHandler (const IRC_Client_GUI_IMessageHandler&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_IMessageHandler& operator= (const IRC_Client_GUI_IMessageHandler&))
};

//////////////////////////////////////////

template <enum Common_UI_FrameworkType GUIType>
class IRC_Client_GUI_MessageHandler_T
 : public IRC_Client_GUI_IMessageHandler
{
 public:
  // ctor for default handler (== server log)
  IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection*,          // connection handle
                                   const std::string&,                   // connection timestamp
                                                                         // *NOTE*: used to lookup the corresponding builder
                                   struct IRC_Client_UI_HandlerCBData*); // UI callback data handle
  // ctor for regular channel handler
  IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection*,          // connection handle
                                   IRC_IControl*,                        // controller handle
                                   const std::string&,                   // identifier (channel/nick)
                                   const std::string&,                   // UI (glade) file directory
                                   const std::string&,                   // connection timestamp
                                                                         // *NOTE*: used to lookup the corresponding builder
                                   struct IRC_Client_UI_HandlerCBData*); // UI callback data handle
  virtual ~IRC_Client_GUI_MessageHandler_T ();

  // implement Common_IGet_T
  inline virtual const IRC_Client_UI_HandlerCBData& getR () const { ACE_ASSERT (CBData_); return *CBData_; }

  inline bool isPrivateDialog () const { return isPrivateDialog_; }
  inline virtual bool isServerLog () const { ACE_ASSERT (CBData_); return CBData_->id.empty (); }

  // [cannot make this private :-(]
  virtual void update ();

  // display (local) text
  virtual void queueForDisplay (const std::string&);

  virtual void setTopic (const std::string&);
  virtual void setModes (const std::string&, // mode string [i.e. "+|-x"]
                         const std::string&, // parameter, if any [i.e. <limit>,<user>,<ban mask>,...]
                         bool = true);       // locked access ?

  // users
  //virtual void user (const std::string&,   // nick
  //                   bool,                 // away ?
  //                   bool,                 // IRC operator ?
  //                   bool,                 // channel operator ?
  //                   bool,                 // voiced ?
  //                   unsigned int,         // hop count
  //                   const std::string&);  // real name
  //virtual void endUsers ();

  // channel members
  virtual void add (const std::string&, // nickname
                    bool = true);       // locked access ?
  virtual void remove (const std::string&, // nickname
                       bool = true);       // locked access ?
  virtual void members (const string_list_t&, // channel members
                        bool = true);         // locked access ?
  virtual void endMembers (bool = true); // locked access ?
  virtual void update (const std::string&); // previous nickname

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler_T (const IRC_Client_GUI_MessageHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler_T& operator= (const IRC_Client_GUI_MessageHandler_T&))

  // helper methods
  void clearMembers (bool = true); // locked access ?

  struct IRC_Client_UI_HandlerCBData* CBData_;
  bool                                isFirstMemberListMsg_;
  bool                                isPrivateDialog_;
  IRC_Client_MessageQueue_t           messageQueue_;
  ACE_SYNCH_MUTEX                     messageQueueLock_;
};

//////////////////////////////////////////

#if defined (GTK_SUPPORT)
// partial specialization (GTK)
template <>
class IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>
 : public IRC_Client_GUI_IMessageHandler
{
 public:
  // ctor for default handler (== server log)
  IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection*,          // connection handle
                                   const std::string&,                   // connection timestamp
                                                                         // *NOTE*: used to lookup the corresponding builder
                                   struct IRC_Client_UI_HandlerCBData*); // UI callback data handle
  // ctor for regular channel handler
  // *WARNING*: must be called with
  //            IRC_Client_UI_CBData::Common_UI_GTKState::lock held !
  IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection*,         // connection handle
                                   IRC_IControl*,                       // controller handle
                                   const std::string&,                  // identifier (channel/nick)
                                   const std::string&,                  // UI (glade) file directory
                                   const std::string&,                  // connection timestamp
                                                                        // *NOTE*: used to lookup the corresponding builder
                                   struct IRC_Client_UI_HandlerCBData*, // UI callback data handle
                                   bool = true);                        // locked access (GDK) ?
  // *WARNING*: must be called with
  //            IRC_Client_UI_CBData::Common_UI_GTKState::lock held !
  virtual ~IRC_Client_GUI_MessageHandler_T ();

  // implement Common_IGet_T
  inline virtual const IRC_Client_UI_HandlerCBData& getR () const { ACE_ASSERT (CBData_); return *CBData_; }

  inline bool isPrivateDialog () const { return isPrivateDialog_; }
  inline bool isServerLog () const { ACE_ASSERT (CBData_); return CBData_->id.empty (); }

  // *WARNING*: to be called from gtk_main context ONLY (trigger with
  //            g_idle_add())
  // [cannot make this private :-(]
  virtual void update ();

//   const std::string getChannel() const;

  // *WARNING*: any method below this point needs to be either:
  // - protected by gdk_threads_enter/gdk_threads_leave
  // - called from gtk_main context

  // display (local) text
  virtual void queueForDisplay (const std::string&);

  // returns the toplevel widget of the channel page tab child
  // *NOTE*: the server log handler page doesn't have dynamic (!) children...
  // *WARNING*: this requires gdk_threads_enter()/leave() protection !
  GtkWidget* getTopLevelPageChild (bool = true) const; // locked access ?

  virtual void setTopic (const std::string&);
  virtual void setModes (const std::string&, // mode string [i.e. "+|-x"]
                         const std::string&, // parameter, if any [i.e. <limit>,<user>,<ban mask>,...]
                         bool = true);       // locked access ?

  // users
  //virtual void user (const std::string&,   // nick
  //                   bool,                 // away ?
  //                   bool,                 // IRC operator ?
  //                   bool,                 // channel operator ?
  //                   bool,                 // voiced ?
  //                   unsigned int,         // hop count
  //                   const std::string&);  // real name
  //virtual void endUsers ();

  // channel members
  virtual void add (const std::string&, // nickname
                    bool = true);       // locked access ?
  virtual void remove (const std::string&, // nickname
                       bool = true);       // locked access ?
  virtual void members (const string_list_t&, // channel members
                        bool = true);         // locked access ?
  virtual void endMembers (bool = true); // locked access ?
  // *WARNING*: callers may need protection from:
  //            - the thread(s) servicing the UI (GTK) event loop
  //            - the event dispatch thread(s) (reactor/proactor)
  virtual void update (const std::string&); // previous nickname

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler_T (const IRC_Client_GUI_MessageHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_MessageHandler_T& operator= (const IRC_Client_GUI_MessageHandler_T&))

  // helper methods
  void clearMembers (bool = true); // locked access ?

  struct IRC_Client_UI_HandlerCBData* CBData_;
  bool                                isFirstMemberListMsg_;
  bool                                isPrivateDialog_;
  IRC_Client_MessageQueue_t           messageQueue_;
  ACE_SYNCH_MUTEX                     messageQueueLock_;
  GtkTextView*                        view_; // --> server log
};
#endif // GTK_SUPPORT

//////////////////////////////////////////

// include template definition
#include "IRC_client_gui_messagehandler.inl"

#endif
