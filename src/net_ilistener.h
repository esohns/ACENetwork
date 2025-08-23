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

#ifndef NET_ILISTENER_H
#define NET_ILISTENER_H

#include "ace/config-macros.h"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_iinitialize.h"

#include "common_itask.h"

class Net_IConnectionState
{
 public:
  virtual void connect (ACE_HANDLE) = 0;
  virtual void disconnect (ACE_HANDLE) = 0;
};

template <typename ConfigurationType> // connection-
class Net_IListener_T
 : public Common_ITask
 , public Common_IGetR_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IDumpState
{
 public:
  // *TODO*: remove ASAP
  virtual bool useReactor () const = 0; // ? : is asynch --> uses proactor

  // *NOTE*: notifies disconnects to the listener (server-side only !)
  virtual void disconnect (ACE_HANDLE) const = 0;
};

#endif
