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

#ifndef NET_TRANSPORTLAYER_NETLINK_H
#define NET_TRANSPORTLAYER_NETLINK_H

#include "ace/Global_Macros.h"

#include "common.h"

#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_itransportlayer.h"

#if defined (NETLINK_SUPPORT)
class Net_NetlinkTransportLayer_Base
 : public virtual Net_ITransportLayer_T<Net_NetlinkSocketConfiguration_t>
{
 public:
  inline virtual ~Net_NetlinkTransportLayer_Base () {}

         // implement (part of) Net_ITransportLayer_T
  inline virtual enum Common_EventDispatchType dispatch () { return dispatch_; }
  inline virtual enum Net_ClientServerRole role () { return role_; }
  virtual bool initialize (enum Common_EventDispatchType,
                           enum Net_ClientServerRole,
                           const Net_NetlinkSocketConfiguration_t&);
  inline virtual enum Net_TransportLayerType transportLayer () { return transportLayer_; }

 protected:
  Net_NetlinkTransportLayer_Base ();

         // implement (part of) Net_ITransportLayer_T
  inline virtual void finalize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  enum Common_EventDispatchType dispatch_;
  enum Net_ClientServerRole     role_;
  enum Net_TransportLayerType   transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base (const Net_NetlinkTransportLayer_Base&))
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base& operator= (const Net_NetlinkTransportLayer_Base&))
};
#endif // NETLINK_SUPPORT

//////////////////////////////////////////

#if defined (NETLINK_SUPPORT)
class Net_TransportLayer_Netlink
 : public Net_NetlinkTransportLayer_Base
{
 public:
  Net_TransportLayer_Netlink ();
  inline virtual ~Net_TransportLayer_Netlink () {}

 private:
  typedef Net_NetlinkTransportLayer_Base inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_Netlink (const Net_TransportLayer_Netlink&))
  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_Netlink& operator= (const Net_TransportLayer_Netlink&))
};
#endif // NETLINK_SUPPORT

#endif
