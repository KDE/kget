/***************************************************************************
*                                http_defaults.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*
*    email        : pch@freeshell.org
*
***************************************************************************/

/* This file is part of the KDE libraries
   Copyright (C) 2001 Waldo Bastian <bastian@kde.org>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KIO_HTTP_SLAVE_DEFAULTS_H
#define _KIO_HTTP_SLAVE_DEFAULTS_H

// CACHE SETTINGS
#define DEFAULT_MAX_CACHE_SIZE          5120	//  5 MB
#define DEFAULT_MAX_CACHE_AGE           60*60*24*14	// 14 DAYS
#define DEFAULT_CACHE_EXPIRE            10	// 10 SECS
#define DEFAULT_CLEAN_CACHE_INTERVAL    30*60	// 30 MINS
#define DEFAULT_CACHE_CONTROL           KIO::CC_Verify	// Verify with remote
#define CACHE_REVISION                  "7\n"	// Cache version

// DEFAULT USER AGENT KEY - ENABLES OS NAME
#define DEFAULT_USER_AGENT_KEYS         "o"	// Show OS

// MAXIMUM AMOUNT OF DATA THAT CAN BE SAFELY SENT OVER IPC
#define MAX_IPC_SIZE                    1024*8

// AMOUNT OF DATA TO OBTAIN FROM THE SERVER BY DEFAULT
#define DEFAULT_BUF_SIZE                1024*4

// SOME DEFAULT HEADER VALUES
#define DEFAULT_LANGUAGE_HEADER         "en"
#define DEFAULT_MIME_TYPE               "text/html"
#define DEFAULT_FULL_CHARSET_HEADER     "iso-8859-1, utf-8, *"
#define DEFAULT_PARIAL_CHARSET_HEADER   ", utf-8, *"

#define DEFAULT_ACCEPT_HEADER           "text/*, image/jpeg, image/png, image/*, */*"

#endif
