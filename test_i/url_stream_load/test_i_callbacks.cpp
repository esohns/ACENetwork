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

#include "test_i_callbacks.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <limits>

#include <iphlpapi.h>
#else
#include <netinet/ether.h>
#include <ifaddrs.h>
#endif
#include <sstream>

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_file_tools.h"
//#include "ace/Synch.h"
#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_macros.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_connection_stream.h"
#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_url_stream_load_common.h"
#include "test_i_url_stream_load_defines.h"

// initialize statics
static bool un_toggling_connect = false;

/////////////////////////////////////////

gboolean
idle_end_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_CONNECT_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        GTK_STOCK_CONNECT);
  GtkBox* box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_VBOX_CONFIGURATION_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), true);

  // stop progress reporting
  GtkSpinner* spinner_p =
    GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
  ACE_ASSERT (spinner_p);
  gtk_spinner_stop (spinner_p);
  gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), false);

  ACE_ASSERT (data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    if (!g_source_remove (data_p->progressData.eventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  data_p->progressData.eventSourceId));
    state_r.eventSourceIds.erase (data_p->progressData.eventSourceId);
    data_p->progressData.eventSourceId = 0;
  } // end lock scope
  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progressbar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), false);

  un_toggling_connect = true;
  gtk_toggle_button_toggled (toggle_button_p);

  return G_SOURCE_REMOVE;
}

