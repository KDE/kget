/***************************************************************************
*                                kmainwidget.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin          : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                       : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email          : pch@freeshell.org
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

#ifdef __svr4__
#define map BULLSHIT		// on Solaris it conflicts with STL ?
#include <net/if.h>
#undef map
#include <sys/sockio.h>		// needed for SIOCGIFFLAGS
#else
#include <net/if.h>
#endif

#include <qdir.h>
#include <qpainter.h>
#include <qclipboard.h>
#include <qregexp.h>
#include <qdragobject.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qdropsite.h>

#include <kfiledialog.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kaudioplayer.h>
#include <kurl.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kglobal.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kstdaction.h>
#include <khelpmenu.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <kio/netaccess.h>
#include <kstatusbar.h>
#include <kconfig.h>


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
#include <qiconset.h>

#include "version.h"
#include "slave.h"
#include "slaveevent.h"


KMainWidget *kmain = 0L;

QGuardedPtr < DropTarget > kdrop = 0L;
QGuardedPtr < DockWidget > kdock = 0L;

Settings ksettings;		// this object contains all settings

static int sockets_open();

// socket constants
int ipx_sock = -1;		/* IPX socket */
int ax25_sock = -1;		/* AX.25 socket */
int inet_sock = -1;		/* INET socket */
int ddp_sock = -1;		/* Appletalk DDP socket */


KMainWidget::KMainWidget(bool bStartDocked):KMainWindow(0, "kget")
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    KConfig *cfg = new KConfig("kioslaverc", false, false);

    cfg->setGroup(QString::null);
    cfg->writeEntry(i18n("AutoResume"), true);
    cfg->sync();
    delete cfg;



    b_online = TRUE;
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

    // Clear clipboard
    kapp->clipboard()->setText("");
    // Load all settings from KConfig
    ksettings.load();

    // Setup log window
    logWindow = new LogWindow();

    setCaption(KGETVERSION);

    setupGUI();
    setupWhatsThis();

    log(i18n("Welcome to Kget"));

    setCentralWidget(myTransferList);

    connect(kapp, SIGNAL(saveYourself()), SLOT(slotSaveYourself()));

    // Enable dropping
    setAcceptDrops(true);

    // Setup connection timer
    connectionTimer = new QTimer(this);
    connect(connectionTimer, SIGNAL(timeout()), SLOT(slotCheckConnection()));

    // setup socket for checking connection
    if ((_sock = sockets_open()) < 0) {
	log(i18n("Couldn't create valid socket"), false);
    } else {
	connectionTimer->start(5000);	// 5 second interval for checking connection
    }

    checkOnline();
    if (!b_online) {
	log(i18n("Starting offline"));
    }
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
    transferTimer->start(10000);	// 10 secs time interval

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

    currentDirectory = "file:" + QDir::currentDirPath();
    readTransfers();

    // Setup special windows
    kdrop = new DropTarget();
    kdock = new DockWidget(this);

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
    m_paUseSound->setChecked(ksettings.b_useSound);
    m_paExpertMode->setChecked(ksettings.b_expertMode);
    m_paUseLastDir->setChecked(ksettings.b_useLastDir);
    if (ksettings.connectionType != PERMANENT) {
	m_paAutoDisconnect->setChecked(ksettings.b_autoDisconnect);
    }
    setAutoDisconnect();

    m_paAutoShutdown->setChecked(ksettings.b_autoShutdown);
    m_paOfflineMode->setChecked(ksettings.b_offlineMode);
    m_paAutoPaste->setChecked(ksettings.b_autoPaste);
    m_paShowStatusbar->setChecked(ksettings.b_showStatusbar);
    m_paShowLog->setChecked(b_viewLogWindow);
    if(bStartDocked)
        ksettings.windowStyle=DOCKED_MIN;
    switch (ksettings.windowStyle) {
    case DROP_TARGET:
	m_paDropTarget->setChecked(true);
	break;
    case DOCKED:
    case DOCKED_MIN:
	m_paDockWindow->setChecked(true);
	break;
    case NORMAL:
	m_paNormal->setChecked(true);
    default:
	break;
    }
    setWindowStyle();
    sDebugOut << endl;

}


KMainWidget::~KMainWidget()
{
    sDebugIn << endl;
    if (animTimer) {
	animTimer->stop();
	delete animTimer;
    }

    delete(DropTarget *) kdrop;

    writeTransfers();

    writeLog();

    sDebugOut << endl;

}


void
 KMainWidget::log(const QString & message, bool statusbar)
{
    sDebugIn << endl;

    sDebug << message << endl;
    logWindow->logGeneral(message);

    if (statusbar) {
	statusBar()->message(message, 1000);
    }
    sDebugOut << endl;
}


void KMainWidget::slotSaveYourself()
{
    sDebugIn << endl;

    writeTransfers();
    ksettings.save();

    sDebugOut << endl;
}


