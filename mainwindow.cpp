/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2006, 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QClipboard>
#include <QTimer>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <knotifyconfigwidget.h>
#include <kfiledialog.h>
#include <ktoolinvocation.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <kstandardaction.h>
#include <klocale.h>
#include <kicon.h>
#include <kactionmenu.h>

#include "mainwindow.h"
#include "core/kget.h"
#include "core/transferhandler.h"
#include "settings.h"
#include "conf/preferencesdialog.h"

#include "ui/viewscontainer.h"
#include "ui/tray.h"
#include "ui/droptarget.h"
#include "ui/groupseditdialog.h"

MainWindow::MainWindow( QWidget * parent )
    : KMainWindow( parent ),
      m_drop(0), m_dock(0)
{
    // create the model
    KGet::self( this );

    // create actions
    setupActions();

    createGUI("kgetui.rc");

    m_viewsContainer = new ViewsContainer(this);

    setCentralWidget(m_viewsContainer);

    // restore position, size and visibility
    move( Settings::mainPosition() );
    setAutoSaveSettings();
    setPlainCaption(i18n("KGet"));

    if ( Settings::showMain() )
        show();
    else
        hide();

    //Some of the widgets are initialized in slotDelayedInit()
    QTimer::singleShot( 0, this, SLOT(slotDelayedInit()) );
}

MainWindow::~MainWindow()
{
    //Save the user's transfers
    KGet::save();

    slotSaveMyself();
    delete clipboardTimer;
    delete m_drop;
    delete m_dock;
    // reset konqueror integration (necessary if user enabled / disabled temporarily integration from tray)
    slotKonquerorIntegration( Settings::konquerorIntegration() );
    // the following call saves options set in above dtors
    Settings::writeConfig();
}


