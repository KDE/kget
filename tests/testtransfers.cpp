/* This file is part of the KDE project

   Copyright (C) 2009 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "testtransfers.h"
#include "transfergrouphandler.h"
#include "transferhandler.h"
#include "kget.h"

#include "kget_interface.h"
#include "transfer_interface.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingReply>
#include <QList>

#include <krun.h>
#include <qtest_kde.h>

Operation::Operation(Type t, OrgKdeKgetTransferInterface * transfer, TestTransfers * parent)
    : m_type(t),
      m_transfer(transfer),
      m_parent(parent)
{

    
    qsrand(1);
}

Operation::Operation(int randomNumber, OrgKdeKgetTransferInterface * transfer, TestTransfers * parent)
    : m_transfer(transfer),
      m_parent(parent)
{
    double probabilityIndex = 1.0 * randomNumber / 1000 * typeProbabilitySum();
    
//     kDebug(5001) << "probabilityIndex = " << probabilityIndex << " typeProbabilitySum = " << typeProbabilitySum();
    
    int type = 0;
    int probabilitySum = 0;
    
    while(int p = typeProbability(type))
    {
        probabilitySum += p;
        
//         kDebug(5001) << "probabilitySum = " << probabilitySum;
        
        if(probabilityIndex <= probabilitySum)
        {
            m_type = type;
            return;
        }

        type++;    
    }
}

void Operation::exec()
{
    switch(m_type)
    {
        case TransferStart:
            if(m_transfer)
                m_transfer->start();
            break;
        case TransferStop:      
            if(m_transfer)
                m_transfer->stop();
            break;
        case CreateTransfer:
            m_parent->createTransfer();
            break;
    }
}

int Operation::typeProbability(int type)
{
    switch(type)
    {
        case TransferStart:
            return 10;
        case TransferStop:            
            return 1;
        case CreateTransfer:
            return 1;
    }
    
    return 0;
}

int Operation::typeProbabilitySum()
{
    int type = 0;
    int probabilitySum = 0;
    
    while(int p = typeProbability(type))
    {
        probabilitySum += p;
        type++;
    }
    
//     kDebug() << "probSum = " << probabilitySum;
    
    return probabilitySum;
}

TestTransfers::TestTransfers()
{
    if(!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget")) 
    {
        KRun::runCommand("kget --showDropTarget --hideMainWindow", "kget", "kget", 0);
    } 
//     else 
//     {
//         OrgKdeKgetInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
//         kgetInterface.setDropTargetVisible(true);
//     }

    m_urlsList << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdebase-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdebase-runtime-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdebase-workspace-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdebindings-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdeedu-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdegames-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdegraphics-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdelibs-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdelibs-experimental-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdemultimedia-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdenetwork-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdepim-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdepim-runtime-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdepimlibs-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdeplasma-addons-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdesdk-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdetoys-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdeutils-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdewebdev-4.3.1.tar.bz2"
               << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/oxygen-icons-4.3.1.tar.bz2";
}

void TestTransfers::createTransfer()
{
    static int i=0;
    
    if(i >= m_urlsList.size())
    {
        kDebug(5001) << "No more URLS!";
        return;
    }
        
    
    QVERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"));
    
    OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
   
    QDBusReply<QStringList> reply = kgetInterface.addTransfer(m_urlsList[i], "/tmp/downloads/" + QString::number(i), false);

    if(reply.isValid() && reply.value().size())
    {
//         QList<QVariant> list = reply.value().variant().toList();
        kDebug(5001) << "TestTransfers::createTransfer -> transfer = " << reply.value();
       
        m_transferIfaces.append(new OrgKdeKgetTransferInterface("org.kde.kget", reply.value().first(), QDBusConnection::sessionBus()));
//         OrgKdeKgetTransferInterface transferInterface("org.kde.kget", reply.value().first(), QDBusConnection::sessionBus());
//         transferInterface.start();
    }
    else
        kDebug(5001) << "Reply NOT VALID!!!";
    
    i++;
}

void TestTransfers::simpleTest()
{
    QVERIFY(1 == 1);

    m_timerId = startTimer(50);
    
    QTest::qWait(10000000);
//     transferInterface.start();
    
//     kgetInterface.setDropTargetVisible(false);
    
//         virtual void addTransfer(const QString& src, const QString& destDir = QString(), 
//                               bool start = false);
   
}

void TestTransfers::timerEvent(QTimerEvent * event)
{
//     foreach(OrgKdeKgetTransferInterface * iface, m_transferIfaces)
//     {
//         if(i%7)
//             iface->start();
//         else if(i%11)
//             iface->stop();
//     }

    Operation * op = randomOperation(randomTransfer());
    op->exec();
    
    delete op;
}

Operation * TestTransfers::randomOperation(OrgKdeKgetTransferInterface * transfer)
{
    double num = 1.0 * qrand() / RAND_MAX * 1000;
    
    Operation * op = new Operation(num, transfer, this);
//     kDebug(5001) << "Random number = " << num << "  type = " << op->type();
    
    return op;
}

OrgKdeKgetTransferInterface * TestTransfers::randomTransfer()
{
    int transferIndex = 1.0 * qrand() / RAND_MAX * m_transferIfaces.size();

//     kDebug(5001) << "Random transfer = " << transferIndex;
    
    if(transferIndex >= m_transferIfaces.size())
        return 0;
    
    return m_transferIfaces[transferIndex];
}

QTEST_KDEMAIN(TestTransfers, GUI)

#include "testtransfers.moc"

