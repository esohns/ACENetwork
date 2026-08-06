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

#include "test_i_gtk_callbacks.h"

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gl/GL.h"
#include "gl/GLU.h"
#else
#include "GL/gl.h"
#include "GL/glu.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "gdk/gdk.h"
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#include "gtkgl/gtkglarea.h"
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#include "gtkgl/gtkglarea.h"
#else
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

#include <iomanip>
#include <limits>
#include <sstream>

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_gl_defines.h"
#include "common_gl_tools.h"

#include "common_file_tools.h"

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_macros.h"

#include "net_client_common_tools.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_common_modules.h"
#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_connection_stream.h"
#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_icecast_client_common.h"
#include "test_i_icecast_client_defines.h"

// initialize statics
static bool un_toggling_connect = false;
#if defined (GTKGL_SUPPORT)
static GLuint dummy_vao_i = 0;
#endif // GTKGL_SUPPORT

/////////////////////////////////////////

gboolean
idle_end_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_cb"));

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersConstIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

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
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), TRUE);

  // stop progress reporting
  GtkSpinner* spinner_p =
    GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
  ACE_ASSERT (spinner_p);
  gtk_spinner_stop (spinner_p);
  gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), FALSE);

  ACE_ASSERT (data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    if (!g_source_remove (data_p->progressData.eventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  data_p->progressData.eventSourceId));
    data_p->UIState->eventSourceIds.erase (data_p->progressData.eventSourceId);
    data_p->progressData.eventSourceId = 0;
  } // end lock scope
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), FALSE);
  //gtk_progress_bar_set_show_text (progress_bar_p, FALSE);

  un_toggling_connect = true;
  gtk_toggle_button_toggled (toggle_button_p);

  return G_SOURCE_REMOVE;
}

gboolean
idle_load_segment_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_load_segment_cb"));

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->streamConfiguration_2.configuration_);
  ACE_ASSERT (data_p->handle != ACE_INVALID_HANDLE);

  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_p)
  {
    iconnection_p->abort ();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF
  data_p->handle = ACE_INVALID_HANDLE;

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // update configuration
  data_p->configuration->streamConfiguration_2.configuration_->URL =
    data_p->URL;
  Test_I_IceCastClient_StreamConfiguration_2_t::ITERATOR_T iterator_3 =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration_2.end ());
  ACE_INET_Addr host_address;
  std::string hostname_string, hostname_string_2, URI_string, URL_string;
  bool use_SSL = false;
  bool URI_is_relative_b;
  if (HTTP_Tools::URLIsURI (data_p->URL,
                            URI_is_relative_b))
  { ACE_ASSERT (URI_is_relative_b);
    if (!HTTP_Tools::parseURL ((*iterator_3).second.second->URL,
                               host_address,
                               hostname_string,
                               URI_string,
                               use_SSL))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                 ACE_TEXT (data_p->URL.c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF
    URL_string = ACE_TEXT_ALWAYS_CHAR ("http");
    URL_string +=
      (use_SSL ? ACE_TEXT_ALWAYS_CHAR ("s") : ACE_TEXT_ALWAYS_CHAR (""));
    URL_string += ACE_TEXT_ALWAYS_CHAR ("://");
    URL_string += hostname_string;
    size_t position = URI_string.find_last_of ('/', std::string::npos);
    ACE_ASSERT (position != std::string::npos);
    URI_string.erase (position + 1, std::string::npos);
    URL_string += URI_string;
    URL_string += data_p->URL;
  } // end IF
  else
    URL_string = data_p->URL;
  (*iterator_3).second.second->URL = URL_string;
  (*iterator_3).second.second->parserConfiguration->messageQueue = NULL;

  // select connector
  size_t position;
  int result;
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("2"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  if (!HTTP_Tools::parseURL (URL_string,
                             host_address,
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
  position = hostname_string_2.find_last_of (':', std::string::npos);
  if (position == std::string::npos)
  {
    hostname_string_2 += ':';
    std::ostringstream converter;
    converter << (use_SSL ? HTTPS_DEFAULT_SERVER_PORT
                          : HTTP_DEFAULT_SERVER_PORT);
    hostname_string_2 += converter.str ();
  } // end IF
  hostname_string =
    Net_Common_Tools::URLToHostName (URL_string,
                                     false,  // return hostname
                                     false); // do not return port#
  static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.hostname =
    hostname_string;
  result =
    static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address.set (hostname_string_2.c_str (),
                                                                                                                            AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (hostname_string_2.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.useLoopBackDevice =
    static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address.is_loopback ();

  Test_I_TCPConnector_2_t connector;
#if defined (SSL_SUPPORT)
  Test_I_SSLConnector_2_t ssl_connector;
#endif // SSL_SUPPORT
  Test_I_AsynchTCPConnector_2_t asynch_connector;
  struct Net_UserData user_data_s;

  // step3: connect to peer
  if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      data_p->handle = Net_Client_Common_Tools::connect (ssl_connector,
                                                         *static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second),
                                                         user_data_s,
                                                         static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address,
                                                         true,
                                                         true,
                                                         0);
    else
#endif // SSL_SUPPORT
      data_p->handle = Net_Client_Common_Tools::connect (connector,
                                                         *static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second),
                                                         user_data_s,
                                                         static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address,
                                                         true,
                                                         true,
                                                         0);
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    data_p->handle = Net_Client_Common_Tools::connect (asynch_connector,
                                                       *static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second),
                                                       user_data_s,
                                                       static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address,
                                                       true,
                                                       true,
                                                       0);
  } // end ELSE
  if (data_p->handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("0x%@: opened TCP socket to %s\n"),
//                data_p->handle,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address).c_str ())));
//#else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%d: opened TCP socket to %s\n"),
//                data_p->handle,
//                ACE_TEXT (Net_Common_Tools::IPAddressToString (dynamic_cast<Test_I_IceCastClient_ConnectionConfiguration_2_t*> ((*iterator_2).second)->address).c_str ())));
//#endif

  Test_I_ConnectionManager_2_t::INTERFACE_T* iconnection_manager_2 =
    TEST_I_CONNECTIONMANAGER_SINGLETON_2::instance ();
  ACE_ASSERT (iconnection_manager_2);
  Test_I_ConnectionManager_2_t::ICONNECTION_T* iconnection_2 = NULL;
  Test_I_IStreamConnection_2_t* istream_connection_p = NULL;
  Stream_IStream_t* istream_p = NULL;
  const Stream_Module_t* module_p = NULL;

  iconnection_2 =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    iconnection_manager_2->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    iconnection_manager_2->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  if (!iconnection_2)
    return G_SOURCE_REMOVE;
  istream_connection_p =
    dynamic_cast<Test_I_IStreamConnection_2_t*> (iconnection_2);
  ACE_ASSERT (istream_connection_p);
  istream_p = &const_cast<Test_I_ConnectionStream_2&> (istream_connection_p->stream ());
  ACE_ASSERT (istream_p);

  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
    iconnection_2->decrease ();
    return G_SOURCE_REMOVE;
  } // end IF
  data_p->fft =
    dynamic_cast<Common_Math_FFT_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (unlikely (!data_p->fft))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_Math_FFT_t*>(%@), returning\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    iconnection_2->decrease ();
    return G_SOURCE_REMOVE;
  } // end IF
  data_p->spectrumAnalyzerCBData.dispatch =
    dynamic_cast<Common_IDispatch*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (unlikely (!data_p->spectrumAnalyzerCBData.dispatch))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_IDispatch*>(%@), returning\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    iconnection_2->decrease ();
    return G_SOURCE_REMOVE;
  } // end IF
  data_p->spectrumAnalyzerCBData.resizeNotification =
    dynamic_cast<Common_ISetP_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (unlikely (!data_p->spectrumAnalyzerCBData.resizeNotification))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_ISetP_t*>(%@), returning\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    iconnection_2->decrease ();
    return G_SOURCE_REMOVE;
  } // end IF
  iconnection_2->decrease (); iconnection_2 = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

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
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->UIState);
  // Common_UI_GTK_Manager_t* gtk_manager_p =
  //   COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  // ACE_ASSERT (gtk_manager_p);
  // Common_UI_GTK_State_t& state_r =
  //   const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

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
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT64>::max ());
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
//  size_t pdu_size_i =
//    (*iterator_2).second->allocatorConfiguration->defaultBufferSize +
//    (*iterator_2).second->allocatorConfiguration->paddingBytes;
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> ((*iterator_2).second->allocatorConfiguration->defaultBufferSize));

  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_URL_NAME)));
  ACE_ASSERT (entry_p);
  Test_I_IceCastClient_StreamConfiguration_2_t::ITERATOR_T iterator_3 =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration_2.end ());
  gchar* text_p =
    Common_UI_GTK_Tools::localeToUTF8 ((*iterator_3).second.second->URL);
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
  if (!(*iterator_3).second.second->targetFileName.empty ())
  {
    // *NOTE*: gtk does not complain if the file doesn't exist, but the button
    //         will display "(None)" --> create empty file
    if (!Common_File_Tools::isReadable ((*iterator_3).second.second->targetFileName))
      if (!Common_File_Tools::create ((*iterator_3).second.second->targetFileName))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::create(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT ((*iterator_3).second.second->targetFileName.c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF

    string_p =
      g_string_new (Common_File_Tools::directory ((*iterator_3).second.second->targetFileName).c_str ());
    filename_p = string_p->str;
      //Common_UI_GTK_Tools::Locale2UTF8 (data_p->configuration->moduleHandlerConfiguration.targetFileName);
    if (!gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                              filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT ((*iterator_3).second.second->targetFileName.c_str ())));
      g_string_free (string_p, FALSE);
      g_free (filename_p);
      return G_SOURCE_REMOVE;
    } // end IF
    g_string_free (string_p, FALSE);
    g_free (filename_p);
  } // end IF
  else
  {
    string_p = g_string_new (Common_File_Tools::getTempDirectory ().c_str ());
    filename_p = string_p->str;
    if (!gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                              filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ())));
      g_string_free (string_p, FALSE);
      g_free (filename_p);
      return G_SOURCE_REMOVE;
    } // end IF
    g_string_free (string_p, FALSE);
    g_free (filename_p);
  } // end ELSE

  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_SAVE_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                !(*iterator_3).second.second->targetFileName.empty ());
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_PROACTOR));
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address.is_loopback ());

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
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_CONTEXT_DATA));
  data_p->UIState->contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_DATA,
                                                      context_id));
  context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_CONTEXT_INFORMATION));
  data_p->UIState->contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                                      context_id));

  // step5: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
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
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                     idle_update_info_display_cb,
                     data_p);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

    // schedule asynchronous updates of the display views
    data_p->eventSourceId =
      g_timeout_add (std::min (COMMON_UI_GTK_REFRESH_DEFAULT_CAIRO_MS, COMMON_UI_GTK_REFRESH_DEFAULT_OPENGL_MS),
                     idle_update_display_cb,
                     userData_in);
    if (data_p->eventSourceId > 0)
      data_p->UIState->eventSourceIds.insert (data_p->eventSourceId);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(idle_update_display_cb): \"%m\", continuing\n")));
  } // end lock scope

  // step6: disable some functions ?
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