void MainWindow::setupActions()
{
    QAction *newDownloadAction = actionCollection()->addAction("new_download");
    newDownloadAction->setText(i18n("&New Download..."));
    newDownloadAction->setIcon(KIcon("filenew"));
    newDownloadAction->setShortcuts(KShortcut("Ctrl+N"));
    connect(newDownloadAction, SIGNAL(triggered()), SLOT(slotNewTransfer()));

    QAction *openAction = actionCollection()->addAction("open");
    openAction->setText(i18n("&Open..."));
    openAction->setIcon(KIcon("fileopen"));
    openAction->setShortcuts(KShortcut("Ctrl+O"));
    connect(openAction, SIGNAL(triggered()), SLOT(slotOpen()));

    QAction *exportAction = actionCollection()->addAction("export_downloads");
    exportAction->setText(i18n("&Export Transfers List..."));
    exportAction->setShortcuts(KShortcut("Ctrl+E"));
    connect(exportAction, SIGNAL(triggered()), SLOT(slotExportTransfers()));

    QAction *editGroupAction = actionCollection()->addAction("edit_groups");
    editGroupAction->setText(i18n("Edit Groups.."));
    editGroupAction->setIcon(KIcon("transfers_list"));
    connect(editGroupAction, SIGNAL(triggered()), SLOT(slotEditGroups()));

    m_autoPasteAction = new KToggleAction(KIcon("tool_clipboard"),
                                          i18n("Auto-Paste Mode"), actionCollection());
    actionCollection()->addAction("auto_paste", m_autoPasteAction);
    m_autoPasteAction->setChecked(Settings::autoPaste());
    m_autoPasteAction->setWhatsThis(i18n("<b>Auto paste</b> button toggles the auto-paste mode "
                                         "on and off.\nWhen set, KGet will periodically scan "
                                         "the clipboard for URLs and paste them automatically."));
    connect(m_autoPasteAction, SIGNAL(triggered()), SLOT(slotToggleAutoPaste()));

    KToggleAction *m_konquerorIntegration = new KToggleAction(KIcon("konqueror"),
                                                              i18n("Use KGet as Konqueror Download Manager"),
                                                              actionCollection());
    actionCollection()->addAction("konqueror_integration", m_konquerorIntegration);
    connect(m_konquerorIntegration, SIGNAL(triggered()), SLOT(slotTrayKonquerorIntegration()));
    m_konquerorIntegration->setChecked(Settings::konquerorIntegration());

    // local - Destroys all sub-windows and exits
    QAction *quitAction = KStandardAction::quit(this, SLOT(slotQuit()), actionCollection());
    actionCollection()->addAction("quit", quitAction);
    // local - Standard configure actions
    QAction *preferencesAction = KStandardAction::preferences(this, SLOT(slotPreferences()), actionCollection());
    actionCollection()->addAction("preferences", preferencesAction);
    QAction *configToolbarAction = KStandardAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());
    actionCollection()->addAction("configure_toolbars", configToolbarAction);
    QAction *keyBindingsAction = KStandardAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
    actionCollection()->addAction("configure_keys", keyBindingsAction);
    QAction *cinfigNotifyAction = KStandardAction::configureNotifications(this, SLOT(slotConfigureNotifications()), actionCollection());
    actionCollection()->addAction("configure_notifications", cinfigNotifyAction);
    m_menubarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenubar()), actionCollection());
    m_menubarAction->setChecked(!menuBar()->isHidden());
    actionCollection()->addAction("settings_showmenubar", m_menubarAction);

    // Transfer related actions
    QAction *deleteSelectedAction = actionCollection()->addAction("delete_selected_download");
    deleteSelectedAction->setText(i18n("Delete Selected"));
    deleteSelectedAction->setIcon(KIcon("editdelete"));
    deleteSelectedAction->setShortcuts(KShortcut("Del"));
    connect(deleteSelectedAction, SIGNAL(triggered()), SLOT(slotDeleteSelected()));

    QAction *startAllAction = actionCollection()->addAction("start_all_download");
    startAllAction->setText(i18n("Start All"));
    startAllAction->setIcon(KIcon("player_fwd"));
    connect(startAllAction, SIGNAL(triggered()), SLOT(slotStartAllDownload()));

    QAction *startSelectedAction = actionCollection()->addAction("start_selected_download");
    startSelectedAction->setText(i18n("Start Selected"));
    startSelectedAction->setIcon(KIcon("player_play"));
    connect(startSelectedAction, SIGNAL(triggered()), SLOT(slotStartSelectedDownload()));

    QAction *stopAllAction = actionCollection()->addAction("stop_all_download");
    stopAllAction->setText(i18n("Stop All"));
    stopAllAction->setIcon(KIcon("player_pause"));
    connect(stopAllAction, SIGNAL(triggered()), SLOT(slotStopAllDownload()));

    QAction *stopSelectedAction = actionCollection()->addAction("stop_selected_download");
    stopSelectedAction->setText(i18n("Stop Selected"));
    stopSelectedAction->setIcon(KIcon("player_pause"));
    connect(stopSelectedAction, SIGNAL(triggered()), SLOT(slotStopSelectedDownload()));

    KActionMenu *startActionMenu = new KActionMenu(KIcon("player_play"), i18n("Start"),
                                                     actionCollection());
    actionCollection()->addAction("start_menu", startActionMenu);
    startActionMenu->setDelayed(true);
    startActionMenu->addAction(startAllAction);
    startActionMenu->addAction(startSelectedAction);
    connect(startActionMenu, SIGNAL(triggered()), SLOT(slotStartDownload()));

    KActionMenu *stopActionMenu = new KActionMenu(KIcon("player_pause"), i18n("Stop"),
                                                    actionCollection());
    actionCollection()->addAction("stop_menu", stopActionMenu);
    stopActionMenu->setDelayed(true);
    stopActionMenu->addAction(stopAllAction);
    stopActionMenu->addAction(stopSelectedAction);
    connect(stopActionMenu, SIGNAL(triggered()), SLOT(slotStopDownload()));

    QAction *openDestAction = actionCollection()->addAction("transfer_open_dest");
    openDestAction->setText(i18n("Open Destination"));
    openDestAction->setIcon(KIcon("folder"));
    connect(openDestAction, SIGNAL(triggered()), SLOT(slotTransfersOpenDest()));

    QAction *showDetailsAction = actionCollection()->addAction("transfer_show_details");
    showDetailsAction->setText(i18n("Show Details"));
    showDetailsAction->setIcon(KIcon("configure"));
    connect(showDetailsAction, SIGNAL(triggered()), SLOT(slotTransfersShowDetails()));

    QAction *copyUrlAction = actionCollection()->addAction("transfer_copy_source_url");
    copyUrlAction->setText(i18n("Copy URL to Clipboard"));
    copyUrlAction->setIcon(KIcon("tool_clipboard"));
    connect(copyUrlAction, SIGNAL(triggered()), SLOT(slotTransfersCopySourceUrl()));
}

