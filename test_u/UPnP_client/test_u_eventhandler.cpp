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
#include "stdafx.h"

#include "test_u_eventhandler.h"

#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "net_macros.h"

#include "http_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "test_u_defines.h"

#include "test_u_upnp_client_defines.h"

#if defined (GUI_SUPPORT)
Test_U_EventHandler::Test_U_EventHandler (struct UPnP_Client_UI_CBData* CBData_in)
#else
Test_U_EventHandler::Test_U_EventHandler ()
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
#else
 : sessionData_ (NULL)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
Test_U_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct UPnP_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = &const_cast<struct UPnP_Client_SessionData&> (sessionData_in);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

//  CBData_->progressData.transferred = 0;
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = NULL;
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  UPnP_Client_MessageData_t& data_r =
    const_cast<UPnP_Client_MessageData_t&> (message_in.getR ());
  struct UPnP_Client_MessageData& record_r =
    const_cast<struct UPnP_Client_MessageData&> (data_r.getR ());
  // *NOTE*: listening on 239.255.255.250:1900 will also receive requests
  //         --> drop requests
  if ((record_r.method == HTTP_Codes::HTTP_METHOD_M_SEARCH) ||
      (record_r.method == HTTP_Codes::HTTP_METHOD_NOTIFY))
    return;

#if defined (GUI_SUPPORT)
  SSDP_StringList_t arguments_a;

  ACE_ASSERT (CBData_->control);
  CBData_->control->notify (SSDP_EVENT_DISCOVERY_RESPONSE,
                            record_r,
                            ACE_TEXT_ALWAYS_CHAR (""),
                            ACE_TEXT_ALWAYS_CHAR (""),
                            arguments_a);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
  CBData_->progressData.transferred += message_in.total_length ();
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
#endif // GUI_SUPPORT
}
void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
#endif // GTK_USE
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_CONNECT;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_DISCONNECT;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    { ACE_ASSERT (sessionData_);
      if (sessionData_->lock)
      {
        result = sessionData_->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE)
      CBData_->progressData.statistic.streamStatistic =
        sessionData_->statistic;
#endif // GTK_USE

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_STATISTIC;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_STEP;
#endif // GTK_USE
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
Test_U_EventHandler_2::Test_U_EventHandler_2 (struct UPnP_Client_UI_CBData* CBData_in)
#else
Test_U_EventHandler_2::Test_U_EventHandler_2 ()
#endif // GUI_SUPPORT
 : state_ (EVENT_HANDLER_STATE_DEVICE)
#if defined (GUI_SUPPORT)
 , CBData_ (CBData_in)
#endif // GUI_SUPPORT
 , sessionData_(NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_2::Test_U_EventHandler_2"));

}

void
Test_U_EventHandler_2::start (Stream_SessionId_t sessionId_in,
                              const struct UPnP_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_2::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = &const_cast<struct UPnP_Client_SessionData&> (sessionData_in);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

//  CBData_->progressData.transferred = 0;
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_U_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_EventHandler_2::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_2::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = NULL;
}

void
Test_U_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_U_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  UPnP_Client_MessageData_t& data_container_r =
    const_cast<UPnP_Client_MessageData_t&> (message_in.getR ());
  struct UPnP_Client_MessageData& data_r =
    const_cast<struct UPnP_Client_MessageData&> (data_container_r.getR ());
  ACE_ASSERT (data_r.document);
  ACE_ASSERT (data_r.xPathObject);
  if (!data_r.xPathObject->nodesetval ||
      !data_r.xPathObject->nodesetval->nodeNr)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no xpath data, continuing\n")));
    goto continue_;
  } // end IF

