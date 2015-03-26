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

#ifndef NET_NETLINKCONNECTION_H
#define NET_NETLINKCONNECTION_H

#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Netlink_Addr.h"
#include "ace/SOCK_Connector.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_exports.h"
#include "net_itransportlayer.h"
#include "net_socket_common.h"
#include "net_socketconnection_base.h"
#include "net_stream_common.h"
#include "net_transportlayer_netlink.h"

// forward declarations
template <typename SVC_HANDLER,
          typename PEER_CONNECTOR> class ACE_Connector;
template <class HANDLER> class ACE_Asynch_Connector;

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
class Net_Export Net_NetlinkConnection
 : public Net_SocketConnectionBase_T<Net_NetlinkHandler_t,
                                     Net_INetlinkTransportLayer_t,
                                     Net_Configuration_t,
                                     Net_SocketHandlerConfiguration_t,
                                     Net_UserData_t,
                                     Net_StreamSessionData_t,
                                     Stream_Statistic_t>
 , public Net_TransportLayer_Netlink
{
  friend class ACE_Connector<Net_NetlinkConnection,
                             ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<Net_Configuration_t,
                                   Net_UserData_t,
                                   Stream_Statistic_t,
                                   Net_INetlinkTransportLayer_t> ICONNECTION_MANAGER_T;

   Net_NetlinkConnection (ICONNECTION_MANAGER_T*, // connection manager handle
                          unsigned int = 0);      // statistics collecting interval (second(s))
                                                  // 0 --> DON'T collect statistics
   virtual ~Net_NetlinkConnection ();

   // override some task-based members
//   virtual int open (void* = NULL); // args
   //  // *NOTE*: enqueue any received data onto our stream for further processing
   //   virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
   //  // *NOTE*: send any enqueued data back to the client...
   //  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
   // *NOTE*: this is called when:
   // - handle_xxx() returns -1
   virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                             ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

   // override / implement (part of) Net_INetlinkTransportLayer
   virtual bool initialize (Net_ClientServerRole_t,            // role
                            const Net_SocketConfiguration_t&); // socket configuration
   virtual void finalize ();
   virtual void info (ACE_HANDLE&,              // return value: handle
                      ACE_Netlink_Addr&,        // return value: local SAP
                      ACE_Netlink_Addr&) const; // return value: remote SAP
   virtual unsigned int id () const;
   virtual void dump_state () const;

 private:
  typedef Net_SocketConnectionBase_T<Net_NetlinkHandler_t,
                                     Net_INetlinkTransportLayer_t,
                                     Net_Configuration_t,
                                     Net_SocketHandlerConfiguration_t,
                                     Net_UserData_t,
                                     Net_StreamSessionData_t,
                                     Stream_Statistic_t> inherited;
  typedef Net_TransportLayer_Netlink inherited2;

  // *TODO*: remove this ASAP
  Net_NetlinkConnection ();
//  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkConnection ());
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkConnection (const Net_NetlinkConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkConnection& operator= (const Net_NetlinkConnection&));
};

/////////////////////////////////////////

class Net_Export Net_AsynchNetlinkConnection
 : public Net_AsynchSocketConnectionBase_T<Net_AsynchNetlinkHandler_t,
                                           Net_INetlinkTransportLayer_t,
                                           Net_Configuration_t,
                                           Net_SocketHandlerConfiguration_t,
                                           Net_UserData_t,
                                           Stream_SessionData_t,
                                           Stream_Statistic_t>
 , public Net_TransportLayer_Netlink
{
 friend class ACE_Asynch_Connector<Net_AsynchNetlinkConnection>;

 public:
  typedef Net_IConnectionManager_T<Net_Configuration_t,
                                   Net_UserData_t,
                                   Stream_Statistic_t,
                                   Net_INetlinkTransportLayer_t> ICONNECTION_MANAGER_T;

  Net_AsynchNetlinkConnection (ICONNECTION_MANAGER_T*, // connection manager handle
                               unsigned int = 0);      // statistics collecting interval (second(s))
                                                       // 0 --> DON'T collect statistics
  virtual ~Net_AsynchNetlinkConnection ();

  // implement (part of) Net_INetlinkTransportLayer
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // configuration
  virtual void finalize ();
  virtual void info (ACE_HANDLE&,              // return value: handle
                     ACE_Netlink_Addr&,        // return value: local SAP
                     ACE_Netlink_Addr&) const; // return value: remote SAP
  virtual unsigned int id () const;
  virtual void dump_state () const;

  // override some ACE_Service_Handler members
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)
  //  // *NOTE*: enqueue any received data onto our stream for further processing
  //   virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
  //  // *NOTE*: send any enqueued data back to the client...
  //  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
    // *NOTE*: this is called when:
    // - handle_xxx() returns -1
    virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                              ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 private:
  typedef Net_AsynchSocketConnectionBase_T<Net_AsynchNetlinkHandler_t,
                                           Net_INetlinkTransportLayer_t,
                                           Net_Configuration_t,
                                           Net_SocketHandlerConfiguration_t,
                                           Net_UserData_t,
                                           Stream_SessionData_t,
                                           Stream_Statistic_t> inherited;
  typedef Net_TransportLayer_Netlink inherited2;

  // *TODO*: remove this ASAP
  Net_AsynchNetlinkConnection ();
  //  ACE_UNIMPLEMENTED_FUNC (Net_AsynchNetlinkConnection ());
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchNetlinkConnection (const Net_AsynchNetlinkConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchNetlinkConnection& operator= (const Net_AsynchNetlinkConnection&));
};
#endif

#endif