void KMainWidget::setupGUI()
{
    sDebugIn << endl;

    // setup transfer list
    myTransferList = new TransferList(this, "transferList");
    myTransferList->setSorting(-1);
    setListFont();

    connect(myTransferList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateActions()));
    connect(myTransferList, SIGNAL(transferSelected(Transfer *)), this, SLOT(slotOpenIndividual()));
    connect(myTransferList, SIGNAL(popupMenu(Transfer *)), this, SLOT(slotPopupMenu(Transfer *)));

    // file actions
    m_paOpenTransfer = KStdAction::open(this, SLOT(slotOpenTransfer()), actionCollection(), "open_transfer");
    m_paPasteTransfer = KStdAction::paste(this, SLOT(slotPasteTransfer()), actionCollection(), "paste_transfer");

    m_paExportTransfers = new KAction(i18n("&Export Transfer List"), 0, this, SLOT(slotExportTransfers()), actionCollection(), "export_transfers");
    m_paImportTransfers = new KAction(i18n("&Import Transfer List"), 0, this, SLOT(slotImportTransfers()), actionCollection(), "import_transfers");

    m_paImportText = new KAction(i18n("Import Text &File"), 0, this, SLOT(slotImportTextFile()), actionCollection(), "import_text");

    m_paQuit = KStdAction::quit(this, SLOT(slotQuit()), actionCollection(), "quit");


    // transfer actions
    m_paCopy = new KAction(i18n("&Copy URL to clipboard"), 0, this, SLOT(slotCopyToClipboard()), actionCollection(), "copy_url");
    m_paIndividual = new KAction(i18n("&Open individual window"), 0, this, SLOT(slotOpenIndividual()), actionCollection(), "open_individual");

    m_paMoveToBegin = new KAction(i18n("Move to &beginning"), 0, this, SLOT(slotMoveToBegin()), actionCollection(), "move_begin");

    m_paMoveToEnd = new KAction(i18n("Move to &end"), 0, this, SLOT(slotMoveToEnd()), actionCollection(), "move_end");

    // TODO CHECK path
    QString path = "kget/pics/";

    sDebug << "Loading pics" << endl;
    QPixmap *tmppix = new QPixmap();

    tmppix->load(locate("data", path + "dock_hand1.xpm"));

    m_paResume = new KAction(i18n("&Resume"), QIconSet(QPixmap(locate("data", "kget/pics/tool_resume.xpm"))), 0, this, SLOT(slotResumeCurrent()), actionCollection(), "resume");

    m_paPause = new KAction(i18n("&Pause"), QIconSet(QPixmap(locate("data", "kget/pics/tool_pause.xpm"))), 0, this, SLOT(slotPauseCurrent()), actionCollection(), "pause");


    m_paDelete = new KAction(i18n("&Delete"), QIconSet(QPixmap(locate("data", "kget/pics/tool_delete.xpm"))), 0, this, SLOT(slotDeleteCurrent()), actionCollection(), "delete");


    m_paRestart = new KAction(i18n("Re&start"), QIconSet(QPixmap(locate("data", "kget/pics/tool_restart.xpm"))), 0, this, SLOT(slotRestartCurrent()), actionCollection(), "restart");

    m_paQueue = new KRadioAction(i18n("&Queue"), QIconSet(QPixmap(locate("data", "kget/pics/tool_queue.xpm"))), 0, this, SLOT(slotQueueCurrent()), actionCollection(), "queue");
    m_paTimer = new KRadioAction(i18n("&Timer"), QIconSet(QPixmap(locate("data", "kget/pics/tool_timer.xpm"))), 0, this, SLOT(slotTimerCurrent()), actionCollection(), "timer");
    m_paDelay = new KRadioAction(i18n("De&lay"), QIconSet(QPixmap(locate("data", "kget/pics/tool_delay.xpm"))), 0, this, SLOT(slotDelayCurrent()), actionCollection(), "delay");

    m_paQueue->setExclusiveGroup("TransferMode");
    m_paTimer->setExclusiveGroup("TransferMode");
    m_paDelay->setExclusiveGroup("TransferMode");

    // options actions
    m_paUseAnimation = new KToggleAction(i18n("Use &Animation"), 0, this, SLOT(slotToggleAnimation()), actionCollection(), "toggle_animation");

    m_paUseSound = new KToggleAction(i18n("Use &Sound"), 0, this, SLOT(slotToggleSound()), actionCollection(), "toggle_sound");

    m_paPreferences = new KAction(i18n("P&references"), QIconSet(QPixmap(locate("data", "kget/pics/tool_preferences.xpm"))), 0, this, SLOT(slotPreferences()), actionCollection(), "preferences");

    m_paExpertMode = new KToggleAction(i18n("&Expert mode"), "tool_expert", 0, this, SLOT(slotToggleExpertMode()), actionCollection(), "expert_mode");

    m_paUseLastDir = new KToggleAction(i18n("&Use-last-directory mode"), "tool_uselastdir", 0, this, SLOT(slotToggleUseLastDir()), actionCollection(), "use_last_dir");

    m_paAutoDisconnect = new KToggleAction(i18n("Auto-&disconnect mode"), "tool_disconnect", 0, this, SLOT(slotToggleAutoDisconnect()), actionCollection(), "auto_disconnect");

    m_paAutoShutdown = new KToggleAction(i18n("Auto-s&hutdown mode"), "tool_shutdown", 0, this, SLOT(slotToggleAutoShutdown()), actionCollection(), "auto_shutdown");

    m_paOfflineMode = new KToggleAction(i18n("&Offline mode"), "tool_offline_mode", 0, this, SLOT(slotToggleOfflineMode()), actionCollection(), "offline_mode");

    m_paAutoPaste = new KToggleAction(i18n("Auto-pas&te mode"), "tool_clipboard", 0, this, SLOT(slotToggleAutoPaste()), actionCollection(), "auto_paste");

    KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection(), "configure_keybinding");

    KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection(), "configure_toolbars");

    // view actions
    m_paShowStatusbar = KStdAction::showStatusbar(this, SLOT(slotToggleStatusbar()), actionCollection(), "show_statusbar");

    m_paShowLog = new KToggleAction(i18n("Show &Log Window"), "tool_logwindow", 0, this, SLOT(slotToggleLogWindow()), actionCollection(), "toggle_log");

    m_paDropTarget = new KRadioAction(i18n("Drop &target"), "tool_drop_target", 0, this, SLOT(slotDropTarget()), actionCollection(), "drop_target");

    m_paDockWindow = new KRadioAction(i18n("&Dock window"), "tool_dock", 0, this, SLOT(slotDock()), actionCollection(), "dock_window");

    m_paNormal = new KRadioAction(i18n("&Normal"), "tool_normal", 0, this, SLOT(slotNormal()), actionCollection(), "normal");

    m_paDropTarget->setExclusiveGroup("WindowMode");
    m_paDockWindow->setExclusiveGroup("WindowMode");
    m_paNormal->setExclusiveGroup("WindowMode");

    menuHelp = new KHelpMenu(this, KGlobal::instance()->aboutData());
    KStdAction::whatsThis(menuHelp, SLOT(contextHelpActivated()), actionCollection(), "whats_this");

    createGUI("kgetui.rc");

    toolBar()->setBarPos(ksettings.toolbarPosition);
    toolBar()->setIconText(KToolBar::IconOnly);
    // setup statusbar
    statusBar()->insertFixedItem(i18n(" Transfers: %1 ").arg(99), ID_TOTAL_TRANSFERS);
    statusBar()->insertFixedItem(i18n(" Files: %1 ").arg(555), ID_TOTAL_FILES);
    statusBar()->insertFixedItem(i18n(" Size: %1 KB ").arg("134.56"), ID_TOTAL_SIZE);
    statusBar()->insertFixedItem(i18n(" Time: 00:00:00 "), ID_TOTAL_TIME);
    statusBar()->insertFixedItem(i18n(" %1 KB/s ").arg("123.34"), ID_TOTAL_SPEED);

    if (ksettings.b_showStatusbar) {
	statusBar()->show();
    } else {
	statusBar()->hide();
    }
    slotUpdateActions();
    updateStatusBar();
    sDebugOut << endl;

}