gboolean
idle_load_segment_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_load_segment_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // select connector
  std::string hostname_string, hostname_string_2, URI_string;
  bool use_SSL = false;
  size_t position = std::string::npos;
  int result = -1;
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("2"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  if (!HTTP_Tools::parseURL (data_p->URL,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (data_p->URL.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  hostname_string_2 = hostname_string;
  position =
    hostname_string_2.find_last_of (':', std::string::npos);
  if (position == std::string::npos)
  {
    hostname_string_2 += ':';
    std::ostringstream converter;
    converter << (use_SSL ? HTTPS_DEFAULT_SERVER_PORT
                          : HTTP_DEFAULT_SERVER_PORT);
    hostname_string_2 += converter.str ();
  } // end IF
  result =
    dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address.set (hostname_string_2.c_str (),
                                                                                                         AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (hostname_string_2.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  (*iterator_2).second->useLoopBackDevice =
    dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address.is_loopback ();

  // update configuration
  Test_I_URLStreamLoad_StreamConfiguration_2_t::ITERATOR_T iterator_3 =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration_2.end ());
  (*iterator_3).second.second.URL = data_p->URL;

  Test_I_TCPConnector_2_t connector (true);
#if defined (SSL_SUPPORT)
  Test_I_SSLConnector_2_t ssl_connector (true);
#endif // SSL_SUPPORT
  Test_I_AsynchTCPConnector_2_t asynch_connector (true);
  Test_I_IConnector_2_t* iconnector_p = NULL;
  Test_I_ConnectionManager_2_t::INTERFACE_T* iconnection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON_2::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_2_t::ICONNECTION_T* iconnection_p = NULL;

  // step3: connect to peer
  if (data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      iconnector_p = &ssl_connector;
    else
#endif // SSL_SUPPORT
      iconnector_p = &connector;
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    iconnector_p = &asynch_connector;
  } // end ELSE
  if (!iconnector_p->initialize (*dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

  // step3b: connect
  data_p->handle =
      iconnector_p->connect (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address);
  // *TODO*: support one-thread operation by scheduling a signal and manually
  //         running the dispatch loop for a limited time...
  if (data_p->configuration->dispatchConfiguration.numberOfProactorThreads > 0)
  {
    data_p->handle = ACE_INVALID_HANDLE;

    // *TODO*: avoid tight loop here
    ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_S, 0);
    //result = ACE_OS::sleep (timeout);
    //if (result == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
    //              &timeout));
    ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
    do
    {
      iconnection_p =
          iconnection_manager_p->get (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address,
                                      true);
      if (iconnection_p)
      {
        data_p->handle =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            reinterpret_cast<ACE_HANDLE> (iconnection_p->id ());
#else
            static_cast<ACE_HANDLE> (iconnection_p->id ());
#endif
        iconnection_p->decrease ();
        break;
      } // end IF
    } while (COMMON_TIME_NOW < deadline);
    if (!iconnection_p)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s (timed out after: %#T), continuing\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address).c_str ()),
                  &timeout));
  } // end IF
  else
    iconnection_p =
      iconnection_manager_p->get (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address,
                                  true);
  if ((data_p->handle == ACE_INVALID_HANDLE) ||
      !iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("0x%@: opened TCP socket to %s\n"),
//                data_p->handle,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address).c_str ())));
//#else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%d: opened TCP socket to %s\n"),
//                data_p->handle,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address).c_str ())));
//#endif

//  // step4: send HTTP request
//  ACE_ASSERT (iconnection_p);
//  istream_connection_p =
//    dynamic_cast<Test_I_IStreamConnection_2_t*> (iconnection_p);
//  ACE_ASSERT (istream_connection_p);

//  ACE_NEW_NORETURN (HTTP_record_p,
//                    struct HTTP_Record ());
//  if (!HTTP_record_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    goto error;
//  } // end IF
//  HTTP_record_p->form = HTTP_form;
//  HTTP_record_p->headers = HTTP_headers;
//  HTTP_record_p->method =
//    (HTTP_form.empty () ? HTTP_Codes::HTTP_METHOD_GET
//                        : HTTP_Codes::HTTP_METHOD_POST);
//  HTTP_record_p->URI = (*iterator_3).second.second.URL;
//  HTTP_record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

//  ACE_NEW_NORETURN (message_data_p,
//                    Test_I_Message::DATA_T ());
//  if (!message_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    delete HTTP_record_p; HTTP_record_p = NULL;
//    goto error;
//  } // end IF
//  // *IMPORTANT NOTE*: fire-and-forget API (HTTP_record_p)
//  message_data_p->setPR (HTTP_record_p);

//  ACE_ASSERT ((*iterator_2).second->allocatorConfiguration);
//  pdu_size_i =
//    (*iterator_2).second->allocatorConfiguration->defaultBufferSize;// +
////      (*iterator_2).second->allocatorConfiguration->paddingBytes;

//  ACE_ASSERT (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->messageAllocator);
//allocate:
//  message_p =
//    static_cast<Test_I_Message*> (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->messageAllocator->malloc (pdu_size_i));
//  // keep retrying ?
//  if (!message_p &&
//      !dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second)->messageAllocator->block ())
//    goto allocate;
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
//    delete message_data_p; message_data_p = NULL;
//    goto error;
//  } // end IF
//  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
//  message_p->initialize (message_data_p,
//                         message_p->sessionId (),
//                         NULL);

//  //Test_I_ConnectionStream_2& stream_r =
//  //    const_cast<Test_I_ConnectionStream_2&> (istream_connection_p->stream ());
//  //const Test_I_URLStreamLoad_SessionData_2_t* session_data_container_p =
//  //  &stream_r.get ();
//  //ACE_ASSERT (session_data_container_p);
//  //struct Test_I_URLStreamLoad_SessionData_2& session_data_r =
//  //    const_cast<struct Test_I_URLStreamLoad_SessionData_2&> (session_data_container_p->get ());

//  message_block_p = message_p;
//    istream_connection_p->send (message_block_p);

  // clean up
  iconnection_p->decrease (); iconnection_p = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
    static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  state_r.eventSourceIds.clear ();

  gtk_main_quit ();

  // one-shot action
  return G_SOURCE_REMOVE;
}

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: initialize dialog window(s)
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  GtkDialog* about_dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_CONNECT_NAME)));
  ACE_ASSERT (toggle_button_p);
  bool use_stock = gtk_button_get_use_stock (GTK_BUTTON (toggle_button_p));
  ACE_ASSERT (use_stock);
  //gtk_button_set_label (GTK_BUTTON (toggle_button_p),
  //                      GTK_STOCK_CONNECT);
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  use_stock = gtk_button_get_use_stock (button_p);
  ACE_ASSERT (use_stock);
  //gtk_button_set_label (button_p,
  //                      GTK_STOCK_CUT);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  size_t pdu_size_i =
    (*iterator_2).second->allocatorConfiguration->defaultBufferSize +
    (*iterator_2).second->allocatorConfiguration->paddingBytes;
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> ((*iterator_2).second->allocatorConfiguration->defaultBufferSize));

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_URL_NAME)));
  ACE_ASSERT (entry_p);
  Test_I_URLStreamLoad_StreamConfiguration_t::ITERATOR_T iterator_3 =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
  gchar* text_p =
    Common_UI_GTK_Tools::localeToUTF8 ((*iterator_3).second.second.URL);
  gtk_entry_set_text (entry_p,
                      text_p);
  g_free (text_p);

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  //struct _GValue property_s = G_VALUE_INIT;
  //g_value_init (&property_s,
  //              G_TYPE_POINTER);
  //g_object_get_property (G_OBJECT (file_chooser_button_p),
  //                       ACE_TEXT_ALWAYS_CHAR ("dialog"),
  //                       &property_s);
  //G_VALUE_HOLDS_POINTER (&property_s);
  //GtkFileChooser* file_chooser_p = NULL;
    //reinterpret_cast<GtkFileChooser*> (g_value_get_pointer (&property_s));
  //g_object_get (G_OBJECT (file_chooser_button_p),
  //              ACE_TEXT_ALWAYS_CHAR ("dialog"),
  //              &file_chooser_p, NULL);
  //ACE_ASSERT (file_chooser_p);
  //ACE_ASSERT (GTK_IS_FILE_CHOOSER_DIALOG (file_chooser_p));
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (file_chooser_p);
  //ACE_ASSERT (file_chooser_dialog_p);
  //GtkPlacesSidebar* places_sidebar_p = NULL;
  //Common_UI_GTK_Tools::dump (GTK_WIDGET (file_chooser_dialog_p));
  //[0].get_children ()[0].get_children ([0].get_children ()[0]
  //  vbox.get_children ()[0].hide ()

  //GError* error_p = NULL;
  //GFile* file_p = NULL;
  struct _GString* string_p = NULL;
  gchar* filename_p = NULL;
  if (!(*iterator_3).second.second.targetFileName.empty ())
  {
    // *NOTE*: gtk does not complain if the file doesn't exist, but the button
    //         will display "(None)" --> create empty file
    if (!Common_File_Tools::isReadable ((*iterator_3).second.second.targetFileName))
      if (!Common_File_Tools::create ((*iterator_3).second.second.targetFileName))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::create(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT ((*iterator_3).second.second.targetFileName.c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF
    //file_p =
    //  g_file_new_for_path (data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ());
    //ACE_ASSERT (file_p);
    //ACE_ASSERT (g_file_query_exists (file_p, NULL));

    //std::string file_uri =
    //  ACE_TEXT_ALWAYS_CHAR ("file://") +
    //  data_p->configuration->moduleHandlerConfiguration.targetFileName;
    //if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
    //                                              file_uri.c_str ()))
    string_p = g_string_new ((*iterator_3).second.second.targetFileName.c_str ());
    filename_p = string_p->str;
      //Common_UI_GTK_Tools::Locale2UTF8 (data_p->configuration->moduleHandlerConfiguration.targetFileName);
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT ((*iterator_3).second.second.targetFileName.c_str ())));

      // clean up
      g_string_free (string_p, FALSE);
      g_free (filename_p);

      return G_SOURCE_REMOVE;
    } // end IF
    g_string_free (string_p, FALSE);
    g_free (filename_p);

    //if (!gtk_file_chooser_select_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
    //                                   file_p,
    //                                   &error_p))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to gtk_file_chooser_select_file(\"%s\"): \"%s\", aborting\n"),
    //              ACE_TEXT (data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ()),
    //              ACE_TEXT (error_p->message)));

    //  // clean up
    //  g_error_free (error_p);
    //  g_object_unref (file_p);

    //  return G_SOURCE_REMOVE;
    //} // end IF
    //g_object_unref (file_p);
  } // end IF
  else
  {
    //file_p =
    //  g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
    //ACE_ASSERT (file_p);

    string_p = g_string_new (Common_File_Tools::getTempDirectory ().c_str ());
    filename_p = string_p->str;
      //Common_UI_GTK_Tools::Locale2UTF8 (Common_File_Tools::getTempDirectory ());
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ())));

      // clean up
      g_string_free (string_p, FALSE);
      g_free (filename_p);

      return G_SOURCE_REMOVE;
    } // end IF
    g_string_free (string_p, FALSE);
    g_free (filename_p);
    //g_object_unref (file_p);
  } // end ELSE

  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri += (*iterator_3).second.second.targetFileName;
  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                default_folder_uri.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (default_folder_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_SAVE_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                !(*iterator_3).second.second.targetFileName.empty ());
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (data_p->configuration->dispatchConfiguration.numberOfProactorThreads > 0));
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                dynamic_cast<Net_TCPSocketConfiguration_t*> ((*iterator_2).second)->address.is_loopback ());

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progressbar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progressbar_p,
                                   1.0 / static_cast<double> (width));

  GtkStatusbar* statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);
  guint context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_STATUSBAR_CONTEXT_DATA));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_DATA,
                                             context_id));
  context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_STATUSBAR_CONTEXT_INFORMATION));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                             context_id));

  // step5: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    //// schedule asynchronous updates of the log view
    //event_source_id = g_timeout_add_seconds (1,
    //                                         idle_update_log_display_cb,
    //                                         data_p);
    //if (event_source_id > 0)
    //  data_p->eventSourceIds.insert (event_source_id);
    //else
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
    //  return G_SOURCE_REMOVE;
    //} // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
                     idle_update_info_display_cb,
                     data_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               data_p);

  // step6a: connect custom signals
  gulong result = g_signal_connect (G_OBJECT (dialog_p),
                                    ACE_TEXT_ALWAYS_CHAR ("destroy"),
                                    G_CALLBACK (button_quit_clicked_cb),
                                    data_p);
