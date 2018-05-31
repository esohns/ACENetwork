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

#include "ace/Asynch_IO.h"
#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Message_Block.h"
#include "ace/Notification_Strategy.h"

#include "common_iinitialize.h"
#include "common_referencecounter_base.h"

#include "net_sockethandler_base.h"

// forward declarations
class Stream_IAllocator;

template <typename SocketType, // implements ACE_SOCK
          typename ConfigurationType>
class Net_AsynchUDPSocketHandler_T
 : public Net_AsynchSocketHandlerBase_T<ConfigurationType>
 , public SocketType
 , public ACE_Service_Handler
 , public ACE_Notification_Strategy
 // *NOTE*: use this to modify the source/target address after initialization
 , public Common_IReset
{
  typedef Net_AsynchSocketHandlerBase_T<ConfigurationType> inherited;
  typedef SocketType inherited2;
  typedef ACE_Service_Handler inherited3;
  typedef ACE_Notification_Strategy inherited4;

 public:
  virtual ~Net_AsynchUDPSocketHandler_T ();

  // override (part of) ACE_Service_Handler
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  inline virtual void addresses (const ACE_INET_Addr& peerSap_in, const ACE_INET_Addr& localSAP_in) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual ACE_HANDLE handle (void) const { return SocketType::get_handle (); }
  inline virtual void handle (ACE_HANDLE handle_in) { inherited3::handle (handle_in); }

  // implement (part of) Net_IAsynchSocketHandler
  virtual void cancel ();

 protected:
  // convenient types
  typedef SocketType SOCKET_T;
  typedef ACE_Service_Handler SVC_HANDLER_T;

  Net_AsynchUDPSocketHandler_T ();

  // override (part of) ACE_Service_Handler
  virtual void handle_write_dgram (const ACE_Asynch_Write_Dgram::Result&); // result

  // implement (part of) Net_IAsynchSocketHandler
  virtual bool initiate_read ();

  ACE_INET_Addr               address_;
  // the number of open write (i.e. send) requests
  Common_ReferenceCounterBase counter_;
#if defined (ACE_LINUX)
  bool                        errorQueue_;
#endif // ACE_LINUX
  ACE_Asynch_Read_Dgram       inputStream_;
  ACE_Asynch_Write_Dgram      outputStream_;
  unsigned int                PDUSize_;
  // *NOTE*: used for read-write connections (i.e. NET_ROLE_CLIENT) only
  ACE_HANDLE                  writeHandle_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPSocketHandler_T (const Net_AsynchUDPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPSocketHandler_T& operator= (const Net_AsynchUDPSocketHandler_T&))

  // override/hide (part of) ACE_Service_Handler
  inline virtual void handle_wakeup () { cancel (); }

  // implement/hide ACE_Notification_Strategy
  virtual int notify (void);
  inline virtual int notify (ACE_Event_Handler*, ACE_Reactor_Mask) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
};

// include template definiton
#include "net_asynch_udpsockethandler.inl"

#endif