void KMainWidget::setupWhatsThis()
{
    sDebugIn << endl;
    static QString tmp1;

    tmp1 = i18n("<b>Resume</b> button starts selected transfers\n" "and sets their mode to <i>queued</i>.");
    m_paResume->setWhatsThis(tmp1);

    static QString tmp2;

    tmp2 = i18n("<b>Pause</b> button stops selected transfers\n" "and sets their mode to <i>delayed</i>.");
    m_paPause->setWhatsThis(tmp2);

    static QString tmp3;

    tmp3 = i18n("<b>Delete</b> button removes selected transfers\n" "from the list.");
    m_paDelete->setWhatsThis(tmp3);

    static QString tmp4;

    tmp4 = i18n("<b>Restart</b> button is a convenience button\n" "that simply does Pause and Resume.");
    m_paRestart->setWhatsThis(tmp4);

    static QString tmp5;

    tmp5 = i18n("<b>Queued</b> button sets the mode of selected\n" "transfers to <i>queued</i>.\n" "\n" "It is a radio button, you can select between\n" "three modes.");
    m_paQueue->setWhatsThis(tmp5);

    static QString tmp6;

    tmp6 = i18n("<b>Scheduled</b> button sets the mode of selected\n" "transfers to <i>scheduled</i>.\n" "\n" "It is a radio button, you can select between\n" "three modes.");
    m_paTimer->setWhatsThis(tmp6);

    static QString tmp7;

    tmp7 = i18n("<b>Delayed</b> button sets the mode of selected\n"
		"transfers to <i>delayed</i>." "This also causes the selected transfers to stop.\n" "\n" "It is a radio button, you can select between\n" "three modes.");
    m_paDelay->setWhatsThis(tmp7);

    static QString tmp8;

    tmp8 = i18n("<b>Preferences</b> button opens a preferences dialog\n" "where you can set various options.\n" "\n" "Some of these options can be more easily set using the toolbar.");
    m_paPreferences->setWhatsThis(tmp8);

    static QString tmp9;

    tmp9 = i18n("<b>Log window</b> button opens a log window.\n" "The log window records all program events that occur\n" "while Kget is running.");
    m_paShowLog->setWhatsThis(tmp9);

    static QString tmp10;

    tmp10 = i18n("<b>Paste transfer</b> button adds a URL from\n" "the clipboard as a new transfer.\n" "\n" "This way you can easily copy&paste URLs between\n" "applications.");
    m_paPasteTransfer->setWhatsThis(tmp10);

    static QString tmp11;

    tmp11 = i18n("<b>Expert mode</b> button toggles the expert mode\n"
		 "on and off.\n"
		 "\n"
		 "Expert mode is recommended for experienced users.\n"
		 "When set, you will not be \"bothered\" by confirmation\n"
		 "messages.\n" "<b>Important!</b>\n" "Turn it on if you are using auto-disconnect or\n" "auto-shutdown features and you want Kget to disconnect\n" "without asking.");
    m_paExpertMode->setWhatsThis(tmp11);

    static QString tmp12;

    tmp12 = i18n("<b>Use last directory</b> button toggles the\n"
		 "use-last-directory feature on and off.\n"
		 "\n" "When set, Kget will ignore the directory settings\n" "and put all new added transfers into the directory\n" "where the last transfer was put.");
    m_paUseLastDir->setWhatsThis(tmp12);

    static QString tmp13;

    tmp13 =
	i18n("<b>Auto disconnect</b> button toggles the auto-disconnect\n"
	     "mode on and off.\n" "\n"
	     "When set, Kget will disconnect automatically\n"
	     "after all queued transfers are finished.\n" "\n" "<b>Important!</b>\n" "Also turn on the expert mode when you want Kget\n" "to disconnect without asking.");
    m_paAutoDisconnect->setWhatsThis(tmp13);

    static QString tmp14;

    tmp14 = i18n("<b>Auto shutdown</b> button toggles the auto-shutdown\n"
		 "mode on and off.\n"
		 "\n"
		 "When set, Kget will quit automatically\n"
		 "after all queued transfers are finished.\n" "<b>Important!</b>\n" "Also turn on the expert mode when you want Kget\n" "to quit without asking.");
    m_paAutoShutdown->setWhatsThis(tmp14);

    static QString tmp15;

    tmp15 = i18n("<b>Offline mode</b> button toggles the offline mode\n"
		 "on and off.\n"
		 "\n" "When set, Kget will act as if it was not connected\n" "to the Internet.\n" "\n" "You can browse offline, while still being able to add\n" "new transfers as queued.");
    m_paOfflineMode->setWhatsThis(tmp15);

    static QString tmp16;

    tmp16 = i18n("<b>Auto paste</b> button toggles the auto-paste mode\n" "on and off.\n" "\n" "When set, Kget will periodically scan the clipboard\n" "for URLs and paste them automatically.");
    m_paAutoPaste->setWhatsThis(tmp16);

    static QString tmp17;

    tmp17 = i18n("<b>Drop target</b> button toggles the window style\n"
		 "between a normal window and a drop target.\n"
		 "\n"
		 "When set, the main window will be hidden and\n" "instead a small shaped window will appear.\n" "\n" "You can show/hide a normal window with a simple click\n" "on a shaped window.");
    m_paDropTarget->setWhatsThis(tmp17);

    static QString tmp18;

    tmp18 = i18n("<b>Dock widget</b> button toggles the window style\n"
		 "between a normal window and a docked widget.\n"
		 "\n"
		 "When set, the main window will be hidden and\n"
		 "instead a docked widget will appear on the panel.\n" "\n" "You can show/hide a normal window by simply clicking\n" "on a docked widget.");
    m_paDockWindow->setWhatsThis(tmp18);
    static QString tmp19;

    tmp19 = i18n("<b>Normal window</b> button sets\n" "\n" "the window style to normal window");
    m_paNormal->setWhatsThis(tmp19);
    sDebugOut << endl;
}



