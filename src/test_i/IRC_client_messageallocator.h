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

#ifndef IRC_CLIENT_MESSAGEALLOCATOR_H
#define IRC_CLIENT_MESSAGEALLOCATOR_H

#include "stream_cachedmessageallocator.h"

#include "IRC_client_message.h"
#include "IRC_client_sessionmessage.h"

// forward declarations
class ACE_Allocator;

class IRC_Client_MessageAllocator
 : public Stream_CachedMessageAllocator_T<IRC_Client_Message,
                                          IRC_Client_SessionMessage>
{
 public:
  IRC_Client_MessageAllocator (unsigned int,    // total number of concurrent messages
                               ACE_Allocator*); // (heap) memory allocator...
  virtual ~IRC_Client_MessageAllocator ();

 private:
  typedef Stream_CachedMessageAllocator_T<IRC_Client_Message,
                                          IRC_Client_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_MessageAllocator (const IRC_Client_MessageAllocator&));
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_MessageAllocator& operator= (const IRC_Client_MessageAllocator&));
};

#endif