#if defined (GTKGL_SUPPORT)
  GtkBox* box_p = NULL;
  Common_UI_GTK_GLContextsIterator_t opengl_contexts_iterator;
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea* gl_area_p = NULL;
  GdkGLContext* gl_context_p = NULL;
  GError* error_p = NULL;
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  /* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GDK_GL_NONE. See glXChooseVisual manpage for further
     explanation.
  */
  int gl_attributes_a[] = {GDK_GL_USE_GL,
                           // GDK_GL_BUFFER_SIZE
                           // GDK_GL_LEVEL
                           GDK_GL_RGBA, GDK_GL_DOUBLEBUFFER,
                           //    GDK_GL_STEREO
                           //    GDK_GL_AUX_BUFFERS
                           GDK_GL_RED_SIZE, 1, GDK_GL_GREEN_SIZE, 1,
                           GDK_GL_BLUE_SIZE, 1, GDK_GL_ALPHA_SIZE, 1,
                           //    GDK_GL_DEPTH_SIZE
                           //    GDK_GL_STENCIL_SIZE
                           //    GDK_GL_ACCUM_RED_SIZE
                           //    GDK_GL_ACCUM_GREEN_SIZE
                           //    GDK_GL_ACCUM_BLUE_SIZE
                           //    GDK_GL_ACCUM_ALPHA_SIZE
                           //
                           //    GDK_GL_X_VISUAL_TYPE_EXT
                           //    GDK_GL_TRANSPARENT_TYPE_EXT
                           //    GDK_GL_TRANSPARENT_INDEX_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_RED_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_GREEN_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_BLUE_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_ALPHA_VALUE_EXT
                           GDK_GL_NONE};
  GtkGLArea* gl_area_p = NULL;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT

#if defined (GTKGL_SUPPORT)
  box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BOX_DISPLAY_NAME)));
  ACE_ASSERT (box_p);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  gl_area_p = GTK_GL_AREA (gtk_gl_area_new ());
  ACE_ASSERT (gl_area_p);
  gtk_widget_realize (GTK_WIDGET (gl_area_p));
  gl_context_p = gtk_gl_area_get_context (gl_area_p);
  //ACE_ASSERT (gl_context_p);
  data_p->UIState->OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                          gl_context_p));
  opengl_contexts_iterator = data_p->UIState->OpenGLContexts.find (gl_area_p);

  gint major_version, minor_version;
  gtk_gl_area_get_required_version (gl_area_p,
                                    &major_version,
                                    &minor_version);
#else
#if defined (GTKGLAREA_SUPPORT)
  /* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GGLA_NONE. See glXChooseVisual manpage for further
     explanation.
  */
  int attribute_list[] = {
    GGLA_RGBA,
    GGLA_RED_SIZE,   1,
    GGLA_GREEN_SIZE, 1,
    GGLA_BLUE_SIZE,  1,
    GGLA_DOUBLEBUFFER,
    GGLA_NONE
  };

  GglaArea* gl_area_p = GGLA_AREA (ggla_area_new (attribute_list));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to ggla_area_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  data_p->UIState->OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                          gl_area_p->glcontext));
  opengl_contexts_iterator = data_p->UIState->OpenGLContexts.find (gl_area_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (G_SOURCE_REMOVE);
  ACE_NOTREACHED (return G_SOURCE_REMOVE;)
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
  gl_area_p =
    GTK_GL_AREA (gtk_gl_area_new (gl_attributes_a));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_gl_area_new(), aborting\n")));
    goto error;
  } // end IF

  data_p->UIState->OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                          gl_area_p->glcontext));
  opengl_contexts_iterator = data_p->UIState->OpenGLContexts.find (gl_area_p);
