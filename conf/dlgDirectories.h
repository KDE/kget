/***************************************************************************
*                               dlgDirectories.h
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

#ifndef _DLGDIRECTORIES_H
#define _DLGDIRECTORIES_H

#include <qstringlist.h>
#include "dlgdirectoriesbase.h"

class DlgDirectories : public DlgDirectoriesBase
{
	Q_OBJECT
	Q_PROPERTY( QStringList list READ list WRITE setList )
public:

    DlgDirectories(QWidget * parent);
    void applyData();
    void setData();
    
    QStringList list() const { return l; }
    void setList( QStringList li ) { l=li; }

public slots:
    void changed( const QStringList &line ) { setList(line); };

private:
    QStringList l;
    
signals:
    void configChanged();

protected slots:
    void selectEntry(QListViewItem * item);
    void addEntry();
    void deleteEntry();
    void changeEntry();
    void browse();

    void upEntry();
    void downEntry();
    void slotDirectoryChanged( );
protected:
    void updateUpDown();

};

#endif                          // _DLGDIRECTORIES_H
