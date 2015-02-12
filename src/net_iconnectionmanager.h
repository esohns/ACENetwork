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

#ifndef Net_ICONNECTIONMANAGER_H
#define Net_ICONNECTIONMANAGER_H

#include "net_iconnection.h"

#include "common_icontrol.h"

template <typename ConfigurationType,
          typename StatisticsContainerType>
class Net_IConnectionManager_T
 : public Common_IControl
{
 public:
  virtual ~Net_IConnectionManager_T () {};

  // API
  virtual void getConfiguration (ConfigurationType&) = 0; // return value: configuration
  virtual bool registerConnection (Net_IConnection_T<StatisticsContainerType>*) = 0; // connection
  virtual void deregisterConnection (const Net_IConnection_T<StatisticsContainerType>*) = 0; // connection
};

#endif
