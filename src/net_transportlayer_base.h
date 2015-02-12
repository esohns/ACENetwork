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

#ifndef Net_TRANSPORTLAYER_BASE_H
#define Net_TRANSPORTLAYER_BASE_H

#include "net_exports.h"
#include "net_common.h"
#include "net_itransportlayer.h"

class Net_Export Net_TransportLayer_Base
 : virtual public Net_ITransportLayer
{
 public:
  virtual ~Net_TransportLayer_Base();

  virtual void init (Net_ClientServerRole_t, // role
                     unsigned short,             // port number
                     bool = false);              // use loopback device ?

  // implement (part of) Net_ITransportLayer
  virtual void info (ACE_HANDLE&,           // return value: I/O handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: remote SAP

 protected:
  Net_TransportLayer_Base(Net_ClientServerRole_t,
                              Net_TransportLayer_t);

  Net_ClientServerRole_t myClientServerRole;
  Net_TransportLayer_t   myTransportLayer;
  unsigned short             myPort;
  bool                       myUseLoopback;

 private:
  ACE_UNIMPLEMENTED_FUNC(Net_TransportLayer_Base());
  ACE_UNIMPLEMENTED_FUNC(Net_TransportLayer_Base(const Net_TransportLayer_Base&));
  ACE_UNIMPLEMENTED_FUNC(Net_TransportLayer_Base& operator=(const Net_TransportLayer_Base&));
};

#endif
