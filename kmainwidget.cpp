/***************************************************************************
*                                kmainwidget.cpp
*                             -------------------
*
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __svr4__
#define map BULLSHIT            // on Solaris it conflicts with STL ?
#include <net/if.h>
#undef map
#include <sys/sockio.h>         // needed for SIOCGIFFLAGS
#else
#include <net/if.h>
#endif

#include <qclipboard.h>
#include <qregexp.h>
#include <qdragobject.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qdropsite.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>
#include <kinputdialog.h>

#include <kprotocolinfo.h>
#include <kfiledialog.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kurl.h>
#include <kurldrag.h>
#include <klocale.h>
#include <kglobal.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <khelpmenu.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kio/netaccess.h>
#include <knotifyclient.h>
#include <knotifydialog.h>
#include <kmenubar.h>
#include <kio/renamedlg.h>

#include "safedelete.h"
#include "settings.h"
#include "transfer.h"
#include "transferlist.h"
#include "kmainwidget.h"
#include "kfileio.h"
#include "dlgPreferences.h"
#include "logwindow.h"
#include "docking.h"
#include "droptarget.h"
#include <assert.h>



#include <kio/authinfo.h>
#include <kio/global.h>
#include <qiconset.h>

#include "version.h"
#include "slave.h"
#include "slaveevent.h"

struct KURLPair
{
	KURL dest;
	KURL src;
};

KMainWidget *kmain = 0L;

#define LOAD_ICON(X) KGlobal::iconLoader()->loadIcon(X, KIcon::MainToolbar)

DropTarget *kdrop = 0L;

Settings ksettings;             // this object contains all settings

static int sockets_open();


// socket constants
int ipx_sock = -1;              /* IPX socket */
int ax25_sock = -1;             /* AX.25 socket */
int inet_sock = -1;             /* INET socket */
int ddp_sock = -1;              /* Appletalk DDP socket */



KMainWidget::KMainWidget(bool bStartDocked)
    : KGetIface( "KGet-Interface" ),
      KMainWindow(0, "kget mainwindow",0),
      prefDlg( 0 ), kdock( 0 )
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    {
        KConfig cfg( "kioslaverc", false, false);
        cfg.setGroup(QString::null);
        cfg.writeEntry("AutoResume", true);
        cfg.sync();
    }

    b_connected = TRUE;
    b_viewLogWindow = FALSE;
    b_viewPreferences = FALSE;

    myTransferList = 0L;
    kmain = this;

    // Set log time, needed for the name of log file
    QDate date = QDateTime::currentDateTime().date();
    QTime time = QDateTime::currentDateTime().time();
    QString tmp;

    tmp.sprintf("log%d:%d:%d-%d:%d:%d", date.day(), date.month(), date.year(), time.hour(), time.minute(), time.second());

    logFileName = locateLocal("appdata", "logs/");
    logFileName += tmp;

    lastClipboard = QApplication::clipboard()->text( QClipboard::Clipboard ).stripWhiteSpace();
    // Load all settings from KConfig
    ksettings.load();

    // Setup log window
    logWindow = new LogWindow();

    m_showDropTarget = false;

    setCaption(KGETVERSION);

    setupGUI();
    setupWhatsThis();

    log(i18n("Welcome to KGet"));

    setCentralWidget(myTransferList);

    connect(kapp, SIGNAL(saveYourself()), SLOT(slotSaveYourself()));

    // Enable dropping
    setAcceptDrops(true);

    // Setup connection timer
    connectionTimer = new QTimer(this);
    connect(connectionTimer, SIGNAL(timeout()), SLOT(slotCheckConnection()));

    // setup socket for checking connection
    if ((_sock = sockets_open()) < 0) {
        log(i18n("Could not create valid socket"), false);
    } else {
        connectionTimer->start(5000);   // 5 second interval for checking connection
    }

    checkOnline();
    ksettings.b_offline=( !b_connected || ksettings.b_offlineMode );

    // Setup animation timer
    animTimer = new QTimer(this);
    animCounter = 0;
    connect(animTimer, SIGNAL(timeout()), SLOT(slotAnimTimeout()));

    if (ksettings.b_useAnimation) {
        animTimer->start(400);
    } else {
        animTimer->start(1000);
    }

    // Setup transfer timer for scheduled downloads and checkQueue()
    transferTimer = new QTimer(this);
    connect(transferTimer, SIGNAL(timeout()), SLOT(slotTransferTimeout()));
    transferTimer->start(10000);        // 10 secs time interval

    // Setup autosave timer
    autosaveTimer = new QTimer(this);
    connect(autosaveTimer, SIGNAL(timeout()), SLOT(slotAutosaveTimeout()));
    setAutoSave();

    // Setup clipboard timer
    clipboardTimer = new QTimer(this);
    connect(clipboardTimer, SIGNAL(timeout()), SLOT(slotCheckClipboard()));
    if (ksettings.b_autoPaste) {
        clipboardTimer->start(1000);
    }

    readTransfers();

    // Setup special windows
    kdrop = new DropTarget(this);
    kdock = new DockWidget(this);
    connect(kdock, SIGNAL(quitSelected()), SLOT(slotQuit()));

    // Set geometry
    if (ksettings.mainPosition.x() != -1) {
        resize(ksettings.mainSize);
        move(ksettings.mainPosition);
        KWin::setState(winId(), ksettings.mainState);
    } else {
        resize(650, 180);
    }

    // update actions
    m_paUseAnimation->setChecked(ksettings.b_useAnimation);
    m_paExpertMode->setChecked(ksettings.b_expertMode);
    m_paUseLastDir->setChecked(ksettings.b_useLastDir);

    if (ksettings.connectionType != PERMANENT) {
        m_paAutoDisconnect->setChecked(ksettings.b_autoDisconnect);
    }
    setAutoDisconnect();

    m_paAutoShutdown->setChecked(ksettings.b_autoShutdown);


    m_paOfflineMode->setChecked(ksettings.b_offline);
    if (!ksettings.b_offlineMode) 
        m_paOfflineMode->setIconSet(LOAD_ICON("tool_offline_mode_on"));

    if (ksettings.b_offline) {
        setCaption(i18n("Offline"), false);
        log(i18n("Starting offline"));
    } else
        setCaption(QString::null, false);
    
    m_paAutoPaste->setChecked(ksettings.b_autoPaste);
    m_paShowLog->setChecked(b_viewLogWindow);

    if (!bStartDocked && ksettings.b_showMain)
        show();

    kdock->show();

    KNotifyClient::startDaemon();

    setStandardToolBarMenuEnabled(true);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


KMainWidget::~KMainWidget()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    delete prefDlg;
    delete kdrop;
    writeTransfers();
    writeLog();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
    delete logWindow;
}


