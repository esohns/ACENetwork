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

#ifndef IRC_Client_GUI_Connection_T_H
#define IRC_Client_GUI_Connection_T_H

#include <map>
#include <string>

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"

#include "common_ui_common.h"

#include "stream_common.h"

#include "irc_record.h"

#include "IRC_client_stream_common.h"

#include "IRC_client_gui_common.h"

// forward declaration(s)
class IRC_Client_GUI_IMessageHandler;
template <enum Common_UI_FrameworkType GUIType>
class IRC_Client_GUI_MessageHandler_T;

//////////////////////////////////////////

class IRC_Client_GUI_IConnection
 : public Common_IGetR_T<struct IRC_Client_UI_ConnectionCBData>
{
 public:
  inline IRC_Client_GUI_IConnection () {}
  inline virtual ~IRC_Client_GUI_IConnection () {}

  virtual void finalize (bool = true) = 0; // locked access ?

  virtual bool isClosing () const = 0;

  virtual const struct IRC_SessionState& state () const = 0;
#if defined (GTK_SUPPORT)  
  virtual gint exists (const std::string&,     // channel/nick
                       bool = true) const = 0; // locked access (GDK) ?
#endif // GTK_SUPPORT
  virtual void channels (string_list_t&) = 0; // return value: list of active channels

  virtual void createMessageHandler (const std::string&, // channel/nickname
                                     bool = true,        // locked access ?
                                     bool = true) = 0;   // locked access (GDK) ?
  virtual void terminateMessageHandler (const std::string&, // channel/nickname
                                        bool = true) = 0;   // locked access ?

  virtual IRC_Client_GUI_IMessageHandler* getActiveHandler (bool = true,            // locked access ?
                                                            bool = true) const = 0; // locked access (GDK) ?

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_IConnection (const IRC_Client_GUI_IConnection&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_IConnection& operator= (const IRC_Client_GUI_IConnection&))
};

//////////////////////////////////////////

template <enum Common_UI_FrameworkType GUIType>
class IRC_Client_GUI_Connection_T
 : public IRC_Client_GUI_IConnection
 , public IRC_Client_ISessionNotify_t
{
  friend class IRC_Client_GUI_MessageHandler_T<GUIType>;

 public:
  IRC_Client_GUI_Connection_T (IRC_Client_GUI_Connections_t*,       // connections handle
                               struct IRC_Client_UI_HandlerCBData*, // UI callback data handle
                               const std::string&,                  // (server tab) label
                               const std::string&);                 // UI (glade) file directory
  virtual ~IRC_Client_GUI_Connection_T ();

  void initialize (struct IRC_SessionState*, // session state handle
                   IRC_IControl*);           // controller handle
  virtual void finalize (bool = true); // locked access ?
  void close ();

  // implement IRC_Client_ISessionNotify_t
  virtual void start (Stream_SessionId_t,
                      const struct IRC_Client_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const IRC_Message&);
  virtual void notify (Stream_SessionId_t,
                       const IRC_Client_SessionMessage&);

  // implement Common_IGet_T
  inline virtual const struct IRC_Client_UI_ConnectionCBData& getR () const { return CBData_; }

  inline virtual bool isClosing () const { return closing_; };

  virtual void channels (string_list_t&); // return value: list of active channels
#if defined (GTK_SUPPORT)  
  inline virtual gint exists (const std::string&, // channel/nick
                              bool = true) const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }; // locked access (GDK) ?
#endif // GTK_SUPPORT

  // *WARNING*: callers may need protection from:
  //            - the thread(s) servicing the UI (GTK) event loop
  //            - the event dispatch thread(s) (reactor/proactor)
  //void current (std::string&,        // return value: nickname
  //              std::string&) const; // return value: channel / nickname
  inline virtual const struct IRC_SessionState& state () const { ACE_ASSERT (sessionState_); return *sessionState_; }
  virtual IRC_Client_GUI_IMessageHandler* getActiveHandler (bool = true,        // locked access ?
                                                            bool = true) const; // locked access (GDK) ?
  virtual void createMessageHandler (const std::string&, // channel/nickname
                                     bool = true,        // locked access ?
                                     bool = true);       // locked access (GDK) ?
  virtual void terminateMessageHandler (const std::string&, // channel/nickname
                                        bool = true);       // locked access ?

 private:
  typedef IRC_Client_GUI_MessageHandler_T<GUIType> MESSAGE_HANDLER_T;
  typedef IRC_Client_GUI_IMessageHandler IMESSAGE_HANDLER_T;
  typedef std::map<std::string,
                   IMESSAGE_HANDLER_T*> MESSAGE_HANDLERS_T;
  typedef typename MESSAGE_HANDLERS_T::const_iterator MESSAGE_HANDLERSCONSTITERATOR_T;
  typedef typename MESSAGE_HANDLERS_T::iterator MESSAGE_HANDLERSITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection_T ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection_T (const IRC_Client_GUI_Connection_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection_T& operator= (const IRC_Client_GUI_Connection_T&))

  // helper methods
  void forward (const std::string&,  // channel/nickname
                const std::string&); // message text
  void log (const std::string&);
  void log (const IRC_Record&);
  void error (const IRC_Record&,
              bool = true); // locked access ?

  IRC_Client_GUI_IMessageHandler* getHandler (const std::string&); // id (channel/nickname)

  struct IRC_Client_UI_ConnectionCBData CBData_;
  bool                                  closing_;
  bool                                  isFirstUsersMsg_;
  struct IRC_SessionState*              sessionState_;
  std::string                           UIFileDirectory_;

  mutable ACE_SYNCH_MUTEX               lock_;
  MESSAGE_HANDLERS_T                    messageHandlers_;
};

