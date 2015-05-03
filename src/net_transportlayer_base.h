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

#ifndef NET_TRANSPORTLAYER_BASE_H
#define NET_TRANSPORTLAYER_BASE_H

#include "ace/Global_Macros.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_exports.h"
#include "net_itransportlayer.h"

class Net_Export Net_InetTransportLayer_Base
 : virtual public Net_ITransportLayer_T<Net_SocketConfiguration_t>
{
 public:
  virtual ~Net_InetTransportLayer_Base ();

  // implement (part of) Net_ITransportLayer_T
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // configuration
  virtual void finalize ();

 protected:
  Net_InetTransportLayer_Base (Net_ClientServerRole_t,
                               Net_TransportLayer_t);

  Net_ClientServerRole_t clientServerRole_;
  Net_TransportLayer_t   transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base ());
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base (const Net_InetTransportLayer_Base&));
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base& operator= (const Net_InetTransportLayer_Base&));
};

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
class Net_Export Net_NetlinkTransportLayer_Base
 : virtual public Net_ITransportLayer_T<Net_SocketConfiguration_t>
{
 public:
  virtual ~Net_NetlinkTransportLayer_Base ();

  // implement (part of) Net_ITransportLayer_T
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // configuration
  virtual void finalize ();

 protected:
  Net_NetlinkTransportLayer_Base ();

  Net_ClientServerRole_t clientServerRole_;
  Net_TransportLayer_t   transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base (const Net_NetlinkTransportLayer_Base&));
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base& operator= (const Net_NetlinkTransportLayer_Base&));
};
#endif

#endif
