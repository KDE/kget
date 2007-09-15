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

#include "mainwindow.h"

#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "dbus/dbusmodelobserver.h"
#include "settings.h"
#include "conf/preferencesdialog.h"
#include "ui/viewscontainer.h"
#include "ui/tray.h"
#include "ui/droptarget.h"
#include "ui/newtransferdialog.h"

#include <kapplication.h>
#include <kstandarddirs.h>
#include <KInputDialog>
#include <kmessagebox.h>
#include <kshortcutsdialog.h>
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

#include <QtDBus>
#include <QClipboard>
#include <QTimer>

MainWindow::MainWindow(bool showMainwindow, bool startWithoutAnimation, QWidget *parent)
    : KXmlGuiWindow( parent ),
      m_drop(0), m_dock(0), m_startWithoutAnimation(startWithoutAnimation)
{
    resize(720, 380); // have a reasonable initial size, will be overwritten later if the user changes it

    // create the model
    KGet::self( this );

    // create actions
    setupActions();

    createGUI("kgetui.rc");

    m_viewsContainer = new ViewsContainer(this);
    // initialize the model observer to export percents over dbus
    m_dbusModelObserver = new DBusModelObserver();

    setCentralWidget(m_viewsContainer);

    // restore position, size and visibility
    move( Settings::mainPosition() );
    setAutoSaveSettings();
    setPlainCaption(i18n("KGet"));

    if ( Settings::showMain() && showMainwindow)
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
    Settings::self()->writeConfig();
}


