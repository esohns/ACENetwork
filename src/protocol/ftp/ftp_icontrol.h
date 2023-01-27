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

#ifndef FTP_ICONTROL_H
#define FTP_ICONTROL_H

#include <string>

#include "ace/config-macros.h"

#include "ftp_common.h"

class FTP_IControl
{
 public:
  inline virtual ~FTP_IControl () {}

  virtual ACE_HANDLE connectControl () = 0;
  virtual ACE_HANDLE connectData () = 0;

  virtual void request (const struct FTP_Request&) = 0; // request
  // *NOTE*: to be executed as soon as the PASV connection has opened
  virtual void queue (const struct FTP_Request&) = 0; // request

  ////////////////////////////////////////
  virtual void responseCB (const struct FTP_Record&) = 0;
};

#endif
