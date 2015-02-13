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

#ifndef NET_CLIENT_ASYNCHCONNECTOR_H
#define NET_CLIENT_ASYNCHCONNECTOR_H

#include "ace/Global_Macros.h"
#include "ace/Asynch_Connector.h"

#include "net_tcpconnection.h"

#include "net_client_exports.h"
#include "net_client_iconnector.h"

class Net_Client_Export Net_Client_AsynchConnector
 : public ACE_Asynch_Connector<Net_AsynchTCPConnection>
 , public Net_Client_IConnector
{
 public:
  Net_Client_AsynchConnector ();
  virtual ~Net_Client_AsynchConnector ();

  // override default creation strategy
  virtual Net_AsynchTCPConnection* make_handler (void);
  // override default connect strategy
  virtual int validate_connection (const ACE_Asynch_Connect::Result&, // result
                                   const ACE_INET_Addr&,              // remote address
                                   const ACE_INET_Addr&);             // local address

  // implement Net_Client_IConnector
  virtual void abort ();
  virtual void connect (const ACE_INET_Addr&);

 private:
  typedef ACE_Asynch_Connector<Net_AsynchTCPConnection> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector (const Net_Client_AsynchConnector&));
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector& operator= (const Net_Client_AsynchConnector&));
};

#endif
