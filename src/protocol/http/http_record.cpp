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

#include "http_record.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "http_tools.h"

HTTP_Record::HTTP_Record ()
 : inherited (1,
              true)
 , method_ (HTTP_Codes::HTTP_METHOD_INVALID)
 , URI_ ()
 , version_ (HTTP_Codes::HTTP_VERSION_INVALID)
 , status_ (HTTP_Codes::HTTP_STATUS_INVALID)
 , headers_ ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Record::HTTP_Record"));

}

HTTP_Record::~HTTP_Record ()
{

}

void
HTTP_Record::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Record::dump_state"));

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s"),
              ACE_TEXT (HTTP_Tools::dump (*this).c_str ())));
}
