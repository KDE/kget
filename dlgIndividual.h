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

#include <qdatetime.h>
#include "bwdatetime.h"

#include <kdialog.h>

class Transfer;

class QLabel;
class QRadioButton;
class QTabWidget;
class QTextEdit;
class QCheckBox;
class KPushButton;
class KProgress;

class DockIndividual;

class DlgIndividual:public KDialog
{
Q_OBJECT 
public:
    DlgIndividual(Transfer * _item);
    ~DlgIndividual()
    {}
    void addLog(const QString & _msg);
    void enableOpenFile();

public slots:
    void setTotalSize(unsigned long bytes);

    void setProcessedSize(unsigned long bytes);

    void setSpeed(unsigned long bytes_per_second, QTime remaining);
    void setPercent(unsigned long percent);

    void setCopying(const KURL & src, const KURL & dest);
    void setCanResume(bool);
    void slotKeepOpenToggled(bool);
    void slotOpenLocation();
    void slotOpenFile();

protected slots:
    void slotToggleAdvanced(bool);
    void slotToggleDock();

protected:
    QLabel *sourceLabel;
    QLabel *destLabel;
    QLabel *speedLabel;
    QLabel *sizeLabel;
    QLabel *resumeLabel;
    QTextEdit *ml_log;

    KProgress *m_pProgressBar;
    DockIndividual * m_pDockIndividual;

    KPushButton   * openFile;
    KPushButton   * openLocation;
    KPushButton   * pbAdvanced ;

    QTabWidget    * panelAdvanced;
    KToggleAction * m_paDock;

    QDateTime qdt;
    BWDateTime *spins;

    Transfer *item;

    KURL m_location;

    bool bKeepDlgOpen;

    unsigned long m_iTotalSize;

}

;

#endif                          // __dlgprogress_h__
