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

#ifndef NET_ASYNCH_TCP_SOCKETHANDLER_T_H
#define NET_ASYNCH_TCP_SOCKETHANDLER_T_H

#include "ace/Asynch_IO.h"
#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Message_Block.h"
#include "ace/Notification_Strategy.h"

#include "common_referencecounter_base.h"

#include "net_sockethandler_base.h"

template <typename ConfigurationType>
class Net_AsynchTCPSocketHandler_T
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Service_Handler
 , public ACE_Notification_Strategy
{
 public:
  virtual ~Net_AsynchTCPSocketHandler_T ();

  // override some ACE_Service_Handler methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  virtual void addresses (const ACE_INET_Addr&,  // remote address
                          const ACE_INET_Addr&); // local address
  //virtual void act (const void*); // act

  virtual int handle_output (ACE_HANDLE) = 0; // (socket) handle
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,                        // handle
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

  // implement ACE_Notification_Strategy
  virtual int notify (void);

 protected:
  Net_AsynchTCPSocketHandler_T ();

  // override some ACE_Handler methods
  virtual void handle_write_stream (const ACE_Asynch_Write_Stream::Result&); // result

  // helper method(s)
  bool initiate_read_stream ();

  // the number of open write (i.e. send) requests
  Common_ReferenceCounterBase counter_;
  ACE_Asynch_Read_Stream      inputStream_;
  ACE_Asynch_Write_Stream     outputStream_;
  bool                        partialWrite_;
  ACE_INET_Addr               localSAP_;
  ACE_INET_Addr               remoteSAP_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_HANDLE                  writeHandle_;
#endif

 private:
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Service_Handler inherited2;
  typedef ACE_Notification_Strategy inherited3;

  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPSocketHandler_T (const Net_AsynchTCPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPSocketHandler_T& operator= (const Net_AsynchTCPSocketHandler_T&))

  // implement ACE_Notification_Strategy
  virtual int notify (ACE_Event_Handler*, // event handler handle
                      ACE_Reactor_Mask);  // mask

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size
};

// include template implementation
#include "net_asynch_tcpsockethandler.inl"

#endif
