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

#ifndef NET_ASYNCH_UDP_SOCKETHANDLER_H
#define NET_ASYNCH_UDP_SOCKETHANDLER_H

#include <ace/Asynch_IO.h>
#include <ace/Event_Handler.h>
#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/Message_Block.h>
#include <ace/Notification_Strategy.h>

#include "common_referencecounter_base.h"

#include "net_sockethandler_base.h"

template <typename ConfigurationType>
class Net_AsynchUDPSocketHandler_T
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Service_Handler
 , public ACE_Notification_Strategy
{
 public:
  virtual ~Net_AsynchUDPSocketHandler_T ();

  // override some handler method(s)
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  virtual void addresses (const ACE_INET_Addr&,  // remote address
                          const ACE_INET_Addr&); // local address
  virtual void handle_wakeup ();

  // implement ACE_Notification_Strategy
  virtual int notify (void);
  virtual int notify (ACE_Event_Handler*, // event handler handle
                      ACE_Reactor_Mask);  // mask

 protected:
  Net_AsynchUDPSocketHandler_T ();

  // helper method(s)
  bool initiate_read_dgram ();
  virtual int handle_close (ACE_HANDLE handle_in,                                           // (socket) handle
                            ACE_Reactor_Mask mask_in = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask
  virtual int handle_output (ACE_HANDLE) = 0; // (socket) handle

  virtual void handle_write_dgram (const ACE_Asynch_Write_Dgram::Result&); // result

  //ACE_Message_Block*          buffer_;
  // the number of open write (i.e. send) requests
  Common_ReferenceCounterBase counter_;
  ACE_Asynch_Read_Dgram       inputStream_;
  ACE_Asynch_Write_Dgram      outputStream_;
  bool                        writeOnly_;

 private:
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Service_Handler inherited2;
  typedef ACE_Notification_Strategy inherited3;

  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPSocketHandler_T (const Net_AsynchUDPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPSocketHandler_T& operator= (const Net_AsynchUDPSocketHandler_T&))

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size
};

// include template implementation
#include "net_asynch_udpsockethandler.inl"

#endif