#else
  GdkGLConfigMode features = static_cast<GdkGLConfigMode> (GDK_GL_MODE_DOUBLE  |
                                                           GDK_GL_MODE_ALPHA   |
                                                           GDK_GL_MODE_DEPTH   |
                                                           GDK_GL_MODE_STENCIL |
                                                           GDK_GL_MODE_ACCUM);
  GdkGLConfigMode configuration_mode =
    static_cast<GdkGLConfigMode> (GDK_GL_MODE_RGBA | features);
  GdkGLConfig* gl_config_p = gdk_gl_config_new_by_mode (configuration_mode);
  if (!gl_config_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_gl_config_new_by_mode(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  if (!gtk_widget_set_gl_capability (GTK_WIDGET (drawing_area_2), // widget
                                     gl_config_p,                 // configuration
                                     NULL,                        // share list
                                     true,                        // direct
                                     GDK_GL_RGBA_TYPE))           // render type
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_widget_set_gl_capability(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  state_r.OpenGLContexts.insert (std::make_pair (gtk_widget_get_window (GTK_WIDGET (drawing_area_2)),
                                                 gl_config_p));
  opengl_contexts_iterator = ui_cb_data_base_p->UIState.OpenGLContexts.find (gl_area_p);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION(3,0,0) */
  ACE_ASSERT (opengl_contexts_iterator != data_p->UIState->OpenGLContexts.end ());

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK     |
                         GDK_BUTTON_PRESS_MASK |
                         GDK_KEY_PRESS_MASK);
#else
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK     |
                         GDK_BUTTON_PRESS_MASK |
                         GDK_KEY_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_events (GTK_WIDGET ((*opengl_contexts_iterator).first),
                         GDK_EXPOSURE_MASK     |
                         GDK_BUTTON_PRESS_MASK |
                         GDK_KEY_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,0,0) */

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
//   // *NOTE*: (try to) enable legacy mode on Win32
// #if defined (ACE_WIN32) || defined (ACE_WIN64)
  // gtk_gl_area_set_required_version ((*opengl_contexts_iterator).first, 2, 1);
// #endif // ACE_WIN32 || ACE_WIN64
  // gtk_gl_area_set_use_es ((*opengl_contexts_iterator).first, TRUE);
  // *WARNING*: the 'renderbuffer' (in place of 'texture') image attachment
  //            concept appears to be broken; setting this to 'false' gives
  //            "fb setup not supported" (see: gtkglarea.c:734)
  // *TODO*: more specifically, glCheckFramebufferStatusEXT() returns
  //         GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT; find out what is
  //         going on
  // *TODO*: the depth buffer feature is broken on Win32
  gtk_gl_area_set_has_alpha ((*opengl_contexts_iterator).first, TRUE);
  gtk_gl_area_set_has_depth_buffer ((*opengl_contexts_iterator).first, TRUE);
  gtk_gl_area_set_has_stencil_buffer ((*opengl_contexts_iterator).first, FALSE);
  gtk_gl_area_set_auto_render ((*opengl_contexts_iterator).first, TRUE);
  gtk_widget_set_app_paintable (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_can_focus (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_focus_on_click (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_hexpand (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_visible (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_double_buffered (GTK_WIDGET ((*opengl_contexts_iterator).first), FALSE);
#else
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_app_paintable (GTK_WIDGET (gl_area_p), TRUE);
  gtk_widget_set_can_focus (GTK_WIDGET (gl_area_p), TRUE);
  //gtk_widget_set_focus_on_click (GTK_WIDGET (gl_area_p), TRUE);
  gtk_widget_set_hexpand (GTK_WIDGET (gl_area_p), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (gl_area_p), TRUE);
  gtk_widget_set_visible (GTK_WIDGET (gl_area_p), TRUE);
  gtk_widget_set_double_buffered (GTK_WIDGET (gl_area_p), FALSE);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */

  GtkDrawingArea* drawing_area_2 =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_PROJECTM_NAME)));
  ACE_ASSERT (drawing_area_2);
  //GList* children_p, *iterator_p;
  //children_p = gtk_container_get_children (GTK_CONTAINER (box_p));
  //for (iterator_p = children_p;
  //     iterator_p != NULL;
  //     iterator_p = g_list_next (iterator_p))
  //  if (GTK_WIDGET (iterator_p->data) == GTK_WIDGET (drawing_area_2))
  //  {
  //    gtk_widget_destroy (GTK_WIDGET (iterator_p->data));
  //    break;
  //  } // end IF
  gtk_widget_destroy (GTK_WIDGET (drawing_area_2));
  gtk_box_pack_start (box_p,
                      GTK_WIDGET ((*opengl_contexts_iterator).first),
                      TRUE, // expand
                      TRUE, // fill
                      0);   // padding
#if GTK_CHECK_VERSION (3,8,0)
//  gtk_builder_expose_object ((*iterator).second.second,
//                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GLAREA_3D_NAME),
//                             G_OBJECT ((*opengl_contexts_iterator).first));
#endif /* GTK_CHECK_VERSION (3,8,0) */
#endif /* GTKGL_SUPPORT */

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

#if defined (GTKGL_SUPPORT)
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("key-press-event"),
                      G_CALLBACK (glarea_key_press_cb),
                      userData_in);
  ACE_ASSERT (result);
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("button-press-event"),
                      G_CALLBACK (glarea_clicked_cb),
                      userData_in);
  ACE_ASSERT (result);
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("realize"),
                      G_CALLBACK (glarea_realize_cb),
                      userData_in);
  ACE_ASSERT (result);
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("unrealize"),
                      G_CALLBACK (glarea_unrealize_cb),
                      userData_in);
  ACE_ASSERT (result);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
 result =
   g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                     ACE_TEXT_ALWAYS_CHAR ("create-context"),
                     G_CALLBACK (glarea_create_context_cb),
                     userData_in);
 ACE_ASSERT (result);
 result =
   g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                     ACE_TEXT_ALWAYS_CHAR ("render"),
                     G_CALLBACK (glarea_render_cb),
                     userData_in);
 ACE_ASSERT (result);
 result =
   g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                     ACE_TEXT_ALWAYS_CHAR ("resize"),
                     G_CALLBACK (glarea_resize_cb),
                     userData_in);
#else
#if defined (GTKGLAREA_SUPPORT)
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (glarea_configure_event_cb),
                      userData_in);
  ACE_ASSERT (result);
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("draw"),
                      G_CALLBACK (glarea_expose_event_cb),
                      userData_in);
  ACE_ASSERT (result);
#else
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                      G_CALLBACK (glarea_size_allocate_event_cb),
                      userData_in);
  ACE_ASSERT (result);
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("draw"),
                      G_CALLBACK (glarea_draw_cb),
                      userData_in);
  ACE_ASSERT (result);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (glarea_configure_event_cb),
                      userData_in);
  ACE_ASSERT (result);
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                      G_CALLBACK (glarea_expose_event_cb),
                      userData_in);
  ACE_ASSERT (result);
#else
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (glarea_configure_event_cb),
                      userData_in);
  ACE_ASSERT (result);
  result =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                      G_CALLBACK (glarea_expose_event_cb),
                      userData_in);
  ACE_ASSERT (result);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT

  //-------------------------------------

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT (window_p);
  ACE_ASSERT ((*iterator_3).second.second->spectrumAnalyzerConfiguration);
  //(*iterator_3).second.second->window = window_p;
  (*iterator_3).second.second->spectrumAnalyzerConfiguration->window = window_p;

  return G_SOURCE_REMOVE;
}

gboolean
idle_reset_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_reset_UI_cb"));

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

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
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

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
  struct Test_I_IceCastClient_UI_ProgressData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
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
    result = ACE_OS::sprintf (buffer_a, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_pulse (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (buffer_a));
  gtk_progress_bar_set_show_text (progress_bar_p, TRUE);

  // --> reschedule
  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
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
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

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
        case COMMON_UI_EVENT_ABORT:
        case COMMON_UI_EVENT_STEP:
        case COMMON_UI_EVENT_FORMAT:
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

gboolean
idle_update_display_cb (gpointer userData_in)
{
  // NETWORK_TRACE (ACE_TEXT ("::idle_update_display_cb"));

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersConstIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // trigger refresh of the 2D area ?
  GtkDrawingArea* drawing_area_p = NULL;
  GdkWindow* window_p = NULL;
  drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  if (unlikely (!window_p))
    goto continue_2; // <-- not realized yet

  gdk_window_invalidate_rect (window_p,
                              NULL,   // whole window
                              FALSE); // invalidate children ?

continue_2:
#if defined (GTKGL_SUPPORT)
  ACE_ASSERT (!data_p->UIState->OpenGLContexts.empty ());
  Common_UI_GTK_GLContextsIterator_t iterator_2 =
    data_p->UIState->OpenGLContexts.begin ();
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  window_p = gtk_widget_get_window (GTK_WIDGET ((*iterator_2).first));
#else
  window_p = gtk_widget_get_window (GTK_WIDGET (&(*iterator_2).first->darea));
#endif // GTK_CHECK_VERSION (3,16,0)
#else
  window_p = gtk_widget_get_window (GTK_WIDGET (&(*iterator_2).first->darea));
#endif // GTK_CHECK_VERSION
  if (unlikely (!window_p))
    goto continue_3; // <-- not realized yet

  gdk_window_invalidate_rect (window_p,
                              NULL,
                              FALSE);
continue_3:
#endif /* GTKGL_SUPPORT */

  return G_SOURCE_CONTINUE;
}

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
  { un_toggling_connect = false;
    return;
  } // end IF

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_2_t::INTERFACE_T* iconnection_manager_2 =
    TEST_I_CONNECTIONMANAGER_SINGLETON_2::instance ();
  ACE_ASSERT (iconnection_manager_2);
  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  Test_I_ConnectionManager_2_t::ICONNECTION_T* iconnection_2 = NULL;
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
    ACE_INET_Addr host_address;
    std::string URI_string, directory_string, hostname_string;
    bool use_SSL = false;
    std::string hostname_string_2;
    size_t position = std::string::npos;
    int result = -1;
    GtkCheckButton* check_button_p = NULL;
    GtkFileChooserButton* file_chooser_button_p = NULL;
    gchar* directory_p = NULL;
    Net_ConnectionConfigurationsIterator_t iterator_2 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
    Test_I_IceCastClient_StreamConfiguration_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
    Test_I_IceCastClient_StreamConfiguration_2_t::ITERATOR_T iterator_4 =
      data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_4 != data_p->configuration->streamConfiguration_2.end ());
    Test_I_TCPConnector_t connector;
