/***************************************************************************
*                               dlgIndividual.h
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

#ifndef __dlgprogress_h__
#define __dlgprogress_h__

#include <qlabel.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qdatetime.h>

#include <kprogress.h>

#include "bwdatetime.h"

#include <kdialog.h>
#include <qtextedit.h>

class Transfer;


class DlgIndividual:public /* QWidget QDialog */ KDialog
{

Q_OBJECT public:
        DlgIndividual(Transfer * _item);
        ~DlgIndividual()
        {}
        /** No descriptions */
        void addLog(const QString & _msg);

        /** No descriptions */

public slots:
        void setTotalSize(unsigned long bytes);
        void setTotalFiles(unsigned long files);
        void setTotalDirs(unsigned long dirs);

        void setProcessedSize(unsigned long bytes);
        void setProcessedFiles(unsigned long files);
        void setProcessedDirs(unsigned long dirs);

        void setSpeed(unsigned long bytes_per_second, QTime remaining);
        void setPercent(unsigned long percent);

        void setCopying(const KURL & src, const KURL & dest);
        void setCanResume(bool);

protected slots:
        void slotToggleAdvanced(bool);

protected:
        QLabel * progressLabel;
        QLabel *sourceLabel;
        QLabel *destLabel;
        QLabel *speedLabel;
        QLabel *sizeLabel;
        QLabel *resumeLabel;
        QTextEdit *ml_log;

        KProgress *m_pProgressBar;

        QPushButton *pbAdvanced;
        QTabWidget *panelAdvanced;

        QDateTime qdt;
        BWDateTime *spins;

        // search stuff
        //        SearchList *listSearch;

        Transfer *item;

        QString m_sFilename;
        unsigned long m_iTotalSize;
        unsigned long m_iTotalFiles;
        unsigned long m_iTotalDirs;

        unsigned long m_iProcessedDirs;
        unsigned long m_iProcessedFiles;

public:                      // Public attributes
}

;

#endif                          // __dlgprogress_h__
