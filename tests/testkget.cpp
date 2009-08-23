/* This file is part of the KDE project

   Copyright (C) 2009 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "testkget.h"
#include "transfergrouphandler.h"
#include "kget.h"
#include "transfertreemodel.h"

TestKGet::TestKGet()
    : QObject(0),
      m_addedGH(0)
{
    connect(KGet::model(), SIGNAL(groupAddedEvent(TransferGroupHandler*)), SLOT(addedTransferGroupEvent(TransferGroupHandler*)));
    connect(KGet::model(), SIGNAL(removedAddedEvent(TransferGroupHandler*)), SLOT(removedTransferGroupEvent(TransferGroupHandler*)));
}

void TestKGet::addedTransferGroupEvent(TransferGroupHandler * group)
{
    m_addedGH = group;
}

void TestKGet::removedTransferGroupEvent(TransferGroupHandler * group)
{
    m_removedGH = group;
}

void TestKGet::simpleTest()
{
    QVERIFY(1 == 1);
}

void TestKGet::transferGroupTest()
{
    KGet::delGroup("testGroup");            // In case you already have one

    m_addedGH = 0;
    m_removedGH = 0;
    
    // Add Group
    QVERIFY(KGet::addGroup("testGroup"));
    QVERIFY(m_addedGH != 0);   // Should already have received the added group notification
    
    // Verify default Group parameters
    QVERIFY(m_addedGH->name() == "testGroup");
    QVERIFY(m_addedGH->status() == JobQueue::Running);
    QVERIFY(m_addedGH->size() == 0);
    QVERIFY(m_addedGH->totalSize() == 0);
    QVERIFY(m_addedGH->downloadedSize() == 0);
    QVERIFY(m_addedGH->uploadedSize() == 0);
    QVERIFY(m_addedGH->percent() == 0);
    QVERIFY(m_addedGH->downloadSpeed() == 0);
    QVERIFY(m_addedGH->uploadSpeed() == 0);
    
    // Delete newly added Group
    KGet::delGroup("testGroup");
    QVERIFY(m_removedGH != 0);   // Should already have received the removed group notification

    QVERIFY(m_removedGH->name() == "testGroup");
}

void TestKGet::transferGroupRepetitiveAddTest()
{
    for(int i=0; i < 100; i++)
    {
        // Adding...
        QVERIFY(KGet::addGroup("testGroup" + QString::number(i)));
        QVERIFY(m_addedGH != 0);   // Should already have received the added group notification
        QVERIFY(m_addedGH->name() == "testGroup" + QString::number(i));
    }
    
    for(int i=0; i < 100; i++)
    {
        // Removing...
        KGet::delGroup("testGroup" + QString::number(i));
        QVERIFY(m_removedGH != 0);   // Should already have received the removed group notification
        QVERIFY(m_removedGH->name() == "testGroup" + QString::number(i));        
    }
}

#include "testkget.moc"
