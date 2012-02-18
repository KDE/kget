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

#include "filedeletertest.h"
#include "../core/filedeleter.h"

#include <QtCore/QFile>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>

#include <KJob>
#include <KTemporaryFile>
#include <KUrl>

void FileDeleterTest::fileDeleterTest()
{
    //nothing started to delete yet
    QVERIFY(!FileDeleter::isFileBeingDeleted(KUrl()));
    QVERIFY(!FileDeleter::isFileBeingDeleted(KUrl("/tmp/aFile")));

    //create file that is going to be deleted
    KTemporaryFile file1;
    file1.setAutoRemove(false);
    QVERIFY(file1.open());
    const QString fileUrl1 = file1.fileName();
    QVERIFY(QFile::exists(fileUrl1));

    //nothing started to delete yet
    QVERIFY(!FileDeleter::isFileBeingDeleted(KUrl(fileUrl1)));

    //create two QObjects that will receive the result signal
    SignalReceiver receiver1;
    QSignalSpy spy1(&receiver1, SIGNAL(result()));
    QVERIFY(spy1.isEmpty());

    SignalReceiver receiver2;
    QSignalSpy spy2(&receiver1, SIGNAL(result()));
    QVERIFY(spy2.isEmpty());

    //delete the file
    FileDeleter::deleteFile(KUrl(fileUrl1), &receiver1, SIGNAL(result()));
    QVERIFY(FileDeleter::isFileBeingDeleted(KUrl(fileUrl1)));

    //deleting twice with the same receiver, still the method should only be called once
    FileDeleter::deleteFile(KUrl(fileUrl1), &receiver1, SIGNAL(result()));

    KJob *job = FileDeleter::deleteFile(KUrl(fileUrl1));
    connect(job, SIGNAL(result(KJob*)), &receiver2, SIGNAL(result()));

    //removal should be done by now
    QTest::qWait(5000);

    QVERIFY(!FileDeleter::isFileBeingDeleted(KUrl(fileUrl1)));
    QVERIFY(!QFile::exists(fileUrl1));

    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy2.count(), 1);
}

QTEST_MAIN(FileDeleterTest)

#include "filedeletertest.moc"
