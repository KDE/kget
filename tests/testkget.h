#ifndef _TESTKGET_H
#define _TESTKGET_H

#include <QtTest/QtTest>

#include "observer.h"

class TestKGet: public QObject, public ModelObserver, public TransferGroupObserver, public TransferObserver
{
Q_OBJECT

public:
    TestKGet();
    
    void addedTransferGroupEvent(TransferGroupHandler * group);
    void removedTransferGroupEvent(TransferGroupHandler * group);

private slots:
    void simpleTest();
    void transferGroupTest();
    void transferGroupRipetitiveAddTest();
    
private:
    TransferGroupHandler * m_addedGH;
    TransferGroupHandler * m_removedGH;
};

#endif
