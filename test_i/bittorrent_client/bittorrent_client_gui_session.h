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

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"

#include "net_defines.h"

#include "bittorrent_client_configuration.h"

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
class BitTorrent_Client_GUI_Session_T
 : public BitTorrent_ISessionProgress
 , public Common_IGetR_T<ConnectionCBDataType>
{
 public:
  BitTorrent_Client_GUI_Session_T (struct BitTorrent_Client_UI_CBData&, // in/out: UI callback data
#if defined (GTK_USE)
                                   guint,                               // (statusbar) context id
#endif // GTK_USE
                                   const std::string&,                  // (session log tab) label
                                   const std::string&,                  // UI (glade) file directory
                                   ///////
                                   BitTorrent_Client_IControl_t*,       // controller handle
                                   const std::string&);                 // metainfo (aka '.torrent') file name
  // *WARNING*: must be called with
  //            BitTorrent_Client_UI_CBData::Common_UI_GTKState::lock held !
  virtual ~BitTorrent_Client_GUI_Session_T ();

  void close ();

  // implement BitTorrent_ISessionProgress
  virtual void log (const std::string&); // log message
  inline virtual void numberOfPieces (unsigned int) {}
  virtual void pieceComplete (unsigned int); // piece index
  virtual void complete (bool = false); // cancelled ?
  inline virtual void trackerConnect (Net_ConnectionId_t) {}
  inline virtual void trackerDisconnect (Net_ConnectionId_t) {}
  inline virtual void peerConnect (Net_ConnectionId_t) {}
  inline virtual void peerDisconnect (Net_ConnectionId_t) {}

  // implement Common_IGet_T
  inline virtual const ConnectionCBDataType& getR () const { return CBData_; }

  bool                 closing_;

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T (const BitTorrent_Client_GUI_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_GUI_Session_T& operator= (const BitTorrent_Client_GUI_Session_T&))

  ConnectionCBDataType CBData_;
#if defined (GTK_USE)
  guint                contextId_;
#endif // GTK_USE
  std::string          UIFileDirectory_;
};

// include template definition
#include "bittorrent_client_gui_session.inl"

#endif
