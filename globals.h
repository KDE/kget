/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _KGET_GLOBALS_H
#define _KGET_GLOBALS_H

#include <qvaluelist.h>

//remove this include
#include <kdebug.h>

#define mDebugIn mDebug << ">>>>Entering "
#define mDebugOut mDebug << ">>>>Leaving "

#define sDebugIn sDebug << ">>>>Entering "
#define sDebugOut sDebug << ">>>>Leaving "

#define mDebug   kdDebug(DKGET)<<k_funcinfo << THREAD_ID
#define sDebug   kdDebug(DKGET)<<k_funcinfo << "|--( GUI )--| "
#define DKGET 0
#define THREAD_ID "|--TH_ID ( "<< currentThread()<<" )--| "

#define KGETVERSION "v1.7.2"
#define KGET_APP_NAME "kget"

// Try to pre-declare as much as you can, do not include headers here!
class KMainWidget;
class QString;

class Scheduler;
class Connection;
class GlobalStatus;

class TransferList;
class Transfer;

// Enumerates the commands that can be executed on a transfer (must be global)
enum TransferCommand { CmdResume, CmdRestart, CmdPause };

enum TransferMessage {
    MSG_FINISHED, MSG_RESUMED, MSG_PAUSED, MSG_REMOVED, MSG_ABORTED,
    MSG_QUEUED, MSG_SCHEDULED, MSG_DELAYED, MSG_CONNECTED, 
    MSG_CAN_RESUME, MSG_TOTSIZE, MSG_UPD_PROGRESS, MSG_UPD_SPEED
};

enum TransferStatus { ST_TRYING, ST_RUNNING, ST_STOPPED, ST_FINISHED };

enum SchedulerOperation { OpPasteTransfer, OpImportTextFile,
    OpImportTransfers, OpExportTransfers, OpRun, OpStop };

struct DirItem {
    QString extRegexp;
    QString defaultDir;
};

typedef QValueList<DirItem> DirList;

#endif
