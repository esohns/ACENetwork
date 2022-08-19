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

#ifndef SSDP_CONTROL_H
#define SSDP_CONTROL_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_task_ex.h"

#include "common_time_common.h"

#include "http_common.h"

#include "ssdp_common.h"
#include "ssdp_isession.h"

enum SSDP_Event
{
  SSDP_EVENT_DISCOVERY_RESPONSE = 0,
  SSDP_EVENT_DEVICE_DESCRIPTION,
  SSDP_EVENT_SERVICE_DESCRIPTION,
  SSDP_EVENT_SOAP_REPLY,
  ////////////////////////////////////////
  SSDP_EVENT_MAX,
  SSDP_EVENT_INVALID,
};

struct SSDP_Control_Event
{
  // *NOTE*: this is really a enum SSDP_Event (but it's abused to carry
  //         ACE_Message_Block::MB_STOP to shutdown the controller)
  int                           type;

  std::string                   data1;
  std::string                   data2;
  SSDP_StringList_t             data3;
  struct HTTP_Record            header;
};

template <typename SessionInterfaceType> // i.e. SSDP_ISession_T
class SSDP_Control_T
 : public Common_Task_Ex_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           struct SSDP_Control_Event>
 , public Common_IGetP_T<SessionInterfaceType>
{
  typedef Common_Task_Ex_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           struct SSDP_Control_Event> inherited;

 public:
  SSDP_Control_T (SessionInterfaceType*);
  inline virtual ~SSDP_Control_T () { stop (true, true); }

  // override Common_ITask
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // high priority ? (i.e. do not wait for queued messages)
  virtual void wait (bool = true) const; // wait for the message queue ? : worker thread(s) only

  // implement Common_ITaskHandler_T
  virtual void handle (struct SSDP_Control_Event*&); // event handle

  // implement Common_IGetP
  inline virtual const SessionInterfaceType* const getP () const { return session_; } // return value: type handle

  void notify (enum SSDP_Event,           // type
               const struct HTTP_Record&, // HTTP header
               const std::string&,        // data 1
               const std::string&,        // data 2
               const SSDP_StringList_t&); // data 3

 private:
  ACE_UNIMPLEMENTED_FUNC (SSDP_Control_T ())
  ACE_UNIMPLEMENTED_FUNC (SSDP_Control_T (const SSDP_Control_T&))
  ACE_UNIMPLEMENTED_FUNC (SSDP_Control_T& operator= (const SSDP_Control_T&))

  SessionInterfaceType* session_;
};

// include template definition
#include "ssdp_control.inl"

#endif
