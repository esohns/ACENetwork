#ifndef NET_SERVER_WXUI_H
#define NET_SERVER_WXUI_H

#include <wx/wx.h>
#include <wx/image.h>

#include "ace/Synch_Traits.h"

#include "stream_allocatorheap.h"

#include "net_configuration.h"

#include "net_server_common.h"
#include "net_server_wxui_base.h"

class dialog_main
 : public dialog_main_base
{
 public:
  dialog_main (wxWindow*,
               wxWindowID,
               const wxString&,
               const wxPoint& = wxDefaultPosition,
               const wxSize& = wxDefaultSize,
               long = wxDEFAULT_DIALOG_STYLE);

  void togglebutton_listen_cb(wxCommandEvent &event); // wxGlade: <event_handler>
  void button_close_cb(wxCommandEvent &event); // wxGlade: <event_handler>
  void button_report_cb(wxCommandEvent &event); // wxGlade: <event_handler>
  void button_about_cb(wxCommandEvent &event); // wxGlade: <event_handler>
  void button_quit_cb(wxCommandEvent &event); // wxGlade: <event_handler>

  DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////

class server
 : public wxApp
{
 public:
  bool OnInit ();
  bool OnInit_2 ();

  struct Server_Configuration configuration_;
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Net_AllocatorConfiguration> heap_allocator_;
  struct Server_UI_CBData CBData_;
  struct Common_EventDispatchState event_dispatch_state_;
};

#endif // NET_SERVER_WXUI_H
