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

#include "IRC_client_inputhandler.h"

#include "ace/Log_Msg.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"

#include "net_macros.h"

IRC_Client_InputHandler::IRC_Client_InputHandler (bool useReactor_in)
 : inherited (NULL,                           // reactor
              ACE_Event_Handler::LO_PRIORITY) // priority
 , configuration_ (NULL)
 , registered_ (false)
 , useReactor_ (useReactor_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_InputHandler::IRC_Client_InputHandler"));

  int result = -1;

  if (useReactor_)
  {
    ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
    ACE_ASSERT (thread_manager_p);
    ACE_Reactor* reactor_p = ACE_Reactor::instance ();
    ACE_ASSERT (reactor_p);
    result = ACE_Event_Handler::register_stdin_handler (this,
                                                        reactor_p,
                                                        thread_manager_p,
                                                        THR_DETACHED);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::register_stdin_handler(): \"%m\", continuing\n")));
    else
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("started input handler...\n")));

      registered_ = true;
    } // end ELSE
  } // end IF
}

IRC_Client_InputHandler::~IRC_Client_InputHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_InputHandler::~IRC_Client_InputHandler"));

  int result = -1;

  if (registered_)
  {
    ACE_HANDLE handle = inherited::get_handle ();
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::handle_close(%u): \"%m\", continuing\n"),
                  handle));

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("stopped input handler...\n")));
  } // end IF
}

bool
IRC_Client_InputHandler::initialize (const IRC_Client_InputHandlerConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_InputHandler::initialize"));

  configuration_ =
    &const_cast<IRC_Client_InputHandlerConfiguration&> (configuration_in);

  return true;
}

int
IRC_Client_InputHandler::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_InputHandler::handle_input"));

  return 0;
}

int
IRC_Client_InputHandler::handle_close (ACE_HANDLE handle_in,
                                       ACE_Reactor_Mask closeMask_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_InputHandler::handle_close"));

  ACE_UNUSED_ARG (closeMask_in);

  int result = 0;
#if defined (ACE_WIN32)
  // *NOTE*: called from dedicated thread context, see ACE_Event_Handler.cpp
  delete this;
#else
  if (useReactor_)
  {
    ACE_Reactor* reactor_p = ACE_Reactor::instance ();
    ACE_ASSERT (reactor_p);
    result = ACE_Event_Handler::remove_stdin_handler (reactor_p,
                                                      NULL);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::remove_stdin_handler(): \"%m\", aborting\n")));
  } // end IF
#endif

  return result;
}