#if defined (GUI_SUPPORT)
  switch (state_)
  {
    case EVENT_HANDLER_STATE_DEVICE:
    {
      state_ = EVENT_HANDLER_STATE_SERVICE;

      // update configuration
      UPnP_Client_StreamConfiguration_t::ITERATOR_T iterator =
        CBData_->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator != CBData_->configuration->streamConfiguration_2.end ());
      (*iterator).second.second->xPathQueryString =
        ACE_TEXT_ALWAYS_CHAR (UPNP_CLIENT_XPATH_QUERY_GATECONNSCPD_STRING);
      (*iterator).second.second->xPathNameSpaces.clear ();
      (*iterator).second.second->xPathNameSpaces.push_back (std::make_pair (ACE_TEXT_ALWAYS_CHAR (UPNP_CLIENT_XPATH_QUERY_NAMESPACE_DESC_STRING),
                                                                            ACE_TEXT_ALWAYS_CHAR (UPNP_XML_SERVICE_SCPD_NAMESPACE_STRING)));



      std::string service_description_URI_string, service_control_URI_string;
      ACE_ASSERT (data_r.xPathObject->nodesetval->nodeNr >= 1);
      service_description_URI_string =
        (char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->next->next->children->content;
      service_control_URI_string =
        (char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->next->next->next->children->content;

      ACE_ASSERT (CBData_->control);
      struct HTTP_Record record_s;
      SSDP_StringList_t arguments_a;
      CBData_->control->notify (SSDP_EVENT_PRESENTATION_URL,
                                record_s,
                                retrievePresentationURL (data_r.document),
                                ACE_TEXT_ALWAYS_CHAR (""),
                                arguments_a);
      CBData_->control->notify (SSDP_EVENT_SERVICE_DESCRIPTION,
                                record_s,
                                service_description_URI_string,
                                service_control_URI_string,
                                arguments_a);

#if defined (GTK_USE)
      guint event_source_id = g_idle_add (idle_discovery_complete_cb,
                                          CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_discovery_complete_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      break;
    }
    case EVENT_HANDLER_STATE_SERVICE:
    {
      // update configuration
      UPnP_Client_StreamConfiguration_t::ITERATOR_T iterator =
        CBData_->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator != CBData_->configuration->streamConfiguration_2.end ());
      (*iterator).second.second->xPathQueryString =
        ACE_TEXT_ALWAYS_CHAR (UPNP_CLIENT_XPATH_QUERY_SOAP_STRING);
      (*iterator).second.second->xPathNameSpaces.clear ();
      (*iterator).second.second->xPathNameSpaces.push_back (std::make_pair (ACE_TEXT_ALWAYS_CHAR (UPNP_CLIENT_XPATH_QUERY_NAMESPACE_DESC_STRING),
                                                                            ACE_TEXT_ALWAYS_CHAR (UPNP_XML_SOAP_NAMESPACE_STRING)));

      SSDP_StringList_t arguments_a;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("action \"%s\" has these input arguments:\n"),
                  ACE_TEXT ("AddPortMapping")));

      std::string argument_string, direction_string;
      for (int i = 0;
           i < data_r.xPathObject->nodesetval->nodeNr;
           ++i)
      {
        direction_string =
          (char*)data_r.xPathObject->nodesetval->nodeTab[i]->children->next->children->content;
        if (ACE_OS::strcmp (direction_string.c_str (),
                            ACE_TEXT_ALWAYS_CHAR ("in")))
          continue;

        argument_string =
          (char*)data_r.xPathObject->nodesetval->nodeTab[i]->children->children->content;
        arguments_a.push_back (argument_string);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("argument %u: \"%s\"\n"),
                    i + 1,
                    ACE_TEXT (argument_string.c_str ())));
      } // end FOR

      ACE_ASSERT (CBData_->control);
      struct HTTP_Record record_s;
      CBData_->control->notify (SSDP_EVENT_SERVICE_ARGUMENTS,
                                record_s,
                                ACE_TEXT_ALWAYS_CHAR (""),
                                ACE_TEXT_ALWAYS_CHAR (""),
                                arguments_a);

#if defined (GTK_USE)
      guint event_source_id = g_idle_add (idle_service_description_complete_cb,
                                          CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_service_description_complete_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      break;
    }
    case EVENT_HANDLER_STATE_EXTERNAL_ADDRESS_CONTROL:
    {
      // sanity check(s)
      ACE_ASSERT (data_r.xPathObject->nodesetval->nodeNr == 1);

      bool success_b = true;
      if (data_r.status != HTTP_Codes::HTTP_STATUS_OK)
      {
        success_b = false;
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("action failed: fault code: \"%s\" string: \"%s\"; code: \"%s\" description: \"%s\", continuing\n"),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->children->content),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->next->children->content),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->next->next->children->children->children->content),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->next->next->children->children->next->children->content)));
      } // end IF
      else
      {
        std::string external_address_string =
          (char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->children->content;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("action succeeded: \"%s\"\n"),
                    ACE_TEXT (external_address_string.c_str ())));

#if defined (GUI_SUPPORT)
        external_address_string += ACE_TEXT_ALWAYS_CHAR (":0");
        CBData_->externalAddress.set (external_address_string.c_str (),
                                      AF_INET);