#if defined (SSL_SUPPORT)
    Test_I_SSLConnector_t ssl_connector;
#endif // SSL_SUPPORT
    Test_I_AsynchTCPConnector_t asynch_connector;
    GtkSpinner* spinner_p = NULL;
    GtkProgressBar* progress_bar_p = NULL;
    struct Net_UserData user_data_s;

    // retrieve buffer size
    spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
    ACE_ASSERT (spin_button_p);
    static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.bufferSize =
      static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));

    (*iterator_3).second.second->parserConfiguration->messageQueue = NULL;
    (*iterator_4).second.second->parserConfiguration->messageQueue = NULL;
    (*iterator_4).second.second->delayConfiguration->mode =
      STREAM_MISCELLANEOUS_DELAY_MODE_INVALID;

    // retrieve stream URL
    entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ENTRY_URL_NAME)));
    ACE_ASSERT (entry_p);
    (*iterator_3).second.second->URL =
      Common_UI_GTK_Tools::UTF8ToLocale (gtk_entry_get_text (entry_p), -1);
    data_p->URL = (*iterator_3).second.second->URL; // remember the request URL
    if (!HTTP_Tools::parseURL ((*iterator_3).second.second->URL,
                               host_address,
                               hostname_string,
                               URI_string,
                               use_SSL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                  ACE_TEXT ((*iterator_3).second.second->URL.c_str ())));
      goto error;
    } // end IF
    hostname_string_2 = hostname_string; // save with (potential) port#
    // remove (potential) port#
    hostname_string =
      Net_Common_Tools::URLToHostName ((*iterator_3).second.second->URL,
                                       false,  // return hostname
                                       false); // do not return port#
    static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.hostname =
      hostname_string;

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
      static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address.set (hostname_string_2.c_str (),
                                                                                                                            AF_INET);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (hostname_string_2.c_str ())));
      goto error;
    } // end IF
    static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.useLoopBackDevice =
      static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address.is_loopback ();

    check_button_p =
      GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
    ACE_ASSERT (check_button_p);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                  static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address.is_loopback ());

    // save to file ?
    check_button_p =
      GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_CHECKBUTTON_SAVE_NAME)));
    ACE_ASSERT (check_button_p);
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_p)))
    {
      (*iterator_3).second.second->targetFileName.clear ();
      goto continue_2;
    } // end IF
    // retrieve output filename
    file_chooser_button_p =
      GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
    ACE_ASSERT (file_chooser_button_p);
    directory_p =
      gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p));
    if (!directory_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_get_current_folder(), aborting\n")));
      goto error;
    } // end IF
    directory_string =
      Common_File_Tools::directory (Common_UI_GTK_Tools::UTF8ToLocale (directory_p, -1));
    g_free (directory_p); directory_p = NULL;
    ACE_ASSERT (Common_File_Tools::isDirectory (directory_string));
    (*iterator_3).second.second->targetFileName = directory_string;
    (*iterator_3).second.second->targetFileName += ACE_DIRECTORY_SEPARATOR_STR_A;
    (*iterator_3).second.second->targetFileName +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_OUTPUT_FILE);

continue_2:
    // step3: connect to peer
    if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
    {
#if defined (SSL_SUPPORT)
      if (use_SSL)
        data_p->handle =
          Net_Client_Common_Tools::connect (ssl_connector,
                                            *static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            true,
                                            true,
                                            0);
      else
#endif // SSL_SUPPORT
        data_p->handle =
            Net_Client_Common_Tools::connect (connector,
                                              *static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second),
                                              user_data_s,
                                              static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                              true,
                                              true,
                                              0);
    } // end IF
    else
    {
#if defined (SSL_SUPPORT)
      // *TODO*: add SSL support to the proactor framework
      ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
      data_p->handle =
          Net_Client_Common_Tools::connect (asynch_connector,
                                            *static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            true,
                                            true,
                                            0);
    } // end ELSE
    if (data_p->handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_IceCastClient_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
      goto error;
    } // end IF

    // step3: start progress reporting
    spinner_p =
      GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
    ACE_ASSERT (spinner_p);
    gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), TRUE);
    gtk_spinner_start (spinner_p);
    progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progress_bar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), TRUE);
    gtk_progress_bar_set_show_text (progress_bar_p, TRUE);

    //ACE_ASSERT (!data_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      data_p->progressData.eventSourceId =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &data_p->progressData,
        //                 NULL);
        g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                       COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                       idle_update_progress_cb,
                       &data_p->progressData);//,
//                       NULL);
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
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_p)
  {
    iconnection_p->abort ();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF

  iconnection_2 =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    iconnection_manager_2->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    iconnection_manager_2->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_2)
  {
    iconnection_2->abort ();
    iconnection_2->decrease (); iconnection_2 = NULL;
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
  gtk_widget_set_sensitive (GTK_WIDGET (box_p), TRUE);

  un_toggling_connect = true;
  gtk_toggle_button_set_active (toggleButton_in, FALSE);
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
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

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

gboolean
drawingarea_query_tooltip_cb (GtkWidget*  widget_in,
                              gint        x_in, gint y_in,
                              gboolean    keyboardMode_in,
                              GtkTooltip* tooltip_in,
                              gpointer    userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_query_tooltip_cb"));

  ACE_UNUSED_ARG (keyboardMode_in);

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  if (!data_p->fft)
    return FALSE;
  Test_I_IceCastClient_StreamConfiguration_2_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration_2.end ());

  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode =
    STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID;
  unsigned int sample_size = 0; // bytes
  bool is_signed_format = true;
  bool is_float_format = false;
  unsigned int channels = 0;

  mode =
    (*modulehandler_configuration_iterator).second.second->spectrumAnalyzerConfiguration->mode;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> ((*modulehandler_configuration_iterator).second.second->outputFormat.pbFormat);
  ACE_ASSERT (waveformatex_p);
  sample_size = waveformatex_p->wBitsPerSample / 8;
  channels = waveformatex_p->nChannels;
  // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
  //         are unsigned values. (Each audio sample has the range 0255.)
  //         If the audio contains 16 bits per sample or higher, the audio
  //         samples are signed values. ..."
  is_signed_format = !(sample_size == 1);
  is_float_format = Stream_MediaFramework_DirectSound_Tools::isFloat (*waveformatex_p);
#else
  is_signed_format =
    snd_pcm_format_signed ((*modulehandler_configuration_iterator).second.second->outputFormat.format);
  sample_size =
    (snd_pcm_format_width ((*modulehandler_configuration_iterator).second.second->outputFormat.format) / 8);
  channels =
    (*modulehandler_configuration_iterator).second.second->outputFormat.channels;
#endif // ACE_WIN32 || ACE_WIN64

  GtkAllocation allocation;
  gtk_widget_get_allocation (widget_in,
                             &allocation);
  double half_height = allocation.height / 2.0;
  uint64_t maximum_value =
    (is_float_format ? 1 : Common_Tools::max<uint64_t> (sample_size,
                                                        is_signed_format));
  std::ostringstream converter;
  if (is_float_format)
    converter << static_cast<float> (((half_height - y_in) * static_cast<int64_t> (maximum_value)) / half_height);
  else if (is_signed_format)
    converter << static_cast<int64_t> (((half_height - y_in) * static_cast<int64_t> (maximum_value)) / half_height);
  else
    converter << (static_cast<uint64_t> (allocation.height - y_in) * maximum_value) / static_cast<uint64_t> (allocation.height);
  switch (mode)
  {
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE:
      break;
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM:
    {
      unsigned int allocation_per_channel = (allocation.width / channels);
      unsigned int slot =
        static_cast<unsigned int> ((x_in % allocation_per_channel) * (data_p->fft->Slots () / static_cast<double> (allocation_per_channel)));
      converter << ACE_TEXT_ALWAYS_CHAR (", ")
                << data_p->fft->Frequency (slot)
                << ACE_TEXT_ALWAYS_CHAR (" Hz");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown mode (was: %d), returning\n"),
                  mode));
      return FALSE;
    }
  } // end SWITCH

  gtk_tooltip_set_text (tooltip_in,
                        converter.str ().c_str ());

  return TRUE;
}

