/***************************************************************************
*                                settings.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin          : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*
*    email        : pch@freeshell.org
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

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kwin.h>
#include <kmessagebox.h>

#include <qdir.h>

#include "kmainwidget.h"
#include "transferlist.h"
#include "droptarget.h"
#include "settings.h"
#include "common.h"
#include "version.h"

QString ConnectionDevices[6] = {
                                   "",
                                   "eth",
                                   "plip",
                                   "slip",
                                   "ppp",
                                   "isdn"
                               };


void
Settings::load()
{
    sDebug << "Loading settings" << endl;

    KConfig *config = kapp->config();

    // read system options
    config->setGroup("System");
    b_useSound = config->readBoolEntry("UseSound", DEF_UseSound);
    sDebug << "locating sounds..." << locate("data", DEF_SoundAdded) << endl;
    audioAdded = config->readEntry("Added", locate("data", DEF_SoundAdded));
    sDebug << "audioadded= " << audioAdded << endl;
    audioStarted = config->readEntry("Started", locate("data", DEF_SoundStarted));
    audioFinished = config->readEntry("Finished", locate("data", DEF_SoundFinished));
    audioFinishedAll = config->readEntry("FinishedAll", locate("data", DEF_SoundFinishedAll));
    b_useAnimation = config->readBoolEntry("UseAnimation", DEF_UseAnimation);


    // read connection options
    config->setGroup("Connection");

    b_reconnectOnError = config->readBoolEntry("ReconnectOnError", DEF_ReconnectOnError);
    reconnectTime = config->readNumEntry("ReconnectTime", DEF_ReconnectTime);
    reconnectRetries = config->readNumEntry("ReconnectRetries", DEF_ReconnectRetries);
    b_reconnectOnBroken = config->readBoolEntry("ReconnectOnBroken", DEF_ReconnectOnBroken);

    timeoutData = config->readNumEntry("TimeoutData", DEF_TimeoutData);
    timeoutDataNoResume = config->readNumEntry("TimeoutDataNoResume", DEF_TimeoutDataNoResume);

    connectionType = config->readNumEntry("ConnectionType", DEF_ConnectionType);
    linkNumber = config->readNumEntry("LinkNumber", DEF_LinkNumber);
    b_offlineMode = config->readBoolEntry("OfflineMode", DEF_OfflineMode);

    // read automation options
    config->setGroup("Automation");

    b_autoSave = config->readBoolEntry("AutoSave", DEF_AutoSave);
    autoSaveInterval = config->readNumEntry("AutoSaveInterval", DEF_AutoSaveInterval);

    b_autoDisconnect = config->readBoolEntry("AutoDisconnect", DEF_AutoDisconnect);
    disconnectCommand = config->readEntry("DisconnectCommand", DEF_DisconnectCommand);

    b_timedDisconnect = config->readBoolEntry("TimedDisconnect", DEF_TimedDisconnect);
    disconnectTime.setHMS(config->readNumEntry("DisconnectTimeHour"), config->readNumEntry("DisconnectTimeMinute"), 0);

    disconnectDate = QDate::currentDate();      // doesn't make sense to save it

    b_autoShutdown = config->readBoolEntry("AutoShutdown", DEF_AutoShutdown);
    b_autoPaste = config->readBoolEntry("AutoPaste", DEF_AutoPaste);

    // read limits options
    config->setGroup("Limits");

    maxSimultaneousConnections = config->readNumEntry("MaxSimConnections", DEF_MaxSimConnections);
    minimumBandwidth = config->readNumEntry("MinimumBandwidth", DEF_MinimumBandwidth);
    maximumBandwidth = config->readNumEntry("MaximumBandwidth", DEF_MaximumBandwidth);

    // read advanced options
    config->setGroup("Advanced");

    b_addQueued = config->readBoolEntry("AddQueued", DEF_AddQueued);
    b_showMain  = config->readBoolEntry("ShowMain", DEF_ShowMain);
    b_showIndividual = config->readBoolEntry("ShowIndividual", DEF_ShowIndividual);
    b_iconifyIndividual = config->readBoolEntry("IconifyIndividual", DEF_IconifyIndividual);
    b_advancedIndividual = config->readBoolEntry("AdvancedIndividual", DEF_AdvancedIndividual);

    b_removeOnSuccess = config->readBoolEntry("RemoveOnSuccess", DEF_RemoveOnSuccess);
    b_getSizes = config->readBoolEntry("GetSizes", DEF_GetSizes);
    b_expertMode = config->readBoolEntry("ExpertMode", DEF_ExpertMode);

    // read if the integration whith konqueror is enabled

    KConfig *cfg = new KConfig("konquerorrc", false, false);
    cfg->setGroup("HTML Settings");
    QString downloadManager=cfg->readEntry("DownloadManager");

    b_KonquerorIntegration=(downloadManager==KGET_APP_NAME)?true:false;

    // check if we already asked about konqueror integration
    if(config->readBoolEntry("FirstRun",true))
    {
        config->writeEntry("FirstRun",false);
        bool bAnswerYes = KMessageBox::questionYesNo(0L,i18n("This is the first time that you have run KGet.\nDo you want to enable the integration with Konqueror?"), i18n("Konqueror Integration")) == KMessageBox::Yes;
        if (bAnswerYes)
        {
            cfg->writeEntry("DownloadManager",KGET_APP_NAME);
            cfg->sync();
            b_KonquerorIntegration=true;
        }
    }
    delete cfg;


    // read search options
    config->setGroup("Search");
    b_searchFastest = config->readBoolEntry("SearchFastest", DEF_SearchFastest);
    searchItems = config->readNumEntry("SearchItems", DEF_SearchItems);
    timeoutSearch = config->readNumEntry("TimeoutSearch", DEF_TimeoutSearch);
    b_switchHosts = config->readBoolEntry("SwitchHosts", DEF_SwitchHosts);

    // read directory options
    config->setGroup("Directories");

    b_useLastDir = config->readBoolEntry("UseLastDirectory", DEF_UseLastDir);
    lastDirectory = config->readEntry("LastDirectory",
                                      QString("file:") + QDir::currentDirPath() );

    QStringList strList;

    strList = config->readListEntry("Items");

    defaultDirList.clear();
    QStringList::Iterator it = strList.begin();
    for (; it != strList.end(); ++it) {
        DirItem item;

        item.extRegexp = *it;
        ++it;
        item.defaultDir = *it;
        defaultDirList.append(item);
    }

    // read misc settings
    config->setGroup("Misc");

    QFont font = kapp->font(kmain->myTransferList);

    listViewFont = config->readFontEntry("Font", &font);

    // read main window geometry settings
    config->setGroup("MainGeometry");
    const QPoint point(-1,-1);
    mainPosition = config->readPointEntry("Position", &point);
    mainSize = config->readSizeEntry("Size");
    mainState = config->readUnsignedLongNumEntry("State", 0);

    // read drop target geometry settings
    config->setGroup("DropGeometry");
    dropPosition = config->readPointEntry("Position", &point);
    dropState = config->readUnsignedLongNumEntry("State", 0);
}


void Settings::save()
{
    sDebug << "Saving settings" << endl;

    KConfig *config = kapp->config();

    // write connection options
    config->setGroup("Connection");
    config->writeEntry("ReconnectOnError", b_reconnectOnError);
    config->writeEntry("ReconnectTime", reconnectTime);
    config->writeEntry("ReconnectRetries", reconnectRetries);
    config->writeEntry("ReconnectOnBroken", b_reconnectOnBroken);
    config->writeEntry("TimeoutData", timeoutData);
    config->writeEntry("TimeoutDataNoResume", timeoutDataNoResume);
    config->writeEntry("ConnectionType", connectionType);
    config->writeEntry("LinkNumber", linkNumber);
    config->writeEntry("OfflineMode", b_offlineMode);

    // write automation options
    config->setGroup("Automation");
    config->writeEntry("AutoSave", b_autoSave);
    config->writeEntry("AutoSaveInterval", autoSaveInterval);
    config->writeEntry("AutoDisconnect", b_autoDisconnect);
    config->writeEntry("DisconnectCommand", disconnectCommand);
    config->writeEntry("TimedDisconnect", b_timedDisconnect);
    config->writeEntry("DisconnectTimeHour", disconnectTime.hour());
    config->writeEntry("DisconnectTimeMinute", disconnectTime.minute());
    config->writeEntry("AutoShutdown", b_autoShutdown);
    config->writeEntry("AutoPaste", b_autoPaste);

    // write limits options
    config->setGroup("Limits");
    config->writeEntry("MaxSimConnections", maxSimultaneousConnections);
    config->writeEntry("MinimumBandwidth", minimumBandwidth);
    config->writeEntry("MaximumBandwidth", maximumBandwidth);

    // write advanced options
    config->setGroup("Advanced");
    config->writeEntry("ShowMain", b_showMain);
    config->writeEntry("AddQueued", b_addQueued);
    config->writeEntry("ShowIndividual", b_showIndividual);
    config->writeEntry("IconifyIndividual", b_iconifyIndividual);
    config->writeEntry("AdvancedIndividual", b_advancedIndividual);
    config->writeEntry("RemoveOnSuccess", b_removeOnSuccess);
    config->writeEntry("GetSizes", b_getSizes);
    config->writeEntry("ExpertMode", b_expertMode);
    config->writeEntry("KonquerorIntegration",b_KonquerorIntegration);


    // write search options
    config->setGroup("Search");
    config->writeEntry("SearchFastest", b_searchFastest);
    config->writeEntry("SearchItems", searchItems);
    config->writeEntry("TimeoutSearch", timeoutSearch);
    config->writeEntry("SwitchHosts", b_switchHosts);

    // write directory options
    config->setGroup("Directories");
    config->writeEntry("UseLastDirectory", b_useLastDir);
    config->writeEntry("LastDirectory", lastDirectory );
    DirList::Iterator it;
    QStringList lst;

    for (it = defaultDirList.begin(); it != defaultDirList.end(); ++it) {
        lst.append((*it).extRegexp);
        lst.append((*it).defaultDir);
    }
    config->writeEntry("Items", lst);

    // write system options
    config->setGroup("System");
    config->writeEntry("UseSound", b_useSound);
    config->writeEntry("Added", audioAdded);
    config->writeEntry("Started", audioStarted);
    config->writeEntry("Finished", audioFinished);
    config->writeEntry("FinishedAll", audioFinishedAll);

    config->writeEntry("UseAnimation", b_useAnimation);

    // write misc options
    config->setGroup("Misc");
    config->writeEntry("Font", listViewFont);

    // write main window geometry properties
    config->setGroup("MainGeometry");
    config->writeEntry("Position", kmain->pos());
    config->writeEntry("Size", kmain->size());
    config->writeEntry("State", KWin::info(kmain->winId()).state);

    // write drop target geometry properties
    config->setGroup("DropGeometry");
    config->writeEntry("Position", kdrop->pos());
    config->writeEntry("State", KWin::info(kdrop->winId()).state);


    config->sync();
}
