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

#include <ktoggleaction.h>
#include <kxmlguiwindow.h>
#include <kurl.h>

#include <kstatusnotifieritem.h>
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
    explicit MainWindow(bool showMainwindow = true, bool startWithoutAnimation = false, bool doTesting = false, QWidget *parent = 0);
    ~MainWindow();

    virtual void setSystemTrayDownloading(bool running);

    //no slot, to make sure that MainWindow is correctly initialized before any transfers get added
    void init();

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
    
    KGet * m_kget;

    // internal widgets
    ViewsContainer * m_viewsContainer;

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
    bool m_doTesting;               // UnitTest flag

    HttpServer *m_webinterface;

#ifdef HAVE_QCA2
    QCA::Initializer m_qcaInit;
#endif //HAVE_QCA2
};

#endif
