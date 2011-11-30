/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2006 - 2008 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 - 2011 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 - 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "mainwindow.h"

#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreemodel.h"
#include "core/transfertreeselectionmodel.h"
#include "settings.h"
#include "conf/autopastemodel.h"
#include "conf/preferencesdialog.h"
#include "ui/viewscontainer.h"
#include "ui/tray.h"
#include "ui/droptarget.h"
#include "ui/newtransferdialog.h"
#include "ui/history/transferhistory.h"
#include "ui/groupsettingsdialog.h"
#include "ui/transfersettingsdialog.h"
#include "ui/linkview/kget_linkview.h"
#include "ui/metalinkcreator/metalinkcreator.h"
#include "extensions/webinterface/httpserver.h"
#ifdef DO_KGET_TEST
    #include "tests/testkget.h"
#endif

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
#include <KSelectAction>
#include <krun.h>
#include <kicondialog.h>
#include "core/verifier.h"
#include <QClipboard>
#include <QTimer>
#include <QtTest/QtTest>

MainWindow::MainWindow(bool showMainwindow, bool startWithoutAnimation, bool doTesting, QWidget *parent)
    : KXmlGuiWindow( parent ),
      m_drop(0),
      m_dock(0),
      m_startWithoutAnimation(startWithoutAnimation),
      m_doTesting(doTesting),
      m_webinterface(0)
{
    // do not quit the app when it has been minimized to system tray and a new transfer dialog
    // gets opened and closed again.
    qApp->setQuitOnLastWindowClosed(false);
    setAttribute(Qt::WA_DeleteOnClose, false);

    // create the model
    m_kget = KGet::self( this );

    m_viewsContainer = new ViewsContainer(this);

    // create actions
    setupActions();

    createGUI("kgetui.rc");

    setCentralWidget(m_viewsContainer);

    // restore position, size and visibility
    move( Settings::mainPosition() );
    setAutoSaveSettings();
    setPlainCaption(i18n("KGet"));

    if ( Settings::showMain() && showMainwindow)
        show();
    else
        hide();
}

MainWindow::~MainWindow()
{
    //Save the user's transfers
    KGet::save();

    slotSaveMyself();
    // reset konqueror integration (necessary if user enabled / disabled temporarily integration from tray)
    slotKonquerorIntegration( Settings::konquerorIntegration() );
    // the following call saves options set in above dtors
    Settings::self()->writeConfig();

    delete m_drop;
    delete m_kget;
}

QSize MainWindow::sizeHint() const
{
    return QSize(738, 380);
}

int MainWindow::transfersPercent()
{
    int percent = 0;
    int activeTransfers = 0;
    foreach (const TransferHandler *handler, KGet::allTransfers()) {
        if (handler->status() == Job::Running) {
            activeTransfers ++;
            percent += handler->percent();
        }
    }

    if (activeTransfers > 0) {
        return percent/activeTransfers;
    }
    else {
        return -1;
    }
}

