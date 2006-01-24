/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _KGET_H_
#define _KGET_H_

#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QToolButton>

#include <kaction.h>
#include <kmainwindow.h>

#include "dcopiface.h"
#include "core/globals.h"

class QSplitter;

class KUrl;
class KUrl::List;

class KGetModel;

class Sidebar;
class ViewsContainer;
class DropTarget;
class Tray;

/**
 * The main window of KGet.
 *
 * Can be collapsed or expanded.
 */
class KGet : public KMainWindow, virtual public DCOPIface
{
Q_OBJECT
public:
    KGet( QWidget * = 0, const char * = 0 );
    ~KGet();

protected:
    // from the DCOP interface
    virtual void addTransfers( const KUrl::List& src, const QString& destDir = QString() );
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
    void slotNewTransfer();
    void slotOpen();
    void slotQuit();
    void slotPreferences();
    void slotExportTransfers();
    void slotDownloadToggled();
    void slotConfigureNotifications();
    void slotConfigureKeys();
    void slotConfigureToolbars();
    void slotToggleAutoPaste();
    void slotShowDropTarget();
    void slotTrayKonquerorIntegration();
    void slotKonquerorIntegration( bool );
    void slotShowMenubar();

    // transfers slots
    void slotTransfersStart();
    void slotTransfersStop();
    void slotTransfersDelete();
    void slotTransfersOpenDest();
    void slotTransfersShowDetails();
    void slotTransfersCopySourceURL();

    // misc slots
    void slotDelayedInit();
    void slotSaveMyself();
    void slotNewToolbarConfig();
    void slotNewConfig();
    void slotCheckClipboard();

signals:
    void viewModeChanged( int );

private:
    // some functions
    void log( const QString & );
    // one-time functions
    void setupActions();

    // internal widgets
    QSplitter      * m_splitter;
    Sidebar        * m_sidebar;
    ViewsContainer * m_viewsContainer;

    // separated widgets
    DropTarget    * m_drop;
    Tray          * m_dock;
    KToggleAction * m_showDropTarget;
    KToggleAction * m_AutoPaste;
    KAction       * m_KonquerorIntegration;
    KToggleAction * m_menubarAction;

    // for autopaste function
    QString lastClipboard;
    // timer for checking clipboard - autopaste function
    QTimer *clipboardTimer;
};

#endif