//                   G_CALLBACK(gtk_widget_destroyed),
//                   &main_dialog_p,
  ACE_ASSERT (result);
//  result = g_signal_connect (G_OBJECT (dialog_p),
//                    ACE_TEXT_ALWAYS_CHAR ("delete-event"),
//                    G_CALLBACK (delete_event_cb),
//                    NULL);
  result = g_signal_connect (dialog_p,
                             ACE_TEXT_ALWAYS_CHAR ("destroy"),
                             G_CALLBACK (gtk_widget_destroyed),
                             NULL);
  ACE_ASSERT (result);
  result = g_signal_connect_swapped (G_OBJECT (about_dialog_p),
                                     ACE_TEXT_ALWAYS_CHAR ("response"),
                                     G_CALLBACK (gtk_widget_hide),
                                     about_dialog_p);
  ACE_ASSERT (result);

  //-------------------------------------

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT (window_p);
  (*iterator_3).second.second.window = window_p;

  return G_SOURCE_REMOVE;
}

gboolean
idle_reset_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_reset_UI_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
    static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    data_p->progressData.transferred = 0;
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_start_session_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
    static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR_2 ();

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_ProgressData* data_p =
    static_cast<struct Test_I_URLStreamLoad_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;
  float speed = 0.0F;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_CONTINUE);
    speed = data_p->statistic.bytesPerSecond;
  } // end lock scope
  std::string magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  if (speed)
  {
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("kbyte(s)/s");
    } // end IF
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("mbyte(s)/s");
    } // end IF
    result = ACE_OS::sprintf (buffer, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (buffer));
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_CONNECT:
        case COMMON_UI_EVENT_DISCONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_spin (spin_button_p,
                                ((*event_p == COMMON_UI_EVENT_CONNECT) ? GTK_SPIN_STEP_FORWARD
                                                                       : GTK_SPIN_STEP_BACKWARD),
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.transferred));

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *event_p));
          break;
        }
      } // end SWITCH
      ACE_UNUSED_ARG (is_session_message);
      gtk_spin_button_spin (spin_button_p,
                            GTK_SPIN_STEP_FORWARD,
                            1.0);
      event_p = NULL;
    } // end FOR

    // clean up
    while (!state_r.eventStack.is_empty ())
    {
      result = state_r.eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

//gboolean
//idle_update_log_display_cb (gpointer userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));
//
//  struct Test_I_URLStreamLoad_UI_CBData* data_p =
//      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//
//  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);
//
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//      data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != data_p->builders.end ());
//
//  GtkTextView* view_p =
//    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
//                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
//  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
//
//  GtkTextIter text_iterator;
//  gtk_text_buffer_get_end_iter (buffer_p,
//                                &text_iterator);
//
//  gchar* string_p = NULL;
//  // sanity check
//  if (data_p->logStack.empty ())
//    return G_SOURCE_CONTINUE;
//
//  // step1: convert text
//  for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
//       iterator_2 != data_p->logStack.end ();
//       iterator_2++)
//  {
//    string_p = Common_UI_GTK_Tools::Locale2UTF8 (*iterator_2);
//    if (!string_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
//                  ACE_TEXT ((*iterator_2).c_str ())));
//      return G_SOURCE_REMOVE;
//    } // end IF
//
//    // step2: display text
//    gtk_text_buffer_insert (buffer_p,
//                            &text_iterator,
//                            string_p,
//                            -1);
//
//    // clean up
//    g_free (string_p);
//  } // end FOR
//
//  data_p->logStack.clear ();
//
//  // step3: scroll the view accordingly
////  // move the iterator to the beginning of line, so it doesn't scroll
////  // in horizontal direction
////  gtk_text_iter_set_line_offset (&text_iterator, 0);
//
////  // ...and place the mark at iter. The mark will stay there after insertion
////  // because it has "right" gravity
////  GtkTextMark* text_mark_p =
////      gtk_text_buffer_get_mark (buffer_p,
////                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCROLLMARK_NAME));
//////  gtk_text_buffer_move_mark (buffer_p,
//////                             text_mark_p,
//////                             &text_iterator);
//
////  // scroll the mark onscreen
////  gtk_text_view_scroll_mark_onscreen (view_p,
////                                      text_mark_p);
//  //GtkAdjustment* adjustment_p =
//  //    GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
//  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ADJUSTMENT_NAME)));
//  //ACE_ASSERT (adjustment_p);
//  //gtk_adjustment_set_value (adjustment_p,
//  //                          adjustment_p->upper - adjustment_p->page_size));
//
//  return G_SOURCE_CONTINUE;
//}

/////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
togglebutton_connect_toggled_cb (GtkToggleButton* toggleButton_in,
                                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_connect_toggled_cb"));

  if (un_toggling_connect)
  {
    un_toggling_connect = false;
    return;
  } // end IF

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  bool success = false;
  GtkBox* box_p = NULL;

  if (is_active)
  {
    // --> connect

    // step1: update widgets
    gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                          GTK_STOCK_DISCONNECT);
    box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_VBOX_CONFIGURATION_NAME)));
    ACE_ASSERT (box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (box_p), FALSE);

    // step2: update configuration
    GtkSpinButton* spin_button_p = NULL;
    GtkEntry* entry_p = NULL;
    std::string URI_string, directory_string, hostname_string;
    bool use_SSL = false;
    std::string hostname_string_2;
    size_t position = std::string::npos;
    int result = -1;
    GtkCheckButton* check_button_p = NULL;
    GtkFileChooserButton* file_chooser_button_p = NULL;
    gchar* URI_p, *directory_p, *hostname_p = NULL;
    GError* error_p = NULL;
    Net_ConnectionConfigurationsIterator_t iterator_2 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
    Test_I_URLStreamLoad_StreamConfiguration_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
    Test_I_TCPConnector_t connector (true);
