/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QClipboard>
#include <QSplitter>
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
#include <kstdaction.h>
#include <klocale.h>

#include "kget.h"
#include "core/model.h"
#include "core/transferhandler.h"
#include "settings.h"
#include "conf/preferencesdialog.h"

#include "ui/sidebar.h"
#include "ui/viewscontainer.h"
#include "ui/tray.h"
#include "ui/droptarget.h"

KGet::KGet( QWidget * parent )
    : KMainWindow( parent ),
        m_drop(0), m_dock(0)
{
    // create the model
    Model::self( this );

    // create actions
    setupActions();

    createGUI("kgetui.rc");

//    m_splitter = new QSplitter(this);
//    m_sidebar = new Sidebar(m_splitter);
    m_viewsContainer = new ViewsContainer(this);

    setCentralWidget(m_viewsContainer);

    // restore position, size and visibility
    move( Settings::mainPosition() );
    setMaximumHeight( 32767 );
    setAutoSaveSettings();
    setPlainCaption(i18n("KGet"));

    if ( Settings::showMain() )
        show();
    else
        hide();

    //Some of the widgets are initialized in slotDelayedInit()
    QTimer::singleShot( 0, this, SLOT(slotDelayedInit()) );
}

KGet::~KGet()
{
    //Save the user's transfers
    Model::save();

    slotSaveMyself();
    delete clipboardTimer;
    delete m_drop;
    delete m_dock;
    // reset konqueror integration (necessary if user enabled / disabled temporarily integration from tray)
    slotKonquerorIntegration( Settings::konquerorIntegration() );
    // the following call saves options set in above dtors
    Settings::writeConfig();
}


void KGet::setupActions()
{
    KActionCollection * ac = actionCollection();
    KAction * action;

    // local - Shows a dialog asking for a new URL to down
    action = new KAction( KIcon("filenew"), i18n("&New Download..."), 
                          ac, "new_transfer" );
    action->setShortcut(KShortcut("CTRL+Key_N"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotNewTransfer()));

    action = new KAction( KIcon("fileopen"), i18n("&Open..."), 
                          ac, "open" );
    action->setShortcut(KShortcut("CTRL+Key_O"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotOpen()));

    action = new KAction( i18n("&Export Transfers List..."), 
                          ac, "export_transfers" );
    action->setShortcut(KShortcut("CTRL+Key_E"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotExportTransfers()));

    KAction * r1;
    KAction * r2;

    r1 = new KAction( KIcon("player_play"), i18n("Start All"),
                      ac, "start_download" );
    connect(r1, SIGNAL(triggered(bool)), SLOT(slotStartDownload()));

    r2 = new KAction( KIcon("player_pause"), i18n("Stop All"),
                      ac, "stop_download" );
    connect(r2, SIGNAL(triggered(bool)), SLOT(slotStopDownload()));

    QActionGroup* scheduler_commands = new QActionGroup(this);
    scheduler_commands->setExclusive(true);
    r1->setActionGroup(scheduler_commands);
    r2->setActionGroup(scheduler_commands);

    r1->setChecked( Settings::downloadAtStartup() );
    r2->setChecked( !Settings::downloadAtStartup() );

    m_AutoPaste =  new KToggleAction( KIcon("tool_clipboard"), i18n("Auto-Paste Mode"),
                                      ac, "auto_paste" );
    connect(m_AutoPaste, SIGNAL(triggered(bool)), SLOT(slotToggleAutoPaste()));
    m_AutoPaste->setChecked(Settings::autoPaste());
    m_AutoPaste->setWhatsThis(i18n("<b>Auto paste</b> button toggles the auto-paste mode "
                                   "on and off.\nWhen set, KGet will periodically scan the clipboard "
                                   "for URLs and paste them automatically."));

    m_showDropTarget =  new KToggleAction( KIcon("tool_drop_target"), i18n("Show Drop Target"),
                                           ac, "show_drop_target" );
    connect(m_showDropTarget, SIGNAL(triggered(bool)), SLOT(slotShowDropTarget()));

    m_KonquerorIntegration =  new KAction( KIcon("konqueror"), i18n("Enable KGet as Konqueror Download Manager"),
                                           ac, "konqueror_integration" );
    connect(m_KonquerorIntegration, SIGNAL(triggered(bool)), SLOT(slotTrayKonquerorIntegration()));
    if (Settings::konquerorIntegration())
        m_KonquerorIntegration->setText(i18n("Disable &KGet as Konqueror Download Manager"));
    slotKonquerorIntegration(Settings::konquerorIntegration());

    // local - Destroys all sub-windows and exits
    KStdAction::quit(this, SLOT(slotQuit()), ac, "quit");
    // local - Standard configure actions
    KStdAction::preferences(this, SLOT(slotPreferences()), ac, "preferences");
    KStdAction::configureToolbars(this, SLOT( slotConfigureToolbars() ), ac, "configure_toolbars");
    KStdAction::keyBindings(this, SLOT( slotConfigureKeys() ), ac, "configure_keys");
    KStdAction::configureNotifications(this, SLOT(slotConfigureNotifications()), ac, "configure_notifications" );
    m_menubarAction = KStdAction::showMenubar(this, SLOT(slotShowMenubar()), ac, "settings_showmenubar" );
    m_menubarAction->setChecked( !menuBar()->isHidden() );

    // Transfer related actions
// KAction *newAct = new KAction("filenew", i18n("&New"), actionCollection(), "new");

//     new KAction( i18n("Start"), "player_play", 0,
//                  this, SLOT(slotTransfersStart()),
//                  actionCollection(), "transfer_start" );

//  newAct->setShortcut(KStdAccel::shortcut(KStdAccel::New));
//  connect(newAct, SIGNAL(triggered(bool)), SLOT(fileNew()));


    action = new KAction( KIcon("player_play"), i18n("Start"), 
                          ac, "transfer_start" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotTransfersStart()));

    action = new KAction( KIcon("player_pause"), i18n("Stop"), 
                          ac, "transfer_stop" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotTransfersStop()));

    action = new KAction( KIcon("editdelete"), i18n("Delete"), 
                          ac, "transfer_remove" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotTransfersDelete()));

    action = new KAction( KIcon("folder"), i18n("Open Destination"), 
                          ac, "transfer_open_dest" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotTransfersOpenDest()));

    action = new KAction( KIcon("configure"), i18n("Show Details"), 
                          ac, "transfer_show_details" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotTransfersShowDetails()));

    action = new KAction( KIcon("tool_clipboard"), i18n("Copy URL to Clipboard"), 
                          ac, "transfer_copy_source_url" );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotTransfersCopySourceURL()));
}

