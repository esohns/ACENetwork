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

#include "common_file_tools.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"
#endif // GTK_SUPPORT

#include "net_macros.h"

#include "irc_tools.h"

#include "IRC_client_network.h"
#include "IRC_client_stream_common.h"

#if defined (GTK_SUPPORT)
#include "IRC_client_gui_callbacks.h"
#endif // GTK_SUPPORT
#include "IRC_client_gui_connection.h"
#include "IRC_client_gui_defines.h"

template <enum Common_UI_FrameworkType GUIType>
IRC_Client_GUI_MessageHandler_T<GUIType>::IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection* connection_in,
                                                                           const std::string& timeStamp_in,
                                                                           struct IRC_Client_UI_HandlerCBData* CBData_in)
 : CBData_ (CBData_in)
 , isFirstMemberListMsg_ (true)
 , isPrivateDialog_ (false)
 , messageQueue_ ()
 , messageQueueLock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::IRC_Client_GUI_MessageHandler_T"));

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (CBData_in);

  // initialize cb data
  //CBData_->acknowledgements = 0;
  //CBData_->builderLabel ();
  //CBData_->channelModes = 0;
  CBData_->connection = connection_in;
  CBData_->controller = NULL;
  //CBData_->eventSourceId = 0;
  CBData_->handler = this;
  CBData_->id.clear ();
  //  CBData_->parameters ();
  CBData_->timeStamp = timeStamp_in;
}

template <enum Common_UI_FrameworkType GUIType>
IRC_Client_GUI_MessageHandler_T<GUIType>::IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection* connection_in,
                                                                           IRC_IControl* controller_in,
                                                                           const std::string& id_in,
                                                                           const std::string& UIFileDirectory_in,
                                                                           const std::string& timestamp_in,
                                                                           struct IRC_Client_UI_HandlerCBData* CBData_in)
 : CBData_ (CBData_in)
 , isFirstMemberListMsg_ (true)
 , isPrivateDialog_ (false)
 , messageQueue_ ()
 , messageQueueLock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::IRC_Client_GUI_MessageHandler_T"));

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (controller_in);
  ACE_ASSERT (!id_in.empty ());
  if (!Common_File_Tools::isDirectory (UIFileDirectory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: \"%s\"): not a directory, returning\n"),
                ACE_TEXT (UIFileDirectory_in.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (CBData_);

  // initialize cb data
  //CBData_->acknowledgements = 0;
  //CBData_->builderLabel ();
  //CBData_->channelModes = 0;
  CBData_->connection = connection_in;
  CBData_->controller = controller_in;
  //CBData_->eventSourceId = 0;
  CBData_->handler = this;
  CBData_->id = id_in;
//  CBData_->parameters ();
  CBData_->timeStamp = timestamp_in;
}

template <enum Common_UI_FrameworkType GUIType>
IRC_Client_GUI_MessageHandler_T<GUIType>::~IRC_Client_GUI_MessageHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::~IRC_Client_GUI_MessageHandler_T"));

  // remove builder ?
  if (!isServerLog ())
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::queueForDisplay (const std::string& text_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::queueForDisplay"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, messageQueueLock_);
    messageQueue_.push_front (text_in);
  } // end lock scope
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::update ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::update"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
}

// const std::string
// IRC_Client_GUI_MessageHandler_T<GUIType>::getChannel() const
// {
//   NETWORK_TRACE(ACE_TEXT("IRC_Client_GUI_MessageHandler_T<GUIType>::getChannel"));
//
//   // sanity check: 'this' might be a private message handler !...
//   ACE_ASSERT(RPG_Net_Protocol_Tools::isValidIRCChannelName(CBData_->id));
//
//   return CBData_->id;
// }

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::setTopic (const std::string& topic_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::setTopic"));

  ACE_UNUSED_ARG (topic_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::setModes (const std::string& modes_in,
                                                    const std::string& parameter_in,
                                                    bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::setModes"));

  ACE_UNUSED_ARG (parameter_in);

  if (lockedAccess_in)
  {
  } // end IF

  CBData_->acknowledgements += IRC_Tools::merge (modes_in,
                                                 CBData_->channelModes);

  if (lockedAccess_in)
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::clearMembers (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::clearMembers"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  if (lockedAccess_in)
  {
  } // end IF

  if (lockedAccess_in)
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::update (const std::string& currentNickName_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::update"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->connection);

  std::string new_nick_name;//, channel;
  //CBData_->connection->current (new_nick_name,
  //                             channel);
  const struct IRC_SessionState& session_state_r =
    CBData_->connection->state ();
  new_nick_name = session_state_r.nickName;
  if (CBData_->channelModes.test (CHANNELMODE_OPERATOR))
    new_nick_name.insert (new_nick_name.begin (), '@');

  remove (currentNickName_in, true);
  add (new_nick_name, true);
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::add (const std::string& nickname_in,
                                               bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::add"));

  ACE_UNUSED_ARG (nickname_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  if (lockedAccess_in)
  {
  } // end IF

  if (lockedAccess_in)
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::remove (const std::string& nick_in,
                                                  bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::remove"));

  ACE_UNUSED_ARG (nick_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  if (lockedAccess_in)
  {
  } // end IF

  if (lockedAccess_in)
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::members (const string_list_t& list_in,
                                                   bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::members"));

  ACE_UNUSED_ARG (list_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  if (lockedAccess_in)
  {
  } // end IF

  if (lockedAccess_in)
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_MessageHandler_T<GUIType>::endMembers (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T<GUIType>::endMembers"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  if (lockedAccess_in)
  {
  } // end IF

  if (lockedAccess_in)
  {
  } // end IF
}
