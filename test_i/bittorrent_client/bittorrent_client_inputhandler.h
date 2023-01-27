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

#ifndef BITTORRENT_CLIENT_INPUTHANDLER_H
#define BITTORRENT_CLIENT_INPUTHANDLER_H

#include <ace/Event_Handler.h>
#include <ace/Global_Macros.h>

#include "common_iinitialize.h"

#include "net_defines.h"

#include "bittorrent_defines.h"

#include "bittorrent_client_configuration.h"

// forward declarations
class ACE_Message_Block;
struct IRC_SessionState;

class BitTorrent_Client_InputHandler
 : public ACE_Event_Handler
 , public Common_IInitialize_T<BitTorrent_Client_InputHandlerConfiguration>
{
 public:
  BitTorrent_Client_InputHandler (BitTorrent_Client_SessionState*, // state handle
                                  bool = NET_EVENT_USE_REACTOR);   // use reactor ?
  virtual ~BitTorrent_Client_InputHandler ();

  // implement Common_IInitialize_T
  virtual bool initialize (const BitTorrent_Client_InputHandlerConfiguration&); // configuration

  // implement (part of) ACE_Event_Handler
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE); // handle
  // *WARNING*: do NOT call this manually !
  // *TODO*: make this private
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

 private:
  typedef ACE_Event_Handler inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_InputHandler ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_InputHandler (const BitTorrent_Client_InputHandler&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_InputHandler& operator= (const BitTorrent_Client_InputHandler&))

  ACE_Message_Block* allocateMessage (unsigned int = IRC_BUFFER_SIZE); // size

  BitTorrent_Client_InputHandlerConfiguration configuration_;
  ACE_Message_Block*                          currentReadBuffer_;
  bool                                        registered_;
  BitTorrent_Client_SessionState*             state_;
  bool                                        useReactor_;
};

#endif