void MainWindow::setupActions()
{
    KAction *newDownloadAction = actionCollection()->addAction("new_download");
    newDownloadAction->setText(i18n("&New Download..."));
    newDownloadAction->setIcon(KIcon("document-new"));
    newDownloadAction->setShortcuts(KShortcut("Ctrl+N"));
    newDownloadAction->setHelpText(i18n("Opens a dialog to add a transfer to the list"));
    connect(newDownloadAction, SIGNAL(triggered()), SLOT(slotNewTransfer()));

    KAction *openAction = actionCollection()->addAction("import_transfers");
    openAction->setText(i18n("&Import Transfers..."));
    openAction->setIcon(KIcon("document-open"));
    openAction->setShortcuts(KShortcut("Ctrl+I"));
    openAction->setHelpText(i18n("Imports a list of transfers"));
    connect(openAction, SIGNAL(triggered()), SLOT(slotImportTransfers()));

    KAction *exportAction = actionCollection()->addAction("export_transfers");
    exportAction->setText(i18n("&Export Transfers List..."));
    exportAction->setIcon(KIcon("document-export"));
    exportAction->setShortcuts(KShortcut("Ctrl+E"));
    exportAction->setHelpText(i18n("Exports the current transfers into a file"));
    connect(exportAction, SIGNAL(triggered()), SLOT(slotExportTransfers()));

    KAction *createMetalinkAction = actionCollection()->addAction("create_metalink");
    createMetalinkAction->setText(i18n("&Create a Metalink..."));
    createMetalinkAction->setIcon(KIcon("journal-new"));
    createMetalinkAction->setHelpText(i18n("Creates or modifies a metalink and saves it on disk"));
    connect(createMetalinkAction, SIGNAL(triggered()), SLOT(slotCreateMetalink()));

    KAction *priorityTop = actionCollection()->addAction("priority_top");
    priorityTop->setText(i18n("Top Priority"));
    priorityTop->setIcon(KIcon("arrow-up-double"));
    priorityTop->setShortcuts(KShortcut("Ctrl+PgUp"));
    priorityTop->setHelpText(i18n("Download selected transfer first"));
    connect(priorityTop, SIGNAL(triggered()), this, SLOT(slotPriorityTop()));

    KAction *priorityBottom = actionCollection()->addAction("priority_bottom");
    priorityBottom->setText(i18n("Least Priority"));
    priorityBottom->setIcon(KIcon("arrow-down-double"));
    priorityBottom->setShortcuts(KShortcut("Ctrl+PgDown"));
    priorityBottom->setHelpText(i18n("Download selected transfer last"));
    connect(priorityBottom, SIGNAL(triggered()), this, SLOT(slotPriorityBottom()));

    KAction *priorityUp = actionCollection()->addAction("priority_up");
    priorityUp->setText(i18n("Increase Priority"));
    priorityUp->setIcon(KIcon("arrow-up"));
    priorityUp->setShortcuts(KShortcut("Ctrl+Up"));
    priorityUp->setHelpText(i18n("Increase priority for selected transfer"));
    connect(priorityUp, SIGNAL(triggered()), this, SLOT(slotPriorityUp()));

    KAction *priorityDown = actionCollection()->addAction("priority_down");
    priorityDown->setText(i18n("Decrease Priority"));
    priorityDown->setIcon(KIcon("arrow-down"));
    priorityDown->setShortcuts(KShortcut("Ctrl+Down"));
    priorityDown->setHelpText(i18n("Decrease priority for selected transfer"));
    connect(priorityDown, SIGNAL(triggered()), this, SLOT(slotPriorityDown()));

    //FIXME: Not needed maybe because the normal delete already deletes groups?
    KAction *deleteGroupAction = actionCollection()->addAction("delete_groups");
    deleteGroupAction->setText(i18n("Delete Group"));
    deleteGroupAction->setIcon(KIcon("edit-delete"));
    deleteGroupAction->setHelpText(i18n("Delete selected group"));
    connect(deleteGroupAction, SIGNAL(triggered()), SLOT(slotDeleteGroup()));

    KAction *renameGroupAction = actionCollection()->addAction("rename_groups");
    renameGroupAction->setText(i18n("Rename Group..."));
    renameGroupAction->setIcon(KIcon("edit-rename"));
    connect(renameGroupAction, SIGNAL(triggered()), SLOT(slotRenameGroup()));

    KAction *setIconGroupAction = actionCollection()->addAction("seticon_groups");
    setIconGroupAction->setText(i18n("Set Icon..."));
    setIconGroupAction->setIcon(KIcon("preferences-desktop-icons"));
    setIconGroupAction->setHelpText(i18n("Select a custom icon for the selected group"));
    connect(setIconGroupAction, SIGNAL(triggered()), SLOT(slotSetIconGroup()));

    m_autoPasteAction = new KToggleAction(KIcon("edit-paste"),
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
    KStandardAction::quit(this, SLOT(slotQuit()), actionCollection());
    // local - Standard configure actions
    KStandardAction::preferences(this, SLOT(slotPreferences()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());
    KStandardAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());

    KStandardAction::configureNotifications(this, SLOT(slotConfigureNotifications()), actionCollection());
    m_menubarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenubar()), actionCollection());
    m_menubarAction->setChecked(!menuBar()->isHidden());

    // Transfer related actions
    actionCollection()->addAction(KStandardAction::SelectAll, "select_all", m_viewsContainer, SLOT(selectAll()));

    KAction *deleteSelectedAction = actionCollection()->addAction("delete_selected_download");
    deleteSelectedAction->setText(i18nc("delete selected transfer item", "Remove Selected"));
    deleteSelectedAction->setIcon(KIcon("edit-delete"));
    deleteSelectedAction->setShortcuts(KShortcut("Del"));
    deleteSelectedAction->setHelpText(i18n("Removes selected transfer and deletes files from disk if it's not finished"));
    connect(deleteSelectedAction, SIGNAL(triggered()), SLOT(slotDeleteSelected()));

    KAction *deleteAllFinishedAction = actionCollection()->addAction("delete_all_finished");
    deleteAllFinishedAction->setText(i18nc("delete all finished transfers", "Remove All Finished"));
    deleteAllFinishedAction->setIcon(KIcon("edit-clear-list"));
    deleteAllFinishedAction->setHelpText(i18n("Removes all finished transfers and leaves all files on disk"));
    connect(deleteAllFinishedAction, SIGNAL(triggered()), SLOT(slotDeleteFinished()));
    
    KAction *deleteSelectedIncludingFilesAction = actionCollection()->addAction("delete_selected_download_including_files");
    deleteSelectedIncludingFilesAction->setText(i18nc("delete selected transfer item and files", "Remove Selected and Delete Files"));
    deleteSelectedIncludingFilesAction->setIcon(KIcon("edit-delete"));
    deleteSelectedIncludingFilesAction->setHelpText(i18n("Removes selected transfer and deletes files from disk in any case"));
    connect(deleteSelectedIncludingFilesAction, SIGNAL(triggered()), SLOT(slotDeleteSelectedIncludingFiles()));

    KAction *redownloadSelectedAction = actionCollection()->addAction("redownload_selected_download");
    redownloadSelectedAction->setText(i18nc("redownload selected transfer item", "Redownload Selected"));
    redownloadSelectedAction->setIcon(KIcon("view-refresh"));
    connect(redownloadSelectedAction, SIGNAL(triggered()), SLOT(slotRedownloadSelected()));

    KAction *startAllAction = actionCollection()->addAction("start_all_download");
    startAllAction->setText(i18n("Start All"));
    startAllAction->setIcon(KIcon("media-seek-forward"));
    startAllAction->setShortcuts(KShortcut("Ctrl+R"));
    startAllAction->setHelpText(i18n("Starts / resumes all transfers"));
    connect(startAllAction, SIGNAL(triggered()), SLOT(slotStartAllDownload()));

    KAction *startSelectedAction = actionCollection()->addAction("start_selected_download");
    startSelectedAction->setText(i18n("Start Selected"));
    startSelectedAction->setIcon(KIcon("media-playback-start"));
    startSelectedAction->setHelpText(i18n("Starts / resumes selected transfer"));
    connect(startSelectedAction, SIGNAL(triggered()), SLOT(slotStartSelectedDownload()));

    KAction *stopAllAction = actionCollection()->addAction("stop_all_download");
    stopAllAction->setText(i18n("Pause All"));
    stopAllAction->setIcon(KIcon("media-playback-pause"));
    stopAllAction->setShortcuts(KShortcut("Ctrl+P"));
    stopAllAction->setHelpText(i18n("Pauses all transfers"));
    connect(stopAllAction, SIGNAL(triggered()), SLOT(slotStopAllDownload()));

    KAction *stopSelectedAction = actionCollection()->addAction("stop_selected_download");
    stopSelectedAction->setText(i18n("Stop Selected"));
    stopSelectedAction->setIcon(KIcon("media-playback-pause"));
    stopSelectedAction->setHelpText(i18n("Pauses selected transfer"));
    connect(stopSelectedAction, SIGNAL(triggered()), SLOT(slotStopSelectedDownload()));

    KActionMenu *startActionMenu = new KActionMenu(KIcon("media-playback-start"), i18n("Start"),
                                                     actionCollection());
    actionCollection()->addAction("start_menu", startActionMenu);
    startActionMenu->setDelayed(true);
    startActionMenu->addAction(startSelectedAction);
    startActionMenu->addAction(startAllAction);
    connect(startActionMenu, SIGNAL(triggered()), SLOT(slotStartDownload()));

    KActionMenu *stopActionMenu = new KActionMenu(KIcon("media-playback-pause"), i18n("Pause"),
                                                    actionCollection());
    actionCollection()->addAction("stop_menu", stopActionMenu);
    stopActionMenu->setDelayed(true);
    stopActionMenu->addAction(stopSelectedAction);
    stopActionMenu->addAction(stopAllAction);
    connect(stopActionMenu, SIGNAL(triggered()), SLOT(slotStopDownload()));
    
    KAction *openDestAction = actionCollection()->addAction("transfer_open_dest");
    openDestAction->setText(i18n("Open Destination"));
    openDestAction->setIcon(KIcon("document-open"));
    connect(openDestAction, SIGNAL(triggered()), SLOT(slotTransfersOpenDest()));

    KAction *openFileAction = actionCollection()->addAction("transfer_open_file");
    openFileAction->setText(i18n("Open File"));
    openFileAction->setIcon(KIcon("document-open"));
    connect(openFileAction, SIGNAL(triggered()), SLOT(slotTransfersOpenFile()));

    KAction *showDetailsAction = new KToggleAction(KIcon("document-properties"), i18n("Show Details"), actionCollection());
    actionCollection()->addAction("transfer_show_details", showDetailsAction);
    connect(showDetailsAction, SIGNAL(triggered()), SLOT(slotTransfersShowDetails()));

    KAction *copyUrlAction = actionCollection()->addAction("transfer_copy_source_url");
    copyUrlAction->setText(i18n("Copy URL to Clipboard"));
    copyUrlAction->setIcon(KIcon("edit-copy"));
    connect(copyUrlAction, SIGNAL(triggered()), SLOT(slotTransfersCopySourceUrl()));

    KAction *transferHistoryAction = actionCollection()->addAction("transfer_history");
    transferHistoryAction->setText(i18n("&Transfer History"));
    transferHistoryAction->setIcon(KIcon("view-history"));
    transferHistoryAction->setShortcuts(KShortcut("Ctrl+H"));
    connect(transferHistoryAction, SIGNAL(triggered()), SLOT(slotTransferHistory()));

    KAction *transferGroupSettingsAction = actionCollection()->addAction("transfer_group_settings");
    transferGroupSettingsAction->setText(i18n("&Group Settings"));
    transferGroupSettingsAction->setIcon(KIcon("preferences-system"));
    transferGroupSettingsAction->setShortcuts(KShortcut("Ctrl+G"));
    connect(transferGroupSettingsAction, SIGNAL(triggered()), SLOT(slotTransferGroupSettings()));

    KAction *transferSettingsAction = actionCollection()->addAction("transfer_settings");
    transferSettingsAction->setText(i18n("&Transfer Settings"));
    transferSettingsAction->setIcon(KIcon("preferences-system"));
    transferSettingsAction->setShortcuts(KShortcut("Ctrl+T"));
    connect(transferSettingsAction, SIGNAL(triggered()), SLOT(slotTransferSettings()));

    KAction *listLinksAction = actionCollection()->addAction("import_links");
    listLinksAction->setText(i18n("Import &Links..."));
    listLinksAction->setIcon(KIcon("view-list-text"));
    listLinksAction->setShortcuts(KShortcut("Ctrl+L"));
    connect(listLinksAction, SIGNAL(triggered()), SLOT(slotShowListLinks()));

    //create the download finished actions which can be displayed in the toolbar
    KSelectAction *downloadFinishedActions = new KSelectAction(i18n("After downloads finished action"), this);//TODO maybe with name??
    actionCollection()->addAction("download_finished_actions", downloadFinishedActions);
    downloadFinishedActions->setHelpText(i18n("Choose an action that is executed after all downloads have been finished."));

    KAction *noAction = downloadFinishedActions->addAction(i18n("No Action"));
    connect(noAction, SIGNAL(triggered()), SLOT(slotDownloadFinishedActions()));
    downloadFinishedActions->addAction(noAction);

    KAction *quitAction = downloadFinishedActions->addAction(i18n("Quit KGet"));
    quitAction->setData(KGet::Quit);
    connect(quitAction, SIGNAL(triggered()), SLOT(slotDownloadFinishedActions()));
    downloadFinishedActions->addAction(quitAction);

