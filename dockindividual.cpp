/***************************************************************************
*                               dockindividual.cpp
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

#include <qtooltip.h>

#include <kiconloader.h>
#include <kpopupmenu.h>

#include "dockindividual.h"
DockIndividual::DockIndividual(QWidget *parent, const char *name ) : KSystemTray(parent,name)
{
    nPic=0;
    setPixmap( UserIcon("bar0") );


}
DockIndividual::~DockIndividual(){
    QToolTip::remove(this);
}


void DockIndividual::setValue(int value){
#ifdef _DEBUG
    //sDebugIn<<" value ="<<value<<endl;
#endif
    int tmpPic=0;
    if (value<20)
        tmpPic=1;
    else if(value<40)
        tmpPic=2;

    else if(value<60)
        tmpPic=3;

    else if(value<80)
        tmpPic=4;

    else if(value<=95)
        tmpPic=5;

    else if(value>=96)
        tmpPic=6;

    if (tmpPic!=nPic)
    {
        nPic=tmpPic;
        QString str = "bar" + QString::number( nPic );
        setPixmap( UserIcon( str ) );
    }

#ifdef _DEBUG
    //sDebugOut<<endl;
#endif
}



void DockIndividual::setTip(const QString & _tip)
{
#ifdef _DEBUG
    //sDebugIn<<"_tip="<<_tip<<endl;
#endif


    QToolTip::add( this, _tip );

#ifdef _DEBUG
    //sDebugOut<<endl;
#endif
}

#include "dockindividual.moc"
/** No descriptions */
void DockIndividual::contextMenuAboutToShow ( KPopupMenu* menu )
{
    menu->removeItemAt (3);
}
