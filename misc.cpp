/***************************************************************************
*                                   misc.cpp
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

#include <stdio.h>

#include <qregexp.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <qfileinfo.h>

#include "misc.h"


/*
QString removeHTML(const QString &str) {
 QString res = str;
 int pos;
 while ( (pos = res.find('<')) != -1) {
   int pos2 = res.find('>', pos);
   if (pos2 == -1) {
     pos2 = res.length()+1;
   }
   res.remove(pos, pos2-pos+1);
 }
 return res;
}
    */

// This function is taken from khtml, distributed under the LGPL
QString encodeString(const QString & e)
{
    static const char *safe = "$-._!*(),";      /* RFC 1738 */
    unsigned pos = 0;
    QString encoded;
    char buffer[5];

    while (pos < e.length()) {
        unsigned char c = (unsigned char) e[pos];

        if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (strchr(safe, c))) {
            encoded += c;
        } else if (c == ' ') {
            encoded += '+';
        } else if (c == '\n') {
            encoded += "%0D%0A";
        } else if (c != '\r') {
            sprintf(buffer, "%%%02X", (int) c);
            encoded += buffer;
        }

        pos++;
    }

    return encoded;
}


QStringList findNewestResources(const char *type, const QString & reldir)
{
    QStringList dirlist = KGlobal::dirs()->findDirs(type, reldir);
    QFileInfoList fileinfos;
    QStringList files;

    for (QStringList::ConstIterator it = dirlist.begin(); it != dirlist.end(); it++) {
        qDebug("Dir: %s", (*it).latin1());
        QDir dir(*it);
        const QFileInfoList *entries = dir.entryInfoList(QDir::Files);
        QFileInfoListIterator it2(*entries);

        for (; it2.current(); ++it2) {
            // Do we have one of the same name already?
            QFileInfoListIterator it3(fileinfos);

            for (; it3.current(); ++it3)
                if (it3.current()->fileName() == it2.current()->fileName())
                    if (it2.current()->lastModified() > it3.current()->lastModified()) {
                        qDebug("Discarded %s", it3.current()->filePath().latin1());
                        goto discarded;
                    } else {
                        qDebug("Discarded %s", it2.current()->filePath().latin1());
                        fileinfos.remove(it3.current());
                        break;
                    }
            fileinfos.append(new QFileInfo(*it2.current()));
            qDebug("Append %s", it2.current()->fileName().latin1());
discarded:
            ;
        }
    }

    QFileInfoListIterator it4(fileinfos);

    for (; it4.current(); ++it4)
        files.append((*it4)->filePath());

    return files;
}
