#include "ace/Synch.h"
#include "net_server_wxui.h"

#include <wx/xrc/xmlres.h>

#include "ace/config-lite.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "common_tools.h"

#include "common_log_tools.h"

#include "stream_allocatorheap.h"

#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_configuration.h"

#include "net_client_defines.h"

#include "net_server_common.h"
#include "net_server_common_tools.h"
#include "net_server_listener_common.h"

#include "test_u_connection_manager_common.h"
#include "test_u_eventhandler.h"
#include "test_u_module_eventhandler.h"

void
dialog_main::togglebutton_listen_cb(wxCommandEvent &event)  // wxGlade: dialog_main.<event_handler>
{
  event.Skip();

  ACE_ASSERT (this->configuration_.listener);
}

void
dialog_main::button_close_cb(wxCommandEvent &event)  // wxGlade: dialog_main.<event_handler>
{
  event.Skip();
}

void
dialog_main::button_report_cb(wxCommandEvent &event)  // wxGlade: dialog_main.<event_handler>
{
  event.Skip();
}

void
dialog_main::button_about_cb(wxCommandEvent &event)  // wxGlade: dialog_main.<event_handler>
{
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("button about, clicked\n")));

  event.Skip();
}

void
dialog_main::button_quit_cb(wxCommandEvent &event)  // wxGlade: dialog_main.<event_handler>
{
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("button quit, clicked\n")));

  event.Skip();

  this->Close (false);
}
