/**************************************************************************
*   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
*   Code mostly from email from Will Stephenson <wstephenson@suse.de>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#ifndef KGET_FILE_DELETER_TEST
#define KGET_FILE_DELETER_TEST

#include <QtCore/QObject>

class SignalReceiver : public QObject
{
    Q_OBJECT

    signals:
        void result();
};

class FileDeleterTest : public QObject
{
    Q_OBJECT

    private slots:
        void fileDeleterTest();
};

#endif
