#include "testkget.h"
#include "transfergrouphandler.h"
#include "kget.h"

TestKGet::TestKGet()
    : m_addedGH(0)
{
    KGet::addObserver(this);
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

void TestKGet::transferGroupRipetitiveAddTest()
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
