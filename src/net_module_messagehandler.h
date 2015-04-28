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

#ifndef NET_MODULE_MESSAGEHANDLER_H
#define NET_MODULE_MESSAGEHANDLER_H

#include <list>

#include "ace/Global_Macros.h"
#include "ace/Synch.h"

#include "common.h"
#include "common_inotify.h"
#include "common_isubscribe.h"
#include "common_iclone.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"
#include "stream_streammodule_base.h"

#include "net_common.h"

template <typename ConfigurationType,
          typename SessionMessageType,
          typename MessageType>
class Net_Module_MessageHandler_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Common_ISubscribe_T<Common_INotify_T<ConfigurationType,
                                               MessageType> >
 , public Common_IClone_T<Common_Module_t>
{
 public:
  typedef Common_INotify_T<ConfigurationType,
                           MessageType> INOTIFY_T;
  typedef std::list<INOTIFY_T*> SUBSCRIBERS_T;

  Net_Module_MessageHandler_T ();
  virtual ~Net_Module_MessageHandler_T ();

  void initialize (SUBSCRIBERS_T* = NULL,               // subscribers (handle)
                   ACE_Recursive_Thread_Mutex* = NULL); // subscribers lock

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ISubscribe_T
  virtual void subscribe (INOTIFY_T*);   // new subscriber
  virtual void unsubscribe (INOTIFY_T*); // existing subscriber

 protected:
   // *NOTE*: recursive so that users may unsubscribe from within the
   // notification callbacks...
   ACE_Recursive_Thread_Mutex* lock_;
   SUBSCRIBERS_T*              subscribers_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  typedef Net_Module_MessageHandler_T<ConfigurationType,
                                      SessionMessageType,
                                      MessageType> OWN_T;
  typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERSITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (Net_Module_MessageHandler_T (const Net_Module_MessageHandler_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_Module_MessageHandler_T& operator= (const Net_Module_MessageHandler_T&));

  bool                         delete_;
};

#include "net_module_messagehandler.inl"

#endif
