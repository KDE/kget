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

#include <qstringlist.h>
#include <qwidget.h>

#include <kcombobox.h>

class KIntNumInput;
class QCheckBox;
class QGroupBox;
class QLabel;

class DlgConnection:public QWidget
{

Q_OBJECT public:

    DlgConnection(QWidget * parent);
    ~DlgConnection()
    {}
    void applyData();
    void setData();

    int type() const
    {
        return cmb_type->currentItem();
    }

private:

    // reconnect settings
    QGroupBox * gb_reconnect;

    QCheckBox *cb_onerror;

    QLabel *lb_after;
    KIntNumInput *le_after;

    QLabel *lb_retries;
    KIntNumInput *le_retries;

    QCheckBox *cb_onbroken;

    QCheckBox *cb_autoresume;

    // timeout settings
    QGroupBox *gb_timeout;

    QLabel *lb_nodata;
    KIntNumInput *le_nodata;

    QLabel *lb_noresume;
    KIntNumInput *le_noresume;
    QLabel *lb_cannot;

    // type settings
    QGroupBox *gb_type;

    KComboBox *cmb_type;

    QLabel *lb_linknum;
    KIntNumInput *le_linknum;
    QCheckBox *cb_offlinemode;

signals:
    void typeChanged(int type);

protected slots:
    void comboActivated(int Index);

};

#endif                          // _DLGCONNECTION_H
