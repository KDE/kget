/***************************************************************************
                                logwindow.h
                             -------------------
    Revision 				: $Id$
    begin						: Tue Jan 29 2002
    copyright				: (C) 2002 by Patrick Charbonnier
									: Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
    email						: pch@freeshell.og
 ***************************************************************************/

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


#ifndef _LOGWINDOW_H
#define _LOGWINDOW_H

#include <qlistview.h>
#include <qtextview.h>
#include <qmap.h>

#include <kdialogbase.h>

class SeparatedLog:public QWidget
{

Q_OBJECT public:
        SeparatedLog(QWidget * parent);
        ~SeparatedLog()
        {}
        void addLog(uint id, const QString & filename, const QString & message);
        void refresh();

protected slots:
        void transferSelected(QListViewItem * item);

private:
        QListView * lv_log;
        QTextView *ml_log;

        typedef QMap < uint, QString > TransferMap;
        TransferMap trMap;

        uint idSelected;
};


class LogWindow:public KDialogBase
{

Q_OBJECT public:
        LogWindow();
        ~LogWindow()
        {}
        void logGeneral(const QString & message);
        QString getText() const;

public slots:
        void logTransfer(uint id, const QString & filename, const QString & message);

protected:
        void closeEvent(QCloseEvent *);

private:
        QTextView * mixed_log;
        SeparatedLog *sep_log;
};


#endif				// _LOGWINDOW_H
