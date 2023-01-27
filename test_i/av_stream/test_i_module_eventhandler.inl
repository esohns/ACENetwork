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

#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"

#include "stream_macros.h"

template <typename ModuleConfigurationType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename UserDataType>
Test_I_Stream_Module_EventHandler_T<ModuleConfigurationType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    MessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                    UserDataType>::Test_I_Stream_Module_EventHandler_T (ISTREAM_T* stream_in)
#else
                                    UserDataType>::Test_I_Stream_Module_EventHandler_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_EventHandler_T::Test_I_Stream_Module_EventHandler_T"));

}

template <typename ModuleConfigurationType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename UserDataType>
ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
Test_I_Stream_Module_EventHandler_T<ModuleConfigurationType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    MessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    UserDataType>::clone ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_EventHandler_T::clone"));

  // initialize return value(s)
  ACE_Task<ACE_MT_SYNCH,
           Common_TimePolicy_t>* task_p = NULL;

  ACE_NEW_NORETURN (task_p,
                    OWN_TYPE_T (NULL));
  if (unlikely (!task_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return NULL;
  } // end IF

  return task_p;
}
