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
#include "net_exports.h"
#include "net_itransportlayer.h"

//class Net_Export Net_InetTransportLayer_Base
class Net_InetTransportLayer_Base
 : virtual public Net_ITransportLayer_T<struct Net_SocketConfiguration>
{
 public:
  virtual ~Net_InetTransportLayer_Base ();

  // implement (part of) Net_ITransportLayer_T
  inline virtual void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  inline virtual enum Common_DispatchType dispatch () { return dispatch_; };
  inline virtual enum Net_ClientServerRole role () { return role_; };
  inline virtual enum Net_TransportLayerType transportLayer () { return transportLayer_; };

 protected:
  Net_InetTransportLayer_Base (enum Net_TransportLayerType);

  // implement (part of) Net_ITransportLayer_T
  virtual bool initialize (enum Common_DispatchType,
                           enum Net_ClientServerRole,
                           const struct Net_SocketConfiguration&);
  virtual void finalize ();

  enum Common_DispatchType    dispatch_;
  enum Net_ClientServerRole   role_;
  enum Net_TransportLayerType transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base ())
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base (const Net_InetTransportLayer_Base&))
  ACE_UNIMPLEMENTED_FUNC (Net_InetTransportLayer_Base& operator= (const Net_InetTransportLayer_Base&))
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//class Net_Export Net_NetlinkTransportLayer_Base
class Net_NetlinkTransportLayer_Base
 : virtual public Net_ITransportLayer_T<Net_SocketConfiguration>
{
 public:
  virtual ~Net_NetlinkTransportLayer_Base ();

  // implement (part of) Net_ITransportLayer_T
  inline virtual enum Common_DispatchType dispatch () { return dispatch_; };
  inline virtual enum Net_ClientServerRole role () { return role_; };
  inline virtual enum Net_TransportLayerType transportLayer () { return transportLayer_; };

 protected:
  Net_NetlinkTransportLayer_Base ();

  // implement (part of) Net_ITransportLayer_T
  virtual bool initialize (enum Common_DispatchType,
                           enum Net_ClientServerRole,
                           const struct Net_SocketConfiguration&);
  virtual void finalize ();

  enum Common_DispatchType    dispatch_;
  enum Net_ClientServerRole   role_;
  enum Net_TransportLayerType transportLayer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base (const Net_NetlinkTransportLayer_Base&))
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkTransportLayer_Base& operator= (const Net_NetlinkTransportLayer_Base&))
};
#endif

#endif
