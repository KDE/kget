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

#include <qdatetime.h>
#include <qvaluelist.h>
#include <qfont.h>
#include "globals.h"

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


    // geometry settings
    QPoint mainPosition;
    QSize mainSize;
    unsigned long int mainState;

    QPoint dropPosition;
    unsigned long int dropState;
};

extern Settings ksettings;

#endif
