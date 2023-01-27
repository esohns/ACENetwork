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

#ifndef NET_MESSAGEHANDLERBASE_H
#define NET_MESSAGEHANDLERBASE_H

#include "common_inotify.h"

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
class Net_MessageHandlerBase_T
 : public Common_INotify_T<ConfigurationType,
                           MessageType,
                           SessionMessageType>
{
 public:
  Net_MessageHandlerBase_T ();
  virtual ~Net_MessageHandlerBase_T ();

  // implement Common_INotify_T
  // *WARNING*: these are NOP stubs
  virtual void start (const ConfigurationType&);
  virtual void notify (const MessageType&);
  virtual void notify (const SessionMessageType&);
  virtual void end ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_MessageHandlerBase_T (const Net_MessageHandlerBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_MessageHandlerBase_T& operator= (const Net_MessageHandlerBase_T&))
};

#include "net_messagehandler_base.inl"

#endif
