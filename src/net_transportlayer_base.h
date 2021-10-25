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

#include "common.h"

#include "net_common.h"
#if defined (NETLINK_SUPPORT)
#include "net_connection_configuration.h"
#endif // NETLINK_SUPPORT
#include "net_itransportlayer.h"

template <typename ConfigurationType> // socket-
class Net_InetTransportLayerBase_T
 : public virtual Net_ITransportLayer_T<ConfigurationType>
{
 public:
  inline virtual ~Net_InetTransportLayerBase_T () {}

  // implement (part of) Net_ITransportLayer_T
  inline virtual enum Common_EventDispatchType dispatch () { return dispatch_; }
  inline virtual enum Net_ClientServerRole role () { return role_; }
  virtual bool initialize (enum Common_EventDispatchType,
                           enum Net_ClientServerRole,
                           const ConfigurationType&);
  inline virtual enum Net_TransportLayerType transportLayer () { return transportLayer_; }

 protected:
  Net_InetTransportLayerBase_T (enum Net_TransportLayerType);

  // implement (part of) Net_ITransportLayer_T
  inline virtual void finalize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  enum Common_EventDispatchType dispatch_;
  enum Net_ClientServerRole     role_;
  enum Net_TransportLayerType   transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayerBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayerBase_T (const Net_InetTransportLayerBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayerBase_T& operator= (const Net_InetTransportLayerBase_T&))
};

//////////////////////////////////////////

#if defined (NETLINK_SUPPORT)
class Net_NetlinkTransportLayer_Base
 : virtual public Net_ITransportLayer_T<Net_NetlinkSocketConfiguration_t>
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

// include template definition
#include "net_transportlayer_base.inl"

#endif
