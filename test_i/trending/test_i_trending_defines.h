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

#ifndef TEST_I_TRENDING_DEFINES_H
#define TEST_I_TRENDING_DEFINES_H

#include "ace/config-lite.h"

#define TEST_I_ISIN_LENGTH                           12 // 2 + 10
#define TEST_I_DEFAULT_ISIN                          "DE000ETFL581"

#define TEST_I_DEFAULT_LIBREOFFICE_REFERENCE_ROW     5
#define TEST_I_DEFAULT_LIBREOFFICE_START_ROW         8

#define TEST_I_LIBREOFFICE_DATE_COLUMN               11 // K
#define TEST_I_LIBREOFFICE_DATE_ROW                  4

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE    "soffice.ini"
#else
#define TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE    "sofficerc"
#endif
#define TEST_I_DEFAULT_INPUT_FILE                    "template.ods"
#define TEST_I_DEFAULT_OUTPUT_FILE                   "output.ods"

#define TEST_I_DEFAULT_PORT                          STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_SERVER_PORT

#define TEST_I_URL_SYMBOL_PLACEHOLDER                "%s"
#define TEST_I_DEFAULT_URL                           "https://www.tagesschau.de/wirtschaft/boersenkurse/ajax/get_search.htn"
#define TEST_I_DEFAULT_FORM_KEY_LIMIT_STRING         "limit"
#define TEST_I_DEFAULT_FORM_KEY_SEARCH_STRING        "suchbegriff"

#endif
