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

#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H

#include "common_inotify.h"

#include "stream_common.h"

#include "net_common.h"

// forward declarations
struct HTTP_Configuration;
class HTTP_Record;
class HTTP_SessionMessage;
struct HTTP_Stream_SessionData;
struct HTTP_Stream_UserData;

typedef Common_INotify_T<HTTP_Stream_SessionData,
                         HTTP_Record,
                         HTTP_SessionMessage> HTTP_IStreamNotify_t;


typedef Stream_Statistic HTTP_RuntimeStatistic_t;

struct HTTP_ConnectionState
 : Net_ConnectionState
{
  inline HTTP_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  HTTP_Configuration*   configuration;

  HTTP_Stream_UserData* userData;
};

#endif
