/***************************************************************************
*                                kmainwidget.h
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


#ifndef _KMAINWIDGET_H_
#define _KMAINWIDGET_H_

#include <kmdimainfrm.h>
#include <kurl.h>
#include "globals.h"

#include "kget_iface.h"
#include "viewinterface.h"

class KAction;
class KRadioAction;

class DockWidget;
class DropTarget;
class LogWindow;

class KMainWidget:public KMdiMainFrm, public ViewInterface, virtual public KGetIface
{

Q_OBJECT 

public:
    KMainWidget();
    ~KMainWidget();

    // dcop interface
    virtual void addTransfers( const KURL::List& src, const QString& destDir = QString::null );
    virtual bool isDropTargetVisible() const;
    virtual void setDropTargetVisible( bool setVisible );

    // called by main.cpp
    void readTransfersEx(const KURL & url);

protected slots:
    void slotNewURL();
    
    void slotToggleLogWindow();
    void slotPreferences();
    void slotToggleExpertMode();
    void slotToggleAutoShutdown();
    void slotToggleDropTarget();
    void slotUpdateActions();

protected slots:
    void slotQuit();

    void slotSaveYourself();
    void slotExportTransfers();
    void slotImportTransfers();
    void slotRun();
    void slotStop();
    
    void slotConfigureKeys();
    void slotConfigureToolbars();
    void slotNewToolbarConfig();
    void slotEditNotifications();
    void slotNewConfig();

protected:
    //From the DCOP iface
    virtual void setOfflineMode( bool online );
    virtual bool isOfflineMode() const;

    virtual bool queryClose();

    // drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);

    void readTransfers(bool ask_for_name = false);
    void writeTransfers(bool ask_for_name = false);

    // utility functions
    void setupActions();
    void setupGUI();
    void updateStatusBar();
    void log(const QString & message, bool statusbar = true);


private:
    bool sanityChecksSuccessful( const KURL& url );

    Scheduler * scheduler;

    // child widgets
    DropTarget *kdrop;
    DockWidget *kdock;
    LogWindow *logWindow;
    KHelpMenu *menuHelp;

    // various timers
    QTimer *transferTimer;      // timer for scheduled transfers
    QTimer *autosaveTimer;      // timer for autosaving transfer list
    // some variables
    QString logFileName;
    QString lastClipboard;
    bool b_viewLogWindow;

    // Actions
    KAction *m_paPreferences, *m_paQuit;
    KAction *m_paOpenTransfer, *m_paExportTransfers, *m_paImportTransfers;
    KAction *m_paImportText;

    KToggleAction *m_paShowLog;
    KAction *m_paMoveToBegin, *m_paMoveToEnd, *m_paIndividual;
    KAction *m_paResume, *m_paPause, *m_paDelete, *m_paRestart;

    KToggleAction *m_paExpertMode;
    KToggleAction *m_paAutoShutdown;

    KToggleAction *m_paDropTarget;

public:
    void activateDropTarget(void){if(!m_paDropTarget->isChecked()) m_paDropTarget->activate();};

};

#endif                          // _KMAINWIDGET_H_