void KMainWidget::log(const QString & message, bool statusbar)
{
#ifdef _DEBUG
    sDebugIn <<" message= "<< message << endl;
#endif

    logWindow->logGeneral(message);

    if (statusbar) {
        statusBar()->message(message, 1000);
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotSaveYourself()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    writeTransfers();
    ksettings.save();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::setupGUI()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    // setup transfer list
    myTransferList = new TransferList(this, "transferList");
    myTransferList->setSorting(-1);
    setListFont();

    KActionCollection *coll = actionCollection();

    connect(myTransferList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateActions()));
    connect(myTransferList, SIGNAL(transferSelected(Transfer *)), this, SLOT(slotOpenIndividual()));
    connect(myTransferList, SIGNAL(popupMenu(Transfer *)), this, SLOT(slotPopupMenu(Transfer *)));

    // file actions
    m_paOpenTransfer = KStdAction::open(this, SLOT(slotOpenTransfer()), coll, "open_transfer");
    m_paPasteTransfer = KStdAction::paste(this, SLOT(slotPasteTransfer()), coll, "paste_transfer");

    m_paExportTransfers = new KAction(i18n("&Export Transfer List..."), 0, this, SLOT(slotExportTransfers()), coll, "export_transfers");
    m_paImportTransfers = new KAction(i18n("&Import Transfer List..."), 0, this, SLOT(slotImportTransfers()), coll, "import_transfers");

    m_paImportText = new KAction(i18n("Import Text &File..."), 0, this, SLOT(slotImportTextFile()), coll, "import_text");

    m_paQuit = KStdAction::quit(this, SLOT(slotQuit()), coll, "quit");


    // transfer actions
    m_paCopy = new KAction(i18n("&Copy URL to Clipboard"), 0, this, SLOT(slotCopyToClipboard()), coll, "copy_url");
    m_paIndividual = new KAction(i18n("&Open Individual Window"), 0, this, SLOT(slotOpenIndividual()), coll, "open_individual");

    m_paMoveToBegin = new KAction(i18n("Move to &Beginning"), 0, this, SLOT(slotMoveToBegin()), coll, "move_begin");

    m_paMoveToEnd = new KAction(i18n("Move to &End"), 0, this, SLOT(slotMoveToEnd()), coll, "move_end");
#ifdef _DEBUG
    sDebug << "Loading pics" << endl;
#endif
    m_paResume = new KAction(i18n("&Resume"),"tool_resume", 0, this, SLOT(slotResumeCurrent()), coll, "resume");
    m_paPause = new KAction(i18n("&Pause"),"tool_pause", 0, this, SLOT(slotPauseCurrent()), coll, "pause");
    m_paDelete = new KAction(i18n("&Delete"),"editdelete", Qt::Key_Delete, this, SLOT(slotDeleteCurrent()), coll, "delete");
    m_paRestart = new KAction(i18n("Re&start"),"tool_restart", 0, this, SLOT(slotRestartCurrent()), coll, "restart");

    m_paQueue = new KRadioAction(i18n("&Queue"),"tool_queue", 0, this, SLOT(slotQueueCurrent()), coll, "queue");
    m_paTimer = new KRadioAction(i18n("&Timer"),"tool_timer", 0, this, SLOT(slotTimerCurrent()), coll, "timer");
    m_paDelay = new KRadioAction(i18n("De&lay"),"tool_delay", 0, this, SLOT(slotDelayCurrent()), coll, "delay");

    m_paQueue->setExclusiveGroup("TransferMode");
    m_paTimer->setExclusiveGroup("TransferMode");
    m_paDelay->setExclusiveGroup("TransferMode");

    // options actions
    m_paUseAnimation   =  new KToggleAction(i18n("Use &Animation"), 0, this, SLOT(slotToggleAnimation()), coll, "toggle_animation");
    m_paExpertMode     =  new KToggleAction(i18n("&Expert Mode"),"tool_expert", 0, this, SLOT(slotToggleExpertMode()), coll, "expert_mode");
    m_paUseLastDir     =  new KToggleAction(i18n("&Use-Last-Folder Mode"),"tool_uselastdir", 0, this, SLOT(slotToggleUseLastDir()), coll, "use_last_dir");
    m_paAutoDisconnect =  new KToggleAction(i18n("Auto-&Disconnect Mode"),"tool_disconnect", 0, this, SLOT(slotToggleAutoDisconnect()), coll, "auto_disconnect");
    m_paAutoShutdown   =  new KToggleAction(i18n("Auto-S&hutdown Mode"), "tool_shutdown", 0, this, SLOT(slotToggleAutoShutdown()), coll, "auto_shutdown");
    m_paOfflineMode    =  new KToggleAction(i18n("&Offline Mode"),"tool_offline_mode_off", 0, this, SLOT(slotToggleOfflineMode()), coll, "offline_mode");
    m_paAutoPaste      =  new KToggleAction(i18n("Auto-Pas&te Mode"),"tool_clipboard", 0, this, SLOT(slotToggleAutoPaste()), coll, "auto_paste");

    m_paPreferences    =  KStdAction::preferences(this, SLOT(slotPreferences()), coll);

    KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()), coll);
    KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), coll);
    KStdAction::configureNotifications(this, SLOT(slotConfigureNotifications()), coll);

    m_menubarAction = KStdAction::showMenubar(this, SLOT(slotShowMenubar()), coll, "settings_showmenubar" );
    m_menubarAction->setChecked( !menuBar()->isHidden() );

    // view actions
    createStandardStatusBarAction();

    m_paShowLog      = new KToggleAction(i18n("Show &Log Window"),"tool_logwindow", 0, this, SLOT(slotToggleLogWindow()), coll, "toggle_log");
    m_paShowLog->setCheckedState(i18n("Hide &Log Window"));
    m_paDropTarget   = new KAction(i18n("Show Drop &Target"),"tool_drop_target", 0, this, SLOT(slotToggleDropTarget()), coll, "drop_target");
    m_paKonquerorIntegration = new KAction(i18n("Enable &KGet as Konqueror Download Manager"), "konqueror", 0, this, SLOT(slotKonquerorIntegration()), coll, "konqueror_integration");
    if (ksettings.b_KonquerorIntegration) {
        m_paKonquerorIntegration->setText(i18n("Disable &KGet as Konqueror Download Manager"));
    }

    menuHelp = new KHelpMenu(this, KGlobal::instance()->aboutData());
    KStdAction::whatsThis(menuHelp, SLOT(contextHelpActivated()), coll, "whats_this");

    createGUI("kgetui.rc");

    // setup statusbar
    statusBar()->insertFixedItem(i18n(" Transfers: %1 ").arg(99), ID_TOTAL_TRANSFERS);
    statusBar()->insertFixedItem(i18n(" Files: %1 ").arg(555), ID_TOTAL_FILES);
    statusBar()->insertFixedItem(i18n(" Size: %1 KB ").arg("134.56"), ID_TOTAL_SIZE);
    statusBar()->insertFixedItem(i18n(" Time: %1 ").arg(KIO::convertSeconds(0)), ID_TOTAL_TIME);
    statusBar()->insertFixedItem(i18n(" %1 KB/s ").arg("123.34"), ID_TOTAL_SPEED);

    setAutoSaveSettings( "MainWindow", false /*Settings takes care of size & pos & state */ );

    slotUpdateActions();

    updateStatusBar();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::setupWhatsThis()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    QString tmp;

    tmp = i18n("<b>Resume</b> button starts selected transfers\n" "and sets their mode to <i>queued</i>.");
    m_paResume->setWhatsThis(tmp);

    tmp = i18n("<b>Pause</b> button stops selected transfers\n" "and sets their mode to <i>delayed</i>.");
    m_paPause->setWhatsThis(tmp);

    tmp = i18n("<b>Delete</b> button removes selected transfers\n" "from the list.");
    m_paDelete->setWhatsThis(tmp);

    tmp = i18n("<b>Restart</b> button is a convenience button\n" "that simply does Pause and Resume.");
    m_paRestart->setWhatsThis(tmp);

    tmp = i18n("<b>Queued</b> button sets the mode of selected\n" "transfers to <i>queued</i>.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paQueue->setWhatsThis(tmp);

    tmp = i18n("<b>Scheduled</b> button sets the mode of selected\n" "transfers to <i>scheduled</i>.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paTimer->setWhatsThis(tmp);

    tmp = i18n("<b>Delayed</b> button sets the mode of selected\n" "transfers to <i>delayed</i>." "This also causes the selected transfers to stop.\n" "\n" "It is a radio button -- you can choose between\n" "three modes.");
    m_paDelay->setWhatsThis(tmp);

    tmp = i18n("<b>Preferences</b> button opens a preferences dialog\n" "where you can set various options.\n" "\n" "Some of these options can be more easily set using the toolbar.");
    m_paPreferences->setWhatsThis(tmp);

    tmp = i18n("<b>Log window</b> button opens a log window.\n" "The log window records all program events that occur\n" "while KGet is running.");
    m_paShowLog->setWhatsThis(tmp);

    tmp = i18n("<b>Paste transfer</b> button adds a URL from\n" "the clipboard as a new transfer.\n" "\n" "This way you can easily copy&paste URLs between\n" "applications.");
    m_paPasteTransfer->setWhatsThis(tmp);

    tmp = i18n("<b>Expert mode</b> button toggles the expert mode\n" "on and off.\n" "\n" "Expert mode is recommended for experienced users.\n" "When set, you will not be \"bothered\" by confirmation\n" "messages.\n" "<b>Important!</b>\n" "Turn it on if you are using auto-disconnect or\n" "auto-shutdown features and you want KGet to disconnect \n" "or shut down without asking.");
    m_paExpertMode->setWhatsThis(tmp);

    tmp = i18n("<b>Use last folder</b> button toggles the\n" "use-last-folder feature on and off.\n" "\n" "When set, KGet will ignore the folder settings\n" "and put all new added transfers into the folder\n" "where the last transfer was put.");
    m_paUseLastDir->setWhatsThis(tmp);

    tmp = i18n("<b>Auto disconnect</b> button toggles the auto-disconnect\n" "mode on and off.\n" "\n" "When set, KGet will disconnect automatically\n" "after all queued transfers are finished.\n" "\n" "<b>Important!</b>\n" "Also turn on the expert mode when you want KGet\n" "to disconnect without asking.");
    m_paAutoDisconnect->setWhatsThis(tmp);

    tmp = i18n("<b>Auto shutdown</b> button toggles the auto-shutdown\n" "mode on and off.\n" "\n" "When set, KGet will quit automatically\n" "after all queued transfers are finished.\n" "<b>Important!</b>\n" "Also turn on the expert mode when you want KGet\n" "to quit without asking.");
    m_paAutoShutdown->setWhatsThis(tmp);

    tmp = i18n("<b>Offline mode</b> button toggles the offline mode\n" "on and off.\n" "\n" "When set, KGet will act as if it was not connected\n" "to the Internet.\n" "\n" "You can browse offline, while still being able to add\n" "new transfers as queued.");
    m_paOfflineMode->setWhatsThis(tmp);

    tmp = i18n("<b>Auto paste</b> button toggles the auto-paste mode\n" "on and off.\n" "\n" "When set, KGet will periodically scan the clipboard\n" "for URLs and paste them automatically.");
    m_paAutoPaste->setWhatsThis(tmp);

    tmp = i18n("<b>Drop target</b> button toggles the window style\n" "between a normal window and a drop target.\n" "\n" "When set, the main window will be hidden and\n" "instead a small shaped window will appear.\n" "\n" "You can show/hide a normal window with a simple click\n" "on a shaped window.");
    m_paDropTarget->setWhatsThis(tmp);
    /*
        tmp = i18n("<b>Dock widget</b> button toggles the window style\n" "between a normal window and a docked widget.\n" "\n" "When set, the main window will be hidden and\n" "instead a docked widget will appear on the panel.\n" "\n" "You can show/hide a normal window by simply clicking\n" "on a docked widget.");
        m_paDockWindow->setWhatsThis(tmp);

        tmp = i18n("<b>Normal window</b> button sets\n" "\n" "the window style to normal window");
        m_paNormal->setWhatsThis(tmp);
      */


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotConfigureToolbars()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    saveMainWindowSettings( KGlobal::config(), "MainWindow" );
    KEditToolbar edit(factory());
    connect(&edit, SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
    edit.exec();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotNewToolbarConfig()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    createGUI("kgetui.rc");
    applyMainWindowSettings( KGlobal::config(), "MainWindow" );

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotImportTextFile()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    QString tmpFile;
    QString list;
    int i, j;

    KURL filename = KFileDialog::getOpenURL(ksettings.lastDirectory);
    if (!filename.isValid())
        return;

    if (KIO::NetAccess::download(filename, tmpFile, this)) {
        list = kFileToString(tmpFile);
        KIO::NetAccess::removeTempFile(tmpFile);
    } else
        list = kFileToString(filename.path()); // file not accessible -> give error message

    i = 0;
    while ((j = list.find('\n', i)) != -1) {
        QString newtransfer = list.mid(i, j - i);
        addTransfer(newtransfer);
        i = j + 1;
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotImportTransfers()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    readTransfers(true);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::readTransfers(bool ask_for_name)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    KURL url;

    if (ask_for_name)
        url = KFileDialog::getOpenURL(ksettings.lastDirectory, i18n("*.kgt|*.kgt\n*|All Files"));
    else
        url.setPath( locateLocal("appdata", "transfers.kgt") );

    readTransfersEx(url);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::readTransfersEx(const KURL & file)
{
#ifdef _DEBUG
  sDebugIn << endl;
#endif

    if (!file.isValid()) {

#ifdef _DEBUG
        sDebugOut<< " string empty" << endl;
#endif
        return;
    }
#ifdef _DEBUG
    sDebug << "Read from file: " << file << endl;
#endif
    myTransferList->readTransfers(file);
    checkQueue();
    slotTransferTimeout();
    myTransferList->clearSelection();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotExportTransfers()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    writeTransfers(true);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::writeTransfers(bool ask_for_name)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    QString str;
    QString txt;

    if (ask_for_name)
        txt = KFileDialog::getSaveFileName(ksettings.lastDirectory, i18n("*.kgt|*.kgt\n*|All Files"));
    else
        txt = locateLocal("appdata", "transfers.kgt");



    if (txt.isEmpty())
    {
#ifdef _DEBUG
        sDebugOut<< " because Destination File name isEmpty"<< endl;
#endif
        return;
    }
    if (!txt.endsWith(".kgt"))
        txt += ".kgt";

#ifdef _DEBUG
    sDebug << "Writing transfers " << txt << endl;
#endif

    myTransferList->writeTransfers(txt);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::writeLog()
{
#ifdef _DEBUG
    sDebugIn << "Writing log to file : " << logFileName.ascii() << endl;
#endif


    kCStringToFile(logWindow->getText().local8Bit(), logFileName, false, false);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotQuit()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    Transfer *item;
    TransferIterator it(myTransferList);

    log(i18n("Quitting..."));

    for (; it.current(); ++it) {
        item = it.current();
        if (item->getStatus() == Transfer::ST_RUNNING && !ksettings.b_expertMode) {
            if (KMessageBox::warningContinueCancel(this, i18n("Some transfers are still running.\nAre you sure you want to quit KGet?"), i18n("Warning"), KStdGuiItem::quit()) != KMessageBox::Continue) {
#ifdef _DEBUG
                sDebugOut << endl;
#endif

                return;
            }
            break;
        }
    }

    ksettings.save();

#ifdef _DEBUG
    sDebugOut << endl;
#endif

    kapp->quit();
}


void KMainWidget::slotResumeCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(myTransferList);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotResume();
    slotUpdateActions();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotPauseCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(myTransferList);

    m_paPause->setEnabled(false);
    m_paRestart->setEnabled(false);
    update();

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestPause();
    slotUpdateActions();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}



void KMainWidget::slotRestartCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(myTransferList);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestRestart();
    slotUpdateActions();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotDeleteCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    m_paDelete->setEnabled(false);
    m_paPause->setEnabled(false);
    update();
    TransferIterator it(myTransferList);
    QValueList<QGuardedPtr<Transfer> > selectedItems;
    QStringList itemNames;

    while (it.current()) {
        if (it.current()->isSelected()) {
            selectedItems.append( QGuardedPtr<Transfer>( it.current() ));
            itemNames.append( it.current()->getSrc().prettyURL() );
        }
        ++it;
    }

    if (!ksettings.b_expertMode)
    {
        if ( selectedItems.count() > 1 )
        {
            if (KMessageBox::warningContinueCancelList(this, i18n("Are you sure you want to delete these transfers?"),
                                               itemNames, i18n("Question"),
                                               KStdGuiItem::del(),
                                               QString("multiple_delete_transfer"))
                != KMessageBox::Continue)
                return; // keep 'em
        }
        else
        {
            if (KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to delete this transfer?"),
                                           i18n("Question"), KStdGuiItem::del(),
                                           QString("delete_transfer"))
                != KMessageBox::Continue)
                return;
        }
    }

    // If we reach this, we want to delete all selected transfers
    // Some of them might have finished in the meantime tho. Good that
    // we used a QGuardedPtr :)

    int transferFinishedMeanwhile = 0;
    QValueListConstIterator<QGuardedPtr<Transfer> > lit = selectedItems.begin();;
    while ( lit != selectedItems.end() )
    {
        if ( *lit )
            (*lit)->slotRequestRemove();
        else
            ++transferFinishedMeanwhile;

        ++lit;
    }

    checkQueue(); // needed !

    if ( !ksettings.b_expertMode && transferFinishedMeanwhile > 0 )
        KMessageBox::information(this, i18n("The transfer you wanted to delete completed before it could be deleted.",
                                            "%n transfers you wanted to delete completed before they could be deleted.",
                                            transferFinishedMeanwhile ),
                                 QString::null, "completedBeforeDeletion" );

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::stopAll()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    log(i18n("Stopping all jobs"), false);

    TransferIterator it(myTransferList);
    Transfer::TransferStatus Status;
    for (; it.current(); ++it) {
        Status = it.current()->getStatus();
        if (Status == Transfer::ST_TRYING || Status == Transfer::ST_RUNNING)
            it.current()->slotStop();
    }
    slotUpdateActions();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotQueueCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
        if (it.current()->isSelected()) {
            it.current()->slotQueue();
        }
    }

    //    myTransferList->clearSelection();
    slotUpdateActions();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotTimerCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif


    TransferIterator it(myTransferList);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestSchedule();

#ifdef _DEBUG
    sDebugOut << endl;
#endif

}


void KMainWidget::slotDelayCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(myTransferList);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestDelay();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotOpenTransfer()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    QString newtransfer;
    bool ok = false;

#ifdef _DEBUG
    //      newtransfer = "ftp://localhost/home/pch/test.gz";
    //      newtransfer = "http://www.kernel.org/pub/linux/kernel/v2.4/linux-2.4.18.tar.gz";
    newtransfer = "ftp://darkmoon/pub/test.gz";
#endif

    while (!ok) {
        newtransfer = KInputDialog::getText(i18n("Open Transfer"), i18n("Open transfer:"), newtransfer, &ok, this);

        // user presses cancel
        if (!ok) {
            return;
        }

        KURL url = KURL::fromPathOrURL(newtransfer);

        if (!url.isValid()) {
            KMessageBox::error(this, i18n("Malformed URL:\n%1").arg(newtransfer), i18n("Error"));
            ok = false;
        }
    }

    addTransfer(newtransfer);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}



