/***************************************************************************
*                                slaveevent.h
*                             -------------------
*
*    Revision     : $Id$
*    begin          : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*    email          :pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/


#ifndef SLAVEEVENT_H
#define SLAVEEVENT_H

#include <qevent.h>

class Transfer;

/**
  *@author Patrick Charbonnier
  */

class SlaveEvent:public QCustomEvent
{
public:
        SlaveEvent(Transfer * _item, unsigned int _event, unsigned long _ldata = 0L);
        SlaveEvent(Transfer * _item, unsigned int _event, const QString & _msg);
        ~SlaveEvent();

        unsigned int getEvent() const;
        Transfer *getItem() const;
        unsigned long getData() const;
        const QString & getMsg() const;


private:
        unsigned int m_event;
        Transfer *m_item;
        unsigned long m_ldata;
        QString m_msg;

};

#endif
