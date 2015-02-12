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

#ifndef Net_CONTROLLER_BASE_H
#define Net_CONTROLLER_BASE_H

#include "ace/Global_Macros.h"

#include "common_icontrol.h"

class Net_Controller_Base
 : public Common_IControl
{
 public:
  Net_Controller_Base ();
  virtual ~Net_Controller_Base ();

  // implement Common_IControl
  virtual void start ();
  virtual void stop ();
  virtual bool isRunning () const;

 private:
  typedef Common_IControl inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Controller_Base (const Net_Controller_Base&));
  ACE_UNIMPLEMENTED_FUNC (Net_Controller_Base& operator= (const Net_Controller_Base&));
};

#endif