#if defined (SSL_SUPPORT)
    Test_I_SSLConnector_t ssl_connector (true);
#endif // SSL_SUPPORT
    Test_I_AsynchTCPConnector_t asynch_connector (true);
    Test_I_IConnector_t* iconnector_p = NULL;
    Test_I_IStreamConnection_t* istream_connection_p = NULL;
    HTTP_Form_t HTTP_form;
    HTTP_Headers_t HTTP_headers;
    struct HTTP_Record* HTTP_record_p = NULL;
    Test_I_Message::DATA_T* message_data_p = NULL;
    Test_I_Message* message_p = NULL;
    ACE_Message_Block* message_block_p = NULL;
    GtkSpinner* spinner_p = NULL;
    GtkProgressBar* progressbar_p = NULL;
    size_t pdu_size_i = 0;

    // retrieve buffer size
    spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
    ACE_ASSERT (spin_button_p);
    (*iterator_2).second->bufferSize =
      static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));
    //data_p->configuration->connectionConfiguration.PDUSize =
    //  static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));

    // retrieve stream URL
    entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_URL_NAME)));
    ACE_ASSERT (entry_p);
    (*iterator_3).second.second.URL =
      Common_UI_GTK_Tools::UTF8ToLocale (gtk_entry_get_text (entry_p), -1);
    if (!HTTP_Tools::parseURL ((*iterator_3).second.second.URL,
                               hostname_string,
                               URI_string,
                               use_SSL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                  ACE_TEXT ((*iterator_3).second.second.URL.c_str ())));
      goto error;
    } // end IF
    hostname_string_2 = hostname_string;
    position =
      hostname_string_2.find_last_of (':', std::string::npos);
    if (position == std::string::npos)
    {
      hostname_string_2 += ':';
      std::ostringstream converter;
      converter << (use_SSL ? HTTPS_DEFAULT_SERVER_PORT
                            : HTTP_DEFAULT_SERVER_PORT);
      hostname_string_2 += converter.str ();
    } // end IF
    result =
      dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address.set (hostname_string_2.c_str (),
                                                                                                         AF_INET);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (hostname_string_2.c_str ())));
      goto error;
    } // end IF
    (*iterator_2).second->useLoopBackDevice =
      dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address.is_loopback ();

    // save to file ?
    check_button_p =
        GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_SAVE_NAME)));
    ACE_ASSERT (check_button_p);
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_p)))
    {
      (*iterator_3).second.second.targetFileName.clear ();
      goto continue_;
    } // end IF
    // retrieve output filename
    file_chooser_button_p =
        GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
    ACE_ASSERT (file_chooser_button_p);
    URI_p =
        gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (file_chooser_button_p));
    if (!URI_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_get_uri(), aborting\n")));
      goto error;
    } // end IF
    directory_p = g_filename_from_uri (URI_p,
                                       &hostname_p,
                                       &error_p);
    g_free (URI_p); URI_p = NULL;
    if (!directory_p)
    { ACE_ASSERT (error_p);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_filename_from_uri(): \"%s\", returning\n"),
                  ACE_TEXT (error_p->message)));

      // clean up
      g_error_free (error_p);

      goto error;
    } // end IF
    ACE_ASSERT (!hostname_p);
    ACE_ASSERT (!error_p);
    directory_string =
      ACE_TEXT_ALWAYS_CHAR (ACE::dirname (directory_p,
                                          ACE_DIRECTORY_SEPARATOR_CHAR));
    g_free (directory_p); directory_p = NULL;
    ACE_ASSERT (Common_File_Tools::isDirectory (directory_string));
    (*iterator_3).second.second.targetFileName = directory_string;
    (*iterator_3).second.second.targetFileName += ACE_DIRECTORY_SEPARATOR_STR_A;
    (*iterator_3).second.second.targetFileName +=
        ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_DEFAULT_OUTPUT_FILE);

    // step3: connect to peer
    if (data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0)
    {
#if defined (SSL_SUPPORT)
      if (use_SSL)
        iconnector_p = &ssl_connector;
      else
#endif // SSL_SUPPORT
        iconnector_p = &connector;
    } // end IF
    else
    {
#if defined (SSL_SUPPORT)
      // *TODO*: add SSL support to the proactor framework
      ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
      iconnector_p = &asynch_connector;
    } // end ELSE
    if (!iconnector_p->initialize (*dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
      goto error;
    } // end IF

//    // step3a: initialize connection manager
//    peer_address =
//        data_p->configuration->socketConfiguration.address;
//    data_p->configuration->socketConfiguration.address =
//        data_p->configuration->listenerConfiguration.address;
//    connection_manager_p->set (*data_p->configuration,
//                               &data_p->configuration->userData);
//    handle_connection_manager = true;

    // step3b: connect
    data_p->handle =
        iconnector_p->connect (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address);
    // *TODO*: support one-thread operation by scheduling a signal and manually
    //         running the dispatch loop for a limited time...
    if (data_p->configuration->dispatchConfiguration.numberOfProactorThreads > 0)
    {
      data_p->handle = ACE_INVALID_HANDLE;

      // *TODO*: avoid tight loop here
      ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_S, 0);
      //result = ACE_OS::sleep (timeout);
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
      //              &timeout));
      ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
      do
      {
        iconnection_p =
            iconnection_manager_p->get (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address,
                                        true);
        if (iconnection_p)
        {
          data_p->handle =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              reinterpret_cast<ACE_HANDLE> (iconnection_p->id ());
#else
              static_cast<ACE_HANDLE> (iconnection_p->id ());
#endif
          iconnection_p->decrease ();
          break;
        } // end IF
      } while (COMMON_TIME_NOW < deadline);
      if (!iconnection_p)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to connect to %s (timed out after: %#T), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address).c_str ()),
                    &timeout));
    } // end IF
    else
      iconnection_p =
        iconnection_manager_p->get (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address,
                                    true);
    if ((data_p->handle == ACE_INVALID_HANDLE) ||
        !iconnection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address).c_str ())));
      goto error;
    } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("0x%@: opened TCP socket to %s\n"),
