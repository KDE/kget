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

#include <qwidget.h>

#include <kaction.h>
#include <kmainwindow.h>

#include "dcopiface.h"
#include "core/viewinterface.h"
#include "core/globals.h"

class KURL;
class KURL::List;

class BrowserBar;
class DropTarget;
class GroupsPanel;
class MainView;
class Sidebar;
class Tray;

/** The main window of KGet2. Can be collapsed or expanded. */
class KMainWidget : public KMainWindow, public ViewInterface, virtual public DCOPIface
{
Q_OBJECT
public:
    KMainWidget( QWidget * = 0, const char * = 0 );
    ~KMainWidget();

    // toggles between small / complete layouts
    enum ViewMode { vm_compact = 0, vm_transfers = 1, vm_downloaded = 2 };
    void setViewMode( enum ViewMode, bool force = false );

    // called by main.cpp
    void readTransfersEx(const KURL & url);

protected:
    // from the DCOP interface
    virtual void addTransfers( const KURL::List& src, const QString& destDir = QString::null );
    virtual bool isDropTargetVisible() const;
    virtual void setDropTargetVisible( bool setVisible );
    virtual void setOfflineMode( bool online );
    virtual bool isOfflineMode() const;

    // ignore/accept quit events
    virtual void closeEvent( QCloseEvent * );
    
    // drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);

private slots:
    // slots connected to actions
    void slotNewURL();
    void slotQuit();
    void slotPreferences();
    void slotExportTransfers();
    void slotImportTransfers();
    void slotDownloadToggled();
    void slotConfigureNotifications();
    void slotConfigureKeys();
    void slotConfigureToolbars();

    // misc slots
    void slotDelayedInit();
    void slotSaveMyself();
    void slotNewToolbarConfig();
    void slotNewConfig();

signals:
    void viewModeChanged( int );

private:
    // some functions
    void updateActions();
    void updateStatusBar();
    void log( const QString &, bool sbar = true );
    // one-time functions
    void setupActions();

    // internals
    Scheduler * scheduler;
    enum ViewMode vMode;

    // internal widgets
    BrowserBar  * browserBar;
    GroupsPanel * groupsPanel;
    Sidebar     * sidebar;
    MainView    * mainView;
    QWidget     * rightWidget;

    // separated widgets
    DropTarget * kdrop;
    Tray       * kdock;
};

#endif
