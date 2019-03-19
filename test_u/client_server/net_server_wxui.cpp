#include "net_server_wxui.h"

#include <wx/xrc/xmlres.h>

#include "ace/config-lite.h"
#include "ace/Log_Msg.h"

extern const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("NetServerStream");

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
  EVT_TOGGLEBUTTON(XRCID("togglebutton_listen"), dialog_main::togglebutton_listen_cb)
  EVT_BUTTON(wxID_CLOSE, dialog_main::button_close_cb)
  EVT_BUTTON(XRCID("button_report"), dialog_main::button_report_cb)
  EVT_BUTTON(wxID_HELP, dialog_main::button_about_cb)
  EVT_BUTTON(wxID_EXIT, dialog_main::button_quit_cb)
END_EVENT_TABLE();

//////////////////////////////////////////

void
dialog_main::togglebutton_listen_cb(wxCommandEvent &event)  // wxGlade: dialog_main.<event_handler>
{
  event.Skip();
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

//////////////////////////////////////////

class server
 : public wxApp
{
 public:
  bool OnInit ();
};

IMPLEMENT_APP(server)

bool
server::OnInit ()
{
  wxInitAllImageHandlers ();
  dialog_main* dialog_main_p = new dialog_main (NULL, wxID_ANY, wxEmptyString);
  SetTopWindow (dialog_main_p);
  dialog_main_p->Show ();
  return true;
}
