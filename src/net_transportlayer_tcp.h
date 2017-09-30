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

#ifndef Net_TRANSPORTLAYER_TCP_H
#define Net_TRANSPORTLAYER_TCP_H

#include "ace/Global_Macros.h"

#include "net_configuration.h"
#include "net_transportlayer_base.h"

class Net_TransportLayer_TCP
 : public Net_InetTransportLayerBase_T<struct Net_TCPSocketConfiguration>
{
  typedef Net_InetTransportLayerBase_T<struct Net_TCPSocketConfiguration> inherited;

 public:
  Net_TransportLayer_TCP ();
  inline virtual ~Net_TransportLayer_TCP () {};

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_TCP (const Net_TransportLayer_TCP&));
  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_TCP& operator= (const Net_TransportLayer_TCP&));
};

#endif