#ifdef HAVE_KWORKSPACE
    KAction *shutdownAction = downloadFinishedActions->addAction(i18n("Turn Off Computer"));
    shutdownAction->setData(KGet::Shutdown);
    connect(shutdownAction, SIGNAL(triggered()), SLOT(slotDownloadFinishedActions()));
    downloadFinishedActions->addAction(shutdownAction);

    KAction *hibernateAction = downloadFinishedActions->addAction(i18n("Hibernate Computer"));
    hibernateAction->setData(KGet::Hibernate);
    connect(hibernateAction, SIGNAL(triggered()), SLOT(slotDownloadFinishedActions()));
    downloadFinishedActions->addAction(hibernateAction);

    KAction *suspendAction = downloadFinishedActions->addAction(i18n("Suspend Computer"));
    suspendAction->setData(KGet::Suspend);
    connect(suspendAction, SIGNAL(triggered()), SLOT(slotDownloadFinishedActions()));
    downloadFinishedActions->addAction(suspendAction);
#endif

    if (Settings::afterFinishActionEnabled()) {
        downloadFinishedActions->setCurrentItem(Settings::afterFinishAction() + 1);
    } else {
        downloadFinishedActions->setCurrentItem(0);
    }
}

void MainWindow::slotDownloadFinishedActions()
{
    KAction *action = static_cast<KAction*>(QObject::sender());
    bool ok;
    const int type = action->data().toInt(&ok);
    if (ok) {
        Settings::self()->setAfterFinishAction(type);
    }

    //only after finish actions have a number assigned
    Settings::self()->setAfterFinishActionEnabled(ok);
    Settings::self()->writeConfig();
    slotNewConfig();
}

