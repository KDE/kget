/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 - 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KStatusNotifierItem>
#include <KToggleAction>
#include <KXmlGuiWindow>

#include "ui/tray.h"
#include "core/transfer.h"
#include "core/transfergroup.h"

#ifdef HAVE_QCA2
#include <QtCrypto>
#endif

class ViewsContainer;
class DropTarget;
class DBusKgetWrapper;
class HttpServer;
class KGet;

/**
 * The main window of KGet.
 *
 * Can be collapsed or expanded.
 */
class MainWindow : public KXmlGuiWindow
{
    friend class DBusKGetWrapper;

Q_OBJECT
public:
    explicit MainWindow(bool showMainwindow = true, bool startWithoutAnimation = false, bool doTesting = false, QWidget *parent = nullptr);
    ~MainWindow() override;

    virtual void setSystemTrayDownloading(bool running);

    //no slot, to make sure that MainWindow is correctly initialized before any transfers get added
    void init();

public Q_SLOTS:
    void slotQuit();
    void slotImportUrl(const QString &url);
    void slotUpdateTitlePercent();

protected:
    // ignore/accept quit events
    void closeEvent(QCloseEvent *) override;
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

    // drag and drop
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;

    // set sensitive initial size
    QSize sizeHint() const override;

private Q_SLOTS:
    // slots connected to actions
    void slotToggleDropTarget();
    void slotNewTransfer();
    void slotImportTransfers();
    void slotExportTransfers();
    void slotPreferences();
    void slotDeleteGroup();
    void slotRenameGroup();
    void slotSetIconGroup();
    void slotStartDownload();
    void slotStopDownload();
    void slotConfigureNotifications();
    void slotToggleAutoPaste();
    void slotTrayKonquerorIntegration(bool);
    void slotKonquerorIntegration( bool );
    void slotShowMenubar();
    void slotTransferGroupSettings();
    void slotTransferSettings();
    void slotCreateMetalink();
    void slotPriorityTop();
    void slotPriorityBottom();
    void slotPriorityUp();
    void slotPriorityDown();
    void slotDownloadFinishedActions();

    // transfers slots
    void slotStopAllDownload();
    void slotStopSelectedDownload();
    void slotStartAllDownload();
    void slotStartSelectedDownload();
    void slotDeleteSelected();
    void slotDeleteSelectedIncludingFiles();
    void slotRedownloadSelected();
    void slotTransfersOpenDest();
    void slotTransfersOpenFile();
    void slotTransfersShowDetails();
    void slotTransfersCopySourceUrl();
    void slotDeleteFinished();

    // misc slots
    void slotSaveMyself();
    void slotNewToolbarConfig();
    void slotNewConfig();
    void slotCheckClipboard();
    void slotTransferHistory();

    // import links slots
    void slotShowListLinks();
    
    //Model changes
    void slotTransfersChanged(QMap<TransferHandler*, Transfer::ChangesFlags> transfers);
    void slotGroupsChanged(QMap<TransferGroupHandler*, TransferGroup::ChangesFlags> groups);

private:
    /**
    * Returns the completed percents of all active transfers
    */
    int transfersPercent();

    // one-time functions
    void setupActions();
    
    KGet * m_kget = nullptr;

    // internal widgets
    ViewsContainer * m_viewsContainer = nullptr;

    // separated widgets
    DropTarget    * m_drop = nullptr;
    Tray          * m_dock = nullptr;

    // actions
    KToggleAction * m_autoPasteAction = nullptr;
    KToggleAction * m_menubarAction = nullptr;
    KToggleAction * m_konquerorIntegration = nullptr;

    // for autopaste function
    QString lastClipboard;
    // timer for checking clipboard - autopaste function
    QTimer *clipboardTimer = nullptr;

    bool m_startWithoutAnimation;
    bool m_doTesting;               // UnitTest flag

    //HttpServer *m_webinterface;

#ifdef HAVE_QCA2
    QCA::Initializer m_qcaInit;
#endif //HAVE_QCA2
};

#endif
