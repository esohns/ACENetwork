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
#include "ace/Thread_Mutex.h"

#include "common_parser_bencoding_common.h"

#include "bittorrent_common.h"
#include "bittorrent_icontrol.h"

template <typename SessionAsynchType,
          typename SessionType,
          typename SessionConfigurationType,
          typename SessionInterfaceType,
          typename SessionStateType>
class BitTorrent_Control_T
 : public BitTorrent_IControl_T<SessionInterfaceType>
{
 public:
  // convenient types
  typedef std::map<std::string, SessionInterfaceType*> SESSIONS_T;
  typedef typename SESSIONS_T::iterator SESSIONS_ITERATOR_T;

  BitTorrent_Control_T (SessionConfigurationType*);
  inline virtual ~BitTorrent_Control_T () { stop (true); }

  // implement BitTorrent_IControl_T
  inline virtual const SESSIONS_T& getR () const { return sessions_; }
  virtual void request (const std::string&); // metainfo (aka '.torrent') file URI
  virtual SessionInterfaceType* get (const std::string&); // metainfo (aka '.torrent') file URI
  virtual void notifyTracker (const std::string&,     // metainfo (aka '.torrent') file URI
                              enum BitTorrent_Event); // event
  virtual void stop (bool = false); // wait ?
  virtual void wait ();
  ////////////////////////////////////////
  // callbacks
  virtual void notify (const std::string&,     // metainfo (aka '.torrent') file URI
                       enum BitTorrent_Event); // event

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Control_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Control_T (const BitTorrent_Control_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Control_T& operator= (const BitTorrent_Control_T&))

  bool getTrackerConnectionAndMessage (SessionInterfaceType*,                                // session handle
                                       const Bencoding_Dictionary_t&,                        // metainfo
                                       typename SessionType::ITRACKER_STREAM_CONNECTION_T*&, // return value: connection handle
                                       typename SessionType::TRACKER_MESSAGE_T*&);           // return value: message handle

  ACE_Condition_Thread_Mutex condition_;
  SessionConfigurationType*  configuration_;
  ACE_Thread_Mutex           lock_;
  SESSIONS_T                 sessions_;
};

// include template definition
#include "bittorrent_control.inl"

#endif
