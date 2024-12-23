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

#ifndef NET_SOCKETHANDLER_BASE_H
#define NET_SOCKETHANDLER_BASE_H

#include "ace/Global_Macros.h"

#include "common_iinitialize.h"

#include "net_iconnection.h"

// forward declarations
class ACE_Message_Block;

template <typename ConfigurationType>
class Net_SocketHandlerBase_T
 : virtual protected Net_ISocketHandler
 , virtual public Common_IInitialize_T<ConfigurationType>
{
 public:
  inline virtual ~Net_SocketHandlerBase_T () {}

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&); // handler configuration

 protected:
  Net_SocketHandlerBase_T ();

  // implement (part of) Net_ISocketHandler
  virtual ACE_Message_Block* allocateMessage (unsigned int); // requested size

  // *TODO*: remove this ASAP
  ConfigurationType* configuration_;
  bool               isInitialized_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_SocketHandlerBase_T (const Net_SocketHandlerBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_SocketHandlerBase_T& operator= (const Net_SocketHandlerBase_T&))
};

//////////////////////////////////////////

template <typename ConfigurationType>
class Net_AsynchSocketHandlerBase_T
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , protected Net_IAsynchSocketHandler
{};

// include template definition
#include "net_sockethandler_base.inl"

#endif
