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

#ifndef Net_STREAM_ASYNCH_TCPSOCKET_BASE_H
#define Net_STREAM_ASYNCH_TCPSOCKET_BASE_H

#include "ace/config-lite.h"
#include "ace/Event_Handler.h"
#include "ace/Message_Block.h"
#include "ace/Asynch_IO.h"

#include "net_stream_common.h"

template <typename ConfigurationType,
          typename StatisticsContainerType,
          typename StreamType,
          typename SocketHandlerType>
class Net_StreamAsynchTCPSocketBase_T
 : public SocketHandlerType
 , public ACE_Event_Handler
{
 public:
  Net_StreamAsynchTCPSocketBase_T ();
  virtual ~Net_StreamAsynchTCPSocketBase_T ();

  // override some service methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  virtual int handle_output (ACE_HANDLE); // (socket) handle
  virtual int handle_close (ACE_HANDLE,        // (socket) handle
                            ACE_Reactor_Mask); // (select) mask
  virtual void act (const void*);

 protected:
  virtual void handle_read_stream (const ACE_Asynch_Read_Stream::Result&); // result

//  // *TODO*: (try to) handle short writes gracefully...
//  ACE_Message_Block* buffer_;
  Net_StreamSocketConfiguration_t configuration_;
  StreamType                      stream_;

 private:
  typedef SocketHandlerType inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchTCPSocketBase_T (const Net_StreamAsynchTCPSocketBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchTCPSocketBase_T& operator= (const Net_StreamAsynchTCPSocketBase_T&));

  ConfigurationType*              userData_;
};

// include template definition
#include "net_stream_asynch_tcpsocket_base.inl"

#endif
