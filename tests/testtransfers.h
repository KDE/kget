/* This file is part of the KDE project

   Copyright (C) 2009 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _TESTTRANSFERS_H
#define _TESTTRANSFERS_H

#include <QtTest/QtTest>

class OrgKdeKgetTransferInterface;
class TestTransfers;

class Operation
{
public:
    // The Type enums numbers must be consecutive!!!
    enum Type
    {
        TransferStart = 0,
        TransferStop  = 1,
        CreateTransfer = 2
    };
    
    Operation(Type t, OrgKdeKgetTransferInterface * transfer, TestTransfers * parent);
    Operation(int randomNumber, OrgKdeKgetTransferInterface * transfer, TestTransfers * parent);
    
    void exec();
    
    int typeProbability(int type);
    int typeProbabilitySum();
    
    int type()   {return m_type;}
    
private:
    
    int m_type;
    OrgKdeKgetTransferInterface * m_transfer;
    TestTransfers * m_parent;

    int m_probabilityIndex;
};

class TestTransfers: public QObject
{
Q_OBJECT

public:
    TestTransfers();

    void createTransfer();
    
private slots:
    void simpleTest();
    
private:
    void timerEvent(QTimerEvent * event);
    
    Operation * randomOperation(OrgKdeKgetTransferInterface * transfer);
    OrgKdeKgetTransferInterface * randomTransfer();
    
    int m_timerId;
    QList<OrgKdeKgetTransferInterface *> m_transferIfaces;
    
    QStringList m_urlsList;
};

#endif
