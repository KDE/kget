/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _KGET_GLOBALS_H
#define _KGET_GLOBALS_H

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

#define KGETVERSION  "v0.8.3"
#define KGET_APP_NAME      "kget"

// Try to pre-declare as much as you can, do not include headers here!
class KMainWidget;
class QString;

class Scheduler;
class GlobalStatus;

class TransferList;
class Transfer;

// Enumerates the operations that can be done on a transfer (must be global)
enum TransferOperation { OpResume, OpPause, OpStop, OpDelete, OtherOpsFollowing };


#endif