void KMainWidget::slotCheckClipboard()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    QString clipData = QApplication::clipboard()->text( QClipboard::Clipboard ).stripWhiteSpace();

    if (clipData != lastClipboard) {
        sDebug << "New clipboard event" << endl;

        lastClipboard = clipData;
        if ( lastClipboard.isEmpty() )
            return;

        KURL url = KURL::fromPathOrURL( lastClipboard );

        if (url.isValid() && !url.isLocalFile())
            slotPasteTransfer();
    }

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}


void KMainWidget::slotPasteTransfer()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    QString newtransfer;

    newtransfer = QApplication::clipboard()->text();
    newtransfer = newtransfer.stripWhiteSpace();

    if (!ksettings.b_expertMode) {
        bool ok = false;
        newtransfer = KInputDialog::getText(i18n("Open Transfer"), i18n("Open transfer:"), newtransfer, &ok, this);

        if (!ok) {
            // cancelled
#ifdef _DEBUG
            sDebugOut << endl;
#endif
            return;
        }

    }

    if (!newtransfer.isEmpty())
        addTransfer(newtransfer);


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

// destFile must be a filename, not a directory! And it will be deleted, if
// it exists already, without further notice.
void KMainWidget::addTransferEx(const KURL& url, const KURL& destFile)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if ( !sanityChecksSuccessful( url ) )
        return;

    KURL destURL = destFile;

    // Malformed destination url means one of two things.
    // 1) The URL is empty.
    // 2) The URL is only a filename, like a default (suggested) filename.
    if ( !destURL.isValid() )
    {
        // Setup destination
        QString destDir = getSaveDirectoryFor( url.fileName() );
        bool b_expertMode = ksettings.b_expertMode;
        bool bDestisMalformed = true;

        while (bDestisMalformed)
        {
            if (!b_expertMode) {
                // open the filedialog for confirmation
                KFileDialog dlg( destDir, QString::null,
                                  0L, "save_as", true);
                dlg.setCaption(i18n("Save As"));
                dlg.setOperationMode(KFileDialog::Saving);

                // Use the destination name if not empty...
                if (destURL.isEmpty())
                    dlg.setSelection(url.fileName());
                else
                    dlg.setSelection(destURL.url());

                if ( dlg.exec() == QDialog::Rejected )
                {
#ifdef _DEBUG
                    sDebugOut << endl;
#endif
                    return;
                }
                else
                {
                    destURL = dlg.selectedURL();
                    ksettings.lastDirectory = destURL.directory();
                }
            }
            else {
                // in expert mode don't open the filedialog
                // if destURL is not empty, it's the suggested filename
                destURL = KURL::fromPathOrURL( destDir + "/" +
                                               ( destURL.isEmpty() ?
                                                   url.fileName() : destURL.url() ));
            }

            //check if destination already exists
            if(KIO::NetAccess::exists(destURL, false, this))
            {
                if (KMessageBox::warningYesNo(this,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL()), QString::null, i18n("Overwrite"), i18n("Do Not Overwrite") )
                                              == KMessageBox::Yes)
                {
                    bDestisMalformed=false;
                    SafeDelete::deleteFile( destURL );
                }
                else
                {
                    b_expertMode=false;
                    ksettings.lastDirectory = destURL.directory();
                }
            }
            else
                bDestisMalformed=false;
        }
    }

    else // destURL was given, check whether the file exists already
    {
        // simply delete it, the calling process should have asked if you
        // really want to delete (at least khtml does)
        if(KIO::NetAccess::exists(destURL, false, this))
            SafeDelete::deleteFile( destURL );
    }

    // create a new transfer item
    Transfer *item = myTransferList->addTransfer(url, destURL);
    KNotifyClient::event(kdock->winId(), "added", i18n("<i>%1</i> has been added.").arg(url.prettyURL()));
    item->updateAll(); // update the remaining fields

    if (ksettings.b_showIndividual)
        item->showIndividual();

    myTransferList->clearSelection();

    checkQueue();