void MainWindow::slotDelayedInit()
{
    //Here we import the user's transfers.
    KGet::load( KStandardDirs::locateLocal("appdata", "transfers.kgt") );

    // DropTarget
    m_drop = new DropTarget(this);

    if ( Settings::showDropTarget() || Settings::firstRun() )
        m_drop->setVisible(true);

    // DockWidget
    m_dock = new Tray(this);
    m_dock->show();

    // enable dropping
    setAcceptDrops(true);

    // enable hide toolbar
    setStandardToolBarMenuEnabled(true);

    // session management stuff
    connect(kapp, SIGNAL(saveYourself()), SLOT(slotSaveMyself()));

    // set auto-resume in kioslaverc (is there a cleaner way?)
    KConfig cfg( "kioslaverc", false, false);
    cfg.setGroup(QString());
    cfg.writeEntry("AutoResume", true);
    cfg.sync();

    // immediately start downloading if configured this way
    if ( Settings::downloadAtStartup() )
        slotStartDownload();

    // reset the FirstRun config option
    Settings::setFirstRun(false);

    //auto paste stuff
    lastClipboard = QApplication::clipboard()->text( QClipboard::Clipboard ).trimmed();
    clipboardTimer = new QTimer(this);
    connect(clipboardTimer, SIGNAL(timeout()), SLOT(slotCheckClipboard()));
    if ( Settings::autoPaste() )
        clipboardTimer->start(1000);
}

void MainWindow::slotNewTransfer()
{
    KGet::addTransfer(KUrl());
}

void MainWindow::slotOpen()
{
    QString filename = KFileDialog::getOpenFileName
        (KUrl(),
         "*.kgt *.torrent *.metalink|" + i18n("All openable files") + " (*.kgt *.torrent *.metalink)",
         this,
         i18n("Open file")
        );

    if(filename.endsWith(".kgt"))
    {
        KGet::load(filename);
        return;
    }

    if(!filename.isEmpty())
        KGet::addTransfer( KUrl( filename ) );
}

void MainWindow::slotQuit()
{
//     if (m_scheduler->countRunningJobs() > 0)
//     {
//         if (KMessageBox::warningYesNo(this,
//                 i18n("Some transfers are still running.\n"
//                      "Are you sure you want to close KGet?"),
//                 i18n("Warning"), KStdGuiItem::yes(), KStdGuiItem::no(),
//                 "ExitWithActiveTransfers") == KMessageBox::No)
//             return;
//     }

    Settings::writeConfig();
    kapp->quit();
}

void MainWindow::slotPreferences()
{
    // an instance the dialog could be already created and could be cached,
    // in which case you want to display the cached dialog
    if ( PreferencesDialog::showDialog( "preferences" ) )
        return;

    // we didn't find an instance of this dialog, so lets create it
    PreferencesDialog * dialog = new PreferencesDialog( this, Settings::self() );
    KGet::setPluginsSettingsWidget( dialog->pluginsWidget() );
    // keep us informed when the user changes settings
    connect( dialog, SIGNAL(settingsChanged(const QString&)),
             this, SLOT(slotNewConfig()) );

    dialog->show();
}

void MainWindow::slotExportTransfers()
{
    QString filename = KFileDialog::getSaveFileName
        (KUrl(),
         "*.kgt|" + i18n("KGet transfer list") + " (*.kgt)",
         this,
         i18n("Export transfers")
        );

    if(!filename.isEmpty())
        KGet::save(filename);
}

void MainWindow::slotEditGroups()
{
    GroupsEditDialog dialog(this);

    dialog.exec();
}

void MainWindow::slotStartDownload()
{
    if(KGet::selectedTransfers().size() == 0)
        slotStartAllDownload();
    else
        slotStartSelectedDownload();
}

void MainWindow::slotStartAllDownload()
{
    m_dock->setDownloading(true);

    KGet::setSchedulerRunning(true);
}

void MainWindow::slotStartSelectedDownload()
{
    m_dock->setDownloading(true);

    foreach(TransferHandler * it, KGet::selectedTransfers())
        it->start();
}

void MainWindow::slotStopDownload()
{
    if(KGet::selectedTransfers().size() == 0)
        slotStopAllDownload();
    else
        slotStopSelectedDownload();
}

void MainWindow::slotStopAllDownload()
{
    m_dock->setDownloading(false);

    KGet::setSchedulerRunning(false);
}

void MainWindow::slotStopSelectedDownload()
{
    m_dock->setDownloading(false);

    foreach(TransferHandler * it, KGet::selectedTransfers())
        it->stop();
}

void MainWindow::slotDeleteSelected()
{
    foreach(TransferHandler * it, KGet::selectedTransfers())
    {
        it->stop();
        KGet::delTransfer(it);
    }
}

void MainWindow::slotTransfersOpenDest()
{
    QStringList openedDirs;
    foreach(TransferHandler * it, KGet::selectedTransfers())
    {
        QString directory = it->dest().directory();
        if( !openedDirs.contains( directory ) )
        {
            KToolInvocation::invokeBrowser( directory );
            openedDirs.append( directory );
        }
    }
}

void MainWindow::slotTransfersShowDetails()
{
    foreach(TransferHandler * it, KGet::selectedTransfers())
    {
        m_viewsContainer->showTransferDetails(it);
    }
}