void KMainWidget::slotConfigureKeys()
{
    sDebugIn << endl;
    KKeyDialog::configureKeys(actionCollection(), xmlFile());
    sDebugOut << endl;
}



void KMainWidget::slotConfigureToolbars()
{
    sDebugIn << endl;
    KEditToolbar edit(factory());

    edit.exec();
    sDebugOut << endl;
}


void KMainWidget::slotImportTextFile()
{
    sDebugIn << endl;
    QString filename, tmpFile;
    QString list;
    int i, j;

    filename = KFileDialog::getOpenURL(currentDirectory).url();
    if (filename.isEmpty()) {
	return;
    }
    if (KIO::NetAccess::download(filename, tmpFile)) {
	list = kFileToString(tmpFile);
	KIO::NetAccess::removeTempFile(tmpFile);
    } else
	list = kFileToString(filename);

    i = 0;
    while ((j = list.find('\n', i)) != -1) {
	QString newtransfer = list.mid(i, j - i);

	addTransfer(newtransfer);
	i = j + 1;
    }
    sDebugOut << endl;
}


void KMainWidget::slotImportTransfers()
{
    sDebugIn << endl;
    readTransfers(true);
    sDebugOut << endl;
}


void KMainWidget::readTransfers(bool ask_for_name)
{
    sDebugIn << endl;

    QString txt;

    if (ask_for_name)
	txt = KFileDialog::getOpenURL(currentDirectory, "*.kgt|*.kgt\n*.*|All files").url();
   else
	txt = locateLocal("appdata", "transfers");


    if (txt.isEmpty()) {
    sDebugOut<<endl;
	return;
    }

    sDebug << "Read from file: " << txt << endl;
    myTransferList->readTransfers(txt);

    checkQueue();
    slotTransferTimeout();

    myTransferList->clearSelection();


    sDebugOut << endl;
}


void KMainWidget::slotExportTransfers()
{
    sDebugIn << endl;
    writeTransfers(true);
    sDebugOut << endl;
}

void KMainWidget::writeTransfers(bool ask_for_name)
{
    sDebugIn << endl;

    QString str;
    QString txt;

    if (ask_for_name) {
	txt = KFileDialog::getSaveFileName(currentDirectory, "*.kgt|*.kgt\n*.*|All files");
    }

    else {
	txt = locateLocal("appdata", "transfers");
    }

    if (txt.isEmpty()) {
    sDebugOut<<endl;
	return;
    }


    if (txt.findRev(".kgt") == -1)
	txt += ".kgt";
    sDebug << "Writing transfers " << txt << endl;
    myTransferList->writeTransfers(txt);
    sDebugOut << endl;
}


void KMainWidget::writeLog()
{
    sDebugIn<<"Writing log to file : " << logFileName.ascii()    << endl;

    kCStringToFile(logWindow->getText().ascii(), logFileName.ascii(), false, false);
    sDebugOut << endl;
}


void KMainWidget::slotQuit()
{
    sDebugIn << endl;

    Transfer *item;
    TransferIterator it(myTransferList);

    log(i18n("Quitting..."));

    for (; it.current(); ++it) {
	item = it.current();
	if (item->getStatus() == Transfer::ST_RUNNING && !ksettings.b_expertMode) {
	    if (KMessageBox::warningYesNo(this, i18n("Some transfers are still running.\nAre you sure you want to close Kget?"), i18n("Warning")) != KMessageBox::Yes) {
		return;
	    }
	}
    }
    sDebugOut << endl;

    ksettings.save();
    delete this;

    kapp->quit();
}


void KMainWidget::slotResumeCurrent()
{
    sDebugIn << endl;
    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
	if (it.current()->isSelected()) {
	    it.current()->slotResume();
	}
    }

    slotUpdateActions();
    sDebugOut << endl;

}


void KMainWidget::slotPauseCurrent()
{
    sDebugIn << endl;
    TransferIterator it(myTransferList);

    m_paPause->setEnabled(false);
    m_paRestart->setEnabled(false);
    update();

    for (; it.current(); ++it) {
	if (it.current()->isSelected()) {
	    it.current()->slotRequestPause();
	}
    }

    slotUpdateActions();
    sDebugOut << endl;
}



void KMainWidget::slotRestartCurrent()
{
    sDebugIn << endl;
    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
	if (it.current()->isSelected()) {
	    it.current()->slotRequestRestart();
	}
    }

    slotUpdateActions();


    sDebugOut << endl;

}


void KMainWidget::slotDeleteCurrent()
{
    sDebugIn << endl;
    m_paDelete->setEnabled(false);
    m_paPause->setEnabled(false);
    update();
    TransferIterator it(myTransferList);

    while (it.current()) {
	if (it.current()->isSelected()) {
	    bool isRunning = false;

	    if (!ksettings.b_expertMode) {
		if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete this transfer?"), i18n("Question")) != KMessageBox::Yes)
		    return;
	    }

	    isRunning = (it.current()->getStatus() == Transfer::ST_RUNNING);
	    it.current()->slotRequestRemove();
	    if (isRunning)
		it++;

	    // don't need to update counts - they are updated automatically when
	    // calling remove() if the thread is not running

	} else {
	    it++;		// update counts
	}
    }

    checkQueue();		// needed !

    sDebugOut << endl;
}