void MainWindow::init()
{
    //Here we import the user's transfers.
    KGet::load( KStandardDirs::locateLocal("appdata", "transfers.kgt") );

    if(Settings::enableSystemTray()) {
        m_dock = new Tray(this);
    }

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

    // DropTarget
    m_drop = new DropTarget(this);

    if (Settings::firstRun()) {
        if (KMessageBox::questionYesNoCancel(this ,i18n("This is the first time you have run KGet.\n"
                                             "Would you like to enable KGet as the download manager for Konqueror?"),
                                             i18n("Konqueror Integration"), KGuiItem(i18n("Enable")),
                                             KGuiItem(i18n("Do Not Enable")))
                                             == KMessageBox::Yes) {
            Settings::setKonquerorIntegration(true);
            m_konquerorIntegration->setChecked(Settings::konquerorIntegration());
            slotKonquerorIntegration(true);
        }

        m_drop->setDropTargetVisible(false);

        // reset the FirstRun config option
        Settings::setFirstRun(false);
    }

    if (Settings::showDropTarget() && !m_startWithoutAnimation)
        m_drop->setDropTargetVisible(true);

    //auto paste stuff
    lastClipboard = QApplication::clipboard()->text( QClipboard::Clipboard ).trimmed();
    clipboardTimer = new QTimer(this);
    connect(clipboardTimer, SIGNAL(timeout()), SLOT(slotCheckClipboard()));
    if ( Settings::autoPaste() )
        clipboardTimer->start(1000);

    if (Settings::webinterfaceEnabled())
        m_webinterface = new HttpServer(this);

    if (Settings::speedLimit())
    {
        KGet::setGlobalDownloadLimit(Settings::globalDownloadLimit());
        KGet::setGlobalUploadLimit(Settings::globalUploadLimit());
    }
    else
    {
        KGet::setGlobalDownloadLimit(0);
        KGet::setGlobalUploadLimit(0);
    }

    connect(KGet::model(), SIGNAL(transfersAddedEvent(QList<TransferHandler*>)), this, SLOT(slotUpdateTitlePercent()));
    connect(KGet::model(), SIGNAL(transfersRemovedEvent(QList<TransferHandler*>)), this, SLOT(slotUpdateTitlePercent()));
    connect(KGet::model(), SIGNAL(transfersChangedEvent(QMap<TransferHandler*,Transfer::ChangesFlags>)), 
                           SLOT(slotTransfersChanged(QMap<TransferHandler*,Transfer::ChangesFlags>)));
    connect(KGet::model(), SIGNAL(groupsChangedEvent(QMap<TransferGroupHandler*,TransferGroup::ChangesFlags>)),
                           SLOT(slotGroupsChanged(QMap<TransferGroupHandler*,TransferGroup::ChangesFlags>)));

#ifdef DO_KGET_TEST
    if (m_doTesting)
    {
        // Unit testing
        TestKGet unitTest;
        QTest::qExec(&unitTest);
    }
#endif
}

