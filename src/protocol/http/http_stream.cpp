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

//#include "ace/Synch.h"
//#include "http_scanner.h"
#include "http_stream.h"

#include "http_defines.h"
//#include "http_exports.h"

//HTTP_Export const char libacenetwork_default_http_marshal_module_name_string[] =
const char libacenetwork_default_http_marshal_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (HTTP_DEFAULT_MODULE_MARSHAL_NAME_STRING);
//HTTP_Export const char libacenetwork_default_http_stream_name_string[] =
const char libacenetwork_default_http_stream_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (HTTP_DEFAULT_STREAM_NAME_STRING);