void KMainWidget::pauseAll()
{
    sDebugIn << endl;

    log(i18n("Pausing all jobs"), false);

    sDebugOut << endl;
}


void KMainWidget::slotQueueCurrent()
{

    sDebugIn << endl;
    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
	if (it.current()->isSelected()) {
	    it.current()->slotQueue();
	}
    }

    myTransferList->clearSelection();
    slotUpdateActions();
    sDebugOut << endl;


}


void KMainWidget::slotTimerCurrent()
{

    sDebugIn << endl;

    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
	if (it.current()->isSelected()) {
	    it.current()->slotRequestSchedule();
	}
    }

    myTransferList->clearSelection();
    sDebugOut << endl;

}


void KMainWidget::slotDelayCurrent()
{
    sDebugIn << endl;
    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
	if (it.current()->isSelected()) {
	    it.current()->slotRequestDelay();
	}
    }

    myTransferList->clearSelection();
    sDebugOut << endl;
}


void KMainWidget::slotOpenTransfer()
{
    sDebugIn << endl;

    QString newtransfer;
    bool ok = false;

#ifdef _DEBUG
    // newtransfer = "ftp://localhost/home/pch/test.gz";
//      newtransfer = "http://www.kernel.org/pub/linux/kernel/v2.4/linux-2.4.18.tar.gz";
    newtransfer = "ftp://darkmoon/pub/test.gz";
#endif
    while (!ok) {
	newtransfer = KLineEditDlg::getText(i18n("Open transfer:"), newtransfer, &ok, this);

	// user presses cancel
	if (!ok) {
	    return;
	}

	KURL url(newtransfer);

	if (url.isMalformed()) {
	    KMessageBox::error(this, i18n("Malformed URL:\n") + newtransfer, i18n("Error"));
	    ok = false;
	}
    }

    addTransfer(newtransfer);

    sDebugOut << endl;

}



void KMainWidget::slotCheckClipboard()
{
    // sDebugIn<<endl;

    QString clipData = kapp->clipboard()->text();

    if (clipData != lastClipboard) {
	sDebug << "New clipboard event" << endl;

	lastClipboard = clipData.copy();
	if (clipData.isEmpty() || clipData.stripWhiteSpace().isEmpty()) {
	    return;
	}

	KURL url(lastClipboard.stripWhiteSpace());

	if (!url.isMalformed() && ksettings.b_autoPaste) {
	    slotPasteTransfer();
	}
    }
//    sDebugOut <<endl;
}


void KMainWidget::slotPasteTransfer()
{
    sDebugIn << endl;

    QString newtransfer;

    newtransfer = kapp->clipboard()->text();
    newtransfer = newtransfer.stripWhiteSpace();

    if (!ksettings.b_expertMode) {
	KLineEditDlg *box = new KLineEditDlg(i18n("Open transfer:"),
					     newtransfer, this);

	box->show();

	if (!box->result()) {	
       // cancelled
       sDebugOut << endl;
	    return;
	}

	newtransfer = box->text();
	delete box;
    }

    if (!newtransfer.isEmpty()) {
	addTransfer(newtransfer);
    }
    sDebugOut << endl;
}

void KMainWidget::addTransferEx(QString s, QString d,bool bShowIndividual)
{
    sDebugIn << endl;

    KURL url(s);

    // don't download file URL's TODO : uncomment
    if (!strcmp(url.protocol(), "file")) {
	sDebug << "File protocol not accepted !" << endl;
	return;
    }

    if (url.isMalformed()) {
	if (!ksettings.b_expertMode) {
	    KMessageBox::error(this, i18n("Malformed URL:\n") + s, i18n("Error"));
	}
	return;
    }
    // if we find this URL in the list
    if (myTransferList->find(s)) {
	if (!ksettings.b_expertMode) {
	    KMessageBox::error(this, i18n("Already saving URL \n") + s, i18n("Error"));
	}
	return;
    }
    // Setup destination

    // first set destination directory to current directory ( which is also last used )
    QString destDir = currentDirectory;

    if (!ksettings.b_useLastDir) {
	// check wildcards for default directory
	DirList::Iterator it;
	for (it = ksettings.defaultDirList.begin(); it != ksettings.defaultDirList.end(); ++it) {
	    QRegExp rexp((*it).extRegexp);

	    rexp.setWildcard(true);

	    if ((rexp.match(url.fileName())) != -1) {
		destDir = (*it).defaultDir;
		break;
	    }
	}
    }

    KURL dest;

    if (d.isNull()) {		// if we didn't provide destination
	if (!ksettings.b_expertMode) {
	    // open the filedialog for confirmation
	    KFileDialog *dlg = new KFileDialog(destDir, "", this, i18n("Save As"), true);

	    dlg->setSelection(url.fileName());
	    dlg->setOperationMode(KFileDialog::Saving);
	    // TODO set the default destiantion
	    dlg->exec();

	    if (!dlg->result()) {	// cancelled
		return;
	    } else {
		dest = dlg->selectedURL().url();
		currentDirectory = dest.directory();
	    }
	} else {
	    // in expert mode don't open the filedialog
	    dest = destDir + "/" + url.fileName();
	}
    } else {
	dest = d;
    }

    QString file = url.fileName();

    // create a new transfer item
    Transfer *item = myTransferList->addTransfer(s, dest);

    // update the remaining fields
    item->updateAll();

    myTransferList->clearSelection();

    if (ksettings.b_useSound) {
	KAudioPlayer::play(ksettings.audioAdded);
    }
   if (bShowIndividual) item->showIndividual();
    checkQueue();
    sDebugOut << endl;

}

void KMainWidget::addTransfer(QString s, QString d)
{
   addTransferEx(s,d,false);
 }


