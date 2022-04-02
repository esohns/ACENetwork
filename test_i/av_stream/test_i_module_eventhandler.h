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

#ifndef TEST_I_MODULE_EVENTHANDLER_H
#define TEST_I_MODULE_EVENTHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Module.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

template <typename ModuleConfigurationType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename UserDataType>
class Test_I_Stream_Module_EventHandler_T
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         ConfigurationType,
                                         ControlMessageType,
                                         MessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         UserDataType>
{
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         ConfigurationType,
                                         ControlMessageType,
                                         MessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         UserDataType> inherited;

 public:
  // convenient types
  typedef ACE_Module<ACE_MT_SYNCH,
                     Common_TimePolicy_t> MODULE_BASE_T;
  typedef Stream_IModuleHandler_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  ConfigurationType> IMODULEHANDLER_T;

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Stream_Module_EventHandler_T (ISTREAM_T*);                     // stream handle
#else
  Test_I_Stream_Module_EventHandler_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  inline virtual ~Test_I_Stream_Module_EventHandler_T () {}

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  // convenient types
  typedef Test_I_Stream_Module_EventHandler_T<ModuleConfigurationType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              MessageType,
                                              SessionMessageType,
                                              SessionDataType,
                                              SessionDataContainerType,
                                              UserDataType> OWN_TYPE_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         SessionDataType,
                                         Stream_SessionMessageType,
                                         ModuleConfigurationType,
                                         ConfigurationType,
                                         libacestream_default_misc_messagehandler_module_name_string,
                                         Stream_INotify_t,
                                         OWN_TYPE_T> MODULE_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Module_EventHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Module_EventHandler_T (const Test_I_Stream_Module_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Module_EventHandler_T& operator= (const Test_I_Stream_Module_EventHandler_T&))
};

// include template definition
#include "test_i_module_eventhandler.inl"

#endif
