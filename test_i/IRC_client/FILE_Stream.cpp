/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

//#include <iostream>
//using namespace std;
//// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
////                   a problem in conjunction with the standard include headers
////                   when ACE_USES_OLD_IOSTREAMS is defined
////                   --> include the necessary headers manually (see above), and
////                       prevent ace/iosfwd.h from causing any harm
//#define ACE_IOSFWD_H

//#include "ace/FILE_Stream.h"
#include "FILE_Stream.h"

#if !defined (__ACE_INLINE__)
//#include "ace/FILE_Stream.inl"
#include "FILE_Stream.inl"
#endif /* __ACE_INLINE__ */

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_ALLOC_HOOK_DEFINE (ACE_FILE_Stream)

void
ACE_FILE_Stream::dump (void) const
{
#if defined (ACE_HAS_DUMP)
  ACE_TRACE ("ACE_FILE_Stream::dump");
#endif /* ACE_HAS_DUMP */
}

//int
//ACE_FILE_Stream::close (void)
//{
//  return ACE_FILE::close ();
//}

ACE_END_VERSIONED_NAMESPACE_DECL