//                data_p->handle,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address).c_str ())));
//#else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%d: opened TCP socket to %s\n"),
//                data_p->handle,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->address).c_str ())));
//#endif

    // step4: send HTTP request
    ACE_ASSERT (iconnection_p);
    istream_connection_p =
      dynamic_cast<Test_I_IStreamConnection_t*> (iconnection_p);
    ACE_ASSERT (istream_connection_p);
    //struct HTTP_ConnectionState& state_r =
    //    const_cast<struct HTTP_ConnectionState&> (istream_connection_p->state ());

    ACE_NEW_NORETURN (HTTP_record_p,
                      struct HTTP_Record ());
    if (!HTTP_record_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      goto error;
    } // end IF
    HTTP_record_p->form = HTTP_form;
    HTTP_record_p->headers = HTTP_headers;
    HTTP_record_p->method =
      (HTTP_form.empty () ? HTTP_Codes::HTTP_METHOD_GET
                          : HTTP_Codes::HTTP_METHOD_POST);
    HTTP_record_p->URI = (*iterator_3).second.second.URL;
    HTTP_record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

    ACE_NEW_NORETURN (message_data_p,
                      Test_I_Message::DATA_T ());
    if (!message_data_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
      delete HTTP_record_p; HTTP_record_p = NULL;
      goto error;
    } // end IF
    // *IMPORTANT NOTE*: fire-and-forget API (HTTP_record_p)
    message_data_p->setPR (HTTP_record_p);

    ACE_ASSERT ((*iterator_2).second->allocatorConfiguration);
    pdu_size_i =
      (*iterator_2).second->allocatorConfiguration->defaultBufferSize;// +
