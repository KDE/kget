/***************************************************************************
*                                settings.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
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


#ifndef _SETTINGS_H
#define _SETTINGS_H

// common connection types
enum ConnectionType { PERMANENT = 0, ETHERNET, PLIP, SLIP, PPP, ISDN };

extern QString ConnectionDevices[];


// Connection settings
#define DEF_ReconnectOnError    true
#define DEF_ReconnectTime       1
#define DEF_ReconnectRetries    10
#define DEF_ReconnectOnBroken   true

#define DEF_TimeoutData         5
#define DEF_TimeoutDataNoResume 15

#define DEF_ConnectionType      PERMANENT
#define DEF_LinkNumber          0
#define DEF_OfflineMode         false

// Automation settings
#define DEF_AutoSave            true
#define DEF_AutoSaveInterval    10
#define DEF_AutoDisconnect      false
#define DEF_DisconnectCommand   "kppp -k"
#define DEF_TimedDisconnect     false
#define DEF_AutoShutdown        false
#define DEF_AutoPaste           false

// Limits settings
#define DEF_MaxSimConnections   2
#define DEF_MinimumBandwidth    1000
#define DEF_MaximumBandwidth    10000

// Advanced settings
#define DEF_AddQueued           true
#define DEF_ShowMain            false
#define DEF_ShowIndividual      false
#define DEF_IconifyIndividual   false
#define DEF_AdvancedIndividual  false
#define DEF_RemoveOnSuccess     true
#define DEF_GetSizes            true
#define DEF_ExpertMode          false

// Search settings
#define DEF_SearchFastest       false
#define DEF_SearchItems         20
#define DEF_TimeoutSearch       30
#define DEF_SwitchHosts         false

// Directories settings
#define DEF_UseLastDir          false

// System settings
#define DEF_UseSound            true

#define DEF_SoundAdded          "kget/sounds/added.wav"
#define DEF_SoundStarted        "kget/sounds/started.wav"
#define DEF_SoundFinished       "kget/sounds/finished.wav"
#define DEF_SoundFinishedAll    "kget/sounds/finishedall.wav"

#define DEF_UseAnimation        true

#define DEF_WindowStyle         NORMAL

// Misc settings
#define DEF_ToolbarPosition     KToolBar::Top
#define DEF_ShowStatusbar       true


#include <qdatetime.h>

#include <kwin.h>
#include <ktoolbar.h>
#include <qvaluelist.h>
struct DirItem
{
    QString extRegexp;
    QString defaultDir;
};

typedef QValueList < DirItem > DirList;

class Settings
{

public:

    Settings()
    {
    }
    ~Settings()
    {
    }

    void load();
    void save();

    // connection options
    bool b_reconnectOnBroken;
    bool b_reconnectOnError;

    uint reconnectTime;
    uint reconnectRetries;

    uint timeoutData;
    uint timeoutDataNoResume;

    uint connectionType;
    uint linkNumber;
    bool b_offlineMode;

    // automation options
    bool b_autoSave;
    uint autoSaveInterval;
    bool b_autoDisconnect;
    QString disconnectCommand;
    bool b_timedDisconnect;
    QDate disconnectDate;
    QTime disconnectTime;
    bool b_autoShutdown;
    bool b_autoPaste;

    // limits options
    uint maxSimultaneousConnections;
    uint minimumBandwidth;
    uint maximumBandwidth;

    // advanced options
    bool b_addQueued;
    bool b_showIndividual;
    bool b_iconifyIndividual;
    bool b_advancedIndividual;
    bool b_removeOnSuccess;
    bool b_getSizes;
    bool b_expertMode;
    bool b_KonquerorIntegration;
    bool b_showMain;
    // search options
    bool b_searchFastest;
    uint searchItems;
    uint timeoutSearch;
    bool b_switchHosts;

    // directories options
    bool b_useLastDir;
    QString lastDirectory;

    DirList defaultDirList;

    // system options
    bool b_useSound;

    QString audioAdded;
    QString audioStarted;
    QString audioFinished;
    QString audioFinishedAll;

    bool b_useAnimation;
    QFont listViewFont;


    KToolBar::BarPosition toolbarPosition;
    bool b_showStatusbar;

    // geometry settings
    QPoint mainPosition;
    QSize mainSize;
    unsigned long int mainState;

    QPoint dropPosition;
    unsigned long int dropState;
};

extern Settings ksettings;

#endif                          // _SETTINGS_H
