/***************************************************************************
*                               dockindividual.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 :
*    email        : pch@freeshell.org
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


#ifndef DOCKINDIVIDUAL_H
#define DOCKINDIVIDUAL_H

#include <ksystemtray.h>
#include "common.h"

class KPopupMenu;

class DockIndividual : public KSystemTray  {
    Q_OBJECT
public:
    DockIndividual(QWidget *parent=0, const char *name=0);
    ~DockIndividual();
    int nPic;
    void setTip(const QString &);
    void setValue(int value);
    /** No descriptions */
    virtual void contextMenuAboutToShow ( KPopupMenu* menu );
};

#endif
