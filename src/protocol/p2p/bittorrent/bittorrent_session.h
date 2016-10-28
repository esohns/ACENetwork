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

#ifndef BITTORRENT_SESSION_H
#define BITTORRENT_SESSION_H

#include <string>

#include <ace/Asynch_Connector.h>
#include <ace/config-macros.h>
#include <ace/Connector.h>
#include <ace/Global_Macros.h>
#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>

#include "stream_common.h"

#include "net_iconnection.h"

#include "bittorrent_common.h"
#include "bittorrent_network.h"
#include "bittorrent_stream_common.h"

// forward declarations
class ACE_Message_Block;

template <typename ConnectionType,
          typename SessionDataType,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType, // ui state (curses/gtk/...) *TODO*: to be removed
          ///////////////////////////////
          // *TODO*: remove these ASAP
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType> // *NOTE*: needs to inherit from ACE_FILE_IO
class BitTorrent_Session_T
 : public ConnectionType
 //, virtual public Net_ISession_T<ACE_INET_Addr,
 //                                Net_SocketConfiguration,
 //                                BitTorrent_Client_Configuration,
 //                                Stream_Statistic,
 //                                BitTorrent_Client_Stream,
 //                                BitTorrent_Client_ConnectionState>
 , public NotificationType
{
 friend class ACE_Connector<BitTorrent_Session_T<ConnectionType,
                                                 SessionDataType,
                                                 NotificationType,
                                                 ConfigurationType,
                                                 SessionMessageType,
                                                 SocketHandlerConfigurationType,
                                                 ModuleHandlerConfigurationType,
                                                 StateType,
                                                 ConnectionManagerType,
                                                 InputHandlerType,
                                                 InputHandlerConfigurationType,
                                                 LogOutputType>,
                            ACE_SOCK_CONNECTOR>;
 friend class ACE_Asynch_Connector<BitTorrent_Session_T<ConnectionType,
                                                        SessionDataType,
                                                        NotificationType,
                                                        ConfigurationType,
                                                        SessionMessageType,
                                                        SocketHandlerConfigurationType,
                                                        ModuleHandlerConfigurationType,
                                                        StateType,
                                                        ConnectionManagerType,
                                                        InputHandlerType,
                                                        InputHandlerConfigurationType,
                                                        LogOutputType> >;

 public:
  BitTorrent_Session_T (ConnectionManagerType* = NULL, // connection manager handle
                        unsigned int = 0);             // statistic collecting interval (second(s)) [0: off]
  virtual ~BitTorrent_Session_T ();

  // implement Net_ISession_T
  //virtual const BitTorrent_SessionState& state () const;

  // implement Common_INotify_T
  virtual void start (const SessionDataType&);
  virtual void notify (const BitTorrent_Record&);
  virtual void notify (const SessionMessageType&);
  virtual void end ();

  // override some task-based members
  // *TODO*: make these private
  virtual int open (void* = NULL); // arg
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)

 private:
  typedef ConnectionType inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T (const BitTorrent_Session_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Session_T& operator= (const BitTorrent_Session_T&))

  void error (const BitTorrent_Record&);
  void log (const BitTorrent_Record&);
  void log (const std::string&,  // channel (empty ? server log : channel)
            const std::string&); // text

  bool              close_;
  InputHandlerType* inputHandler_;
  bool              logToFile_;
  LogOutputType     output_;
  bool              shutDownOnEnd_;
  StateType*        UIState_;
};

// include template definition
#include "bittorrent_session.inl"

#endif
