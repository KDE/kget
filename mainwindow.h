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
class HttpServer;

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
    virtual void addTransfer(const QString& src, const QString& destDir = QString(), 
                              bool start = false);
    virtual void showNewTransferDialog(const QStringList &urls);
    virtual bool dropTargetVisible() const;
    virtual void setDropTargetVisible( bool setVisible );
    virtual void setOfflineMode( bool online );
    virtual bool offlineMode() const;
    virtual QVariantMap transfers() const;
    virtual int transfersSpeed() const;
    virtual void setSystemTrayDownloading(bool running);
    virtual void importLinks(const QList <QString> &links);

    KSystemTrayIcon *systemTray() const { return m_dock;};

public slots:
    void slotQuit();
    void slotImportUrl(const QString &url);
    void slotUpdateTitlePercent();

protected:
    // ignore/accept quit events
    virtual void closeEvent(QCloseEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);

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
    void slotExportPlainTransfers();
    void slotPreferences();
    void slotDeleteGroup();
    void slotRenameGroup();
    void slotSetIconGroup();
    void slotStartDownload();
    void slotStopDownload();
    void slotConfigureNotifications();
    void slotConfigureKeys();
    void slotConfigureToolbars();
    void slotToggleAutoPaste();
    void slotTrayKonquerorIntegration(bool);
    void slotKonquerorIntegration( bool );
    void slotShowMenubar();
    void slotTransferGroupSettings();
    void slotTransferSettings();

    // transfers slots
    void slotStopAllDownload();
    void slotStopSelectedDownload();
    void slotStartAllDownload();
    void slotStartSelectedDownload();
    void slotDeleteSelected();
    void slotRedownloadSelected();
    void slotTransfersOpenDest();
    void slotTransfersOpenFile();
    void slotTransfersShowDetails();
    void slotTransfersCopySourceUrl();
    void slotDeleteFinished();

    // misc slots
    void slotDelayedInit();
    void slotSaveMyself();
    void slotNewToolbarConfig();
    void slotNewConfig();
    void slotCheckClipboard();
    void slotTransferHistory();

    // import links slots
    void slotShowListLinks();

private:
    void exportTransfers(bool plain=false);

    /**
    * Returns the completed percents of all active transfers
    */
    int transfersPercent();

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

    HttpServer *m_webinterface;
};

/**
 * Checks every transfer for a percent change to update the mainwindow title
 */
class MainWindowTransferObserver : public QObject, public TransferObserver
{
    public:
        MainWindowTransferObserver(MainWindow *window);
        virtual ~MainWindowTransferObserver(){}

        virtual void transferChangedEvent(TransferHandler * transfer);

//        virtual void deleteEvent(TransferHandler * transfer);

    private:
        MainWindow *m_window;
};

/**
* Used to update the mainwindow caption when the groups percents change
*/
class MainWindowGroupObserver : public QObject, public TransferGroupObserver
{
    Q_OBJECT
    public:
        MainWindowGroupObserver(MainWindow *window);
        virtual ~MainWindowGroupObserver() {}

        virtual void groupChangedEvent(TransferGroupHandler * group);

        virtual void addedTransferEvent(TransferHandler * transfer, TransferHandler * after);

        virtual void removedTransferEvent(TransferHandler * transfer);
/**
        virtual void movedTransferEvent(TransferHandler * transfer, TransferHandler * after);**/

    private:
        MainWindow *m_window;
        MainWindowTransferObserver *m_transferObserver;
};

class MainWindowModelObserver : public QObject, public ModelObserver
{
    Q_OBJECT
    public:
        MainWindowModelObserver(MainWindow *window);
        virtual ~MainWindowModelObserver (){}

        virtual void addedTransferGroupEvent(TransferGroupHandler * group);

        virtual void removedTransferGroupEvent(TransferGroupHandler * group);

    private:
        MainWindow *m_window;
        MainWindowGroupObserver *m_groupObserver;
};

#endif