void
drawingarea_realize_cb (GtkWidget* widget_in,
                        gpointer   userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_realize_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_I_IceCastClient_StreamConfiguration_2_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration_2.end ());
} // drawingarea_realize_cb

void
drawingarea_size_allocate_cb (GtkWidget* widget_in,
                              GdkRectangle* allocation_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_size_allocate_cb"));

  ACE_UNUSED_ARG (allocation_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  if (!data_p->spectrumAnalyzerCBData.resizeNotification)
    return;

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  if (!window_p)
    return; // <-- not realized yet

  try {
    data_p->spectrumAnalyzerCBData.resizeNotification->setP (window_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_ISetP_T::setP(), continuing\n")));
  }
} // drawingarea_size_allocate_cb

#if GTK_CHECK_VERSION (3,0,0)
gboolean
drawingarea_configure_event_cb (GtkWidget* widget_in,
                                GdkEvent* event_in,
                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  if (unlikely (event_in->type != GDK_CONFIGURE))
    return FALSE;
  ACE_ASSERT (userData_in);

  drawingarea_size_allocate_cb (widget_in,
                                NULL,
                                userData_in);

  return TRUE;
} // drawingarea_configure_event_cb

gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (context_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  // sanity check(s)
  data_p->spectrumAnalyzerCBData.window =
    gtk_widget_get_window (widget_in);
  if (!data_p->spectrumAnalyzerCBData.window)
    return FALSE; // not realized (yet)
  if (!data_p->spectrumAnalyzerCBData.dispatch)
    return FALSE; // stream not running (yet)
  data_p->spectrumAnalyzerCBData.context = context_in;

  try {
    data_p->spectrumAnalyzerCBData.dispatch->dispatch (&data_p->spectrumAnalyzerCBData);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  return FALSE;
}
#else
gboolean
drawingarea_expose_event_cb (GtkWidget* widget_in,
                             GdkEvent* event_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_expose_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (context_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  // sanity check(s)
  data_p->spectrumAnalyzerCBData.window = gtk_widget_get_window (widget_in);
  if (!data_p->spectrumAnalyzerCBData.window)
    return FALSE; // not realized (yet)
  if (!data_p->spectrumAnalyzerCBData.dispatch)
    return FALSE; // stream not running (yet)
  if (!data_p->spectrumAnalyzerCBData.context)
  {
    data_p->spectrumAnalyzerCBData.context =
      gdk_cairo_create (GDK_DRAWABLE (data_p->spectrumAnalyzerCBData.window));
    if (unlikely (!data_p->spectrumAnalyzerCBData.context))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
      return FALSE;
    } // end IF
  } // end IF

  try {
    data_p->spectrumAnalyzerCBData.dispatch->dispatch (&data_p->spectrumAnalyzerCBData);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }
//  cairo_destroy (data_p->spectrumAnalyzerCBData.context); data_p->spectrumAnalyzerCBData.context = NULL;

  return FALSE;
} // drawingarea_expose_event_cb
#endif // GTK_CHECK_VERSION (3,0,0)

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
      static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

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
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
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
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (data_p);

  // step1: remove event sources
  { ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->UIState->lock);
    for (Common_UI_GTK_EventSourceIdsIterator_t iterator = data_p->UIState->eventSourceIds.begin ();
         iterator != data_p->UIState->eventSourceIds.end ();
         iterator++)
      if (!g_source_remove (*iterator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    *iterator));
    data_p->UIState->eventSourceIds.clear ();
  } // end lock scope

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
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  return FALSE;
} // button_quit_clicked_cb

gboolean
glarea_key_press_cb (GtkWidget* widget_in,
                     GdkEventKey event_in,
                     gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_key_press_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
  ACE_ASSERT (data_p->projectMConfiguration->playlist);
#endif // PROJECTM_SUPPORT

  switch (event_in.keyval)
  {
#if GTK_CHECK_VERSION (3,0,0)
    case GDK_KEY_l:
    case GDK_KEY_L:
#else
    case GDK_l:
    case GDK_L:
#endif // GTK_CHECK_VERSION (3,0,0)
    {
#if defined (PROJECTM_SUPPORT)
      data_p->projectMConfiguration->presetIsLocked =
        !data_p->projectMConfiguration->presetIsLocked;
      projectm_set_preset_locked (data_p->projectMConfiguration->handle,
                                  data_p->projectMConfiguration->presetIsLocked);
#endif // PROJECTM_SUPPORT
      break;
    }
#if GTK_CHECK_VERSION (3,0,0)
    case GDK_KEY_space:
    case GDK_KEY_n:
    case GDK_KEY_N:
#else
    case GDK_space:
    case GDK_n:
    case GDK_N:
#endif // GTK_CHECK_VERSION (3,0,0)
    {
#if defined (PROJECTM_SUPPORT)
      projectm_playlist_play_next (data_p->projectMConfiguration->playlist,
                                   false);
#endif // PROJECTM_SUPPORT
      break;
    }
#if GTK_CHECK_VERSION (3,0,0)
    case GDK_KEY_p:
    case GDK_KEY_P:
#else
    case GDK_p:
    case GDK_P:
#endif // GTK_CHECK_VERSION (3,0,0)
    {
#if defined (PROJECTM_SUPPORT)
      projectm_playlist_play_previous (data_p->projectMConfiguration->playlist,
                                       false);
#endif // PROJECTM_SUPPORT
      break;
    }
    default:
      return FALSE; // propagate
  } // end SWITCH

  return TRUE; // do not propagate
}

gboolean
glarea_clicked_cb (GtkWidget* widget_in,
                   GdkEventButton event_in,
                   gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
  ACE_ASSERT (data_p->projectMConfiguration->playlist);
#endif // PROJECTM_SUPPORT

#if defined (PROJECTM_SUPPORT)
  projectm_playlist_play_next (data_p->projectMConfiguration->playlist,
                               false);
#endif // PROJECTM_SUPPORT

  gtk_widget_grab_focus (widget_in);

  return TRUE; // do not propagate
}

void
glarea_realize_cb (GtkWidget* widget_in,
                   gpointer   userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_realize_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
#endif // PROJECTM_SUPPORT

  GtkAllocation allocation;

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  // sanity check(s)
  ACE_ASSERT (widget_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;

  // sanity check(s)
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea* gl_area_p = GTK_GL_AREA (widget_in);
  ACE_ASSERT (gl_area_p);
  // NOTE*: the OpenGL context has been created at this point
  // GdkGLContext* context_p = gtk_gl_area_get_context (gl_area_p);
  // if (!context_p)
  // {
  //   ACE_DEBUG ((LM_ERROR,
  //               ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
  //               gl_area_p));
  //   return;
  // } // end IF

  gtk_gl_area_attach_buffers (gl_area_p);
  gtk_gl_area_make_current (gl_area_p);
  // gdk_gl_context_make_current (context_p);

  // sanity check(s)
  // ACE_ASSERT (gtk_gl_area_get_has_depth_buffer (gl_area_p));
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
    return;
#else
  GdkGLContext* context_p = gtk_gl_area_get_context (GTK_GL_AREA (widget_in));
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    goto error;
  } // end IF

  bool result = gdk_gl_drawable_make_current (drawable_p,
                                              context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  result = gdk_gl_drawable_gl_begin (drawable_p,
                                     context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if defined (GLEW_SUPPORT)
  GLenum err = glewInit ();
  if (GLEW_OK != err)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glewInit(): \"%s\", continuing\n"),
                ACE_TEXT (glewGetErrorString (err))));
#endif // GLEW_SUPPORT

  glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
  glEnable (GL_BLEND);                                // Enable Semi-Transparency
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable (GL_DEPTH_TEST);                           // Disables Depth Testing
  //glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
  //glDepthMask (GL_TRUE);

  // initialize options
  glClearColor (0.0F, 0.0F, 0.0F, 1.0F); // Black Background

  if (!dummy_vao_i)
    glGenVertexArrays(1, &dummy_vao_i);
  glBindVertexArray (dummy_vao_i);

#if defined (PROJECTM_SUPPORT)
  data_p->projectMConfiguration->handle = projectm_create ();
  if (unlikely (!data_p->projectMConfiguration->handle))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to projectm_create(): \"%s\", returning\n")));
    return;
  } // end IF
  //projectm_set_fps (projectm_configuration.handle,
  //                  TEST_I_ICECAST_CLIENT_DEFAULT_FPS);
  projectm_set_mesh_size (data_p->projectMConfiguration->handle,
                          TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PER_VERTEX_MESH_RESOLUTION_X,
                          TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PER_VERTEX_MESH_RESOLUTION_Y);
  projectm_set_aspect_correction (data_p->projectMConfiguration->handle,
                                  true);
  projectm_set_preset_locked (data_p->projectMConfiguration->handle,
                              false);

  projectm_set_preset_duration (data_p->projectMConfiguration->handle,
                                TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PRESET_DURATION_D);
  projectm_set_soft_cut_duration (data_p->projectMConfiguration->handle,
                                  TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PRESET_TRANSITION_DURATION_D);
  projectm_set_hard_cut_enabled (data_p->projectMConfiguration->handle, 
                                  false);
  projectm_set_hard_cut_duration (data_p->projectMConfiguration->handle,
                                  20.0);
  projectm_set_hard_cut_sensitivity (data_p->projectMConfiguration->handle,
                                     1.0f);
  projectm_set_beat_sensitivity (data_p->projectMConfiguration->handle,
                                 1.0f);

  projectm_set_texel_offset (data_p->projectMConfiguration->handle,
                             TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTEL_OFFSET_X,
                             TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTEL_OFFSET_Y);

  //projectm_set_texture_load_event_callback (data_p->projectMConfiguration->handle,
  //                                          acestream_projectm_texture_load_cb,
  //                                          data_p->projectMConfiguration);

  data_p->projectMConfiguration->playlist =
    projectm_playlist_create (data_p->projectMConfiguration->handle);
  ACE_ASSERT (data_p->projectMConfiguration->playlist);

  //projectm_playlist_set_preset_switched_event_callback (data_p->projectMConfiguration->playlist,
  //                                                      acestream_projectm_preset_switch_cb,
  //                                                      data_p->projectMConfiguration);
  //projectm_playlist_set_preset_switch_failed_event_callback (data_p->projectMConfiguration->playlist,
  //                                                            acestream_projectm_preset_switch_failed_cb,
  //                                                            data_p->projectMConfiguration);

  const char* lib_root_p =
    ACE_OS::getenv (ACE_TEXT_ALWAYS_CHAR (COMMON_ENVIRONMENT_DIRECTORY_ROOT_LIB));
  ACE_ASSERT (lib_root_p);

  std::string textures_path_string = lib_root_p;
  textures_path_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  textures_path_string +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_TEXTURES_DIRECTORY);
  textures_path_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  textures_path_string += ACE_TEXT_ALWAYS_CHAR ("textures");
  const char* texture_paths_a[1] = { textures_path_string.c_str () };
  projectm_set_texture_search_paths (data_p->projectMConfiguration->handle,
                                      texture_paths_a,
                                      1);

  std::string presets_path_string = lib_root_p;
  presets_path_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  presets_path_string +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_PROJECTM_DEFAULT_PRESETS_DIRECTORY);
  projectm_playlist_add_path (data_p->projectMConfiguration->playlist,
                              presets_path_string.c_str (),
                              true,
                              false);

  projectm_playlist_set_shuffle (data_p->projectMConfiguration->playlist,
                                 true);
  // projectm_playlist_play_next (data_p->projectMConfiguration->playlist,
  //                              true);

  //projectm_reset_textures (data_p->projectMConfiguration->handle);
