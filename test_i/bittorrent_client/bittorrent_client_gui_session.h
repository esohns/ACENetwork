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

#ifndef BitTorrent_Client_GUI_Session_T_H
#define BitTorrent_Client_GUI_Session_T_H

#include <map>
#include <string>

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include <gtk/gtk.h>

#include "common_iget.h"

#include "stream_common.h"

#include "bittorrent_common.h"

#include "bittorrent_client_gui_common.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

template <typename ConnectionType>
class BitTorrent_Client_GUI_Session_T
 : public BitTorrent_Client_IPeerNotify_t
 , public Common_IGet_T<struct BitTorrent_Client_GTK_SessionCBData>
{
 public:
  BitTorrent_Client_GUI_Session_T (struct BitTorrent_Client_GTK_SessionCBData*, // GTK state handle
                                   BitTorrent_Client_GUI_Sessions_t*,           // sessions handle
                                   guint,                                       // (statusbar) context ID
                                   const std::string&,                          // (server tab) label
                                   const std::string&);                         // UI (glade) file directory
  // *WARNING*: must be called with
  //            BitTorrent_Client_GTK_CBData::Common_UI_GTKState::lock held !
  virtual ~BitTorrent_Client_GUI_Session_T ();

  void initialize (struct BitTorrent_Client_SessionState*, // session state handle
                   BitTorrent_IControl*);                  // controller handle
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
                       const BitTorrent_Client_PeerMessage&);
  virtual void notify (Stream_SessionId_t,
                       const BitTorrent_Client_SessionMessage&);

  // implement Common_IGet_T
  virtual const struct BitTorrent_Client_GTK_SessionCBData& get () const;

  // *WARNING*: callers may need protection from:
  //            - the thread(s) servicing the UI (GTK) event loop
  //            - the event dispatch thread(s) (reactor/proactor)
  //void current (std::string&,        // return value: nickname
  //              std::string&) const; // return value: channel / nickname
  const struct BitTorrent_Client_SessionState& state () const;
  ConnectionType* getConnection (bool = true,        // locked access ?
                                 bool = true) const; // locked access (GDK) ?
  ConnectionType* getConnection (const std::string&); // id (channel/nickname)
  void createConnection (const std::string&, // channel/nickname
                         bool = true,        // locked access ?
                         bool = true);       // locked access (GDK) ?
  void terminateConnection (const std::string&, // channel/nickname
                            bool = true);       // locked access ?

  bool closing_;

 private:
  typedef std::map<std::string,
                   ConnectionType*> CONNECTIONS_T;
  typedef typename CONNECTIONS_T::const_iterator CONNECTIONS_CONSTITERATOR_T;
  typedef typename CONNECTIONS_T::iterator CONNECTIONS_ITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T (const BitTorrent_Client_GUI_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T& operator= (const BitTorrent_Client_GUI_Session_T&))

  // helper methods
  void forward (const std::string&,  // channel/nickname
                const std::string&); // message text
  void log (const std::string&);
  void log (const struct BitTorrent_Record&);
  void error (const struct BitTorrent_Record&,
              bool = true); // locked access ?

  struct BitTorrent_Client_GTK_SessionCBData* CBData_;
  guint                                       contextID_;
  bool                                        isFirstUsersMsg_;
  struct BitTorrent_Client_SessionState*      sessionState_;
  std::string                                 UIFileDirectory_;

  mutable ACE_SYNCH_MUTEX                     lock_;
  CONNECTIONS_T                               messageHandlers_;
};

// include template definition
#include "bittorrent_client_gui_session.inl"

#endif