void MainWindow::setupActions()
{
    QAction *newDownloadAction = actionCollection()->addAction("new_download");
    newDownloadAction->setText(i18n("&New Download..."));
    newDownloadAction->setIcon(KIcon("document-new"));
    newDownloadAction->setShortcuts(KShortcut("Ctrl+N"));
    connect(newDownloadAction, SIGNAL(triggered()), SLOT(slotNewTransfer()));

    QAction *openAction = actionCollection()->addAction("import_transfers");
    openAction->setText(i18n("&Import Transfers..."));
    openAction->setIcon(KIcon("document-open"));
    openAction->setShortcuts(KShortcut("Ctrl+I"));
    connect(openAction, SIGNAL(triggered()), SLOT(slotImportTransfers()));

    QAction *exportAction = actionCollection()->addAction("export_transfers");
    exportAction->setText(i18n("&Export Transfers List..."));
    exportAction->setIcon(KIcon("file-export"));
    exportAction->setShortcuts(KShortcut("Ctrl+E"));
    connect(exportAction, SIGNAL(triggered()), SLOT(slotExportTransfers()));

    QAction *deleteGroupAction = actionCollection()->addAction("delete_groups");
    deleteGroupAction->setText(i18n("Delete Group"));
    deleteGroupAction->setIcon(KIcon("edit-delete"));
    connect(deleteGroupAction, SIGNAL(triggered()), SLOT(slotDeleteGroup()));

    QAction *renameGroupAction = actionCollection()->addAction("rename_groups");
    renameGroupAction->setText(i18n("Rename Group"));
    renameGroupAction->setIcon(KIcon("editinput"));
    connect(renameGroupAction, SIGNAL(triggered()), SLOT(slotRenameGroup()));

    m_autoPasteAction = new KToggleAction(KIcon("klipper"),
                                          i18n("Auto-Paste Mode"), actionCollection());
    actionCollection()->addAction("auto_paste", m_autoPasteAction);
    m_autoPasteAction->setChecked(Settings::autoPaste());
    m_autoPasteAction->setWhatsThis(i18n("<b>Auto paste</b> button toggles the auto-paste mode "
                                         "on and off.\nWhen set, KGet will periodically scan "
                                         "the clipboard for URLs and paste them automatically."));
    connect(m_autoPasteAction, SIGNAL(triggered()), SLOT(slotToggleAutoPaste()));

    m_konquerorIntegration = new KToggleAction(KIcon("konqueror"),
                                               i18n("Use KGet as Konqueror Download Manager"), actionCollection());
    actionCollection()->addAction("konqueror_integration", m_konquerorIntegration);
    connect(m_konquerorIntegration, SIGNAL(triggered(bool)), SLOT(slotTrayKonquerorIntegration(bool)));
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
    deleteSelectedAction->setIcon(KIcon("edit-delete"));
    deleteSelectedAction->setShortcuts(KShortcut("Del"));
    connect(deleteSelectedAction, SIGNAL(triggered()), SLOT(slotDeleteSelected()));

    QAction *startAllAction = actionCollection()->addAction("start_all_download");
    startAllAction->setText(i18n("Start / Resume All"));
    startAllAction->setIcon(KIcon("media-seek-forward"));
    startAllAction->setShortcuts(KShortcut("Ctrl+R"));
    connect(startAllAction, SIGNAL(triggered()), SLOT(slotStartAllDownload()));

    QAction *startSelectedAction = actionCollection()->addAction("start_selected_download");
    startSelectedAction->setText(i18n("Start / Resume Selected"));
    startSelectedAction->setIcon(KIcon("media-playback-start"));
    connect(startSelectedAction, SIGNAL(triggered()), SLOT(slotStartSelectedDownload()));

    QAction *stopAllAction = actionCollection()->addAction("stop_all_download");
    stopAllAction->setText(i18n("Stop All"));
    stopAllAction->setIcon(KIcon("media-playback-pause"));
    stopAllAction->setShortcuts(KShortcut("Ctrl+P"));
    connect(stopAllAction, SIGNAL(triggered()), SLOT(slotStopAllDownload()));

    QAction *stopSelectedAction = actionCollection()->addAction("stop_selected_download");
    stopSelectedAction->setText(i18n("Stop Selected"));
    stopSelectedAction->setIcon(KIcon("media-playback-pause"));
    connect(stopSelectedAction, SIGNAL(triggered()), SLOT(slotStopSelectedDownload()));

    KActionMenu *startActionMenu = new KActionMenu(KIcon("media-playback-start"), i18n("Start / Resume"),
                                                     actionCollection());
    actionCollection()->addAction("start_menu", startActionMenu);
    startActionMenu->setDelayed(true);
    startActionMenu->addAction(startAllAction);
    startActionMenu->addAction(startSelectedAction);
    connect(startActionMenu, SIGNAL(triggered()), SLOT(slotStartDownload()));

    KActionMenu *stopActionMenu = new KActionMenu(KIcon("media-playback-pause"), i18n("Stop"),
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
    copyUrlAction->setIcon(KIcon("klipper"));
    connect(copyUrlAction, SIGNAL(triggered()), SLOT(slotTransfersCopySourceUrl()));

    KToggleAction *showDropTargetAction = new KToggleAction(KIcon("kget"),
                                          i18n("Show Drop Target"), actionCollection());
    actionCollection()->addAction("show_drop_target", showDropTargetAction);
    showDropTargetAction->setChecked(Settings::showDropTarget());
    connect(showDropTargetAction, SIGNAL(triggered()), SLOT(slotToggleDropTarget()));
}

void MainWindow::slotDelayedInit()
{
    //Here we import the user's transfers.
    KGet::load( KStandardDirs::locateLocal("appdata", "transfers.kgt") );

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
    KConfig cfg("kioslaverc", KConfig::NoGlobals);
    cfg.group(QString()).writeEntry("AutoResume", true);
    cfg.sync();

    // immediately start downloading if configured this way
    if ( Settings::downloadAtStartup() )
        slotStartDownload();

    // DropTarget
    m_drop = new DropTarget(this);

    if (Settings::firstRun()) {
        if (KMessageBox::questionYesNoCancel(this ,i18n("This is the first time you have run KGet.\n"
                                             "Would you like to enable KGet as the download manager for Konqueror?"),
                                             i18n("Konqueror Integration"), KGuiItem(i18n("Enable"), KIcon("dialog-apply")),
                                             KGuiItem(i18n("Do Not Enable"), KIcon("edit-delete")))
                                             == KMessageBox::Yes) {
            Settings::setKonquerorIntegration(true);
            slotKonquerorIntegration(true);
        }

        m_drop->setVisible(true);

        // reset the FirstRun config option
        Settings::setFirstRun(false);
    }

    if (Settings::showDropTarget() && !m_startWithoutAnimation)
        m_drop->setVisible(true);

    //auto paste stuff
    lastClipboard = QApplication::clipboard()->text( QClipboard::Clipboard ).trimmed();
    clipboardTimer = new QTimer(this);
    connect(clipboardTimer, SIGNAL(timeout()), SLOT(slotCheckClipboard()));
    if ( Settings::autoPaste() )
        clipboardTimer->start(1000);
}

void MainWindow::slotToggleDropTarget()
{
    actionCollection()->action("show_drop_target")->setChecked(!m_drop->isVisible());

    m_drop->setVisible(!m_drop->isVisible());
}

void MainWindow::slotNewTransfer()
{
    NewTransferDialog::showNewTransferDialog();
}

void MainWindow::slotImportTransfers()
{
    QString filename = KFileDialog::getOpenFileName(KUrl(),
                                                    "*.kgt *.metalink|" + i18n("All Openable Files") +
                                                    " (*.kgt *.metalink)", this, i18n("Open File"));

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
    if (KGet::schedulerRunning()) {
        if (KMessageBox::warningYesNoCancel(this,
                i18n("Some transfers are still running.\n"
                     "Are you sure you want to close KGet?"),
                i18n("Confirm Quit"),
                KStandardGuiItem::yes(), KStandardGuiItem::no(), KStandardGuiItem::cancel(),
                "ExitWithActiveTransfers") != KMessageBox::Yes)
            return;

        KGet::setSchedulerRunning(false);
    }

    Settings::self()->writeConfig();
    qApp->quit();
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
         "*.kgt|" + i18n("KGet Transfer List") + " (*.kgt)",
         this,
         i18n("Export Transfers")
        );

    if(!filename.isEmpty())
        KGet::save(filename);
}

