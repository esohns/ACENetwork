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

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include <gtk/gtk.h>

#include "common_iget.h"

//#include "stream_common.h"

//#include "net_common.h"
#include "net_defines.h"

#include "bittorrent_client_configuration.h"

//#include "bittorrent_client_gui_common.h"

// forward declarations
//struct BitTorrent_Client_SessionState;

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
class BitTorrent_Client_GUI_Session_T
 : public Common_IGetR_T<ConnectionCBDataType>
{
 public:
  BitTorrent_Client_GUI_Session_T (const struct BitTorrent_Client_Configuration&, // configuration
                                   const struct Common_UI_GTK_State&,             // GTK state
                                   guint,                                         // (statusbar) context id
                                   const std::string&,                            // (session log tab) label
                                   const std::string&,                            // UI (glade) file directory
                                   ///////
                                   BitTorrent_Client_IControl_t*,                 // controller handle
                                   const std::string&);                           // metainfo (aka '.torrent') file name
  // *WARNING*: must be called with
  //            BitTorrent_Client_GTK_CBData::Common_UI_GTKState::lock held !
  virtual ~BitTorrent_Client_GUI_Session_T ();

//  void initialize (struct BitTorrent_Client_SessionState*, // session state handle
//                   BitTorrent_IControl*);                  // controller handle
//  // *WARNING*: this requires gdk_threads_enter()/leave() protection !
//  void finalize (bool = true); // locked access ?
  void close ();

  // implement Common_IGet_T
  inline virtual const ConnectionCBDataType& getR () const { return CBData_; }

  // *WARNING*: callers may need protection from:
  //            - the thread(s) servicing the UI (GTK) event loop
  //            - the event dispatch thread(s) (reactor/proactor)
  //void current (std::string&,        // return value: nickname
  //              std::string&) const; // return value: channel / nickname
//  inline const struct BitTorrent_Client_SessionState& state () const { ACE_ASSERT (sessionState_); return *sessionState_; };

//  void createConnection (const ACE_INET_Addr&, // address
//                         bool = true,          // locked access ?
//                         bool = true);         // locked access (GDK) ?
//  // *IMPORTANT NOTE*: callers must invoke ConnectionType::decrease () when done
//  ConnectionType* getConnection (const ACE_INET_Addr&) const; // address
//  void terminateConnection (const ACE_INET_Addr&, // address
//                            bool = true);         // locked access ?

  bool                 closing_;

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T (const BitTorrent_Client_GUI_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T& operator= (const BitTorrent_Client_GUI_Session_T&))

  // helper methods
  void log (const std::string&);
//  void log (const struct BitTorrent_Record&);
//  void error (const struct BitTorrent_Record&,
//              bool = true); // locked access ?

  ConnectionCBDataType CBData_;
  guint                contextId_;
//  struct BitTorrent_Client_SessionState*     sessionState_;
  std::string          UIFileDirectory_;
};

// include template definition
#include "bittorrent_client_gui_session.inl"

#endif
