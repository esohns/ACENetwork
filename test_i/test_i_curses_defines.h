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

#ifndef TEST_I_CURSES_DEFINES_H
#define TEST_I_CURSES_DEFINES_H

// colors
#define TEST_I_CURSES_COLOR_LOG                     1
#define TEST_I_CURSES_COLOR_STATUS                  2

// cursor
#define TEST_I_CURSES_CURSOR_MODE_INVISIBLE         0
#define TEST_I_CURSES_CURSOR_MODE_NORMAL            1 // terminal-specific
#define TEST_I_CURSES_CURSOR_MODE_HIGHLIGHTED       2 // terminal-specific
#define TEST_I_CURSES_DEFAULT_CURSOR_MODE           TEST_I_CURSES_CURSOR_MODE_NORMAL

#endif
