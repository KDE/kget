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

#include <qguardedptr.h>

#include <kmainwindow.h>
#include <kaction.h>
#include "common.h"
class KAction;
//class KToggleAction;
class KRadioAction;

class DockWidget;
class DropTarget;
class LogWindow;
class DlgPreferences;

class Transfer;
class TransferList;
class Settings;


class KMainWidget:public KMainWindow
{

Q_OBJECT public:
    enum StatusbarFields { ID_TOTAL_TRANSFERS = 1, ID_TOTAL_FILES, ID_TOTAL_SIZE,
                           ID_TOTAL_TIME         , ID_TOTAL_SPEED                };

    KMainWidget(bool bShowMain = false);
    ~KMainWidget();

    void addTransfer(QString src, QString dest = QString::null);
    void addTransferEx(QString src, QString dest = QString::null, bool bShowIndividual = false);
    void addDropTransfers(QStrList * list);


    void checkQueue();

    void setListFont();
    void setAutoSave();
    void setAutoDisconnect();

    LogWindow *logwin() { return logWindow;}
    friend class Settings;

    // Actions
    KToggleAction *m_paShowLog;
    KAction *m_paPreferences;
    KAction *m_paQuit;
    bool b_viewLogWindow;

    void readTransfersEx(const QString & txt);
 
public slots:
    void slotPasteTransfer();
    void slotToggleLogWindow();
    void slotPreferences();
    void slotToggleExpertMode();
    void slotToggleOfflineMode();
    void slotToggleUseLastDir();
    void slotToggleAutoDisconnect();
    void slotToggleAutoShutdown();
    void slotToggleAutoPaste();
    void slotToggleDropTarget();
    void slotToggleAnimation();
    void slotToggleSound();
    void slotUpdateActions();
protected slots:
    void slotQuit();

    void slotOpenTransfer();
    void slotExportTransfers();
    void slotImportTransfers();
    void slotImportTextFile();

    void slotSaveYourself();
    void slotCheckConnection();


    void slotStatusChanged(Transfer * item, int _operation);

    void slotResumeCurrent();
    void slotPauseCurrent();
    void slotDeleteCurrent();
    void slotRestartCurrent();

    void slotQueueCurrent();
    void slotTimerCurrent();
    void slotDelayCurrent();

    void slotOpenIndividual();

    void slotToggleStatusbar();

    void slotAnimTimeout();
    void slotTransferTimeout();
    void slotAutosaveTimeout();

    void slotMoveToBegin();
    void slotMoveToEnd();

    void slotCopyToClipboard();
    void slotCheckClipboard();

    void slotConfigureKeys();
    void slotConfigureToolbars();

    void slotPopupMenu(Transfer * item);

protected:
    void closeEvent(QCloseEvent *);
    void writeLog();

    // drag and drop
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    void readTransfers(bool ask_for_name = false);
    void writeTransfers(bool ask_for_name = false);


    void setupGUI();
    void setupWhatsThis();

    void updateStatusBar();

    // some flags
    bool b_online;
    bool b_viewPreferences;

    // utility functions
    void disconnect();
    void checkOnline();
    void pauseAll();
    void log(const QString & message, bool statusbar = true);

    /** No descriptions */
    virtual void customEvent(QCustomEvent * e);

    // various timers
    QTimer *animTimer;          // animation timer
    QTimer *connectionTimer;    // timer that checks whether we are online
    QTimer *transferTimer;      // timer for scheduled transfers
    QTimer *autosaveTimer;      // timer for autosaving transfer list
    QTimer *clipboardTimer;     // timer for checking clipboard - autopaste function

    QString logFileName;



private:
    TransferList * myTransferList;
    KHelpMenu *menuHelp;

    LogWindow *logWindow;
    DlgPreferences *prefDlg;

    QString lastClipboard;

    QString currentDirectory;

    uint animCounter;

    int _sock;

    // Actions
    KAction *m_paOpenTransfer, *m_paPasteTransfer, *m_paExportTransfers, *m_paImportTransfers;
    KAction *m_paImportText;

    KAction *m_paMoveToBegin, *m_paMoveToEnd, *m_paCopy, *m_paIndividual;
    KAction *m_paResume, *m_paPause, *m_paDelete, *m_paRestart;
    KRadioAction *m_paQueue, *m_paTimer, *m_paDelay;

    KToggleAction *m_paUseAnimation, *m_paUseSound;
    KToggleAction *m_paExpertMode, *m_paUseLastDir, *m_paOfflineMode;
    KToggleAction *m_paAutoDisconnect, *m_paAutoShutdown, *m_paAutoPaste;

    KToggleAction *m_paShowStatusbar;
    KToggleAction *m_paDropTarget;

    public:
    void activateDropTarget(void){if(!m_paDropTarget->isChecked()) m_paDropTarget->activate();};

};

extern KMainWidget *kmain;
extern QGuardedPtr < DropTarget > kdrop;

#endif                          // _KMAINWIDGET_H_