#endif // GUI_SUPPORT
      } // end ELSE

#if defined (GTK_USE)
      guint event_source_id =
        g_idle_add (success_b ? idle_end_session_success_cb : idle_end_session_error_cb,
                    CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_end_session_UI_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      break;
    }
    case EVENT_HANDLER_STATE_MAP_CONTROL:
    {
      // sanity check(s)
      ACE_ASSERT (data_r.xPathObject->nodesetval->nodeNr == 1);

      bool success_b = true;
      if (data_r.status != HTTP_Codes::HTTP_STATUS_OK)
      {
        success_b = false;
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("action failed: fault code: \"%s\" string: \"%s\"; code: \"%s\" description: \"%s\", continuing\n"),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->children->content),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->next->children->content),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->next->next->children->children->children->content),
                    ACE_TEXT ((char*)data_r.xPathObject->nodesetval->nodeTab[0]->children->children->next->next->children->children->next->children->content)));
      } // end IF
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("action succeeded\n")));

#if defined (GTK_USE)
      guint event_source_id =
        g_idle_add (success_b ? idle_end_session_success_cb : idle_end_session_error_cb,
                    CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_end_session_UI_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown state (was: %d), continuing\n"),
                  state_));
      break;
    }
  } // end SWITCH
#endif // GUI_SUPPORT

continue_:
  xmlXPathFreeObject (data_r.xPathObject); data_r.xPathObject = NULL;
  xmlFreeDoc (data_r.document); data_r.document = NULL;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  CBData_->progressData.transferred += message_in.total_length ();
#endif // GTK_USE
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
#endif // GUI_SUPPORT
}
void
Test_U_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_U_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_2::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
#endif // GTK_USE
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_CONNECT;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_DISCONNECT;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    { ACE_ASSERT (sessionData_);
      if (sessionData_->lock)
      {
        result = sessionData_->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE)
      CBData_->progressData.statistic.streamStatistic =
        sessionData_->statistic;
#endif // GTK_USE

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_STATISTIC;
#endif // GTK_USE
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
#if defined (GTK_USE)
      event_e = COMMON_UI_EVENT_STEP;
#endif // GTK_USE
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
#if defined (GTK_USE)
  state_r.eventStack.push (event_e);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

std::string
Test_U_EventHandler_2::retrievePresentationURL (xmlDocPtr document_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler_2::retrievePresentationURL"));

  std::string result;

  // step1: create a query context
  xmlXPathContextPtr xpath_context_p = xmlXPathNewContext (document_in);
  if (!xpath_context_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to xmlXPathNewContext(); \"%m\", aborting\n")));
    return result;
  } // end IF

  // step2: register given namespaces
  int result_2 =
    xmlXPathRegisterNs (xpath_context_p,
                        BAD_CAST (UPNP_CLIENT_XPATH_QUERY_NAMESPACE_DESC_STRING),
                        BAD_CAST (UPNP_XML_DEVICE_ROOT_NAMESPACE_STRING));
  if (unlikely (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlXPathRegisterNs(\"%s\" --> \"%s\"), aborting\n"),
                ACE_TEXT (UPNP_CLIENT_XPATH_QUERY_NAMESPACE_DESC_STRING),
                ACE_TEXT (UPNP_XML_DEVICE_ROOT_NAMESPACE_STRING)));
    return result;
  } // end IF

  // step3: perform query
  xmlXPathObjectPtr xpath_object_p = 
    xmlXPathEvalExpression (BAD_CAST (UPNP_CLIENT_XPATH_QUERY_GATEDESC_URL_STRING),
                            xpath_context_p);
  if (!xpath_object_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlXPathEvalExpression(\"%s\"); \"%m\", aborting\n"),
                ACE_TEXT (UPNP_CLIENT_XPATH_QUERY_GATEDESC_URL_STRING)));
    xmlXPathFreeContext (xpath_context_p); xpath_context_p = NULL;
    return result;
  } // end IF
  ACE_ASSERT (xpath_object_p->nodesetval);
  ACE_ASSERT (xpath_object_p->nodesetval->nodeNr == 1);

  // step4: retrieve result(s)
  result = (char*)xpath_object_p->nodesetval->nodeTab[0]->children->content;

  // step5: clean up
  xmlXPathFreeObject (xpath_object_p); xpath_object_p = NULL;
  xmlXPathFreeContext (xpath_context_p); xpath_context_p = NULL;

  return result;
}
