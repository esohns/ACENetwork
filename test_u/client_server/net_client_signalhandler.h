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

#ifndef Client_SignalHandler_H
#define Client_SignalHandler_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "common.h"
#include "common_isignal.h"
#include "common_signal_handler.h"

#include "net_common.h"

#include "net_client_common.h"
#include "net_client_connector_common.h"

class Client_SignalHandler
 : public Common_SignalHandler_T<struct Client_SignalHandlerConfiguration>
{
  typedef Common_SignalHandler_T<struct Client_SignalHandlerConfiguration> inherited;

 public:
  Client_SignalHandler (enum Common_EventDispatchType,  // event dispatch mode
                        enum Common_SignalDispatchType, // signal dispatch mode
                        ACE_SYNCH_RECURSIVE_MUTEX*);    // lock handle
  inline virtual ~Client_SignalHandler () {}

  // override Common_IInitialize_T
  virtual bool initialize (const struct Client_SignalHandlerConfiguration&);

  // implement Common_ISignal
  virtual void handle (const struct Common_Signal&); // signal

 private:
  ACE_UNIMPLEMENTED_FUNC (Client_SignalHandler ())
  ACE_UNIMPLEMENTED_FUNC (Client_SignalHandler (const Client_SignalHandler&))
  ACE_UNIMPLEMENTED_FUNC (Client_SignalHandler& operator= (const Client_SignalHandler&))

  ACE_INET_Addr                 address_;
  enum Common_EventDispatchType eventDispatch_;
  long                          timerId_;

  Client_TCP_AsynchConnector_t  AsynchTCPConnector_;
  Client_TCP_Connector_t        TCPConnector_;
  Client_UDP_AsynchConnector_t  AsynchUDPConnector_;
  Client_UDP_Connector_t        UDPConnector_;
#if defined (SSL_SUPPORT)
  Client_SSL_Connector_t        SSLConnector_;
#endif // SSL_SUPPORT
};

#endif
