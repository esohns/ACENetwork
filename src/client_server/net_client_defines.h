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

#ifndef NET_CLIENT_DEFINES_H
#define NET_CLIENT_DEFINES_H

#define NET_CLIENT_DEFAULT_NUMBER_OF_REACTOR_DISPATCH_THREADS        1
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS       1
#else
// *IMPORTANT NOTE*: on Linux, specifying 1 will not work correctly for proactor
//                   scenarios using the default (rt signal) proactor
//                   implementation. The thread blocked in sigwaitinfo (see man
//                   pages) will not awaken when the dispatch set is changed
//                   (*TODO*: to be verified)
#define NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS       2
#endif // ACE_WIN32 || ACE_WIN64

#endif