#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::addTransfers( const KURL::List& src, const QString& destDir )
{
    QValueList<KURLPair> urls_orig;

    for ( KURL::List::ConstIterator it = src.begin(); it != src.end(); ++it )
    {
        KURLPair url;
        url.src = *it;
        if ( url.src.fileName().endsWith( ".kgt" ) )
            readTransfersEx(url.src);
        else
            urls_orig.append( url );
    }

    if ( urls_orig.isEmpty() )
        return;

    if ( urls_orig.count() == 1 ) // just one file -> ask for filename
    {
        KURL destFile;

        if ( !destDir.isEmpty() )
        {
            // create a proper destination file from destDir
            KURL destURL = KURL::fromPathOrURL( destDir );
            QString fileName = urls_orig.first().src.fileName();

            // in case the fileName is empty, we simply ask for a filename in
            // addTransferEx. Do NOT attempt to use an empty filename, that
            // would be a directory (and we don't want to overwrite that!)
            if ( !fileName.isEmpty() )
            {
                destURL.adjustPath( +1 );
                destURL.setFileName( fileName );
                if(KIO::NetAccess::exists(destURL, false, this))
                {
                    if (KMessageBox::warningYesNo(this,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL()), QString::null, i18n("Overwrite"), i18n("Do Not Overwrite") )
                        == KMessageBox::Yes)
                    {
                        SafeDelete::deleteFile( destURL );
                        destFile = destURL;
                    }
                }
                else 
                {
                  destFile = destURL;
                }
            }
        }

        addTransferEx( urls_orig.first().src, destFile );
        return;
    }

    // multiple files -> ask for directory, not for every single filename
	
	bool dir_accepted = false;
	QValueList<KURLPair>::Iterator it;
	QValueList<KURLPair> urls;
	KURL::List urlsToDelete;
	while ( !dir_accepted )
	{
		urlsToDelete.clear();
		urls = urls_orig;	// copy the list here, urls might be changed, yet when we return here (Cancel),
							// we want to start again with the original list 
		dir_accepted = true; //Set to false later when Cancel is pressed
		KURL dest;
		if ( destDir.isEmpty() || !QFileInfo( destDir ).isDir() )
		{
			if ( !destDir.isEmpty()  )
				dest.setPath( destDir );
			else
				dest.setPath( getSaveDirectoryFor( src.first().fileName() ) );

            // ask in any case, when destDir is empty
			if ( destDir.isEmpty() || !QFileInfo( dest.path() ).isDir() )
			{
				QString dir = KFileDialog::getExistingDirectory( dest.path() );
				if ( dir.isEmpty() ) // aborted
					return;

				dest.setPath( dir );
				ksettings.lastDirectory = dir;
			}
		}

        // dest is now finally the real destination directory for all the files
		dest.adjustPath(+1);

        // create new transfer items
		bool skip_all = false;
		bool overwrite_all = false;
		it = urls.begin();
		KIO::RenameDlg_Result result;
		while ( it != urls.end() )
		{
			
			if ( !sanityChecksSuccessful( (*it).src ) )
			{
				it = urls.erase( it );
				continue; // shouldn't we notify the user??
			}
			
			(*it).dest = dest;
			QString fileName = (*it).src.fileName();
			if ( fileName.isEmpty() ) // simply use the full url as filename
				fileName = KURL::encode_string_no_slash( (*it).src.prettyURL() );

			(*it).dest.setFileName( fileName );

			if( KIO::NetAccess::exists((*it).dest, false, this))
			{
				QString newdest;
				if (skip_all)
					result = KIO::R_SKIP;
				else if( overwrite_all )
					result = KIO::R_OVERWRITE;
				else
				{
					QFileInfo finfo( (*it).dest.path() );
					QString caption = i18n( "File Already exists" ) + " - KGet";
					result = KIO::open_RenameDlg( caption, (*it).src.url(), (*it).dest.url(), KIO::RenameDlg_Mode(KIO::M_OVERWRITE|KIO::M_SKIP|KIO::M_MULTI), newdest, (KIO::filesize_t) -1, (KIO::filesize_t)finfo.size(), (time_t) -1, (time_t) -1, (time_t) -1, finfo.lastModified().toTime_t());
				}
				switch (result)
				{
					case KIO::R_RENAME:
						(*it).dest = KURL::fromPathOrURL( newdest );
						break;
					case KIO::R_OVERWRITE_ALL:
						overwrite_all = true; //fall through
					case KIO::R_OVERWRITE:
						urlsToDelete.append( (*it).dest );
						break;
					case KIO::R_AUTO_SKIP:
						skip_all = true;	
					case KIO::R_SKIP:			//fall through
						it = urls.erase( it );
						continue;
						break;
					default:               // Cancel, ask again for directory
						dir_accepted = false;
				}
				if ( !dir_accepted )
					break;
			}   // if(KIO::NetAccess::exists
			++it;
		} //  while ( it != urls.end() )
	} //  while ( !dir_accepted )

	KURL::List::Iterator it_1 = urlsToDelete.begin();
	for ( ; it_1 != urlsToDelete.end(); ++it_1 )
	{
		SafeDelete::deleteFile( *it_1 );
	}
	
	int numdl = 0;
	it = urls.begin();
	for ( ; it != urls.end(); ++it )
	{
		Transfer *item = myTransferList->addTransfer((*it).src, (*it).dest);
		item->updateAll(); // update the remaining fields
		numdl++;
	}

    KNotifyClient::event(kdock->winId(), "added", i18n("1 download has been added.", "%n downloads have been added.", numdl));

    myTransferList->clearSelection();

    checkQueue();
}