void MainWindow::slotDeleteGroup()
{
    foreach(TransferGroupHandler * it, KGet::selectedTransferGroups())
    {
        it->stop();
        KGet::delGroup(it->name());
    }
}

void MainWindow::slotRenameGroup()
{
    bool ok = true;
    QString groupName;

    foreach(TransferGroupHandler * it, KGet::selectedTransferGroups())
    {
        groupName = KInputDialog::getText(i18n("Enter Group Name"),
                                          i18n("Group name:"), it->name(), &ok, this);
        if(ok)
            it->setName(groupName);
    }
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
    KShortcutsDialog::configure(actionCollection());
}

void MainWindow::slotConfigureToolbars()
{
    KEditToolBar edit( actionCollection() );
    connect(&edit, SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
    edit.exec();
}


void MainWindow::slotSaveMyself()
{
    // save last parameters ..
    Settings::setMainPosition( pos() );
    // .. and write config to disk
    Settings::self()->writeConfig();
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
    m_konquerorIntegration->setChecked(Settings::konquerorIntegration());

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

void MainWindow::slotTrayKonquerorIntegration(bool enable)
{
    slotKonquerorIntegration(enable);
    if (!enable && Settings::konquerorIntegration() && !Settings::expertMode())
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
    KConfig cfgKonqueror("konquerorrc", KConfig::NoGlobals);
    cfgKonqueror.group("HTML Settings").writeEntry("DownloadManager",
                                                   QString(konquerorIntegration ? "kget" : QString()));
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

void MainWindow::addTransfers(const QString& src, const QString& dest, bool start)
{
    // split src for the case it is a QStringList (e.g. from konqueror plugin)
    KGet::addTransfer(src.split(";"), dest, QString(), start);
}

bool MainWindow::dropTargetVisible() const
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
    KGet::setSchedulerRunning(offline);
}

bool MainWindow::offlineMode() const
{
    return !KGet::schedulerRunning();
}

QVariantMap MainWindow::transfers() const
{
    return m_dbusModelObserver->transfers();
}

int MainWindow::transfersSpeed() const
{
    return m_dbusModelObserver->transfersSpeed();
}

#include "mainwindow.moc"