void MainWindow::slotToggleDropTarget()
{
    m_drop->setDropTargetVisible(!m_drop->isVisible());
}

void MainWindow::slotNewTransfer()
{
    NewTransferDialogHandler::showNewTransferDialog(KUrl());
}

void MainWindow::slotImportTransfers()
{
    QString filename = KFileDialog::getOpenFileName(KUrl(),
                                                    "*.kgt *.metalink *.meta4 *.torrent|" + i18n("All Openable Files") +
                                                    " (*.kgt *.metalink *.meta4 *.torrent)", this, i18n("Open File"));

    if(filename.endsWith(QLatin1String(".kgt")))
    {
        KGet::load(filename);
        return;
    }

    if(!filename.isEmpty())
        KGet::addTransfer( KUrl( filename ) );
}

void MainWindow::slotUpdateTitlePercent()
{
    int percent = transfersPercent();
    if (percent != -1) {
        setPlainCaption(i18nc("window title including overall download progress in percent", "KGet - %1%", percent));
    } else {
        setPlainCaption(i18n("KGet"));
    }
}

void MainWindow::slotTransfersChanged(QMap<TransferHandler*, Transfer::ChangesFlags> transfers)
{
    QMapIterator<TransferHandler*, Transfer::ChangesFlags> it(transfers);
    
    //QList<TransferHandler *> finishedTransfers;
    bool update = false;
    
    while (it.hasNext()) {
        it.next();
        
        //TransferHandler * transfer = it.key();
        Transfer::ChangesFlags transferFlags = it.value();

        if (transferFlags & Transfer::Tc_Percent || transferFlags & Transfer::Tc_Status) {
            update = true;
            break;
        }
        
//         kDebug(5001) << it.key() << ": " << it.value() << endl;
    }
    
    if (update)
        slotUpdateTitlePercent();
}

void MainWindow::slotGroupsChanged(QMap<TransferGroupHandler*, TransferGroup::ChangesFlags> groups)
{
    bool update = false;
    foreach (const TransferGroup::ChangesFlags &groupFlags, groups)
    {
        if (groupFlags & TransferGroup::Gc_Percent)
        {
            update = true;
            break;
        }
    }
    if (update)
        slotUpdateTitlePercent();
}

void MainWindow::slotQuit()
{
    if (KGet::schedulerRunning()) {
        if (KMessageBox::warningYesNo(this,
                i18n("Some transfers are still running.\n"
                     "Are you sure you want to close KGet?"),
                i18n("Confirm Quit"),
                KStandardGuiItem::quit(), KStandardGuiItem::cancel(),
                "ExitWithActiveTransfers") != KMessageBox::Yes)
            return;
    }

    Settings::self()->writeConfig();
    qApp->quit();
}

void MainWindow::slotPreferences()
{
    //never reuse the preference dialog, to make sure its settings are always reloaded
    PreferencesDialog * dialog = new PreferencesDialog( this, Settings::self() );
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // keep us informed when the user changes settings
    connect( dialog, SIGNAL(settingsChanged(QString)),
             this, SLOT(slotNewConfig()) );

    dialog->show();
}

void MainWindow::slotExportTransfers()
{
    const QString filename = KFileDialog::getSaveFileName
        (KUrl(),
         "*.kgt|" + i18n("KGet Transfer List") + " (*.kgt)\n*.txt|" + i18n("Text File") + " (*.txt)",
         this,
         i18n("Export Transfers")
        );

    if (!filename.isEmpty()) {
        const bool plain = !filename.endsWith("kgt");
        KGet::save(filename, plain);
    }
}