void KMainWidget::addTransfer(const QString& src)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if ( src.isEmpty() )
        return;

    addTransferEx( KURL::fromPathOrURL( src ) );

#ifdef _DEBUG
    sDebugOut << endl;
#endif

}


void KMainWidget::checkQueue()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    uint numRun = 0;
    int status;
    Transfer *item;
    TransferIterator it(myTransferList);

    if (!ksettings.b_offline) {

        // count running transfers
        for (; it.current(); ++it) {
            status = it.current()->getStatus();
            if (status == Transfer::ST_RUNNING || status == Transfer::ST_TRYING)
                numRun++;
        }
        sDebug << "Found " << numRun << " Running Jobs" << endl;
        it.reset();
        bool isRunning;
        bool isQuequed;
        for (; it.current() && numRun < ksettings.maxSimultaneousConnections; ++it) {
            item = it.current();
            isRunning = (item->getStatus() == Transfer::ST_RUNNING) || ((item->getStatus() == Transfer::ST_TRYING));

            isQuequed = (item->getMode() == Transfer::MD_QUEUED || item->getMode() == Transfer::MD_NEW);

            if     (!isRunning && isQuequed && !ksettings.b_offline)
            {
                log(i18n("Starting another queued job."));
                item->slotResume();
                numRun++;
            }
        }

        slotUpdateActions();

        updateStatusBar();

//    } else {//TODO this has to be solved different
//        log(i18n("Cannot continue offline status"));
    }

    it.reset();
    for (; it.current(); ++it)
    {
        item = it.current();
        if (item->getMode() == Transfer::MD_NEW && item->getStatus() == Transfer::ST_STOPPED)
        {
            item->checkCache();
        }
    }
#ifdef _DEBUG
    sDebugOut << endl;
#endif

}


void KMainWidget::slotAnimTimeout()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    bool isTransfer;

    animCounter++;
    if (animCounter == myTransferList->getPhasesNum()) {
       //updateStatusBar();
        animCounter = 0;
    }
    // update status of all items of transferList
    isTransfer = myTransferList->updateStatus(animCounter);

    //if (this->isVisible()) {
        updateStatusBar();
    //}
#ifdef _DEBUG
    //sDebugOut << endl;
#endif

}


void KMainWidget::slotTransferTimeout()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    Transfer *item;
    TransferIterator it(myTransferList);

    bool flag = false;

    for (; it.current(); ++it) {
        item = it.current();
        if (item->getMode() == Transfer::MD_SCHEDULED && item->getStartTime() <= QDateTime::currentDateTime()) {
            item->setMode(Transfer::MD_QUEUED);
            flag = true;
        }
    }

    if (flag) {
        checkQueue();
    }

    if (ksettings.b_autoDisconnect && ksettings.b_timedDisconnect && ksettings.disconnectTime <= QTime::currentTime() && ksettings.disconnectDate == QDate::currentDate()) {
        onlineDisconnect();
    }

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}


