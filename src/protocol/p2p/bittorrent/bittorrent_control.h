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

#ifndef BITTORRENT_CONTROL_H
#define BITTORRENT_CONTROL_H

#include <map>
#include <string>

#include "ace/Condition_Thread_Mutex.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Thread_Mutex.h"

#include "common_task_ex.h"

#include "common_time_common.h"

#include "bittorrent_common.h"
#include "bittorrent_icontrol.h"

struct BitTorrent_Control_Event
{
  // *NOTE*: this is really a enum BitTorrent_Event (but it's abused to carry
  //         ACE_Message_Block::MB_STOP to shutdown the controller)
  int         type;
  std::string metaInfoFileName;
  std::string data;
};

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
class BitTorrent_Control_T
 : public Common_Task_Ex_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           struct BitTorrent_Control_Event>
 , public BitTorrent_IControl_T<SessionInterfaceType>
{
  typedef Common_Task_Ex_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           struct BitTorrent_Control_Event> inherited;

 public:
  // convenient types
  typedef std::map<std::string, SessionInterfaceType*> SESSIONS_T;
  typedef typename SESSIONS_T::iterator SESSIONS_ITERATOR_T;

  BitTorrent_Control_T (SessionConfigurationType*);
  inline virtual ~BitTorrent_Control_T () { stop (true, true); }

  // override Common_ITask
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // high priority ? (i.e. do not wait for queued messages)
  virtual void wait (bool = true) const; // wait for the message queue ? : worker thread(s) only

  // implement Common_ITaskHandler_T
  virtual void handle (struct BitTorrent_Control_Event*&); // event handle

  // implement BitTorrent_IControl_T
  inline virtual const SESSIONS_T& getR () const { return sessions_; }
  virtual void request (const std::string&); // metainfo (aka '.torrent') file URI
  virtual SessionInterfaceType* get (const std::string&); // metainfo (aka '.torrent') file URI
  virtual void notifyTracker (const std::string&,     // metainfo (aka '.torrent') file URI
                              enum BitTorrent_Event); // event
  ////////////////////////////////////////
  // callbacks
  virtual void notify (const std::string&,    // metainfo (aka '.torrent') file URI
                       enum BitTorrent_Event, // event
                       const std::string&);   // (optional) data

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Control_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Control_T (const BitTorrent_Control_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Control_T& operator= (const BitTorrent_Control_T&))

  bool getTrackerConnectionAndMessage (SessionInterfaceType*,                                // session handle
                                       typename SessionType::ITRACKER_STREAM_CONNECTION_T*&, // return value: connection handle
                                       typename SessionType::TRACKER_MESSAGE_T*&);           // return value: message handle
  void requestRedirected (SessionInterfaceType*, // session handle
                          const std::string&);   // redirected URL

  mutable ACE_Condition_Thread_Mutex condition_;
  mutable ACE_Thread_Mutex           lock_;
  SessionConfigurationType*          sessionConfiguration_;
  SESSIONS_T                         sessions_;
};

// include template definition
#include "bittorrent_control.inl"

#endif