//////////////////////////////////////////

#if defined (GTK_SUPPORT)
// partial specialization (GTK)
template <>
class IRC_Client_GUI_Connection_T<COMMON_UI_FRAMEWORK_GTK>
 : public IRC_Client_GUI_IConnection
 , public IRC_Client_ISessionNotify_t
{
  friend class IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>;

 public:
  IRC_Client_GUI_Connection_T (IRC_Client_GUI_Connections_t*,       // connections handle
                               struct IRC_Client_UI_HandlerCBData*, // UI callback data handle
                               guint,                               // (statusbar) context id
                               const std::string&,                  // (server tab) label
                               const std::string&);                 // UI (glade) file directory
  // *WARNING*: must be called with
  //            IRC_Client_GTK_CBData::Common_UI_GTKState::lock held !
  virtual ~IRC_Client_GUI_Connection_T ();

  void initialize (struct IRC_SessionState*, // session state handle
                   IRC_IControl*);           // controller handle
  // *WARNING*: this requires gdk_threads_enter()/leave() protection !
  virtual void finalize (bool = true); // locked access ?
  void close ();

  // implement IRC_Client_ISessionNotify_t
  virtual void start (Stream_SessionId_t,
                      const struct IRC_Client_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const IRC_Message&);
  virtual void notify (Stream_SessionId_t,
                       const IRC_Client_SessionMessage&);

  // implement Common_IGet_T
  inline virtual const struct IRC_Client_UI_ConnectionCBData& getR () const { return CBData_; }

  inline virtual bool isClosing () const { return closing_; };

  // *NOTE*: a return value of -1 indicates non-existence
  virtual gint exists (const std::string&, // channel/nick
                       bool = true) const; // locked access (GDK) ?
  virtual void channels (string_list_t&); // return value: list of active channels

  // *WARNING*: callers may need protection from:
  //            - the thread(s) servicing the UI (GTK) event loop
  //            - the event dispatch thread(s) (reactor/proactor)
  //void current (std::string&,        // return value: nickname
  //              std::string&) const; // return value: channel / nickname
  inline virtual const struct IRC_SessionState& state () const { ACE_ASSERT (sessionState_); return *sessionState_; }
  virtual IRC_Client_GUI_IMessageHandler* getActiveHandler (bool = true,        // locked access ?
                                                            bool = true) const; // locked access (GDK) ?
  virtual void createMessageHandler (const std::string&, // channel/nickname
                                     bool = true,        // locked access ?
                                     bool = true);       // locked access (GDK) ?
  virtual void terminateMessageHandler (const std::string&, // channel/nickname
                                        bool = true);       // locked access ?

 private:
  typedef IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK> MESSAGE_HANDLER_T;
  typedef std::map<std::string,
                   MESSAGE_HANDLER_T*> MESSAGE_HANDLERS_T;
  typedef MESSAGE_HANDLERS_T::const_iterator MESSAGE_HANDLERSCONSTITERATOR_T;
  typedef MESSAGE_HANDLERS_T::iterator MESSAGE_HANDLERSITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection_T ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection_T (const IRC_Client_GUI_Connection_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_GUI_Connection_T& operator= (const IRC_Client_GUI_Connection_T&))

  // helper methods
  void forward (const std::string&,  // channel/nickname
                const std::string&); // message text
  void log (const std::string&);
  void log (const IRC_Record&);
  void error (const IRC_Record&,
              bool = true); // locked access ?

  IRC_Client_GUI_IMessageHandler* getHandler (const std::string&); // id (channel/nickname)

  struct IRC_Client_UI_ConnectionCBData CBData_;
  bool                                  closing_;
  guint                                 contextId_;
  bool                                  isFirstUsersMsg_;
  struct IRC_SessionState*              sessionState_;
  std::string                           UIFileDirectory_;

  mutable ACE_SYNCH_MUTEX               lock_;
  MESSAGE_HANDLERS_T                    messageHandlers_;
};
#endif // GTK_SUPPORT

//////////////////////////////////////////

// include template definition
#include "IRC_client_gui_connection.inl"

#endif
