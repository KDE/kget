#include <qsizepolicy.h>
#include <qstring.h>

#include "globals.h"

#include "testview.h"
#include "transfer.h"
#include "transferlist.h"
#include "scheduler.h"

TestViewItem::TestViewItem( KListView * parent, Transfer * _transfer )
    : KListViewItem(parent),
      transfer(_transfer)
{
    update();
}
 
TestViewItem::~TestViewItem()
{

}

//Maybe I'll implement this function
QString TestViewItem::key(int column, bool ascending)
{
    if(column == 0)
        {
        //return 
    }
    return KListViewItem::key(column, ascending);
}

bool TestViewItem::update(Transfer * t)
{
    if ( t==0 || t==transfer )
        {
        setText(0, QString().setNum(transfer->getPriority()));
        setText(1, QString().setNum(transfer->getStatus()));
        setText(2, QString().setNum(transfer->getPercent()));
        setText(3, transfer->getSrc().url());
        setText(4, transfer->getDest().url());
        return true;
    }
    return false;
}


TestView::TestView(Scheduler * _scheduler, QWidget * parent)
    : ViewInterface(_scheduler),
      scheduler(_scheduler)
{
    //listView
    listView = new KListView(this);
    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, false);
    listView->setSelectionMode(QListView::Extended);
    initTable();
    
    //button area
    btSetPrior1 = new KPushButton("Priority -> 1", this);
    btSetPrior2 = new KPushButton("Priority -> 2", this);
    btSetPrior3 = new KPushButton("Priority -> 3", this);
    btSetPrior4 = new KPushButton("Priority -> 4", this);
    btSetPrior5 = new KPushButton("Priority -> 5", this);
    btResume = new KPushButton("Resume", this);
    btStop = new KPushButton("Stop", this);

    btSetPrior1->setFixedHeight(30);
    btSetPrior2->setFixedHeight(30);
    btSetPrior3->setFixedHeight(30);
    btSetPrior4->setFixedHeight(30);
    btSetPrior5->setFixedHeight(30);
    btResume->setFixedHeight(30);
    btStop->setFixedHeight(30);

    //layout adjust
    layout2 = new QGridLayout(0, 2, 5);
    layout2->addWidget(btSetPrior1, 1, 0);
    layout2->addWidget(btSetPrior2, 1, 1);
    layout2->addWidget(btSetPrior3, 1, 2);
    layout2->addWidget(btSetPrior4, 1, 3);
    layout2->addWidget(btSetPrior5, 1, 4);
    layout2->addWidget(btResume, 0, 0);
    layout2->addWidget(btStop, 0, 1);
    
    
    layout1 = new QGridLayout(this, 2, 1);
    layout1->addWidget(listView, 1, 1);
    layout1->addLayout(layout2, 2, 1);
    
    //Connections
    initConnections();
}

TestView::~TestView()
{

}

void TestView::initConnections()
{
    //list view
    connect(listView, SIGNAL( selectionChanged(QListViewItem *)), this, SLOT(updateSelection( QListViewItem * )));

    //buttons
    connect(btSetPrior1, SIGNAL( clicked() ), this, SLOT( setPriority1() ));
    connect(btSetPrior2, SIGNAL( clicked() ), this, SLOT( setPriority2() ));
    connect(btSetPrior3, SIGNAL( clicked() ), this, SLOT( setPriority3() ));
    connect(btSetPrior4, SIGNAL( clicked() ), this, SLOT( setPriority4() ));
    connect(btSetPrior5, SIGNAL( clicked() ), this, SLOT( setPriority5() ));

}

void TestView::initTable()
{
    listView->addColumn(QString("Priority"));
    listView->addColumn(QString("Status"));
    listView->addColumn(QString("Progress"));
    listView->addColumn(QString("Source"));
    listView->addColumn(QString("Dest"));
}

void TestView::setPriority(int n)
{
    sDebugIn << endl;
    
    TransferList list(scheduler);
    
    QListViewItemIterator it(listView);
    
    while ( it.current() ) 
        {
        if ( it.current()->isSelected() )
            list.addTransfer( ((TestViewItem *)it.current())->getTransfer() );
        ++it;
    }
    schedSetPriority(list, n);

    sDebugOut << endl;
}

/**
 * SLOTS
 */

void TestView::schedulerCleared()
{

}

void TestView::schedulerAddedItems( TransferList & list)
{
    sDebugIn << endl;
    
    TransferList::iterator it;
    TransferList::iterator endList = list.end();
    
    for(it = list.begin(); it != endList; ++it)
        {
        listView->insertItem(new TestViewItem(listView, *it));
    }
    
    sDebugOut << endl;
}

void TestView::schedulerRemovedItems( TransferList & list)
{

}

void TestView::schedulerChangedItems( TransferList & list)
{
    sDebugIn << endl;
    
    TransferList::iterator it;
    TransferList::iterator endList = list.end();
    
    
    for(it = list.begin(); it != endList; ++it)
        {
        QListViewItemIterator itemIter(listView);
        while(!itemIter.current())
            {
            ((TestViewItem *)itemIter.current())->update(*it);
        }
    }
    
    sDebugOut << endl;
}

void TestView::schedulerStatus( GlobalStatus * )
{

}


#include "testview.moc"