void KGet::slotDelayedInit()
{
    //Here we import the user's transfers.
    Model::load( KStandardDirs::locateLocal("appdata", "transfers.kgt") );

    // DropTarget
    m_drop = new DropTarget(this);

    if ( Settings::showDropTarget() || Settings::firstRun() )
        m_drop->show();

    if ( Settings::firstRun() )
        m_drop->playAnimation();

    m_showDropTarget->setChecked( m_drop->isVisible() );

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

void KGet::slotNewTransfer()
{
    Model::addTransfer(KUrl());
}

void KGet::slotOpen()
{
    QString filename = KFileDialog::getOpenFileName
        (KUrl(),
         "*.kgt *.torrent|" + i18n("All openable files") + " (*.kgt *.torrent)",
         this,
         i18n("Open file")
        );

    if(filename.endsWith(".kgt"))
    {
        Model::load(filename);
        return;
    }

    if(!filename.isEmpty())
        Model::addTransfer( KUrl( filename ) );
}

void KGet::slotQuit()
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

void KGet::slotPreferences()
{
    // an instance the dialog could be already created and could be cached, 
    // in which case you want to display the cached dialog
    if ( PreferencesDialog::showDialog( "preferences" ) ) 
        return;

    // we didn't find an instance of this dialog, so lets create it
    PreferencesDialog * dialog = new PreferencesDialog( this, Settings::self() );

    // keep us informed when the user changes settings
    connect( dialog, SIGNAL(settingsChanged(const QString&)),
             this, SLOT(slotNewConfig()) );

    dialog->show();
}

void KGet::slotExportTransfers()
{
    QString filename = KFileDialog::getSaveFileName
        (KUrl(),
         "*.kgt|" + i18n("KGet transfer list") + " (*.kgt)",
         this,
         i18n("Export transfers")
        );

    if(!filename.isEmpty())
        Model::save(filename);
}

void KGet::slotStartDownload()
{
    m_dock->setDownloading(true);    

    Model::setSchedulerRunning(true);
}

void KGet::slotStopDownload()
{
    m_dock->setDownloading(false);    

    Model::setSchedulerRunning(false);
}

void KGet::slotConfigureNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void KGet::slotConfigureKeys()
{
    KKeyDialog::configure(actionCollection());
}

void KGet::slotConfigureToolbars()
{
    KEditToolbar edit( "kget_toolbar", actionCollection() );
    connect(&edit, SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
    edit.exec();
}

void KGet::slotTransfersStart()
{
    foreach(TransferHandler * it, Model::selectedTransfers())
        it->start();
}

void KGet::slotTransfersStop()
{
    foreach(TransferHandler * it, Model::selectedTransfers())
        it->stop();
}

void KGet::slotTransfersDelete()
{
    foreach(TransferHandler * it, Model::selectedTransfers())
    {
        it->stop();
        Model::delTransfer(it);
    }
}

void KGet::slotTransfersOpenDest()
{
    QStringList openedDirs;
    foreach(TransferHandler * it, Model::selectedTransfers())
    {
        QString directory = it->dest().directory();
        if( !openedDirs.contains( directory ) )
        {
            KToolInvocation::invokeBrowser( directory );
            openedDirs.append( directory );
        }
    }
}

void KGet::slotTransfersShowDetails()
{
    foreach(TransferHandler * it, Model::selectedTransfers())
    {
        m_viewsContainer->showTransferDetails(it);
    }
}

void KGet::slotTransfersCopySourceURL()
{
    foreach(TransferHandler * it, Model::selectedTransfers())
    {
        QString sourceurl = it->source().url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText(sourceurl, QClipboard::Selection);
        cb->setText(sourceurl, QClipboard::Clipboard);
    }
}

void KGet::slotSaveMyself()
{
    // save last parameters ..
    Settings::setMainPosition( pos() );
    // .. and write config to disk
    Settings::writeConfig();
}

void KGet::slotNewToolbarConfig()
{
    createGUI();
}

void KGet::slotNewConfig()
{
    // Update here properties modified in the config dialog and not
    // parsed often by the code. When clicking Ok or Apply of
    // PreferencesDialog, this function is called.

    m_drop->setVisible(Settings::showDropTarget(), false);
    m_showDropTarget->setChecked(Settings::showDropTarget());

    slotKonquerorIntegration(Settings::konquerorIntegration());

    if (Settings::autoPaste())
        clipboardTimer->start(1000);
    else
        clipboardTimer->stop();
    m_AutoPaste->setChecked(Settings::autoPaste());
}

void KGet::slotToggleAutoPaste()
{
    bool autoPaste = !Settings::autoPaste();
    Settings::setAutoPaste( autoPaste );

    if (autoPaste)
        clipboardTimer->start(1000);
    else
        clipboardTimer->stop();
    m_AutoPaste->setChecked(autoPaste);
}

void KGet::slotCheckClipboard()
{
    QString clipData = QApplication::clipboard()->text( QClipboard::Clipboard ).trimmed();

    if (clipData != lastClipboard)
    {
        lastClipboard = clipData;
        if (lastClipboard.isEmpty())
            return;

        KUrl url = KUrl(lastClipboard);

        if (url.isValid() && !url.isLocalFile())
            Model::addTransfer( url );
    }
}

void KGet::slotShowDropTarget()
{
    m_showDropTarget->setChecked( !m_drop->isVisible() );
    m_drop->setVisible( !m_drop->isVisible() );
}

void KGet::slotTrayKonquerorIntegration()
{
    static bool tempIntegration = Settings::konquerorIntegration();
    tempIntegration = !tempIntegration;
    slotKonquerorIntegration(tempIntegration);
    if (!tempIntegration && Settings::konquerorIntegration() && !Settings::expertMode())
    {
        KMessageBox::information(0,
            i18n("KGet has been temporarily disabled as download manager for Konqueror. "
            "If you want to disable it forever, go to Settings->Advanced and disable \"Use "
            "as download manager for Konqueror\"."),
            i18n("Konqueror Integration disabled"),
            "KonquerorIntegrationDisabled");
    }
}

void KGet::slotKonquerorIntegration(bool konquerorIntegration)
{
    KConfig cfgKonqueror("konquerorrc", false, false);
    cfgKonqueror.setGroup("HTML Settings");
    cfgKonqueror.writePathEntry("DownloadManager",QString(konquerorIntegration?"kget":""));
    cfgKonqueror.sync();
    if ( konquerorIntegration )
        m_KonquerorIntegration->setText(i18n("Disable &KGet as Konqueror Download Manager"));
    else
        m_KonquerorIntegration->setText(i18n("Enable &KGet as Konqueror Download Manager"));
}

void KGet::slotShowMenubar()
{
    if(m_menubarAction->isChecked())
        menuBar()->show();
    else
        menuBar()->hide();
}

void KGet::log(const QString & message)
{
    Q_UNUSED(message);
    //The logWindow has been removed. Maybe we could implement 
    //a new one. The old one was used as follows:
    //logWindow->logGeneral(message);
}

/** widget events */

void KGet::closeEvent( QCloseEvent * e )
{
    e->ignore();
    hide();
}

void KGet::dragEnterEvent(QDragEnterEvent * event)
{
    event->setAccepted(KUrl::List::canDecode(event->mimeData())
                  || event->mimeData()->hasText());
}

void KGet::dropEvent(QDropEvent * event)
{
    KUrl::List list = KUrl::List::fromMimeData(event->mimeData());
    QString str;

    if (!list.isEmpty())
        Model::addTransfer(list);
    else
    {
        str = event->mimeData()->text();
        Model::addTransfer(KUrl(str));
    }
}


/** DCOP interface */

void KGet::addTransfers( const KUrl::List& src, const QString& dest)
{
    Model::addTransfer( src, dest );
}

bool KGet::isDropTargetVisible() const
{
    return m_drop->isVisible();
}

void KGet::setDropTargetVisible( bool setVisible )
{
    if ( setVisible != Settings::showDropTarget() )
        m_drop->setVisible( setVisible );
}

void KGet::setOfflineMode( bool offline )
{
    Q_UNUSED(offline);
    //TODO Re-enable this
//     schedRequestOperation( offline ? OpStop : OpRun );
}

bool KGet::isOfflineMode() const
{
    //TODO Re-enable this
//     return scheduler->isRunning();
    return false;
}

#include "kget.moc"

//BEGIN auto-disconnection 
/*
KToggleAction *m_paAutoDisconnect,
    m_paAutoDisconnect =  new KToggleAction(i18n("Auto-&Disconnect Mode"),"connect_creating", 0, this, SLOT(slotToggleAutoDisconnect()), ac, "auto_disconnect");
    tmp = i18n("<b>Auto disconnect</b> button toggles the auto-disconnect\n" "mode on and off.\n" "\n" "When set, KGet will disconnect automatically\n" "after all queued transfers are finished.\n" "\n" "<b>Important!</b>\n" "Also turn on the expert mode when you want KGet\n" "to disconnect without asking.");
    m_paAutoDisconnect->setWhatsThis(tmp);

    if (Settings::connectionType() != Settings::Permanent) {
        //m_paAutoDisconnect->setChecked(Settings::autoDisconnect());
    }
    setAutoDisconnect();

void KGet::slotToggleAutoDisconnect()
{
    Settings::setAutoDisconnect( !Settings::autoDisconnect() );

    if (Settings::autoDisconnect()) {
        log(i18n("Auto disconnect on."));
    } else {
        log(i18n("Auto disconnect off."));
    }
    m_paAutoDisconnect->setChecked(Settings::autoDisconnect());
}

void KGet::setAutoDisconnect()
{
    // disable action when we are connected permanently
    //m_paAutoDisconnect->setEnabled(Settings::connectionType() != Settings::Permanent);
}
*/
//END 

//BEGIN Auto saving transfer list 
/*
    // Setup autosave timer
    QTimer *autosaveTimer;      // timer for autosaving transfer list
    autosaveTimer = new QTimer(this);
    connect(autosaveTimer, SIGNAL(timeout()), SLOT(slotAutosaveTimeout()));
    setAutoSave();

void KGet::slotAutosaveTimeout()
{
    schedRequestOperation( OpExportTransfers );
}

void KGet::setAutoSave()
{
    autosaveTimer->stop();
    if (Settings::autoSave()) {
        autosaveTimer->start(Settings::autoSaveInterval() * 60000);
    }
}
*/
//END 

//BEGIN queue-timer-delay 
/*
//construct actions
    KRadioAction *m_paQueue, *m_paTimer, *m_paDelay;
    m_paQueue = new KRadioAction(i18n("&Queue"),"tool_queue", 0, this, SLOT(slotQueueCurrent()), ac, "queue");
    m_paTimer = new KRadioAction(i18n("&Timer"),"history", 0, this, SLOT(slotTimerCurrent()), ac, "timer");
    m_paDelay = new KRadioAction(i18n("De&lay"),"button_cancel", 0, this, SLOT(slotDelayCurrent()), ac, "delay");
    m_paQueue->setExclusiveGroup("TransferMode");
    m_paTimer->setExclusiveGroup("TransferMode");
    m_paDelay->setExclusiveGroup("TransferMode");
    tmp = i18n("<b>Queued</b> button sets the mode of selected\n" "transfers to <i>queued</i>.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paQueue->setWhatsThis(tmp);
    tmp = i18n("<b>Scheduled</b> button sets the mode of selected\n" "transfers to <i>scheduled</i>.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paTimer->setWhatsThis(tmp);
    tmp = i18n("<b>Delayed</b> button sets the mode of selected\n" "transfers to <i>delayed</i>." "This also causes the selected transfers to stop.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paDelay->setWhatsThis(tmp);

    // Setup transfer timer for scheduled downloads and checkQueue()
    QTimer *transferTimer;      // timer for scheduled transfers
    transferTimer = new QTimer(this);
    connect(transferTimer, SIGNAL(timeout()), SLOT(slotTransferTimeout()));
    transferTimer->start(10000);        // 10 secs time interval

//FOLLOWING CODE WAS IN UPDATEACTIONS
    // disable all signals
    m_paQueue->blockSignals(true);
    m_paTimer->blockSignals(true);
    m_paDelay->blockSignals(true);

    // at first turn off all buttons like when nothing is selected
    m_paQueue->setChecked(false);
    m_paTimer->setChecked(false);
    m_paDelay->setChecked(false);

    m_paQueue->setEnabled(false);
    m_paTimer->setEnabled(false);
    m_paDelay->setEnabled(false);

       if ( L'ITEM E' SELECTED )
       {
            //CONTROLLA SE L'ITEM E' IL PRIMO AD ESSERE SELEZIONATO, ALTRIMENTI
	    //DEVE VEDERE SE TUTTI GLI ALTRI SELECTED HANNO LO STESSO MODO, ALTRIMENTI
	    //DISABILITANO LE AZIONI
            if (item == first_selected_item) {
                if (item->getStatus() != Transfer::ST_FINISHED) {
                    m_paQueue->setEnabled(true);
                    m_paTimer->setEnabled(true);
                    m_paDelay->setEnabled(true);

                    switch (item->getMode()) {
                    case Transfer::MD_QUEUED:
                        m_paQueue->setChecked(true);
                        break;
                    case Transfer::MD_SCHEDULED:
                        m_paTimer->setChecked(true);
                        break;
                    case Transfer::MD_DELAYED:
                        m_paDelay->setChecked(true);
                        break;
                    }
                }
            } else if (item->getMode() != first_item->getMode()) {
                // unset all when all selected items don't have the same mode
                m_paQueue->setChecked(false);
                m_paTimer->setChecked(false);
                m_paDelay->setChecked(false);
                m_paQueue->setEnabled(false);
                m_paTimer->setEnabled(false);
                m_paDelay->setEnabled(false);
            }
       }
    // enale all signals
    m_paQueue->blockSignals(false);
    m_paTimer->blockSignals(false);
    m_paDelay->blockSignals(false);
*/
//END 

//BEGIN use last directory Action 
/*
KToggleAction *m_paExpertMode, *m_paUseLastDir
    m_paUseLastDir     =  new KToggleAction(i18n("&Use-Last-Folder Mode"),"folder", 0, this, SLOT(slotToggleUseLastDir()), ac, "use_last_dir");
    m_paUseLastDir->setWhatsThis(i18n("<b>Use last folder</b> button toggles the\n" "use-last-folder feature on and off.\n" "\n" "When set, KGet will ignore the folder settings\n" "and put all new added transfers into the folder\n" "where the last transfer was put."));
    m_paUseLastDir->setChecked(Settings::useLastDir());

void slotToggleUseLastDir();

void KGet::slotToggleUseLastDir()
{
    Settings::setUseLastDirectory( !Settings::useLastDirectory() );

    if (Settings::useLastDirectory()) {
        log(i18n("Use last folder on."));
    } else {
        log(i18n("Use last folder off."));
    }
}
*/
//END 
