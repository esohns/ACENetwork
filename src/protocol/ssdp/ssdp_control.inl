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

#include "net_defines.h"
#include "net_macros.h"

#include "ssdp_defines.h"

template <typename SessionInterfaceType>
SSDP_Control_T<SessionInterfaceType>::SSDP_Control_T (SessionInterfaceType* session_in)
 : inherited (ACE_TEXT_ALWAYS_CHAR (SSDP_CONTROL_HANDLER_THREAD_NAME), // thread name
              SSDP_CONTROL_HANDLER_THREAD_GROUP_ID,                    // group id
              1,                                                       // # thread(s)
              false,                                                   // auto-start ?
              NULL)                                                    // queue handle
 , session_ (session_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Control_T::SSDP_Control_T"));

}

template <typename SessionInterfaceType>
void
SSDP_Control_T<SessionInterfaceType>::stop (bool waitForCompletion_in,
                                            bool highPriority_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Control_T::stop"));

  session_->close ();

  // stop worker thread
  inherited::stop (false,
                   highPriority_in);

  if (waitForCompletion_in)
    wait ();
}

template <typename SessionInterfaceType>
void
SSDP_Control_T<SessionInterfaceType>::wait (bool waitForQueue_in) const
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Control_T::wait"));

  session_->wait ();

  inherited::wait (waitForQueue_in);
}

template <typename SessionInterfaceType>
void
SSDP_Control_T<SessionInterfaceType>::handle (struct SSDP_Control_Event*& event_inout)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Control_T::handle"));

  // sanity check(s)
  ACE_ASSERT (event_inout);

  switch (event_inout->type)
  {
    case SSDP_EVENT_DISCOVERY_RESPONSE:
    {
      session_->notifySSDPResponse (event_inout->header);
      break;
    }
    case SSDP_EVENT_DEVICE_DESCRIPTION:
    {
      session_->notifyServiceDescriptionControlURIs (event_inout->data1,
                                                     event_inout->data2);
      break;
    }
    case SSDP_EVENT_SERVICE_DESCRIPTION:
    {
      session_->notifyServiceActionArguments (event_inout->data3);
      break;
    }
    case SSDP_EVENT_SOAP_REPLY:
    {
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown event (type was: %d), returning\n"),
                  event_inout->type));
      break;
    }
  } // end SWITCH

  delete event_inout; event_inout = NULL;
}

template <typename SessionInterfaceType>
void
SSDP_Control_T<SessionInterfaceType>::notify (enum SSDP_Event event_in,
                                              const struct HTTP_Record& header_in,
                                              const std::string& data1_in,
                                              const std::string& data2_in,
                                              const SSDP_StringList_t& data3_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Control_T::notify"));

  struct SSDP_Control_Event* event_p = NULL;
  ACE_NEW_NORETURN (event_p,
                    struct SSDP_Control_Event ());
  ACE_ASSERT (event_p);
  event_p->type = event_in;
  event_p->header = header_in;
  event_p->data1 = data1_in;
  event_p->data2 = data2_in;
  event_p->data3 = data3_in;

  int result = inherited::put (event_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Task_Ex_T::put(): \"%m\", returning\n")));
    delete event_p; event_p = NULL;
  } // end IF
}
