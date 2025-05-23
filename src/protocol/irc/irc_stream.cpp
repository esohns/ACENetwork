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

#include "irc_stream.h"

#include "irc_defines.h"
//#include "irc_exports.h"

//IRC_Export const char libacenetwork_default_irc_marshal_module_name_string[] =
const char libacenetwork_default_irc_marshal_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_MODULE_MARSHAL_NAME_STRING);
//IRC_Export const char libacenetwork_default_irc_parser_module_name_string[] =
const char libacenetwork_default_irc_parser_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_MODULE_PARSER_NAME_STRING);
//IRC_Export const char libacenetwork_default_irc_stream_name_string[] =
const char libacenetwork_default_irc_stream_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_STREAM_NAME_STRING);