//      (*iterator_2).second->allocatorConfiguration->paddingBytes;

    ACE_ASSERT (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator);
allocate:
    message_p =
      static_cast<Test_I_Message*> (dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator->malloc (pdu_size_i));
    // keep retrying ?
    if (!message_p &&
        !dynamic_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator->block ())
      goto allocate;
    if (!message_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
      delete message_data_p; message_data_p = NULL;
      goto error;
    } // end IF
    // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
    message_p->initialize (message_data_p,
                           message_p->sessionId (),
                           NULL);

    //Test_I_ConnectionStream& stream_r =
    //    const_cast<Test_I_ConnectionStream&> (istream_connection_p->stream ());
    //const Test_I_URLStreamLoad_SessionData_t* session_data_container_p =
    //  &stream_r.get ();
    //ACE_ASSERT (session_data_container_p);
    //struct Test_I_URLStreamLoad_SessionData& session_data_r =
    //    const_cast<struct Test_I_URLStreamLoad_SessionData&> (session_data_container_p->get ());

    message_block_p = message_p;
//    istream_connection_p->send (message_block_p);
    message_p->release ();

    // clean up
    iconnection_p->decrease (); iconnection_p = NULL;

    success = true;

continue_:
    if (!success)
      goto error;

    // step3: start progress reporting
    spinner_p =
      GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
    ACE_ASSERT (spinner_p);
    gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), TRUE);
    gtk_spinner_start (spinner_p);
    progressbar_p =
        GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), TRUE);

    ACE_ASSERT (!data_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      data_p->progressData.eventSourceId =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &data_p->progressData,
        //                 NULL);
        g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                            COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
                            idle_update_progress_cb,
                            &data_p->progressData,
                            NULL);
      if (data_p->progressData.eventSourceId > 0)
        state_r.eventSourceIds.insert (data_p->progressData.eventSourceId);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", aborting\n")));
        goto error;
      } // end IF
    } // end lock scope

    return;
  } // end IF

  // --> disconnect

  ACE_ASSERT (data_p->handle != ACE_INVALID_HANDLE);
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif
  if (iconnection_p)
  {
    iconnection_p->close ();
    iconnection_p->decrease ();
  } // end IF
  data_p->handle = ACE_INVALID_HANDLE;

  return;

