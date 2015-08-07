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

#include "net_module_eventhandler.h"

#include "net_macros.h"

Net_Module_EventHandler::Net_Module_EventHandler ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_EventHandler::Net_Module_EventHandler"));

}

Net_Module_EventHandler::~Net_Module_EventHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_EventHandler::~Net_Module_EventHandler"));

}

Stream_Module_t*
Net_Module_EventHandler::clone ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Module_EventHandler::clone"));

  // initialize return value(s)
  Stream_Module_t* module_p = NULL;

  ACE_NEW_NORETURN (module_p,
                    Net_Module_EventHandler_Module (ACE_TEXT_ALWAYS_CHAR (inherited::name ()),
                                                    NULL,
                                                    true));
  if (!module_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  else
  {
    Net_Module_EventHandler* eventHandler_impl = NULL;
    eventHandler_impl =
      dynamic_cast<Net_Module_EventHandler*> (module_p->writer ());
    if (!eventHandler_impl)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("dynamic_cast<Net_Module_EventHandler> failed, aborting\n")));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
    eventHandler_impl->initialize (inherited::subscribers_, inherited::lock_);
  } // end ELSE

  return module_p;
}