void MainWindow::slotTransfersCopySourceUrl()
{
    foreach(TransferHandler * it, KGet::selectedTransfers())
    {
        QString sourceurl = it->source().url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(sourceurl, QClipboard::Selection);
        cb->setText(sourceurl, QClipboard::Clipboard);
    }
}

void MainWindow::slotConfigureNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void MainWindow::slotConfigureKeys()
{
    KKeyDialog::configure(actionCollection());
}

void MainWindow::slotConfigureToolbars()
{
    KEditToolbar edit( "kget_toolbar", actionCollection() );
    connect(&edit, SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
    edit.exec();
}


void MainWindow::slotSaveMyself()
{
    // save last parameters ..
    Settings::setMainPosition( pos() );
    // .. and write config to disk
    Settings::writeConfig();
}

void MainWindow::slotNewToolbarConfig()
{
    createGUI();
}

void MainWindow::slotNewConfig()
{
    // Update here properties modified in the config dialog and not
    // parsed often by the code. When clicking Ok or Apply of
    // PreferencesDialog, this function is called.

    m_drop->setVisible(Settings::showDropTarget(), false);

    slotKonquerorIntegration(Settings::konquerorIntegration());

    if (Settings::autoPaste())
        clipboardTimer->start(1000);
    else
        clipboardTimer->stop();
    m_autoPasteAction->setChecked(Settings::autoPaste());
}

void MainWindow::slotToggleAutoPaste()
{
    bool autoPaste = !Settings::autoPaste();
    Settings::setAutoPaste( autoPaste );

    if (autoPaste)
        clipboardTimer->start(1000);
    else
        clipboardTimer->stop();
    m_autoPasteAction->setChecked(autoPaste);
}

void MainWindow::slotCheckClipboard()
{
    QString clipData = QApplication::clipboard()->text( QClipboard::Clipboard ).trimmed();

    if (clipData != lastClipboard)
    {
        lastClipboard = clipData;
        if (lastClipboard.isEmpty())
            return;

        KUrl url = KUrl(lastClipboard);

        if (url.isValid() && !url.isLocalFile())
            KGet::addTransfer( url );
    }
}

void MainWindow::slotTrayKonquerorIntegration()
{
    static bool tempIntegration = Settings::konquerorIntegration();
    tempIntegration = !tempIntegration;
    slotKonquerorIntegration(tempIntegration);
    if (!tempIntegration && Settings::konquerorIntegration() && !Settings::expertMode())
    {
        KMessageBox::information(this,
            i18n("KGet has been temporarily disabled as download manager for Konqueror. "
            "If you want to disable it forever, go to Settings->Advanced and disable \"Use "
            "as download manager for Konqueror\"."),
            i18n("Konqueror Integration disabled"),
            "KonquerorIntegrationDisabled");
    }
}

void MainWindow::slotKonquerorIntegration(bool konquerorIntegration)
{
    KConfig cfgKonqueror("konquerorrc", false, false);
    cfgKonqueror.setGroup("HTML Settings");
    cfgKonqueror.writePathEntry("DownloadManager", QString(konquerorIntegration?"kget":QString()));
    cfgKonqueror.sync();
}

void MainWindow::slotShowMenubar()
{
    if(m_menubarAction->isChecked())
        menuBar()->show();
    else
        menuBar()->hide();
}

/** widget events */

void MainWindow::closeEvent( QCloseEvent * e )
{
    e->ignore();
    hide();
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    event->setAccepted(KUrl::List::canDecode(event->mimeData())
                  || event->mimeData()->hasText());
}

void MainWindow::dropEvent(QDropEvent * event)
{
    KUrl::List list = KUrl::List::fromMimeData(event->mimeData());
    QString str;

    if (!list.isEmpty())
        KGet::addTransfer(list);
    else
    {
        str = event->mimeData()->text();
        KGet::addTransfer(KUrl(str));
    }
}


/** DBUS interface */

void MainWindow::addTransfers( const KUrl::List& src, const QString& dest)
{
    //TODO Implement it in the dbus interface
    KGet::addTransfer( src, dest );
}

void MainWindow::addTransfer( const KUrl& src, const QString& dest)
{
    KGet::addTransfer( src, dest );
}

bool MainWindow::isDropTargetVisible() const
{
    return m_drop->isVisible();
}

void MainWindow::setDropTargetVisible( bool setVisible )
{
    if ( setVisible != Settings::showDropTarget() )
        m_drop->setVisible( setVisible );
}

void MainWindow::setOfflineMode( bool offline )
{
    Q_UNUSED(offline);
    //TODO Re-enable this
//     schedRequestOperation( offline ? OpStop : OpRun );
}

bool MainWindow::isOfflineMode() const
{
    //TODO Re-enable this
//     return scheduler->isRunning();
    return false;
}

#include "mainwindow.moc"
