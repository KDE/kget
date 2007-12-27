/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ktoggleaction.h>
#include <kxmlguiwindow.h>
#include <kurl.h>

#include "ui/tray.h"

class ViewsContainer;
class DropTarget;
class DBusModelObserver;

/**
 * The main window of KGet.
 *
 * Can be collapsed or expanded.
 */
class MainWindow : public KXmlGuiWindow
{
Q_OBJECT
public:
    explicit MainWindow(bool showMainwindow = true, bool startWithoutAnimation = false, QWidget *parent = 0);
    ~MainWindow();

    // from the DBUS interface
    virtual void addTransfers(const QString& src, const QString& destDir = QString(), 
                              bool start = false);
    virtual bool dropTargetVisible() const;
    virtual void setDropTargetVisible( bool setVisible );
    virtual void setOfflineMode( bool online );
    virtual bool offlineMode() const;
    virtual QVariantMap transfers() const;
    virtual int transfersSpeed() const;
    virtual void setSystemTrayDownloading(bool running);

    KSystemTrayIcon *systemTray() const { return m_dock;};

public slots:
    void slotQuit();

protected:
    // ignore/accept quit events
    virtual void closeEvent( QCloseEvent * );

    // drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);

    // set sensitive initial size
    virtual QSize sizeHint() const;

private slots:
    // slots connected to actions
    void slotToggleDropTarget();
    void slotNewTransfer();
    void slotImportTransfers();
    void slotExportTransfers();
    void slotPreferences();
    void slotDeleteGroup();
    void slotRenameGroup();
    void slotStartDownload();
    void slotStopDownload();
    void slotConfigureNotifications();
    void slotConfigureKeys();
    void slotConfigureToolbars();
    void slotToggleAutoPaste();
    void slotTrayKonquerorIntegration(bool);
    void slotKonquerorIntegration( bool );
    void slotShowMenubar();

    // transfers slots
    void slotStopAllDownload();
    void slotStopSelectedDownload();
    void slotStartAllDownload();
    void slotStartSelectedDownload();
    void slotDeleteSelected();
    void slotTransfersOpenDest();
    void slotTransfersShowDetails();
    void slotTransfersCopySourceUrl();

    // misc slots
    void slotDelayedInit();
    void slotSaveMyself();
    void slotNewToolbarConfig();
    void slotNewConfig();
    void slotCheckClipboard();

private:
    // one-time functions
    void setupActions();

    // internal widgets
    ViewsContainer * m_viewsContainer;
    // dbus modelObserver to export the transfer percents
    DBusModelObserver *m_dbusModelObserver;

    // separated widgets
    DropTarget    * m_drop;
    Tray          * m_dock;

    // actions
    KToggleAction * m_autoPasteAction;
    KToggleAction * m_menubarAction;
    KToggleAction * m_konquerorIntegration;

    // for autopaste function
    QString lastClipboard;
    // timer for checking clipboard - autopaste function
    QTimer *clipboardTimer;

    bool m_startWithoutAnimation;
};

#endif
