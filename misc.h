/***************************************************************************
*                                   misc.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*
*    email        : pch@freeshell.org
*
***************************************************************************/

/*
 *  Copyright (C) 1999-2000 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef _MISC_H_
#define _MISC_H_


#include <qstring.h>
#include <qstringlist.h>
//QString removeHTML(const QString &str);
QString encodeString(const QString & e);
QStringList findNewestResources(const char *type, const QString & reldir);


#endif


// Local Variables:
// c-basic-offset: 4
// End:
