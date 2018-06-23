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
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Message_Block.h"
#include "ace/Notification_Strategy.h"

#include "common_referencecounter_base.h"

#include "net_sockethandler_base.h"

// forward declarations
typedef unsigned long ACE_Reactor_Mask;
class ACE_Event_Handler;
class Stream_IAllocator;

template <typename ConfigurationType>
class Net_AsynchTCPSocketHandler_T
 : public Net_AsynchSocketHandlerBase_T<ConfigurationType>
 , public ACE_Service_Handler // implements ACE_Handler
 , public ACE_Notification_Strategy
{
  typedef Net_AsynchSocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Service_Handler inherited2;
  typedef ACE_Notification_Strategy inherited3;

 public:
  virtual ~Net_AsynchTCPSocketHandler_T ();

  // override (part of) ACE_Service_Handler
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  inline virtual void addresses (const ACE_INET_Addr& peerAddress_in, const ACE_INET_Addr& localAddress_in) { peerSAP_ = peerAddress_in; localSAP_ = localAddress_in; }

  virtual int handle_close (ACE_HANDLE,        // handle
                            ACE_Reactor_Mask); // event mask

  // implement (part of) Net_IAsynchSocketHandler
  virtual void cancel ();

 protected:
  // convenient types
  typedef Net_AsynchSocketHandlerBase_T<ConfigurationType> SOCKETHANDLER_BASE_T;

  Net_AsynchTCPSocketHandler_T ();

  // override some ACE_Handler methods
  virtual void handle_write_stream (const ACE_Asynch_Write_Stream::Result&); // result

  // implement (part of) Net_IAsynchSocketHandler
  virtual bool initiate_read ();

  // the number of open write (i.e. send) requests
  mutable Common_ReferenceCounterBase counter_;
  ACE_Asynch_Read_Stream              inputStream_;
  ACE_Asynch_Write_Stream             outputStream_;
  bool                                partialWrite_;
  unsigned int                        PDUSize_;
  ACE_INET_Addr                       localSAP_;
  ACE_INET_Addr                       peerSAP_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_HANDLE                          writeHandle_;
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPSocketHandler_T (const Net_AsynchTCPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPSocketHandler_T& operator= (const Net_AsynchTCPSocketHandler_T&))

  // implement/hide ACE_Notification_Strategy
  virtual int notify (void);
  inline virtual int notify (ACE_Event_Handler*, ACE_Reactor_Mask) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
};

// include template definition
#include "net_asynch_tcpsockethandler.inl"

#endif