void KMainWidget::slotAutosaveTimeout()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    writeTransfers();

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}


void KMainWidget::slotStatusChanged(Transfer * item, int _operation)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    switch (_operation) {

    case Transfer::OP_FINISHED:
    {
        QString srcurl = item->getSrc().prettyURL();
        if (ksettings.b_removeOnSuccess && !item->keepDialogOpen() )
        {
            delete item;
            item = 0L;
        }
        else
            item->setMode(Transfer::MD_NONE);

        if (!myTransferList->areTransfersQueuedOrScheduled()) {
            // no items or only delayed and finished items in the TransferList
            if (ksettings.b_autoDisconnect)
                onlineDisconnect();

            if (ksettings.b_autoShutdown) {
                slotQuit();
                return;
            }
            KNotifyClient::event(kdock->winId(), "finishedall", i18n("All the downloads are finished."));
        }
        else
        {
            KNotifyClient::event(kdock->winId(), "finished", i18n("<i>%1</i> successfully downloaded.").arg(srcurl));
        }

        if ( item )
            item->slotUpdateActions();

        break;
    }
    case Transfer::OP_RESUMED:
        slotUpdateActions();
        item->slotUpdateActions();
        //                play(ksettings.audioStarted);
        break;
    case Transfer::OP_PAUSED:
        break;
    case Transfer::OP_REMOVED:
        delete item;
        return;                 // checkQueue() will be called only once after all deletions

    case Transfer::OP_ABORTED:
        break;
    case Transfer::OP_DELAYED:
    case Transfer::OP_QUEUED:
        slotUpdateActions();
        item->slotUpdateActions();
        break;
    case Transfer::OP_SCHEDULED:
        slotUpdateActions();
        item->slotUpdateActions();
        slotTransferTimeout();  // this will check schedule times
        return;                 // checkQueue() is called from slotTransferTimeout()
    }
    checkQueue();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::dragEnterEvent(QDragEnterEvent * event)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    event->accept(KURLDrag::canDecode(event) || QTextDrag::canDecode(event));

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::dropEvent(QDropEvent * event)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    KURL::List list;
    QString str;

    if (KURLDrag::decode(event, list)) {
        addTransfers(list);
    } else if (QTextDrag::decode(event, str)) {
        addTransfer(str);
    }
    sDebugOut << endl;
}