void KMainWidget::checkQueue()
{
    uint numRun = 0;
    int status;

    sDebugIn << endl;

    Transfer *item;

    if (!ksettings.b_offlineMode && b_online) {

	TransferIterator it(myTransferList);

	// count running transfers
	for (; it.current(); ++it) {
	    status = it.current()->getStatus();
	    if (status == Transfer::ST_RUNNING||status==Transfer::ST_TRYING)

		numRun++;
	}
	sDebug << "Found " << numRun << " Running Jobs" << endl;
	it.reset();
	for (; it.current() && numRun < ksettings.maxSimultaneousConnections; ++it) {
	    item = it.current();
	    if ((item->getMode() == Transfer::MD_QUEUED)
		&& (item->getStatus() != Transfer::ST_RUNNING && item->getStatus()!=Transfer::ST_TRYING)) {
		 log(i18n("Starting another queued job."));
		item->slotResume();
		numRun++;
	    }
	}

	slotUpdateActions();

	updateStatusBar();

	sDebugOut << endl;
    } else {
	log("Cannot continue offline status");
	// assert(0);
    }
}


void KMainWidget::slotAnimTimeout()
{
    //sDebugIn<<endl;
    bool isTransfer;

    animCounter++;
    if (animCounter == myTransferList->getPhasesNum()) {
	animCounter = 0;
    }
    // update status of all items of transferList
    isTransfer = myTransferList->updateStatus(animCounter);

    if (this->isVisible()) {
	updateStatusBar();
    }
    // update dock widget or drop target
    if (ksettings.windowStyle == DOCKED || ksettings.windowStyle == DROP_TARGET) {
	int count = 0;
	int progindex[4];

	for (int i = 0; i < 4; i++) {
	    progindex[i] = 0;
	}

	if (isTransfer) {
	    TransferIterator it(myTransferList);
	    Transfer *item;

	    while (count < 4 && it.current()) {
		item = it.current();
		if ((item->getStatus() == Transfer::ST_RUNNING)
		    && item->getMode() == Transfer::MD_QUEUED) {
		    progindex[count] = item->getPercent();
		    count++;
		}
		it++;
	    }

	    if (progindex[0] == 0) {	// this is a hack, so that dock widget and drop target show
		progindex[0]++;	// transfer in progress, even if percent is = 0
	    }
	}

	if (ksettings.windowStyle == DOCKED) {
	    kdock->setAnim(progindex[0], progindex[1], progindex[2], b_online);
	} else {
	    kdrop->setAnim(progindex[0], progindex[1], progindex[2], progindex[3], b_online);
	}
    }
    //sDebugOut<<endl;


}


void KMainWidget::slotTransferTimeout()
{
    //sDebugIn<<endl;

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
	disconnect();
    }
    //sDebugOut<<endl;

}


void KMainWidget::slotAutosaveTimeout()
{
    sDebugIn << endl;

    writeTransfers();
    sDebugOut << endl;
}


void KMainWidget::slotStatusChanged(Transfer * item, int _operation)
{
    sDebugIn << endl;


    switch (_operation) {

    case Transfer::OP_FINISHED:
	if (ksettings.b_removeOnSuccess) {
	    item->setMode(Transfer::MD_NONE);
	    if (myTransferList->isQueueEmpty()) {
		// no items in the TransferList or we have donwload all items
		if (ksettings.b_autoDisconnect)
		    disconnect();

		if (ksettings.b_autoShutdown) {
		    slotQuit();
		    return;
		}
		// play(ksettings.audioFinishedAll);
	    }
	    item->slotUpdateActions();
	} else
	    delete item;
	break;

    case Transfer::OP_RESUMED:
	slotUpdateActions();
	item->slotUpdateActions();
//                play(ksettings.audioStarted);
	break;
    case Transfer::OP_PAUSED:
	break;
    case Transfer::OP_REMOVED:
	delete item;
	return;			// checkQueue() will be called only once after all deletions

    case Transfer::OP_ABORTED:
	break;
    case Transfer::OP_QUEUED:
	slotUpdateActions();
	item->slotUpdateActions();
	break;
    case Transfer::OP_SCHEDULED:
	slotUpdateActions();
	item->slotUpdateActions();
	slotTransferTimeout();	// this will check schedule times
	return;			// checkQueue() is called from slotTransferTimeout()

    case Transfer::OP_DELAYED:
	break;
    }


    checkQueue();
    sDebugOut << endl;

}


void KMainWidget::dragEnterEvent(QDragEnterEvent * event)
{
    sDebugIn << endl;
    event->accept(QUriDrag::canDecode(event)
		  || QTextDrag::canDecode(event));
    sDebugOut << endl;
}


void KMainWidget::dropEvent(QDropEvent * event)
{
    sDebugIn << endl;
    QStrList list;
    QString str;

    if (QUriDrag::decode(event, list)) {
	addDropTransfers(&list);
    } else if (QTextDrag::decode(event, str)) {
	addTransfer(str);
    }
    sDebugOut << endl;
}


void KMainWidget::addDropTransfers(QStrList * list)
{
    sDebugIn << endl;
    QString s;

    for (s = list->first(); s != 0L; s = list->next()) {
	addTransfer(s);
    }

    myTransferList->clearSelection();

    sDebugOut << endl;
}


void KMainWidget::slotCopyToClipboard()
{
    sDebugIn << endl;
    Transfer *item = (Transfer *) myTransferList->currentItem();

    if (item) {
	QClipboard *cb = QApplication::clipboard();

	// kapp->lock();
	cb->setText(item->getSrc().url());
	// kapp->unlock();
	myTransferList->clearSelection();
    }
    sDebugOut << endl;
}


void KMainWidget::slotMoveToBegin()
{
    sDebugIn << endl;
    myTransferList->moveToBegin((Transfer *) myTransferList->currentItem());
    sDebugOut << endl;
}


void KMainWidget::slotMoveToEnd()
{
    sDebugIn << endl;
    myTransferList->moveToEnd((Transfer *) myTransferList->currentItem());
    sDebugOut << endl;
}


void KMainWidget::slotOpenIndividual()
{
    sDebugIn << endl;
    Transfer *item = (Transfer *) myTransferList->currentItem();

    if (item) {
	item->showIndividual();
    }
    sDebugOut << endl;
}

void KMainWidget::hideEvent(QHideEvent * _hev)
{
    _hev = _hev;
    sDebugIn << endl;
    if (ksettings.windowStyle != NORMAL)
	hide();
    sDebugOut << endl;

}

