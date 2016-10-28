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

#ifndef BITTORRENT_CLIENT_GUI_CONNECTION_H
#define BITTORRENT_CLIENT_GUI_CONNECTION_H

#include <map>
#include <string>

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include <gtk/gtk.h>

#include "common_iget.h"

#include "stream_common.h"

#include "bittorrent_common.h"

#include "bittorrent_client_gui_common.h"
#include "bittorrent_client_stream_common.h"

// forward declaration(s)
struct Common_UI_GTKState;
class BitTorrent_Client_GUI_MessageHandler;

/**
  @author Erik Sohns <eriksohns@123mail.org>
*/
class BitTorrent_Client_GUI_Connection
 : public BitTorrent_Client_ISessionNotify_t
 , public Common_IGet_T<BitTorrent_Client_GTK_ConnectionCBData>
{
  friend class BitTorrent_Client_GUI_MessageHandler;

 public:
  BitTorrent_Client_GUI_Connection (Common_UI_GTKState*,           // GTK state handle
                             BitTorrent_Client_GUI_Connections_t*, // connections handle
                             guint,                         // (statusbar) context ID
                             const std::string&,            // (server tab) label
                             const std::string&);           // UI (glade) file directory
  // *WARNING*: must be called with
  //            BitTorrent_Client_GTK_CBData::Common_UI_GTKState::lock held !
  virtual ~BitTorrent_Client_GUI_Connection ();

  void initialize (BitTorrent_Client_SessionState*, // session state handle
                   IRC_IControl*);           // controller handle
  // *WARNING*: this requires gdk_threads_enter()/leave() protection !
  void finalize (bool = true); // locked access ?
  void close ();

  // implement BitTorrent_Client_ISessionNotify_t
  virtual void start (Stream_SessionId_t,
                      const BitTorrent_Client_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const IRC_Message&);
  virtual void notify (Stream_SessionId_t,
                       const BitTorrent_Client_SessionMessage&);

  // implement Common_IGet_T
  virtual const BitTorrent_Client_GTK_ConnectionCBData& get () const;

  // *NOTE*: a return value of -1 indicates non-existence
  gint exists (const std::string&, // channel/nick
               bool = true) const; // locked access (GDK) ?
  void channels (string_list_t&); // return value: list of active channels

  // *WARNING*: callers may need protection from:
  //            - the thread(s) servicing the UI (GTK) event loop
  //            - the event dispatch thread(s) (reactor/proactor)
  //void current (std::string&,        // return value: nickname
  //              std::string&) const; // return value: channel / nickname
  const BitTorrent_Client_SessionState& state () const;
  BitTorrent_Client_GUI_MessageHandler* getActiveHandler (bool = true,        // locked access ?
                                                   bool = true) const; // locked access (GDK) ?
  void createMessageHandler (const std::string&, // channel/nickname
                             bool = true,        // locked access ?
                             bool = true);       // locked access (GDK) ?
  void terminateMessageHandler (const std::string&, // channel/nickname
                                bool = true);       // locked access ?

  bool closing_;

 private:
  typedef std::map<std::string,
                   BitTorrent_Client_GUI_MessageHandler* > MESSAGE_HANDLERS_T;
  typedef MESSAGE_HANDLERS_T::const_iterator MESSAGE_HANDLERSCONSTITERATOR_T;
  typedef MESSAGE_HANDLERS_T::iterator MESSAGE_HANDLERSITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Connection ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Connection (const BitTorrent_Client_GUI_Connection&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Connection& operator= (const BitTorrent_Client_GUI_Connection&))

  // helper methods
  void forward (const std::string&,  // channel/nickname
                const std::string&); // message text
  void log (const std::string&);
  void log (const IRC_Record&);
  void error (const IRC_Record&,
              bool = true); // locked access ?

  BitTorrent_Client_GUI_MessageHandler* getHandler (const std::string&); // id (channel/nickname)

  BitTorrent_Client_GTK_ConnectionCBData CBData_;
  guint                           contextID_;
  bool                            isFirstUsersMsg_;
  BitTorrent_Client_SessionState*        sessionState_;
  std::string                     UIFileDirectory_;

  mutable ACE_SYNCH_MUTEX         lock_;
  MESSAGE_HANDLERS_T              messageHandlers_;
};

#endif