void KMainWidget::slotCopyToClipboard()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    Transfer *item = (Transfer *) myTransferList->currentItem();

    if (item) {
        QString url = item->getSrc().url();
        QClipboard *cb = QApplication::clipboard();
        cb->setText( url, QClipboard::Selection );
        cb->setText( url, QClipboard::Clipboard);
        myTransferList->clearSelection();
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotMoveToBegin()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    myTransferList->moveToBegin((Transfer *) myTransferList->currentItem());


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotMoveToEnd()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    myTransferList->moveToEnd((Transfer *) myTransferList->currentItem());

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotOpenIndividual()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    Transfer *item = (Transfer *) myTransferList->currentItem();
    if (item)
        item->showIndividual();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

bool KMainWidget::queryClose()
{
    if( kapp->sessionSaving())
        return true;
    hide();
    return false;
}

void KMainWidget::setAutoSave()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    autosaveTimer->stop();
    if (ksettings.b_autoSave) {
        autosaveTimer->start(ksettings.autoSaveInterval * 60000);
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}



void KMainWidget::setAutoDisconnect()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    // disable action when we are connected permanently
    m_paAutoDisconnect->setEnabled(ksettings.connectionType != PERMANENT);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotPreferences()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if ( !prefDlg )
        prefDlg = new DlgPreferences(this);

    prefDlg->show();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotConfigureNotifications()
{
    KNotifyDialog::configure(this);
}

void KMainWidget::slotToggleLogWindow()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    b_viewLogWindow = !b_viewLogWindow;
    if (b_viewLogWindow)
        logWindow->show();
    else
        logWindow->hide();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleAnimation()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    ksettings.b_useAnimation = !ksettings.b_useAnimation;

    if (!ksettings.b_useAnimation && animTimer->isActive()) {
        animTimer->stop();
        animTimer->start(1000);
        animCounter = 0;
    } else {
        animTimer->stop();
        animTimer->start(400);
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotToggleOfflineMode()
{
#ifdef _DEBUG
    sDebugIn "ksettings.b_offlineMode = " << ksettings.b_offlineMode << endl;
#endif

    ksettings.b_offlineMode = !ksettings.b_offlineMode;
    ksettings.b_offline=(ksettings.b_offlineMode || !b_connected);
    if (ksettings.b_offline) {
        log(i18n("Offline mode on."));
        stopAll();
        setCaption(i18n("Offline"), false);
        m_paOfflineMode->setIconSet(LOAD_ICON("tool_offline_mode_off"));
    } else {
        log(i18n("Offline mode off."));
        setCaption(QString::null, false);
        m_paOfflineMode->setIconSet(LOAD_ICON("tool_offline_mode_on"));
    }

    m_paOfflineMode->setChecked(ksettings.b_offline);


    slotUpdateActions();
    checkQueue();
#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleExpertMode()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    ksettings.b_expertMode = !ksettings.b_expertMode;

    if (ksettings.b_expertMode) {
        log(i18n("Expert mode on."));
    } else {
        log(i18n("Expert mode off."));
    }
    m_paExpertMode->setChecked(ksettings.b_expertMode);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleUseLastDir()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    ksettings.b_useLastDir = !ksettings.b_useLastDir;

    if (ksettings.b_useLastDir) {
        log(i18n("Use last folder on."));
    } else {
        log(i18n("Use last folder off."));
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleAutoDisconnect()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    ksettings.b_autoDisconnect = !ksettings.b_autoDisconnect;

    if (ksettings.b_autoDisconnect) {
        log(i18n("Auto disconnect on."));
    } else {
        log(i18n("Auto disconnect off."));
    }
    m_paAutoDisconnect->setChecked(ksettings.b_autoDisconnect);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleAutoShutdown()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    ksettings.b_autoShutdown = !ksettings.b_autoShutdown;

    if (ksettings.b_autoShutdown) {
        log(i18n("Auto shutdown on."));
    } else {
        log(i18n("Auto shutdown off."));
    }

    m_paAutoShutdown->setChecked(ksettings.b_autoShutdown);


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleAutoPaste()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    ksettings.b_autoPaste = !ksettings.b_autoPaste;

    if (ksettings.b_autoPaste) {
        log(i18n("Auto paste on."));
        clipboardTimer->start(1000);
    } else {
        log(i18n("Auto paste off."));
        clipboardTimer->stop();
    }
    m_paAutoPaste->setChecked(ksettings.b_autoPaste);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotToggleDropTarget()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif
    m_showDropTarget = !m_showDropTarget;

    if (m_showDropTarget) {
        kdrop->show();
        kdrop->updateStickyState();
        m_paDropTarget->setText(i18n("Hide Drop &Target"));
    }
    else {
        kdrop->hide();
        m_paDropTarget->setText(i18n("Show Drop &Target"));
    }


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotKonquerorIntegration()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

        bool bIsKonquiEnable=!ksettings.b_KonquerorIntegration;
        ksettings.b_KonquerorIntegration=!ksettings.b_KonquerorIntegration;
        KConfig cfg("konquerorrc", false, false);
        cfg.setGroup("HTML Settings");
        cfg.writePathEntry("DownloadManager",QString((bIsKonquiEnable)?"kget":""));
        cfg.sync();
        if (bIsKonquiEnable) 
        {
            m_paKonquerorIntegration->setText(i18n("Disable &KGet as Konqueror Download Manager"));
        }
        else
        {
            m_paKonquerorIntegration->setText(i18n("Enable &KGet as Konqueror Download Manager"));
        }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotPopupMenu(Transfer * item)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    // select current item
    myTransferList->setCurrentItem(item);

    // set action properties
    slotUpdateActions();

    // popup transfer menu at the position
    QWidget *menu = guiFactory()->container("transfer",this);
    ((QPopupMenu *) menu)->popup(QCursor::pos());


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::setListFont()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    myTransferList->setFont(ksettings.listViewFont);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void KMainWidget::slotUpdateActions()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif


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

    m_paDelete->setEnabled(false);
    m_paResume->setEnabled(false);
    m_paPause->setEnabled(false);
    m_paRestart->setEnabled(false);

    m_paCopy->setEnabled(false);
    m_paIndividual->setEnabled(false);
    m_paMoveToBegin->setEnabled(false);
    m_paMoveToEnd->setEnabled(false);

    Transfer *item;
    Transfer *first_item = 0L;
    TransferIterator it(myTransferList);
    int index = 0;
    int totals_items = 0;
    int sel_items = 0;

    for (; it.current(); ++it, ++totals_items) {

        // update action on visibles windows
        if (it.current()->isVisible())
            it.current()->slotUpdateActions();

        if (it.current()->isSelected()) {
            item = it.current();
            sel_items = totals_items;
            index++;            // counting number of selected items
            if (index == 1) {
                first_item = item;      // store first selected item
                if (totals_items > 0)
                    m_paMoveToBegin->setEnabled(true);

                m_paMoveToEnd->setEnabled(true);
            } else {

                m_paMoveToBegin->setEnabled(false);
                m_paMoveToEnd->setEnabled(false);
            }
            // enable PAUSE, RESUME and RESTART only when we are online and not in offline mode
#ifdef _DEBUG
            sDebug << "-->ONLINE= " << ksettings.b_offline << endl;
#endif
            if (item == first_item && !ksettings.b_offline) {
                switch (item->getStatus()) {
                case Transfer::ST_TRYING:
                case Transfer::ST_RUNNING:
                    m_paResume->setEnabled(false);
                    m_paPause->setEnabled(true);
                    m_paRestart->setEnabled(true);
                    break;
                case Transfer::ST_STOPPED:
                    m_paResume->setEnabled(true);
                    m_paPause->setEnabled(false);
                    m_paRestart->setEnabled(false);
#ifdef _DEBUG
                    sDebug << "STATUS IS  stopped" << item->getStatus() << endl;
#endif
                    break;
                case Transfer::ST_FINISHED:
                    m_paResume->setEnabled(false);
                    m_paPause->setEnabled(false);
                    m_paRestart->setEnabled(false);
                    break;


                }               //end switch

            } else if (item->getStatus() != first_item->getStatus()) {
                // disable all when all selected items don't have the same status
                m_paResume->setEnabled(false);
                m_paPause->setEnabled(false);
                m_paRestart->setEnabled(false);
            }


            if (item == first_item) {
                m_paDelete->setEnabled(true);
                m_paCopy->setEnabled(true);
                m_paIndividual->setEnabled(true);
                if (item->getStatus() != Transfer::ST_FINISHED) {
                    m_paQueue->setEnabled(true);
                    m_paTimer->setEnabled(true);
                    m_paDelay->setEnabled(true);

                    switch (item->getMode()) {
                    case Transfer::MD_QUEUED:
#ifdef _DEBUG
                        sDebug << "....................THE MODE  IS  MD_QUEUED " << item->getMode() << endl;
#endif
                        m_paQueue->setChecked(true);
                        break;
                    case Transfer::MD_SCHEDULED:
#ifdef _DEBUG
                        sDebug << "....................THE MODE  IS  MD_SCHEDULED " << item->getMode() << endl;
#endif
                        m_paTimer->setChecked(true);
                        break;
                    case Transfer::MD_DELAYED:
#ifdef _DEBUG
                        sDebug << "....................THE MODE  IS  MD_DELAYED " << item->getMode() << endl;
#endif
                        m_paDelay->setChecked(true);
                        break;
                    }
                }
            } else if (item->getMode() != first_item->getMode()) {
                // unset all when all selected items don't have the same mode
                m_paQueue->setChecked(false);
                m_paTimer->setChecked(false);
                m_paDelay->setChecked(false);
                m_paMoveToBegin->setEnabled(false);
                m_paMoveToEnd->setEnabled(false);
                m_paQueue->setEnabled(false);
                m_paTimer->setEnabled(false);
                m_paDelay->setEnabled(false);
            }

        }                       // when item is selected
    }                           // loop



    if (sel_items == totals_items - 1)
        m_paMoveToEnd->setEnabled(false);

    // enable all signals



    m_paQueue->blockSignals(false);
    m_paTimer->blockSignals(false);
    m_paDelay->blockSignals(false);


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::updateStatusBar()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    Transfer *item;
    QString tmpstr;

    int totalFiles = 0;
    KIO::filesize_t totalSize = 0;
    int totalSpeed = 0;
    unsigned int remTime = 0;

    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
        item = it.current();
        if (item->getTotalSize() != 0) {
            totalSize += (item->getTotalSize() - item->getProcessedSize());
        }
        totalFiles++;
        totalSpeed += item->getSpeed();

        if (item->getRemainingTime() > remTime) {
            remTime = item->getRemainingTime();
        }
    }

    statusBar()->changeItem(i18n(" Transfers: %1 ").arg(myTransferList->childCount()), ID_TOTAL_TRANSFERS);
    statusBar()->changeItem(i18n(" Files: %1 ").arg(totalFiles), ID_TOTAL_FILES);
    statusBar()->changeItem(i18n(" Size: %1 ").arg(KIO::convertSize(totalSize)), ID_TOTAL_SIZE);
    statusBar()->changeItem(i18n(" Time: %1 ").arg(KIO::convertSeconds(remTime)), ID_TOTAL_TIME);
    statusBar()->changeItem(i18n(" %1/s ").arg(KIO::convertSize(totalSpeed)), ID_TOTAL_SPEED);
    //update size for each statusbar field
    statusBar()->setItemFixed(ID_TOTAL_TRANSFERS, -1);
    statusBar()->setItemFixed(ID_TOTAL_FILES, -1);
    statusBar()->setItemFixed(ID_TOTAL_SIZE, -1);
    statusBar()->setItemFixed(ID_TOTAL_TIME, -1);
    statusBar()->setItemFixed(ID_TOTAL_SPEED, -1);

    if (kdock) {
        tmpstr = i18n("<b>Transfers:</b> %1 ").arg(myTransferList->childCount()) +
                 i18n("<br /><b>Files:</b> %1 ").arg(totalFiles) +
                 i18n("<br /><b>Size:</b> %1 ").arg(KIO::convertSize(totalSize)) +
                 i18n("<br /><b>Time:</b> %1 ").arg(KIO::convertSeconds(remTime)) +
                 i18n("<br /><b>Speed:</b> %1/s").arg(KIO::convertSize(totalSpeed));
        kdock->updateToolTip( tmpstr );
        //trayicon changes if download is in progress
        if (totalSpeed == 0)
        {
            kdock->changeIcon( "kget_dock" );
        }
        else
        {
            kdock->changeIcon( "kget_dock_download" );
        }
    }
#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}


void KMainWidget::onlineDisconnect()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if (!b_connected) {
        return;
    }

    if (!ksettings.b_expertMode) {
        if (KMessageBox::questionYesNo(this, i18n("Do you really want to disconnect?"),
                                       i18n("Question"),
                                       i18n("Disconnect"), i18n("Stay Connected"),
                                       "kget_AutoOnlineDisconnect")
            != KMessageBox::Yes) {
            return;
        }
    }
    log(i18n("Disconnecting..."));
    system(QFile::encodeName(ksettings.disconnectCommand));

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotCheckConnection()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif
    bool old = b_connected;
    checkOnline();
    if (b_connected != old) {
        if (b_connected) {
            log(i18n("We are online."));
            setCaption(QString::null, false);
            ksettings.b_offline=ksettings.b_offlineMode;
            checkQueue();
        } else {
            log(i18n("We are offline."));
            setCaption(i18n("Offline"), false);
            ksettings.b_offline=true;
            stopAll();
        }
    }
#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}


void KMainWidget::checkOnline()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifreq));

    // setup the device name according to the type of connection and link number
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s%d", ConnectionDevices[ksettings.connectionType].ascii(), ksettings.linkNumber);

    bool flag = false;

    if (ksettings.connectionType != PERMANENT) {
        // get the flags for particular device
        if (ioctl(_sock, SIOCGIFFLAGS, &ifr) < 0) {
            flag = true;
            b_connected = false;
        } else if (ifr.ifr_flags == 0) {
#ifdef _DEBUG
            sDebug << "Can't get flags from interface " << ifr.ifr_name << endl;
#endif
            b_connected = false;
        } else if (ifr.ifr_flags & IFF_UP) {    // if (ifr.ifr_flags & IFF_RUNNING)
            b_connected = true;
        } else {
            b_connected = false;
        }
    } else {
        b_connected = true;        // PERMANENT connection
    }

    m_paOfflineMode->setEnabled(b_connected);

    if (flag) {
#ifdef _DEBUG
            sDebug << "Unknown interface " << ifr.ifr_name << endl;
#endif
        }
#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}


// Helper method for opening device socket




static int sockets_open()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef AF_IPX
    ipx_sock = socket(AF_IPX, SOCK_DGRAM, 0);
#else
    ipx_sock = -1;
#endif

#ifdef AF_AX25
    ax25_sock = socket(AF_AX25, SOCK_DGRAM, 0);
#else
    ax25_sock = -1;
#endif

    ddp_sock = socket(AF_APPLETALK, SOCK_DGRAM, 0);
    /*
     *    Now pick any (exisiting) useful socket family for generic queries
     */

    sDebug << "<<<<Leaving -> sockets_open () " << endl;
    if (inet_sock != -1)
        return inet_sock;
    if (ipx_sock != -1)
        return ipx_sock;
    if (ax25_sock != -1)
        return ax25_sock;
    /*
     *    If this is -1 we have no known network layers and its time to jump.
     */

#ifdef _DEBUG
    sDebugOut << endl;
#endif

    return ddp_sock;
}


