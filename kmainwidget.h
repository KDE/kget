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
#include <kaction.h>
#include <kurl.h>
#include "globals.h"

#include "kget_iface.h"
#include "scheduler.h"

class KAction;
//class KToggleAction;
class KRadioAction;

class DockWidget;
class DropTarget;
class LogWindow;
class DlgPreferences;

class Settings;


class KMainWidget:public KMdiMainFrm, virtual public KGetIface
{

Q_OBJECT 

public:
    enum StatusbarFields { ID_TOTAL_TRANSFERS = 1, ID_TOTAL_FILES, ID_TOTAL_SIZE,
                           ID_TOTAL_TIME         , ID_TOTAL_SPEED                };

    KMainWidget(bool bShowMain = false);
    ~KMainWidget();

    // dcop interface
    virtual void addTransfers( const KURL::List& src, const QString& destDir = QString::null );
    virtual bool isDropTargetVisible() const;
    virtual void setDropTargetVisible( bool setVisible );

    LogWindow *logwin()const { return logWindow;}

    // Actions
    KToggleAction *m_paShowLog;
    KAction *m_paPreferences;
    KAction *m_paQuit;
    bool b_viewLogWindow;

    void readTransfersEx(const KURL & url);

public slots:
    void slotNewURL();
    
    void slotToggleLogWindow();
    void slotPreferences();
    void slotToggleExpertMode();
    void slotToggleUseLastDir();
    void slotToggleAutoShutdown();
    void slotToggleDropTarget();
    void slotToggleSound();
    void slotUpdateActions();
protected slots:
    void slotQuit();

    void slotSaveYourself();

//    void slotTransferTimeout();
//    void slotAutosaveTimeout();

//    void slotCheckClipboard();

    void slotConfigureKeys();
    void slotConfigureToolbars();
    void slotNewToolbarConfig();

protected:
    //From the DCOP iface
    virtual void setOfflineMode( bool online );
    virtual bool isOfflineMode() const;

    virtual bool queryClose();
    void writeLog();

    // drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);

    void readTransfers(bool ask_for_name = false);
    void writeTransfers(bool ask_for_name = false);


    void setupActions();
    void setupGUI(bool startDocked);
    void setupConnections();
    void setupWhatsThis();
    void setupUserSettings();
    
    void updateStatusBar();

    // some flags
    bool b_viewPreferences;

    // utility functions
    void log(const QString & message, bool statusbar = true);

    // various timers
    QTimer *transferTimer;      // timer for scheduled transfers
    QTimer *autosaveTimer;      // timer for autosaving transfer list

    QString logFileName;


private:
    bool sanityChecksSuccessful( const KURL& url );

    Scheduler * scheduler;
    
    KHelpMenu *menuHelp;

    LogWindow *logWindow;
    DlgPreferences *prefDlg;
    DockWidget *kdock;

    QString lastClipboard;

    int _sock;

    // Actions
    KAction *m_paOpenTransfer, *m_paExportTransfers, *m_paImportTransfers;
    KAction *m_paImportText;

    KAction *m_paMoveToBegin, *m_paMoveToEnd, *m_paIndividual;
    KAction *m_paResume, *m_paPause, *m_paDelete, *m_paRestart;

    KToggleAction *m_paUseSound;
    KToggleAction *m_paExpertMode, *m_paUseLastDir;
    KToggleAction *m_paAutoShutdown;

    KToggleAction *m_paDropTarget;

    public:
    void activateDropTarget(void){if(!m_paDropTarget->isChecked()) m_paDropTarget->activate();};

};

extern KMainWidget *kmain;
extern DropTarget *kdrop;

#endif                          // _KMAINWIDGET_H_
