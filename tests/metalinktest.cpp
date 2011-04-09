/***************************************************************************
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

#include "metalinktest.h"
#include "../ui/metalinkcreator/metalinker.h"

#include <QtTest/QtTest>

void MetalinkTest::testFilePath()
{
    QFETCH(QString, string);
    QFETCH(bool, result);

    KGetMetalink::File file;
    file.name = string;

    QCOMPARE(file.isValidNameAttribute(), result);
}

void MetalinkTest::testFilePath_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<bool>("result");

    QTest::newRow("traversal up relative to root") << "/../foo/bla" << false;
    QTest::newRow("traversal up at beginning") << "../foo/bla" << false;
    QTest::newRow("traversal up inside") << "bla/../foo/bla" << false;
    QTest::newRow("traversal up at end") << "foo/bla/.." << false;
    QTest::newRow("no file name") << "foo/bla/" << false;
    QTest::newRow("acceptable traversal down, contains path component") << "foo/bla" << true;
}

void MetalinkTest::testUrl()
{
    QFETCH(KUrl, url);
    QFETCH(bool, result);

    KGetMetalink::Url data;
    data.url = url;

    QCOMPARE(data.isValid(), result);
}

void MetalinkTest::testUrl_data()
{
    QTest::addColumn<KUrl>("url");
    QTest::addColumn<bool>("result");

    QTest::newRow("empty url") << KUrl() << false;
    QTest::newRow("no host") << KUrl("http://") << false;
    QTest::newRow("empty protocol") << KUrl("www.example.com") << false;
    QTest::newRow("valid url") << KUrl("http://www.example.com") << true;
}

void MetalinkTest::testMetaUrl()
{
    QFETCH(KUrl, url);
    QFETCH(QString, type);
    QFETCH(bool, result);

    KGetMetalink::Metaurl data;
    data.url = url;
    data.type = type;

    QCOMPARE(data.isValid(), result);
}

void MetalinkTest::testMetaUrl_data()
{
    QTest::addColumn<KUrl>("url");
    QTest::addColumn<QString>("type");
    QTest::addColumn<bool>("result");

    QTest::newRow("empty url") << KUrl() << "torrent" << false;
    QTest::newRow("no host") << KUrl("http://") << "torrent" << false;
    QTest::newRow("empty protocol") << KUrl("www.example.com") << "torrent" << false;
    QTest::newRow("empty type") << KUrl("http://www.example.com") << QString() << false;
    QTest::newRow("valid url") << KUrl("http://www.example.com") << "torrent" << true;
}

QTEST_MAIN(MetalinkTest)

#include "metalinktest.moc"
