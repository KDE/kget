/***************************************************************************
*                               dlgConnection.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
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

#ifndef _DLGCONNECTION_H
#define _DLGCONNECTION_H

#include "dlgconnectionbase.h"

class DlgConnection : public DlgConnectionBase
{

Q_OBJECT public:

    DlgConnection(QWidget * parent);
    ~DlgConnection() {}
    void applyData();
    void setData();

    int type() const;

signals:
    void typeChanged(int type);
    void configChanged();

protected slots:
    void comboActivated(int Index);
    void slotChanged();
};

#endif                          // _DLGCONNECTION_H
