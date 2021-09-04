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

#ifndef NET_ITRANSPORTLAYER_H
#define NET_ITRANSPORTLAYER_H

#include "common.h"

#include "net_common.h"

template <typename ConfigurationType> // socket-
class Net_ILinkLayer_T
{
 public:
  // information
  // *TODO*: move this somewhere else
  virtual enum Common_EventDispatchType dispatch () = 0;
  virtual enum Net_ClientServerRole role () = 0;
  virtual void set (enum Net_ClientServerRole) = 0;

  virtual bool initialize (enum Common_EventDispatchType,
                           enum Net_ClientServerRole,
                           const ConfigurationType&) = 0;
  virtual void finalize () = 0;
};

template <typename ConfigurationType> // socket-
class Net_IIPLinkLayer_T
 : public Net_ILinkLayer_T<ConfigurationType>
{
 public:
  // *NOTE*: this is an ICMP function
  virtual void ping () = 0;
};

//////////////////////////////////////////

template <typename ConfigurationType> // socket-
class Net_ITransportLayer_T
 : public Net_ILinkLayer_T<ConfigurationType>
{
 public:
  // information
  virtual enum Net_TransportLayerType transportLayer () = 0;
};

#endif