#endif // PROJECTM_SUPPORT

  // initialize perspective
  gtk_widget_get_allocation (widget_in,
                             &allocation);
  glViewport (0, 0,
              static_cast<GLsizei> (allocation.width), static_cast<GLsizei> (allocation.height));

#if defined (PROJECTM_SUPPORT)
  projectm_set_window_size (data_p->projectMConfiguration->handle,
                            allocation.width,
                            allocation.height);
#endif // PROJECTM_SUPPORT

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  gdk_gl_drawable_gl_end (drawable_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
} // glarea_realize_cb

void
glarea_unrealize_cb (GtkWidget* widget_in,
                     gpointer   userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_unrealize_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
#endif // PROJECTM_SUPPORT

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  // sanity check(s)
  ACE_ASSERT (widget_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;

  // sanity check(s)
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea* gl_area_p = GTK_GL_AREA (widget_in);
  ACE_ASSERT (gl_area_p);
  // NOTE*: the OpenGL context has been created at this point
  GdkGLContext* context_p = gtk_gl_area_get_context (gl_area_p);
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    return;
  } // end IF

  // load the texture
  gtk_gl_area_attach_buffers (gl_area_p);
  gdk_gl_context_make_current (context_p);

  // sanity check(s)
  ACE_ASSERT (gtk_gl_area_get_has_depth_buffer (gl_area_p));
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
    return;
#else
  GdkGLContext* context_p = gtk_gl_area_get_context (GTK_GL_AREA (widget_in));
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    return;
  } // end IF

  bool result = gdk_gl_drawable_make_current (drawable_p,
                                              context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  result = gdk_gl_drawable_gl_begin (drawable_p,
                                     context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

  if (dummy_vao_i)
  {
    glDeleteVertexArrays (1, &dummy_vao_i);
    dummy_vao_i = 0;
  } // end IF

#if defined (PROJECTM_SUPPORT)
  if (data_p->projectMConfiguration->playlist)
  {
    projectm_playlist_destroy (data_p->projectMConfiguration->playlist); data_p->projectMConfiguration->playlist = NULL;
  } // end IF
  if (data_p->projectMConfiguration->handle)
  {
    projectm_destroy (data_p->projectMConfiguration->handle); data_p->projectMConfiguration->handle = NULL;
  } // end IF
#endif // PROJECTM_SUPPORT

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  gdk_gl_drawable_gl_end (drawable_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
} // glarea_unrealize_cb

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
GdkGLContext*
glarea_create_context_cb (GtkGLArea* GLArea_in,
                          gpointer userData_in)
{
  // sanity check(s)
  ACE_ASSERT (GLArea_in);
  ACE_ASSERT (userData_in);
  ACE_ASSERT (!gtk_gl_area_get_context (GLArea_in));

  GdkGLContext* result_p = NULL;

  GError* error_p = NULL;
  GdkWindow* window_p = gtk_widget_get_window (GTK_WIDGET (GLArea_in));
  ACE_ASSERT (window_p);
  // *TODO*: this currently fails on Wayland (Gnome 3.22.24)
  // *WORKAROUND*: set GDK_BACKEND=x11 environment to force XWayland
  result_p = gdk_window_create_gl_context (window_p,
                                           &error_p);
  if (!result_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_window_create_gl_context(): \"%s\", aborting\n"),
                ACE_TEXT (error_p->message)));
    gtk_gl_area_set_error (GLArea_in, error_p);
    g_error_free (error_p); error_p = NULL;
    return NULL;
  } // end IF

  gdk_gl_context_set_required_version (result_p,
                                       3, 2);
