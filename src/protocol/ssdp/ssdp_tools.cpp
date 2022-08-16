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

#include "ssdp_tools.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

std::string
SSDP_Tools::MethodToString (const SSDP_Method_t& method_in)
{
  NETWORK_TRACE (ACE_TEXT ("SSDP_Tools::MethodToString"));

  std::string result;

  switch (method_in)
  {
    case SSDP_Codes::SSDP_METHOD_M_SEARCH:
      result = ACE_TEXT_ALWAYS_CHAR ("M-SEARCH"); break;
    case SSDP_Codes::SSDP_METHOD_NOTIFY:
      result = ACE_TEXT_ALWAYS_CHAR ("NOTIFY"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown method (was: %d), aborting\n"),
                  method_in));
      break;
    }
  } // end SWITCH

  return result;
}

//SSDP_Method_t
//SSDP_Tools::MethodToType (const std::string& method_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("SSDP_Tools::MethodToType"));
//
//  if (method_in == ACE_TEXT_ALWAYS_CHAR ("M-SEARCH"))
//    return SSDP_Codes::HTTP_METHOD_M_SEARCH;
//  else
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid/unknown method (was: \"%s\"), aborting\n"),
//                ACE_TEXT (method_in.c_str ())));
//  } // end IF
//
//  return SSDP_Codes::HTTP_METHOD_INVALID;
//}