error:
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_CONNECT);
  box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_VBOX_CONFIGURATION_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), true);

  un_toggling_connect = true;
  gtk_toggle_button_set_active (toggleButton_in, false);
} // toggle_button_connect_toggled_cb

gint
button_cut_clicked_cb (GtkWidget* widget_in,
                       gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_cut_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

  int result = -1;

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
  int signal = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signal = SIGBREAK;
#else
  signal = SIGUSR1;
#endif // ACE_WIN32 || ACE_WIN64
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(\"%S\" (%d)): \"%m\", continuing\n"),
                signal, signal));

  return FALSE;
} // button_cut_clicked_cb

void
entry_url_activate_cb (GtkEntry* entry_in,
                       gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::entry_url_activate_cb"));

  ACE_UNUSED_ARG (entry_in);

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR_2 ();

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_CONNECT_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_toggled (toggle_button_p);
}

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_URLStreamLoad_UI_CBData* data_p =
      static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR_2 ();

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
    return TRUE; // propagate
  } // end IF

  // run dialog
  gint result = gtk_dialog_run (about_dialog);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  gtk_widget_hide (GTK_WIDGET (about_dialog));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);
  //struct Test_I_URLStreamLoad_UI_CBData* data_p =
  //  static_cast<struct Test_I_URLStreamLoad_UI_CBData*> (userData_in);
  //// sanity check(s)
  //ACE_ASSERT (data_p);

  //// step1: remove event sources
  //{ ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->lock);
  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->eventSourceIds.begin ();
  //       iterator != data_p->eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->eventSourceIds.clear ();
  //} // end lock scope

  // step2: initiate shutdown sequence
  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int signal = SIGINT;
#else
  int signal = SIGQUIT;
#endif
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, true);

  return FALSE;
} // button_quit_clicked_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