#if defined (_DEBUG)
  gdk_gl_context_set_debug_enabled (result_p,
                                    TRUE);
#endif // _DEBUG
  gdk_gl_context_set_forward_compatible (result_p,
                                         FALSE);
  gdk_gl_context_set_use_es (result_p,
                             FALSE);

  // if (!gdk_gl_context_realize (result_p,
  //                              &error_p))
  // {
  //   ACE_DEBUG ((LM_ERROR,
  //               ACE_TEXT ("failed to realize OpenGL context: \"%s\", continuing\n"),
  //               ACE_TEXT (error_p->message)));
  //   gtk_gl_area_set_error (GLArea_in, error_p);
  //   g_error_free (error_p); error_p = NULL;
  //   return NULL;
  // } // end IF

  // gdk_gl_context_make_current (result_p);

  // initialize options
  // glClearColor (0.0F, 0.0F, 0.0F, 1.0F);              // Black Background
  //glClearDepth (1.0);                                 // Depth Buffer Setup
  /* speedups */
  //  glDisable (GL_CULL_FACE);
  //  glEnable (GL_DITHER);
  //  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  //  glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
  //glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  //glEnable (GL_COLOR_MATERIAL);
  //glEnable (GL_LIGHTING);
  // glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
  // glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
  // glDepthMask (GL_TRUE);
  // glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
  // // glShadeModel (GL_SMOOTH);                           // Enable Smooth Shading
  // // glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  // glEnable (GL_BLEND);                                // Enable Semi-Transparency
  // glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing

  return result_p;
}

gboolean
glarea_render_cb (GtkGLArea* GLArea_in,
                  GdkGLContext* context_in,
                  gpointer userData_in)
{
  // NETWORK_TRACE (ACE_TEXT ("::glarea_render_cb"));

  // sanity check(s)
  ACE_ASSERT (GLArea_in);
  ACE_UNUSED_ARG (context_in);
  ACE_ASSERT (userData_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
  ACE_ASSERT (data_p->projectMConfiguration->handle);
#endif // PROJECTM_SUPPORT

  // compute fps
  static int last_frame_count_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static std::chrono::steady_clock::time_point last_second = std::chrono::high_resolution_clock::now ();
  std::chrono::steady_clock::time_point current_second = std::chrono::high_resolution_clock::now ();
#elif defined (ACE_LINUX)
  static std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> last_second = std::chrono::high_resolution_clock::now ();
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> current_second = std::chrono::high_resolution_clock::now ();
#else
#error missing implementation, aborting
#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX
  std::chrono::duration<float> elapsed_seconds = current_second - last_second;
  if (elapsed_seconds.count () > 1.0f)
  {
    float fps_f = last_frame_count_i / elapsed_seconds.count ();
    std::string title_string =
      ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_WINDOW_TITLE);
    title_string += ACE_TEXT_ALWAYS_CHAR (" [");
    std::ostringstream converter;
    converter << std::setprecision (2) << std::fixed << fps_f;
    title_string += converter.str ();
    title_string += ACE_TEXT_ALWAYS_CHAR (" fps]");

    title_string += ACE_TEXT_ALWAYS_CHAR (" \"");
    char* preset_name_p =
      projectm_playlist_item (data_p->projectMConfiguration->playlist,
                              projectm_playlist_get_position (data_p->projectMConfiguration->playlist));
    ACE_ASSERT (preset_name_p);
    title_string += Common_File_Tools::basename (preset_name_p, true);
    projectm_playlist_free_string (preset_name_p);
    preset_name_p = NULL;
    title_string += ACE_TEXT_ALWAYS_CHAR ("\"");
    if (data_p->projectMConfiguration->presetIsLocked)
      title_string += ACE_TEXT_ALWAYS_CHAR (" [LOCKED]");

    // sanity check(s)
    Common_UI_GTK_BuildersConstIterator_t iterator =
      data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    ACE_ASSERT (iterator != data_p->UIState->builders.end ());
    GtkDialog* dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
    ACE_ASSERT (dialog_p);
    gtk_window_set_title (GTK_WINDOW (dialog_p),
                          title_string.c_str ());

    last_second = current_second;
    last_frame_count_i = 0;

    projectm_set_fps (data_p->projectMConfiguration->handle,
                      static_cast<int32_t> (std::round (fps_f)));
  } // end IF
  else
    ++last_frame_count_i;

  gtk_gl_area_make_current (GLArea_in);

  GLint gtk_default_fbo = 0;
  glGetIntegerv (GL_FRAMEBUFFER_BINDING, &gtk_default_fbo);

  glBindFramebuffer (GL_FRAMEBUFFER, (GLuint)gtk_default_fbo);
  glBindVertexArray (dummy_vao_i);

  // *IMPORTANT NOTE*: does not render without this; why ?
  glDisable (GL_DEPTH_TEST);                           // Disables Depth Testing

  // glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if defined (PROJECTM_SUPPORT)
  // projectm_opengl_render_frame (data_p->projectMConfiguration->handle);
  projectm_opengl_render_frame_fbo (data_p->projectMConfiguration->handle,
                                    (GLuint)gtk_default_fbo);
#endif // PROJECTM_SUPPORT

  gtk_gl_area_queue_render (GLArea_in);

  return FALSE;
}

