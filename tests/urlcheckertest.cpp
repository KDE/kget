/* This file is part of the KDE project

   Copyright (C) 2025 Johnny Jazeix <jazeix@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "urlcheckertest.h"
#include "../core/urlchecker.h"

#include <QtTest>

UrlCheckerTest::UrlCheckerTest(QObject *parent)
    : QObject(parent)
{
}

void UrlCheckerTest::testRemoveDuplicates()
{
    using namespace Qt::Literals::StringLiterals;
    UrlChecker urlchecker(UrlChecker::Folder);
    QList<QUrl> urls;
    urls << QUrl(u"file:different/path"_s) << QUrl(u"file:same/path"_s) << QUrl(u"file:same//path"_s) << QUrl(u"file:different/path/"_s)
         << QUrl(u"file:same/path/../path"_s) << QUrl(u"file:same/path/"_s);
    urlchecker.removeDuplicates(urls);
    QCOMPARE(urls.size(), 2);
}

QTEST_MAIN(UrlCheckerTest)

#include "moc_urlcheckertest.cpp"
