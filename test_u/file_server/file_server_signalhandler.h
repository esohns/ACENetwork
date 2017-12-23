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

#ifndef FileServer_SignalHandler_H
#define FileServer_SignalHandler_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_signalhandler.h"

#include "file_server_common.h"

class FileServer_SignalHandler
 : public Common_SignalHandler_T<struct FileServer_SignalHandlerConfiguration>
{
  typedef Common_SignalHandler_T<struct FileServer_SignalHandlerConfiguration> inherited;

 public:
  FileServer_SignalHandler (enum Common_SignalDispatchType, // dispatch mode
                            ACE_SYNCH_MUTEX*);              // lock handle
  inline virtual ~FileServer_SignalHandler () {}

  // implement Common_ISignal
  virtual void handle (const struct Common_Signal&); // signal

 private:
  ACE_UNIMPLEMENTED_FUNC (FileServer_SignalHandler ())
  ACE_UNIMPLEMENTED_FUNC (FileServer_SignalHandler (const FileServer_SignalHandler&))
  ACE_UNIMPLEMENTED_FUNC (FileServer_SignalHandler& operator= (const FileServer_SignalHandler&))
};

#endif