void KMainWidget::closeEvent(QCloseEvent *)
{
    sDebugIn << endl;
    slotQuit();
    sDebugOut << endl;
}



void KMainWidget::setAutoSave()
{
    sDebugIn << endl;
    autosaveTimer->stop();
    if (ksettings.b_autoSave) {
	autosaveTimer->start(ksettings.autoSaveInterval * 60000);
    }
    sDebugOut << endl;

}



void KMainWidget::setAutoDisconnect()
{
    sDebugIn << endl;
    // disable action when we are connected permanently
    m_paAutoDisconnect->setEnabled(ksettings.connectionType != PERMANENT);
    sDebugOut << endl;

}



void KMainWidget::slotToggleStatusbar()
{
    sDebugIn << endl;
    ksettings.b_showStatusbar = !ksettings.b_showStatusbar;

    if (!ksettings.b_showStatusbar) {
	statusBar()->hide();
    } else {
	statusBar()->show();
    }

    resizeEvent(0L);
    sDebugOut << endl;
}


void KMainWidget::slotPreferences()
{
    sDebugIn << endl;
    // prefDlg = new DlgPreferences (0L);
    prefDlg = new DlgPreferences(this);
    // m_paPreferences->setEnabled (false);
    sDebugOut << endl;
}


void KMainWidget::slotToggleLogWindow()
{
    sDebugIn << endl;
    b_viewLogWindow = !b_viewLogWindow;
    if (b_viewLogWindow)
	logWindow->show();
    else
	logWindow->hide();
    sDebugOut << endl;
}


void KMainWidget::slotToggleAnimation()
{
    sDebugIn << endl;
    ksettings.b_useAnimation = !ksettings.b_useAnimation;

    if (!ksettings.b_useAnimation && animTimer->isActive()) {
	animTimer->stop();
	animTimer->start(1000);
	animCounter = 0;
    } else {
	animTimer->stop();
	animTimer->start(400);
    }
    sDebugOut << endl;
}


void KMainWidget::slotToggleSound()
{
    sDebugIn << endl;
    ksettings.b_useSound = !ksettings.b_useSound;
    sDebugOut << endl;
}


void KMainWidget::slotToggleOfflineMode()
{
    sDebugIn "ksettings.b_offlineMode ="<<ksettings.b_offlineMode<< endl;
    ksettings.b_offlineMode = !ksettings.b_offlineMode;
    if (ksettings.b_offlineMode) {
	log(i18n("Offline mode on."));
	pauseAll();
    } else {
	log(i18n("Offline mode off."));
    }
    m_paOfflineMode->setChecked(ksettings.b_offlineMode);

    checkQueue();
    sDebugOut << endl;
}


void KMainWidget::slotToggleExpertMode()
{
    sDebugIn << endl;
    ksettings.b_expertMode = !ksettings.b_expertMode;

    if (ksettings.b_expertMode) {
	log(i18n("Expert mode on."));
    } else {
	log(i18n("Expert mode off."));
    }
    m_paExpertMode->setChecked(ksettings.b_expertMode);
    sDebugOut << endl;
}


void KMainWidget::slotToggleUseLastDir()
{
    sDebugIn << endl;
    ksettings.b_useLastDir = !ksettings.b_useLastDir;

    if (ksettings.b_useLastDir) {
	log(i18n("Use last directory on."));
    } else {
	log(i18n("Use last directory off."));
    }
    sDebugOut << endl;
}


void KMainWidget::slotToggleAutoDisconnect()
{
    sDebugIn << endl;

    ksettings.b_autoDisconnect = !ksettings.b_autoDisconnect;

    if (ksettings.b_autoDisconnect) {
	log(i18n("Auto disconnect on."));
    } else {
	log(i18n("Auto disconnect off."));
    }
    m_paAutoDisconnect->setChecked(ksettings.b_autoDisconnect);
    sDebugOut << endl;

}


void KMainWidget::slotToggleAutoShutdown()
{


    sDebugIn << endl;
    ksettings.b_autoShutdown = !ksettings.b_autoShutdown;

    if (ksettings.b_autoShutdown) {
	log(i18n("Auto shutdown on."));
    } else {
	log(i18n("Auto shutdown off."));
    }
    m_paAutoShutdown->setChecked(ksettings.b_autoShutdown);
    sDebugOut << endl;

}


void KMainWidget::slotToggleAutoPaste()
{
    sDebugIn << endl;
    ksettings.b_autoPaste = !ksettings.b_autoPaste;

    if (ksettings.b_autoPaste) {
	log(i18n("Auto paste on."));
    } else {
	log(i18n("Auto paste off."));
    }
    m_paAutoPaste->setChecked(ksettings.b_autoPaste);
    sDebugOut << endl;


}

void KMainWidget::slotDock()
{
    sDebugIn << endl;
    if (ksettings.windowStyle == DOCKED) {
	ksettings.windowStyle = NORMAL;
    } else {
	ksettings.windowStyle = DOCKED;
    }
    setWindowStyle();
    sDebugOut << endl;
}


void KMainWidget::slotDropTarget()
{
    sDebugIn << endl;
    if (ksettings.windowStyle == DROP_TARGET) {
	ksettings.windowStyle = NORMAL;
    } else {
	ksettings.windowStyle = DROP_TARGET;
    }
    setWindowStyle();
    sDebugOut << endl;

}

void KMainWidget::slotNormal()
{
    sDebugIn << endl;
    if (ksettings.windowStyle == NORMAL) {
	ksettings.windowStyle = DROP_TARGET;
    } else {
	ksettings.windowStyle = NORMAL;
    }
    setWindowStyle();
    sDebugOut << endl;
}


void KMainWidget::slotPopupMenu(Transfer * item)
{
    sDebugIn << endl;
    myTransferList->clearSelection();
    myTransferList->setSelected(item, true);

    // select current item
    myTransferList->setCurrentItem(item);

    // set action properties only for this item
    slotUpdateActions();

    // popup transfer menu at the position
    QWidget *menu = guiFactory()->container("transfer", this);

    ((QPopupMenu *) menu)->popup(QCursor::pos());
    sDebugOut << endl;
}


