/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _TESTVIEW_H
#define _TESTVIEW_H

#include <qwidget.h>
#include <qlayout.h>
#include <qobject.h>

#include <kpushbutton.h>
#include <klistview.h>
#include <kmdichildview.h>

#include "core/viewinterface.h"
#include "core/transfer.h"

/**
 * This class are only to test the scheduler functions
 *
 */
 
  
class TestViewItem : public KListViewItem
{
public:
    TestViewItem( KListView * parent, Transfer * _tranfer);
    ~TestViewItem();

    QString key(int column, bool ascending);    

    Transfer * getTransfer() {return transfer;}
        
    /**
     * Update function: if transfer is the same object the TestViewItem has the
     * item is update, otherwise not. If transfer = 0, no check is done and the 
     * item is updated.
     */
    inline bool update(Transfer * t = 0);

private:        
    Transfer * transfer;
    Transfer::Info transferInfo;
};
 

class TestView : public KMdiChildView, public ViewInterface
{
Q_OBJECT
public:
    TestView(QWidget * parent = 0);
    ~TestView();
    
    void initTable();
    void initConnections();

    // public methods inherited from the ViewInterface
    void schedulerCleared();
    void schedulerAddedItems( const TransferList& );
    void schedulerRemovedItems( const TransferList& );
    void schedulerChangedItems( const TransferList& );
    void schedulerStatus( GlobalStatus * );

public slots:
    void setPriority1() {setPriority(1);}
    void setPriority2() {setPriority(2);} 
    void setPriority3() {setPriority(3);}
    void setPriority4() {setPriority(4);}
    void setPriority5() {setPriority(5);}
    
    void resume();
    void pause();
    void remove();
    void printList();

private:
    void setPriority(int n);
    
    KListView * listView;

    KPushButton * btSetPrior1;
    KPushButton * btSetPrior2;
    KPushButton * btSetPrior3;
    KPushButton * btSetPrior4;
    KPushButton * btSetPrior5;
    
    KPushButton * btResume;
    KPushButton * btPause;
    KPushButton * btRemove;
    KPushButton * btPrintT;
    
    QGridLayout * layout1;
    QGridLayout * layout2;
};


#endif
