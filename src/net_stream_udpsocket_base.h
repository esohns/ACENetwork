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

#ifndef Net_STREAM_UDPSOCKET_BASE_H
#define Net_STREAM_UDPSOCKET_BASE_H

#include "net_stream_common.h"
#include "net_sockethandler_base.h"

#include "ace/config-lite.h"
#include "ace/INET_Addr.h"
#include "ace/Event_Handler.h"
#include "ace/Message_Block.h"
#include "ace/Synch.h"

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
class Net_StreamUDPSocketBase_T
 : public SocketType
 , public SocketHandlerType
{
 public:
  int open (const ConfigurationType&, // configuration
            const ACE_INET_Addr&);    // peer address
  //virtual int close(u_long = 0); // args

  // *NOTE*: enqueue any received data onto our stream for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

  // implement Common_IStatistic
  // *NOTE*: delegate these to our stream
  virtual bool collect (StatisticsContainerType&) const; // return value: statistic data
  virtual void report () const;

 protected:
  Net_StreamUDPSocketBase_T ();
  virtual ~Net_StreamUDPSocketBase_T ();

  Net_StreamSocketConfiguration_t configuration_;
  StreamType                      stream_;
  ACE_Message_Block*              currentReadBuffer_;
  ACE_Thread_Mutex                sendLock_;
  ACE_Message_Block*              currentWriteBuffer_;
  // *IMPORTANT NOTE*: in a threaded environment, workers MAY
  // dispatch the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
  bool                            serializeOutput_;

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size

 private:
  typedef SocketType inherited;
  typedef SocketHandlerType inherited2;

//  ACE_UNIMPLEMENTED_FUNC (Net_StreamSocketBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&));
};

// include template implementation
#include "net_stream_udpsocket_base.inl"

#endif