void KMainWidget::setListFont()
{
    sDebugIn << endl;
    myTransferList->setFont(ksettings.listViewFont);
    sDebugOut << endl;

}


void KMainWidget::setWindowStyle()
{

    sDebugIn << endl;
    switch (ksettings.windowStyle) {
    case NORMAL:
	this->show();
	kdock->hide();
	kdrop->hide();
	// KWM::switchToDesktop( KWM::desktop( winId() ) );
	break;

  case DOCKED_MIN:
	this->hide();
	kdock->show();
	kdrop->hide();
	break;

  case DOCKED:
	this->show();
	kdock->show();
	kdrop->hide();
	break;

    case DROP_TARGET:
	this->show();
	kdock->hide();
	kdrop->show();
	break;
    }
    sDebugOut << endl;

}


void KMainWidget::slotUpdateActions()
{
    sDebugIn << endl;

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
	if (it.current()->isSelected()) {
	    item = it.current();
	    sel_items = totals_items;
	    index++;		// counting number of selected items
	    if (index == 1) {
		first_item = item;	// store first selected item
		if (totals_items > 0)
		    m_paMoveToBegin->setEnabled(true);

		m_paMoveToEnd->setEnabled(true);
	    } else {

		m_paMoveToBegin->setEnabled(false);
		m_paMoveToEnd->setEnabled(false);
	    }
	    // enable PAUSE, RESUME and RESTART only when we are online and not in offline mode
    sDebug << "-->ONLINE= " <<  ksettings.b_offlineMode << endl;
	    if (item == first_item &&  ksettings.b_offlineMode) {
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
		    sDebug << "STATUS IS  stopped" << item->getStatus() << endl;
		    break;
		}
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
			sDebug << "....................THE MODE  IS  MD_QUEUED " << item->getMode() << endl;
			m_paQueue->setChecked(true);
			break;
		    case Transfer::MD_SCHEDULED:
			sDebug << "....................THE MODE  IS  MD_SCHEDULED " << item->getMode() << endl;

			m_paTimer->setChecked(true);
			break;
		    case Transfer::MD_DELAYED:
			sDebug << "....................THE MODE  IS  MD_DELAYED " << item->getMode() << endl;

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

	}			// when item is selected
    }				// loop



    if (sel_items == totals_items - 1)
	m_paMoveToEnd->setEnabled(false);

    // enable all signals



    m_paQueue->blockSignals(false);
    m_paTimer->blockSignals(false);
    m_paDelay->blockSignals(false);
    sDebugOut << endl;
}


void KMainWidget::updateStatusBar()
{
    //sDebugIn<<endl;

    Transfer *item;
    QString tmpstr;

    int totalFiles = 0;
    int totalSize = 0;
    int totalSpeed = 0;
    QTime remTime;

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
    statusBar()->changeItem(i18n(" Time: %1 ").arg(remTime.toString()), ID_TOTAL_TIME);
    statusBar()->changeItem(i18n(" %1/s ").arg(KIO::convertSize(totalSpeed)), ID_TOTAL_SPEED);
    //sDebugOut<<endl;

}


void KMainWidget::disconnect()
{
    sDebugIn << endl;
    if (!b_online) {
	return;
    }

    if (!ksettings.b_expertMode) {
	if (KMessageBox::questionYesNo(this, i18n("Do you really want to disconnect?"), i18n("Question")) != KMessageBox::Yes) {
	    return;
	}
    }
    log(i18n("Disconnecting..."));
    system(ksettings.disconnectCommand.ascii());
    sDebugOut << endl;
}


void KMainWidget::slotCheckConnection()
{
    //sDebugIn<<endl;

    checkOnline();
    //sDebugOut<<endl;
}


void KMainWidget::checkOnline()
{
    //sDebugIn<<"b_online="<<b_online<<endl;

    bool old = b_online;

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifreq));

    // setup the device name according to the type of connection and link number
    sprintf(ifr.ifr_name, "%s%d", ConnectionDevices[ksettings.connectionType].ascii(), ksettings.linkNumber);

    bool flag = false;

    if (ksettings.connectionType != PERMANENT) {
	// get the flags for particular device
	if (ioctl(_sock, SIOCGIFFLAGS, &ifr) < 0) {
	    flag = true;
	    b_online = false;
	} else if (ifr.ifr_flags == 0) {
	    sDebug << "Can't get flags from interface " << ifr.ifr_name << endl;
	    b_online = false;
	} else if (ifr.ifr_flags & IFF_UP) {	// if (ifr.ifr_flags & IFF_RUNNING)
	    b_online = true;
	} else {
	    b_online = false;
	}
    } else {
	b_online = true;	// PERMANENT connection
    }

    m_paOfflineMode->setEnabled(b_online);

    if (b_online != old) {
	if (flag) {		// so that we write this only once when connection is changed
	    sDebug << "Unknown interface " << ifr.ifr_name << endl;
	}

	if (b_online) {
	    log(i18n("We are online!"));
	    checkQueue();
	} else {
	    log(i18n("We are offline!"));
	    pauseAll();
	}
    }
    //sDebugOut<<"b_online="<<b_online<<endl;

}


// Helper method for opening device socket




static int sockets_open()
{
    sDebugIn << endl;
    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
    ipx_sock = socket(AF_IPX, SOCK_DGRAM, 0);
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

    return ddp_sock;
}


/** No descriptions */
void KMainWidget::customEvent(QCustomEvent * _e)
{
    //sDebugIn<<endl;


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

    case Slave::SLV_ABORTED:
	e->getItem()->slotExecAbort(e->getMsg());
	break;
    case Slave::SLV_REMOVED:
	e->getItem()->slotExecRemove();
	break;

    case Slave::SLV_INFO:
	e->getItem()->logMessage(e->getMsg(),true);
	break;
    default:
	sDebug << "Unkow Result..die" << result << endl;
	assert(0);


    }

    //sDebugOut<<endl;

}








#include "kmainwidget.moc"
