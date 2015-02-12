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

#include "net_controller_base.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

Net_Controller_Base::Net_Controller_Base ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Controller_Base::Net_Controller_Base"));

}

Net_Controller_Base::~Net_Controller_Base ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Controller_Base::~Net_Controller_Base"));

}

void
Net_Controller_Base::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Controller_Base::start"));

  ACE_DEBUG ((LM_NOTICE,
              ACE_TEXT ("%D: starting...\n")));
}

void
Net_Controller_Base::stop ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Controller_Base::stop"));

  ACE_DEBUG ((LM_NOTICE,
              ACE_TEXT ("%D: stopping...\n")));
}

bool
Net_Controller_Base::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Controller_Base::isRunning"));

  ACE_DEBUG ((LM_NOTICE,
              ACE_TEXT ("%D: isRunning ?...\n")));

  // *TODO*
  ACE_ASSERT (false);

  return true;
}
