#include "net_server_wxui.h"

#include "wx/xrc/xmlres.h"

#include "ace/config-lite.h"
#include "ace/Log_Msg.h"

#include "common_event_tools.h"

#include "common_log_tools.h"

#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"

#include "net_client_defines.h"

#include "net_server_common_tools.h"
#include "net_server_listener_common.h"

#include "test_u_connection_manager_common.h"
#include "test_u_eventhandler.h"
#include "test_u_module_eventhandler.h"

//extern const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("NetServerStream");

dialog_main::dialog_main (wxWindow* parent,
                          wxWindowID id,
                          const wxString& title,
                          const wxPoint& pos,
                          const wxSize& size,
                          long style)
 : dialog_main_base (parent,
                     id,
                     title,
                     pos,
                     size,
                     wxDEFAULT_DIALOG_STYLE)
{

}

//////////////////////////////////////////

BEGIN_EVENT_TABLE(dialog_main, dialog_main_base)
  EVT_TOGGLEBUTTON(wxID_OPEN, dialog_main::togglebutton_listen_cb)
  EVT_BUTTON(wxID_CLOSE, dialog_main::button_close_cb)
  EVT_BUTTON(wxID_NEW, dialog_main::button_report_cb)
  EVT_BUTTON(wxID_HELP, dialog_main::button_about_cb)
  EVT_BUTTON(wxID_EXIT, dialog_main::button_quit_cb)
END_EVENT_TABLE();

//////////////////////////////////////////

IMPLEMENT_APP(server)

bool
server::OnInit ()
{
  if (!OnInit_2 ())
    return false;

  wxInitAllImageHandlers ();
  dialog_main* dialog_main_p = new dialog_main (NULL, wxID_ANY, wxEmptyString);
  SetTopWindow (dialog_main_p);
  dialog_main_p->Show ();

  return true;
}

bool
server::OnInit_2 ()
{
  // initialize randomness
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false, true);
#else
  Common_Tools::initialize (true);
#endif // ACE_WIN32 || ACE_WIN64

  std::string log_file_name =
      Net_Server_Common_Tools::getNextLogFileName (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                   ACE_TEXT_ALWAYS_CHAR (NET_SERVER_LOG_FILENAME_PREFIX));
  if (!Common_Log_Tools::initializeLogging (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME), // program name
                                            log_file_name,                 // log file name
                                            true,                          // log to syslog ?
                                            false,                         // trace messages ?
#if defined (_DEBUG)
                                            true,                          // debug messages ?
#else
                                            false,                         // debug messages ?
#endif // _DEBUG
                                            NULL))                         // (ui) logger ?
    return false;

  if (!heap_allocator_.initialize (this->configuration_.allocatorConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize allocator, returning\n")));
    return false;
  } // end IF
  Test_U_MessageAllocator_t* message_allocator_p = NULL;
  ACE_NEW_NORETURN (message_allocator_p,
                    Test_U_MessageAllocator_t (NET_STREAM_MAX_MESSAGES,  // maximum #buffers
                                               &(this->heap_allocator_), // heap allocator handle
                                               true));                   // block ?

  Test_U_EventHandler_t* ui_event_handler_p = NULL;
  ACE_NEW_NORETURN (ui_event_handler_p,
                    Test_U_EventHandler_t (&CBData_));
  Test_U_Module_EventHandler_Module* event_handler_module_p = NULL;
  ACE_NEW_NORETURN (event_handler_module_p,
                    Test_U_Module_EventHandler_Module (NULL,
                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING)));

  // ******************** protocol configuration data **************************
//  configuration_.protocolConfiguration.pingInterval = pingInterval_in;
  configuration_.protocolConfiguration.printPongMessages = false;
  // ********************** stream configuration data **************************
  configuration_.streamConfiguration.configuration_->cloneModule = true;
  configuration_.streamConfiguration.configuration_->messageAllocator =
    message_allocator_p;
  configuration_.streamConfiguration.configuration_->module =
      event_handler_module_p;

  struct Stream_ModuleConfiguration module_configuration;
  struct ClientServer_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_StreamConfiguration stream_configuration;
  modulehandler_configuration.printFinalReport = true;
  modulehandler_configuration.protocolConfiguration =
    &configuration_.protocolConfiguration;
