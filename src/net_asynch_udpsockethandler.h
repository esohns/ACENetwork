/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef NET_ASYNCHUDPSOCKETHANDLER_H
#define NET_ASYNCHUDPSOCKETHANDLER_H

#include "net_exports.h"
#include "net_common.h"

#include "ace/Asynch_IO.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

class Net_Export Net_AsynchUDPSocketHandler
 : public ACE_Handler
{
 public:
  Net_AsynchUDPSocketHandler ();
  virtual ~Net_AsynchUDPSocketHandler ();

  //// implement (part of) Net_IConnection
  //virtual void ping ();

  // override some handler method(s)
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)

 private:
  typedef ACE_Handler inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPSocketHandler (const Net_AsynchUDPSocketHandler&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPSocketHandler& operator= (const Net_AsynchUDPSocketHandler&));
};

#endif
