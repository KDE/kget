/***************************************************************************
*                               dlgPreferences.h
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

#ifndef _DLGPREFERENCES_H
#define _DLGPREFERENCES_H

#include <qdialog.h>
#include <qstringlist.h>

#include <kdialogbase.h>

class DlgConnection;
class DlgAutomation;
class DlgLimits;
class DlgAdvanced;
class DlgSearch;
class DlgDirectories;
class DlgSystem;


class DlgPreferences:public KDialogBase
{

Q_OBJECT public:

    DlgPreferences(QWidget * parent);
    ~DlgPreferences()
    {}
protected:
    void closeEvent(QCloseEvent *);

private:

    DlgConnection * conDlg;
    DlgAutomation *autDlg;
    DlgLimits *limDlg;
    DlgAdvanced *advDlg;

    //        DlgSearch *seaDlg;
    DlgDirectories *dirDlg;
    DlgSystem *sysDlg;

protected slots:
    virtual void done(int r);
    void applySettings();

};

#endif                          // _DLGPREFERENCES_H
