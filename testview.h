#ifndef _TESTVIEW_H
#define _TESTVIEW_H

#include <qwidget.h>
#include <qlayout.h>
#include <qobject.h>

#include <kpushbutton.h>
#include <klistview.h>
#include <kmdichildview.h>

#include "viewinterface.h"

/**
 * This class are only to test the scheduler functions
 *
 */
 
class Scheduler;
class Transfer;

  
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
    bool update(Transfer * t = 0);

private:        
    Transfer * transfer;
};
 

class TestView : public KMdiChildView, public ViewInterface
{
Q_OBJECT
public:
    TestView(Scheduler * _scheduler, QWidget * parent = 0);
    ~TestView();  
    
    void initTable();
    void initConnections();

    // public methods inherited from the ViewInterface
    void schedulerCleared();
    void schedulerAddedItems( TransferList &);
    void schedulerRemovedItems( TransferList &);
    void schedulerChangedItems( TransferList &);
    void schedulerStatus( GlobalStatus * );

public slots:
    void setPriority1() {setPriority(1);}
    void setPriority2() {setPriority(2);} 
    void setPriority3() {setPriority(3);}
    void setPriority4() {setPriority(4);}
    void setPriority5() {setPriority(5);}
    
private:
    void setPriority(int n);
    
    Scheduler * scheduler;
    KListView * listView;

    KPushButton * btSetPrior1;
    KPushButton * btSetPrior2;
    KPushButton * btSetPrior3;
    KPushButton * btSetPrior4;
    KPushButton * btSetPrior5;
    
    KPushButton * btResume;
    KPushButton * btStop;
    
    QGridLayout * layout1;
    QGridLayout * layout2;
};


#endif
