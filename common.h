/***************************************************************************
*                                   common.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _COMMON_H
#define _COMMON_H


#include <kdebug.h>

#define mDebugIn mDebug << ">>>>Entering"
#define mDebugOut mDebug << ">>>>Leaving"

#define sDebugIn sDebug << ">>>>Entering"
#define sDebugOut sDebug << ">>>>Leaving"


#define mDebug   kdDebug(DKGET)<<k_funcinfo << THREAD_ID
#define sDebug   kdDebug(DKGET)<<k_funcinfo << "|--( GUI )--| "
#define DKGET 0
#define THREAD_ID "|--TH_ID ( "<< currentThread()<<" )--| "

#define Min(x, y)       (((x) < (y)) ? (x) : (y))

#define TIME_OUT 35
#endif				// common
