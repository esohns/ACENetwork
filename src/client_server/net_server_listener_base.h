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

#ifndef NET_SERVER_LISTENER_BASE_H
#define NET_SERVER_LISTENER_BASE_H

#include <list>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"

#include "net_ilistener.h"

class Net_Server_Listener_Base
 : public Common_ISubscribe_T<Net_IConnectionState>
{
 public:
  // convenient types
  typedef Net_IConnectionState INOTIFY_T;
  typedef Common_ISubscribe_T<INOTIFY_T> ISUBSCRIBE_T;

  inline virtual ~Net_Server_Listener_Base () {}

  // implement Common_ISubscribe_T
  virtual void subscribe (INOTIFY_T*);
  virtual void unsubscribe (INOTIFY_T*);

 protected:
  // convenient types
  typedef std::list<INOTIFY_T*> SUBSCRIBERS_T;
  typedef SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;
  typedef SUBSCRIBERS_T::const_iterator SUBSCRIBERS_CONST_ITERATOR_T;
   
  Net_Server_Listener_Base ();

  mutable ACE_MT_SYNCH::MUTEX lock_;
  SUBSCRIBERS_T               subscribers_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Server_Listener_Base (const Net_Server_Listener_Base&))
  ACE_UNIMPLEMENTED_FUNC (Net_Server_Listener_Base& operator= (const Net_Server_Listener_Base&))
};

#endif
