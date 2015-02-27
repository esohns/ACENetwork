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

#ifndef Net_InetTransportLayer_Base_H
#define Net_InetTransportLayer_Base_H

#include "ace/Global_Macros.h"

#include "net_exports.h"
#include "net_common.h"
#include "net_itransportlayer.h"

class Net_Export Net_InetTransportLayer_Base
 : virtual public Net_IInetTransportLayer_t
{
 public:
  virtual ~Net_InetTransportLayer_Base ();

  virtual void initialize (Net_ClientServerRole_t, // role
                           const ACE_INET_Addr&);  // target address

 protected:
  Net_InetTransportLayer_Base (Net_ClientServerRole_t,
                               Net_TransportLayer_t);

  Net_ClientServerRole_t clientServerRole_;
  Net_TransportLayer_t   transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base ());
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base (const Net_InetTransportLayer_Base&));
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base& operator= (const Net_InetTransportLayer_Base&));

  // implement (part of) Net_ITransportLayer_T
  virtual void initialize (const ACE_INET_Addr&); // target address
};

class Net_Export Net_NetlinkTransportLayer_Base
 : virtual public Net_INetlinkTransportLayer_t
{
 public:
  virtual ~Net_NetlinkTransportLayer_Base ();

  // implement (part of) Net_ITransportLayer_T
  virtual void initialize (const ACE_INET_Addr&); // target address

 protected:
  Net_NetlinkTransportLayer_Base ();

  Net_ClientServerRole_t clientServerRole_;
  Net_TransportLayer_t   transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base (const Net_NetlinkTransportLayer_Base&));
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base& operator= (const Net_NetlinkTransportLayer_Base&));
};

#endif