void MainWindow::slotCreateMetalink()
{
    MetalinkCreator *dialog = new MetalinkCreator(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::slotDeleteGroup()
{
    bool mainSelected;
    QList<TransferGroupHandler*> groups = KGet::selectedTransferGroups(&mainSelected);
    if (!mainSelected) {
        KGet::delGroups(groups);
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

void MainWindow::slotSetIconGroup()
{
    KIconDialog dialog(this);
    QString iconName = dialog.getIcon();
    TransferTreeSelectionModel *selModel = KGet::selectionModel();

    QModelIndexList indexList = selModel->selectedRows();

    if (!iconName.isEmpty())
    {
        foreach (TransferGroupHandler *group, KGet::selectedTransferGroups())
        {
            group->setIconName(iconName);
        }
    }
    //emit dataChanged(indexList.first(),indexList.last());
}

void MainWindow::slotStartDownload()
{
    if (KGet::selectedTransfers().size() == 0 && KGet::selectedTransferGroups().size() == 0) {
        slotStartAllDownload();
    } else {
        slotStartSelectedDownload();
    }
}

void MainWindow::slotStartAllDownload()
{
    KGet::setSchedulerRunning(true);
}

void MainWindow::slotStartSelectedDownload()
{
    KGet::setSuspendScheduler(true);
    foreach (TransferHandler *transfer, KGet::selectedTransfers()) {
        transfer->start();
    }
    foreach (TransferGroupHandler *group, KGet::selectedTransferGroups()) {
        group->start();
    }
    KGet::setSuspendScheduler(false);
}

void MainWindow::slotStopDownload()
{
    if (KGet::selectedTransfers().size() == 0 && KGet::selectedTransferGroups().size() == 0) {
        slotStopAllDownload();
    } else {
        slotStopSelectedDownload();
    }
}

void MainWindow::slotStopAllDownload()
{
    KGet::setSchedulerRunning(false);
    
    // This line ensures that each transfer is stopped. In the handler class
    // the policy of the transfer will be correctly setted to None
    foreach (TransferHandler * it, KGet::allTransfers())
        it->stop();
}

void MainWindow::slotStopSelectedDownload()
{
    KGet::setSuspendScheduler(true);
    foreach (TransferHandler *transfer, KGet::selectedTransfers()) {
        transfer->stop();
    }
    foreach (TransferGroupHandler *group, KGet::selectedTransferGroups()) {
        group->stop();
    }
    KGet::setSuspendScheduler(false);
}

void MainWindow::slotDeleteSelected()
{
    foreach (TransferHandler * it, KGet::selectedTransfers())
    {
        if (it->status() != Job::Finished && it->status() != Job::FinishedKeepAlive) {
            if (KMessageBox::warningYesNo(this,
                    i18np("Are you sure you want to delete the selected transfer?", 
                          "Are you sure you want to delete the selected transfers?", KGet::selectedTransfers().count()),
                    i18n("Confirm transfer delete"),
                    KStandardGuiItem::remove(), KStandardGuiItem::cancel()) == KMessageBox::No)
            {
                return;
            }
            break;
        }
    }

    const QList<TransferHandler*> selectedTransfers = KGet::selectedTransfers();
    if (!selectedTransfers.isEmpty()) {
        foreach (TransferHandler *it, selectedTransfers) {
            m_viewsContainer->closeTransferDetails(it);//TODO make it take QList?
        }
        KGet::delTransfers(KGet::selectedTransfers());
    } else {
        //no transfers selected, delete groups if any are selected
        slotDeleteGroup();
    }
}

void MainWindow::slotDeleteSelectedIncludingFiles()
{
    const QList<TransferHandler*> selectedTransfers = KGet::selectedTransfers();

    if (!selectedTransfers.isEmpty()) {
        if (KMessageBox::warningYesNo(this,
                i18np("Are you sure you want to delete the selected transfer including files?", 
                        "Are you sure you want to delete the selected transfers including files?", selectedTransfers.count()),
                i18n("Confirm transfer delete"),
                KStandardGuiItem::remove(), KStandardGuiItem::cancel()) == KMessageBox::No) {
            return;
        }
        foreach (TransferHandler *it, selectedTransfers) {
            m_viewsContainer->closeTransferDetails(it);//TODO make it take QList?
        }
        KGet::delTransfers(KGet::selectedTransfers(), KGet::DeleteFiles);
    } else {
        //no transfers selected, delete groups if any are selected
        slotDeleteGroup();
    }
}

void MainWindow::slotRedownloadSelected()
{
    foreach(TransferHandler * it, KGet::selectedTransfers())
    {
        KGet::redownloadTransfer(it);
    }
}

void MainWindow::slotPriorityTop()
{
    QList<TransferHandler*> selected = KGet::selectedTransfers();
    TransferHandler *after = 0;
    TransferGroupHandler *group = 0;
    foreach (TransferHandler *transfer, selected) {
        if (!transfer) {
            continue;
        }

        //previous transfer was not of the same group, so after has to be reset as the group
        if (!group || (group != transfer->group())) {
            after = 0;
            group = transfer->group();
        }
        KGet::model()->moveTransfer(transfer, group, after);
        after = transfer;
    }
}

void MainWindow::slotPriorityBottom()
{
    QList<TransferHandler*> selected = KGet::selectedTransfers();
    QList<TransferHandler*> groupTransfers;
    TransferHandler *after = 0;
    TransferGroupHandler *group = 0;
    foreach (TransferHandler *transfer, selected) {
        if (!transfer) {
            continue;
        }

        //previous transfer was not of the same group, so after has to be reset as the group
        if (!group || (group != transfer->group())) {
            group = transfer->group();
            groupTransfers = group->transfers();
            if (groupTransfers.isEmpty()) {
                after = 0;
            } else {
                after = groupTransfers.last();
            }
        }

        KGet::model()->moveTransfer(transfer, group, after);
        after = transfer;
    }
}

void MainWindow::slotPriorityUp()
{
    QList<TransferHandler*> selected = KGet::selectedTransfers();
    QList<TransferHandler*> groupTransfers;
    TransferHandler *after = 0;
    int newIndex = -1;
    TransferGroupHandler *group = 0;
    foreach (TransferHandler *transfer, selected) {
        if (!transfer) {
            continue;
        }

        //previous transfer was not of the same group, so group has to be reset
        if (!group || (group != transfer->group())) {
            group = transfer->group();
            groupTransfers = group->transfers();
        }

        after = 0;
        if (!groupTransfers.isEmpty()) {
            int index = groupTransfers.indexOf(transfer);

            //do not move higher than the first place
            if (index > 0) {
                //if only Transfers at the top are select do not change their order
                if ((index - 1) != newIndex) {
                    newIndex = index - 1;
                    if (newIndex - 1 >= 0) {
                        after = groupTransfers[newIndex - 1];
                    }

                    //keep the list with the actual movements synchronised
                    groupTransfers.move(index, newIndex);

                    KGet::model()->moveTransfer(transfer, group, after);
                } else {
                    newIndex = index;
                }
            } else {
                newIndex = 0;
            }
        }
    }
}

void MainWindow::slotPriorityDown()
{
    QList<TransferHandler*> selected = KGet::selectedTransfers();
    QList<TransferHandler*> groupTransfers;
    int newIndex = -1;
    TransferGroupHandler *group = 0;
    for (int i = selected.count() - 1; i >= 0; --i) {
        TransferHandler *transfer = selected[i];
        if (!transfer) {
            continue;
        }

        //previous transfer was not of the same group, so group has to be reset
        if (!group || (group != transfer->group())) {
            group = transfer->group();
            groupTransfers = group->transfers();
        }

        if (!groupTransfers.isEmpty()) {
            int index = groupTransfers.indexOf(transfer);

            //do not move lower than the last place
            if ((index != -1) && (index + 1 < groupTransfers.count())) {
                //if only Transfers at the top are select do not change their order
                if ((index + 1) != newIndex) {
                    newIndex = index + 1;
                    TransferHandler *after = groupTransfers[newIndex];


                    //keep the list with the actual movements synchronised
                    groupTransfers.move(index, newIndex);

                    KGet::model()->moveTransfer(transfer, group, after);
                } else {
                    newIndex = index;
                }
            } else {
                newIndex = index;
            }
        }
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
            new KRun(directory, this, 0, true, false);
            openedDirs.append( directory );
        }
    }
}

void MainWindow::slotTransfersOpenFile()
{
    foreach(TransferHandler * it, KGet::selectedTransfers())
    {
        new KRun(it->dest(), this, 0, true, false);
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

void MainWindow::slotDeleteFinished()
{
    foreach(TransferHandler * it, KGet::finishedTransfers()) {
        m_viewsContainer->closeTransferDetails(it);
    }
    KGet::delTransfers(KGet::finishedTransfers());
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
    connect(&edit, SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()));
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

    m_drop->setDropTargetVisible(Settings::showDropTarget(), false);

    if(Settings::enableSystemTray() && !m_dock)
    {
        m_dock = new Tray(this);
    }
    else if(!Settings::enableSystemTray() && m_dock)
    {
        setVisible(true);
        delete m_dock;
        m_dock = 0;
    }

    slotKonquerorIntegration(Settings::konquerorIntegration());
    m_konquerorIntegration->setChecked(Settings::konquerorIntegration());

    if (Settings::autoPaste())
        clipboardTimer->start(1000);
    else
        clipboardTimer->stop();
    m_autoPasteAction->setChecked(Settings::autoPaste());

    if (Settings::webinterfaceEnabled() && !m_webinterface) {
        m_webinterface = new HttpServer(this);
    } else if (m_webinterface && !Settings::webinterfaceEnabled()) {
        delete m_webinterface;
        m_webinterface = 0;
    } else if (m_webinterface) {
        m_webinterface->settingsChanged();
    }

    if (Settings::speedLimit())
    {
        KGet::setGlobalDownloadLimit(Settings::globalDownloadLimit());
        KGet::setGlobalUploadLimit(Settings::globalUploadLimit());
    }
    else
    {
        KGet::setGlobalDownloadLimit(0);
        KGet::setGlobalUploadLimit(0);
    }

    KGet::settingsChanged();
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
    const QString clipData = QApplication::clipboard()->text( QClipboard::Clipboard ).trimmed();

    if (clipData != lastClipboard) {
        lastClipboard = clipData;
        if (lastClipboard.isEmpty())
            return;

        const KUrl url = KUrl(lastClipboard);
        if (url.isValid() && !url.protocol().isEmpty() && url.hasPath() && url.hasHost() && !url.isLocalFile()) {
            bool add = false;
            const QString urlString = url.url();

            //check the combined whitelist and blacklist
            const QList<int> types = Settings::autoPasteTypes();
            const QList<int> syntaxes = Settings::autoPastePatternSyntaxes();
            const QStringList patterns = Settings::autoPastePatterns();
            const Qt::CaseSensitivity cs = (Settings::autoPasteCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
            for (int i = 0; i < types.count(); ++i) {
                const QRegExp::PatternSyntax syntax = (syntaxes[i] == AutoPasteModel::Wildcard ? QRegExp::Wildcard : QRegExp::RegExp2);
                QRegExp rx(patterns[i], cs, syntax);
                if (rx.exactMatch(urlString)) {
                    add = (types[i] == AutoPasteModel::Include);
                    break;
                }
            }

            if (add) {
                KGet::addTransfer(url);
            }
        }
    }
}

void MainWindow::slotTrayKonquerorIntegration(bool enable)
{
    slotKonquerorIntegration(enable);
    if (!enable && Settings::konquerorIntegration())
    {
        KGet::showNotification(this, "notification",
                                     i18n("KGet has been temporarily disabled as download manager for Konqueror. "
            "If you want to disable it forever, go to Settings->Advanced and disable \"Use "
            "as download manager for Konqueror\"."),
                                     "dialog-info");
        /*KMessageBox::information(this,
            i18n("KGet has been temporarily disabled as download manager for Konqueror. "
            "If you want to disable it forever, go to Settings->Advanced and disable \"Use "
            "as download manager for Konqueror\"."),
            i18n("Konqueror Integration disabled"),
            "KonquerorIntegrationDisabled");*/
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
    if (m_menubarAction->isChecked())
        menuBar()->show();
    else
        menuBar()->hide();
}

void MainWindow::setSystemTrayDownloading(bool running)
{
    kDebug(5001);

    if (m_dock)
        m_dock->setDownloading(running);
}

void MainWindow::slotTransferHistory()
{
    TransferHistory *history = new TransferHistory();
    history->exec();
}

void MainWindow::slotTransferGroupSettings()
{
    kDebug(5001);
    QList<TransferGroupHandler*> list = KGet::selectedTransferGroups();
    foreach(TransferGroupHandler* group, list)
    {
        QPointer<GroupSettingsDialog> settings = new GroupSettingsDialog(this, group);
        settings->exec();
        delete settings;
    }
}

void MainWindow::slotTransferSettings()
{
    kDebug(5001);
    QList<TransferHandler*> list = KGet::selectedTransfers();
    foreach(TransferHandler* transfer, list)
    {
        QPointer<TransferSettingsDialog> settings = new TransferSettingsDialog(this, transfer);
        settings->exec();
        delete settings;
    }
}

/** slots for link list **/
void MainWindow::slotShowListLinks()
{
    KGetLinkView *link_view = new KGetLinkView(this);
    link_view->importUrl();
    link_view->show();
}

void MainWindow::slotImportUrl(const QString &url)
{
    KGetLinkView *link_view = new KGetLinkView(this);
    link_view->importUrl(url);
    link_view->show();
}

/** widget events **/
void MainWindow::closeEvent( QCloseEvent * e )
{
    // if the event comes from out the application (close event) we decide between close or hide
    // if the event comes from the application (system shutdown) we say goodbye
    if(e->spontaneous()) {
        e->ignore();
        if(!Settings::enableSystemTray())
            slotQuit();
        else
            hide();
    }
}

void MainWindow::hideEvent(QHideEvent *)
{
    Settings::setShowMain(false);
}

void MainWindow::showEvent(QShowEvent *)
{
    Settings::setShowMain(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    event->setAccepted(KUrl::List::canDecode(event->mimeData())
                  || event->mimeData()->hasText());
}

void MainWindow::dropEvent(QDropEvent * event)
{
    KUrl::List list = KUrl::List::fromMimeData(event->mimeData());

    if (!list.isEmpty())
    {
        if (list.count() == 1 && list.first().url().endsWith(QLatin1String(".kgt")))
        {
            int msgBoxResult = KMessageBox::questionYesNoCancel(this, i18n("The dropped file is a KGet Transfer List"), "KGet",
                                   KGuiItem(i18n("&Download"), KIcon("document-save")), 
                                       KGuiItem(i18n("&Load transfer list"), KIcon("list-add")), KStandardGuiItem::cancel());

            if (msgBoxResult == 3) //Download
                NewTransferDialogHandler::showNewTransferDialog(list.first().url());
            if (msgBoxResult == 4) //Load
                KGet::load(list.first().url());
        }
        else
        {
            if (list.count() == 1)
                NewTransferDialogHandler::showNewTransferDialog(list.first().url());
            else
                NewTransferDialogHandler::showNewTransferDialog(list);
        }
    }
    else
    {
        NewTransferDialogHandler::showNewTransferDialog(KUrl());
    }
}

#include "mainwindow.moc"