//  modulehandler_configuration.statisticReportingInterval =
//    statisticReportingInterval_in;
  modulehandler_configuration.streamConfiguration =
    &configuration_.streamConfiguration;
  modulehandler_configuration.subscriber = ui_event_handler_p;
  modulehandler_configuration.subscribers = &CBData_.subscribers;
  modulehandler_configuration.lock = &CBData_.UIState->subscribersLock;
  configuration_.streamConfiguration.initialize (module_configuration,
                                                 modulehandler_configuration,
                                                 stream_configuration);

  // *TODO*: is this correct ?
  configuration_.streamConfiguration.configuration_->serializeOutput =
    false;

  // ********************** connection configuration data **********************
  Test_U_TCPConnectionConfiguration* connection_configuration_p = NULL;
  Test_U_UDPConnectionConfiguration* connection_configuration_2 = NULL;
  if (false) //(useUDP_in) *TODO*
  {
    ACE_NEW_NORETURN (connection_configuration_2,
                      Test_U_UDPConnectionConfiguration ());
    connection_configuration_2->allocatorConfiguration =
      &configuration_.allocatorConfiguration,
    connection_configuration_2->messageAllocator = message_allocator_p;
    connection_configuration_2->streamConfiguration =
      &configuration_.streamConfiguration;
    configuration_.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                    connection_configuration_2));
  } // end IF
  else
  {
    ACE_NEW_NORETURN (connection_configuration_p,
                      Test_U_TCPConnectionConfiguration ());
    connection_configuration_p->allocatorConfiguration =
      &configuration_.allocatorConfiguration,
    connection_configuration_p->messageAllocator = message_allocator_p;
    connection_configuration_p->socketConfiguration.address.set_port_number (NET_SERVER_DEFAULT_LISTENING_PORT,
                                                                             1);
    connection_configuration_p->streamConfiguration =
      &configuration_.streamConfiguration;
    configuration_.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                    connection_configuration_p));
  } // end ELSE

  // step2: initialize event dispatch
  event_dispatch_state_.configuration = &configuration_.dispatchConfiguration;
  if (false) //(useReactor_in) *TODO*
    configuration_.dispatchConfiguration.numberOfReactorThreads =
      NET_SERVER_DEFAULT_NUMBER_OF_REACTOR_DISPATCH_THREADS;
  else
    configuration_.dispatchConfiguration.numberOfProactorThreads =
      NET_SERVER_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS;
  if (!Common_Event_Tools::initializeEventDispatch (configuration_.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), aborting\n")));
    return false;
  } // end IF

  // step3: initialize connection manager
  Test_U_TCPConnectionManager_t* connection_manager_p =
    TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
  Test_U_UDPConnectionManager_t* connection_manager_2 =
    TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  ACE_ASSERT (connection_manager_2);
  connection_manager_p->initialize (0,
                                    ACE_Time_Value::zero);
  connection_manager_p->set (*connection_configuration_p,
                             NULL);
  connection_manager_2->initialize (0,
                                    ACE_Time_Value::zero);
  connection_manager_2->set (*connection_configuration_2,
                             NULL);

  // step4b: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, aborting\n")));
    return false;
  } // end IF


  if (true) //(!useUDP_in) *TODO*
  {
//    if (useReactor_in)
//      configuration_in.listener = SERVER_LISTENER_SINGLETON::instance ();
//    else
    configuration_.TCPListener =
      SERVER_ASYNCH_TCP_LISTENER_SINGLETON::instance ();

    ACE_ASSERT (connection_configuration_p);
    if (!configuration_.TCPListener->initialize (*connection_configuration_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize listener, aborting\n")));
      return false;
    } // end IF
//    ACE_thread_t thread_id = 0;
//    configuration_.listener->start (thread_id);
//    ACE_UNUSED_ARG (thread_id);
//    if (!configuration_.listener->isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start listener (port: %u), aborting\n"),
//                  NET_SERVER_DEFAULT_LISTENING_PORT));
//      return false;
//    } // end IF
  } // end IF
  else
  {
    Server_UDP_AsynchConnector_t udp_asynch_connector (true);
    Server_UDP_Connector_t udp_connector (true);

//    if (useReactor_in)
//      configuration_in.connector = &udp_connector;
//    else
      configuration_.UDPConnector = &udp_asynch_connector;
      ACE_ASSERT (connection_configuration_2);
    if (!configuration_.UDPConnector->initialize (*connection_configuration_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize connector, aborting\n")));
      return false;
    } // end IF

    ACE_HANDLE handle_h =
      configuration_.UDPConnector->connect (connection_configuration_2->socketConfiguration.listenAddress);
    Test_U_UDPConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
    if (configuration_.UDPConnector->useReactor ())
    {
      if (handle_h != ACE_INVALID_HANDLE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        iconnection_p =
          connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle_h));
#else
        iconnection_p =
          connection_manager_2->get (static_cast<Net_ConnectionId_t> (handle_h));
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
    else
    {
      // step1: wait for the connection to register with the manager
      // *TODO*: avoid these tight loops
      ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S, 0);
      ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
      // *TODO*: this may not be accurate/applicable for/to all protocols
      do
      {
        // *TODO*: avoid these tight loops
        iconnection_p =
          connection_manager_2->get (connection_configuration_2->socketConfiguration.listenAddress,
                                     false);
        if (iconnection_p)
          break;
      } while (COMMON_TIME_NOW < deadline);
    } // end ELSE
    if (!iconnection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (connection_configuration_2->socketConfiguration.listenAddress).c_str ())));
      return false;
    } // end IF
    iconnection_p->decrease ();
  } // end ELSE

  // *NOTE*: from this point on, clean up any remote connections !

  return true;
}
