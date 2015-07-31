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

//// forward declarations
//enum Common_DispatchType;
//enum Net_ClientServerRole;
//enum Net_TransportLayerType;

template <typename ConfigurationType>
class Net_ITransportLayer_T
{
 public:
  inline virtual ~Net_ITransportLayer_T () {};

  virtual void ping () = 0;

  // information
  // *TODO*: move this somewhere else
  virtual Common_DispatchType dispatch () = 0;
  virtual Net_ClientServerRole role () = 0;
  virtual void set (Net_ClientServerRole) = 0;
  virtual Net_TransportLayerType transportLayer () = 0;

 //protected:
  virtual bool initialize (Common_DispatchType,
                           Net_ClientServerRole,
                           const ConfigurationType&) = 0;
  virtual void finalize () = 0;
};

#endif