/** No descriptions */
void KMainWidget::customEvent(QCustomEvent * _e)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif


    SlaveEvent *e = (SlaveEvent *) _e;
    unsigned int result = e->getEvent();

    switch (result) {

        // running cases..
    case Slave::SLV_PROGRESS_SIZE:
        e->getItem()->slotProcessedSize(e->getData());
        break;
    case Slave::SLV_PROGRESS_SPEED:
        e->getItem()->slotSpeed(e->getData());
        break;

    case Slave::SLV_RESUMED:
        e->getItem()->slotExecResume();
        break;

        // stopping cases
    case Slave::SLV_FINISHED:
        e->getItem()->slotFinished();
        break;
    case Slave::SLV_PAUSED:
        e->getItem()->slotExecPause();
        break;
    case Slave::SLV_SCHEDULED:
        e->getItem()->slotExecSchedule();
        break;

    case Slave::SLV_DELAYED:
        e->getItem()->slotExecDelay();
        break;
    case Slave::SLV_CONNECTED:
        e->getItem()->slotExecConnected();
        break;

    case Slave::SLV_CAN_RESUME:
        e->getItem()->slotCanResume((bool) e->getData());
        break;

    case Slave::SLV_TOTAL_SIZE:
        e->getItem()->slotTotalSize(e->getData());
        break;

    case Slave::SLV_ERROR:
        e->getItem()->slotExecError();
        break;

    case Slave::SLV_BROKEN:
        e->getItem()->slotExecBroken();
        break;

    case Slave::SLV_REMOVED:
        e->getItem()->slotExecRemove();
        break;

    case Slave::SLV_INFO:
        e->getItem()->logMessage(e->getMsg());
        break;
        
        case Slave::SLV_NOTINCACHE:
        e->getItem()->NotInCache();
        break;
    default:
#ifdef _DEBUG
        sDebug << "Unknown Result..die" << result << endl;
#endif
        assert(0);
    }


#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}

QString KMainWidget::getSaveDirectoryFor( const QString& filename ) const
{
    // first set destination directory to current directory ( which is also last used )
    QString destDir = ksettings.lastDirectory;

    if (!ksettings.b_useLastDir) {
        // check wildcards for default directory
        DirList::Iterator it;
        for (it = ksettings.defaultDirList.begin(); it != ksettings.defaultDirList.end(); ++it) {
            QRegExp rexp((*it).extRegexp);

            rexp.setWildcard(true);

            if ((rexp.search( filename )) != -1) {
                destDir = (*it).defaultDir;
                break;
            }
        }
    }

    return destDir;
}

bool KMainWidget::sanityChecksSuccessful( const KURL& url )
{
    if (!url.isValid() || !KProtocolInfo::supportsReading( url ) )
    {
        if (!ksettings.b_expertMode)
            KMessageBox::error(this, i18n("Malformed URL:\n%1").arg(url.prettyURL()), i18n("Error"));

        return false;
    }
    // if we find this URL in the list
    Transfer *transfer = myTransferList->find( url );
    if ( transfer )
    {
        if ( transfer->getStatus() != Transfer::ST_FINISHED )
        {
            if ( !ksettings.b_expertMode )
            {
                KMessageBox::error(this, i18n("Already saving URL\n%1").arg(url.prettyURL()), i18n("Error"));
            }

            transfer->showIndividual();
            return false;
        }

        else // transfer is finished, ask if we want to download again
        {
            if ( ksettings.b_expertMode ||
                 (KMessageBox::questionYesNo(this, i18n("Already saved URL\n%1\nDownload again?").arg(url.prettyURL()),i18n("Question"),i18n("Download Again"),KStdGuiItem::cancel() )
                     == KMessageBox::Yes) )
            {
                transfer->slotRequestRemove();
                checkQueue();
                return true;
            }
        }

        return false;
    }

    // why restrict this to ftp and http? (pfeiffer)
//     // don't download file URL's TODO : uncomment?
//     if (url.protocol() == "http" && url.protocol() != "ftp") {
//         KMessageBox::error(this, i18n("File protocol not accepted!\n%1").arg(url.prettyURL()), i18n("Error"));
// #ifdef _DEBUG
//         sDebugOut << endl;
// #endif
//         return false;
//     }

    return true;
}

bool KMainWidget::isDropTargetVisible() const
{
    return m_showDropTarget;
}

void KMainWidget::setDropTargetVisible( bool setVisible )
{
    if ( setVisible != isDropTargetVisible() )
    {
        m_paDropTarget->activate();
    }
}

void KMainWidget::setOfflineMode( bool offline )
{
    if ( ksettings.b_offlineMode != offline )
        slotToggleOfflineMode();
}

bool KMainWidget::isOfflineMode() const
{
    return ksettings.b_offlineMode;
}

void KMainWidget::activateDropTarget()
{
    setDropTargetVisible( true );
}

void KMainWidget::slotShowMenubar()
{
    if(m_menubarAction->isChecked())
        menuBar()->show();
    else
        menuBar()->hide();
}

#include "kmainwidget.moc"