void
glarea_resize_cb (GtkGLArea* GLArea_in,
                  gint width_in,
                  gint height_in,
                  gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_resize_cb"));

  // sanity check(s)
  ACE_ASSERT (GLArea_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
  ACE_ASSERT (data_p->projectMConfiguration->handle);
#endif // PROJECTM_SUPPORT

  gtk_gl_area_make_current (GLArea_in);

  glViewport (0, 0,
              static_cast<GLsizei> (width_in), static_cast<GLsizei> (height_in));

#if defined (PROJECTM_SUPPORT)
  projectm_set_window_size (data_p->projectMConfiguration->handle,
                            width_in,
                            height_in);
#endif // PROJECTM_SUPPORT

  //glMatrixMode (GL_PROJECTION);

  //glLoadIdentity ();

  //ACE_ASSERT (height_in);
  //gluPerspective (45.0,
  //                static_cast<GLdouble> (width_in) / static_cast<GLdouble> (height_in),
  //                0.1, 100.0);

  //glMatrixMode (GL_MODELVIEW);
}
#else
#if defined (GTKGLAREA_SUPPORT)
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (event_in && event_in->type == GDK_CONFIGURE);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
  if (!data_p->projectMConfiguration->handle)
    return;
#endif // PROJECTM_SUPPORT
  // sanity check(s)
  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return;

  glViewport (0, 0,
              event_in->configure.width, event_in->configure.height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

#if defined (PROJECTM_SUPPORT)
  projectm_set_window_size (data_p->projectMConfiguration->handle,
                            event_in->configure.width,
                            event_in->configure.height);
#endif // PROJECTM_SUPPORT
}

gboolean
glarea_expose_event_cb (GtkWidget* widget_in,
                        GdkEvent* event_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_expose_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
#if defined (PROJECTM_SUPPORT)
  ACE_ASSERT (data_p->projectMConfiguration);
  ACE_ASSERT (data_p->projectMConfiguration->handle);
  ACE_ASSERT (data_p->projectMConfiguration->playlist);
#endif // PROJECTM_SUPPORT

  // compute fps
  static int last_frame_count_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static std::chrono::steady_clock::time_point last_second = std::chrono::high_resolution_clock::now ();
  std::chrono::steady_clock::time_point current_second = std::chrono::high_resolution_clock::now ();
#elif defined (ACE_LINUX)
  static std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> last_second = std::chrono::high_resolution_clock::now ();
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> current_second = std::chrono::high_resolution_clock::now ();
#else
#error missing implementation, aborting
#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX
  std::chrono::duration<float> elapsed_seconds = current_second - last_second;
  if (elapsed_seconds.count () > 1.0f)
  {
    float fps_f = last_frame_count_i / elapsed_seconds.count ();
    std::string title_string =
      ACE_TEXT_ALWAYS_CHAR (TEST_I_ICECAST_CLIENT_DEFAULT_WINDOW_TITLE);
    title_string += ACE_TEXT_ALWAYS_CHAR (" [");
    std::ostringstream converter;
    converter << std::setprecision (2) << std::fixed << fps_f;
    title_string += converter.str ();
    title_string += ACE_TEXT_ALWAYS_CHAR (" fps]");

    title_string += ACE_TEXT_ALWAYS_CHAR (" \"");
    char* preset_name_p =
      projectm_playlist_item (data_p->projectMConfiguration->playlist,
                              projectm_playlist_get_position (data_p->projectMConfiguration->playlist));
    ACE_ASSERT (preset_name_p);
    title_string += Common_File_Tools::basename (preset_name_p, true);
    projectm_playlist_free_string (preset_name_p);
    preset_name_p = NULL;
    title_string += ACE_TEXT_ALWAYS_CHAR ("\"");
    if (data_p->projectMConfiguration->presetIsLocked)
      title_string += ACE_TEXT_ALWAYS_CHAR (" [LOCKED]");

    // sanity check(s)
    Common_UI_GTK_BuildersConstIterator_t iterator =
      data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    ACE_ASSERT (iterator != data_p->UIState->builders.end ());
    GtkDialog* dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
    ACE_ASSERT (dialog_p);
    gtk_window_set_title (GTK_WINDOW (dialog_p),
                          title_string.c_str ());

    last_second = current_second;
    last_frame_count_i = 0;

    projectm_set_fps (data_p->projectMConfiguration->handle,
                      static_cast<int32_t> (std::round (fps_f)));
  } // end IF
  else
    ++last_frame_count_i;

  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if defined (PROJECTM_SUPPORT)
  projectm_opengl_render_frame (data_p->projectMConfiguration->handle);
#endif // PROJECTM_SUPPORT

  ggla_area_swap_buffers (GGLA_AREA (widget_in));

  return TRUE;
}
#else // !GTKGLAREA_SUPPORT
void
glarea_size_allocate_event_cb (GtkWidget* widget_in,
                               GdkRectangle* allocation_in,
                               gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_size_allocate_event_cb"));

  // sanity check(s)
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_I_IceCastClient_StreamConfiguration_2_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration_2.end ());
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);

  if (!gdk_gl_drawable_make_current (drawable_p,
                                     context_p))
    return;

  glViewport (0, 0,
              allocation_in->width, allocation_in->height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  //glMatrixMode (GL_PROJECTION);
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);
  //glLoadIdentity (); // Reset The Projection Matrix
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);

  //gluPerspective (45.0,
  //                allocation_in->width / (GLdouble)allocation_in->height,
  //                0.1,
  //                100.0); // Calculate The Aspect Ratio Of The Window
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);

  //glMatrixMode (GL_MODELVIEW);
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);
}

gboolean
glarea_draw_cb (GtkWidget* widget_in,
                cairo_t* context_in,
                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_draw_cb"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Test_I_IceCastClient_StreamConfiguration_2_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration_2.end ());
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);

  bool result = gdk_gl_drawable_make_current (drawable_p,
                                              context_p);
  if (!result)
    return FALSE;
  result = gdk_gl_drawable_gl_begin (drawable_p,
                                     context_p);
  if (!result)
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  gdk_gl_drawable_gl_end (drawable_p);

  gdk_gl_drawable_swap_buffers (drawable_p);

  return TRUE;
}
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (event_in && event_in->type == GDK_CONFIGURE);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
    return;

  glViewport (0, 0,
              event_in->configure.width, event_in->configure.height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

//  glMatrixMode (GL_PROJECTION);
//  ACE_ASSERT (glGetError () == GL_NO_ERROR);
//  glLoadIdentity (); // Reset The Projection Matrix
//  ACE_ASSERT (glGetError () == GL_NO_ERROR);
//
//#if defined (GLU_SUPPORT)
//  gluPerspective (45.0,
//                  event_in->configure.width / (GLdouble)event_in->configure.height,
//                  0.1,
//                  100.0); // Calculate The Aspect Ratio Of The Window
//#else
//  GLdouble fW, fH;
//
//  //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
//  fH = tan (45.0 / 360 * M_PI) * 0.1;
//  fW = fH * (event_in->configure.width / (GLdouble)event_in->configure.height);
//
//  glFrustum (-fW, fW, -fH, fH, 0.1, 100.0);
//#endif // GLU_SUPPORT
//  COMMON_GL_ASSERT;
//
//  glMatrixMode (GL_MODELVIEW);
//  COMMON_GL_ASSERT;
} // glarea_configure_event_cb

gboolean
glarea_expose_event_cb (GtkWidget* widget_in,
                        GdkEvent* event_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_expose_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  if (!gtk_gl_area_begingl (GTK_GL_AREA (widget_in)))
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  COMMON_GL_ASSERT;

  gtk_gl_area_endgl (GTK_GL_AREA (widget_in));

  gtk_gl_area_swap_buffers (GTK_GL_AREA (widget_in));

  return TRUE;
} // glarea_expose_event_cb
#else
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (event_in && event_in->type == GDK_CONFIGURE);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  //GdkGLDrawable* gl_drawable_p = gtk_widget_get_gl_drawable (widget_in);
  //ACE_ASSERT (gl_drawable_p);
  //GdkGLContext* gl_context_p = gtk_widget_get_gl_context (widget_in);
  //ACE_ASSERT (gl_context_p);

  glViewport (0, 0,
              event_in->configure.width, event_in->configure.height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  //glMatrixMode (GL_PROJECTION);
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);
  //glLoadIdentity (); // Reset The Projection Matrix
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);

  //gluPerspective (45.0,
  //                event_in->configure.width / (GLdouble)event_in->configure.height,
  //                0.1,
  //                100.0); // Calculate The Aspect Ratio Of The Window
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);

  //glMatrixMode (GL_MODELVIEW);
  //ACE_ASSERT (glGetError () == GL_NO_ERROR);
} // glarea_configure_event_cb

gboolean
glarea_expose_event_cb (GtkWidget* widget_in,
                        GdkEvent* event_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::glarea_expose_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_UNUSED_ARG (event_in);
  struct Test_I_IceCastClient_UI_CBData* data_p =
    static_cast<struct Test_I_IceCastClient_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  ACE_ASSERT (window_p);

  GdkGLDrawable* gl_drawable_p = gtk_widget_get_gl_drawable (widget_in);
  ACE_ASSERT (gl_drawable_p);
  GdkGLContext* gl_context_p = gtk_widget_get_gl_context (widget_in);
  ACE_ASSERT (gl_context_p);

  gdk_gl_drawable_gl_begin (gl_drawable_p,
                            gl_context_p);

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  COMMON_GL_ASSERT;

  gdk_gl_drawable_gl_end (gl_drawable_p);

  gdk_gl_drawable_swap_buffers (gl_drawable_p);

  return TRUE;
} // glarea_expose_event_cb
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,0,0) */
#ifdef __cplusplus
}
#endif /* __cplusplus */
