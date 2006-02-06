/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef KGET_GLOBALS_H
#define KGET_GLOBALS_H

#define mDebugIn mDebug << ">>>>Entering "
#define mDebugOut mDebug << ">>>>Leaving "

#define sDebugIn sDebug << ">>>>Entering "
#define sDebugOut sDebug << ">>>>Leaving "

#define mDebug   kDebug(DKGET)<<k_funcinfo << THREAD_ID
#define sDebug   kDebug(DKGET)<<k_funcinfo << "|--( GUI )--| "
#define DKGET 0
#define THREAD_ID "|--TH_ID ( "<< currentThread()<<" )--| "

#define KGETVERSION "2dev"
#define KGET_APP_NAME "kget"

// Try to pre-declare as much as you can, do not include headers here!
class Scheduler;
class Connection;
class GlobalStatus;

class TransferList;
class Transfer;

// Enumerates the commands that can be executed on a transfer (must be global)
enum TransferCommand { CmdResume, CmdRestart, CmdPause };

// enum TransferMessage {
//     MSG_FINISHED, MSG_RESUMED, MSG_STOPPED, MSG_REMOVED, MSG_ABORTED,
//     MSG_QUEUED, MSG_SCHEDULED, MSG_DELAYED, MSG_CONNECTED, 
//     MSG_CAN_RESUME, MSG_TOTSIZE, MSG_UPD_PROGRESS, MSG_UPD_SPEED,
//     MSG_DELAY_FINISHED
// };

// enum TransferStatus { ST_TRYING, ST_RUNNING, ST_STOPPED, ST_FINISHED };

enum SchedulerOperation { OpPasteTransfer, OpImportTextFile,
    OpImportTransfers, OpExportTransfers, OpRun, OpStop };

// used to debug the scheduler from the views
enum SchedulerDebugOp { OpPrintTransferList };
#endif
