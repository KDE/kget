#include <qsizepolicy.h>
#include <qstring.h>

#include "globals.h"

#include "testview.h"
#include "transfer.h"
#include "transferlist.h"

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
        //Re-read the transfer informations
        transferInfo = transfer->getInfo();

        setText(0, QString().setNum(transferInfo.priority));
        setText(1, QString().setNum(transferInfo.status));
        setText(2, QString().setNum(transferInfo.percent));
        setText(3, transferInfo.src.url());
        setText(4, transferInfo.dest.url());
        return true;
    }
    return false;
}


TestView::TestView(QWidget * parent)
    : KMdiChildView(parent, "TestView-CV"), ViewInterface()
{
    //listView
    listView = new KListView(this);
    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, false);
    listView->setAllColumnsShowFocus(true);
    listView->setSelectionMode(QListView::Extended);
    listView->setSorting(0);
    initTable();
    
    //button area
    btSetPrior1 = new KPushButton("Priority -> 1", this);
    btSetPrior2 = new KPushButton("Priority -> 2", this);
    btSetPrior3 = new KPushButton("Priority -> 3", this);
    btSetPrior4 = new KPushButton("Priority -> 4", this);
    btSetPrior5 = new KPushButton("Priority -> 5", this);
    btResume = new KPushButton("Resume", this);
    btPause = new KPushButton("Stop", this);
    btRemove = new KPushButton("Remove", this);
    btPrintT = new KPushButton("Print list", this);
    
    btSetPrior1->setFixedHeight(30);
    btSetPrior2->setFixedHeight(30);
    btSetPrior3->setFixedHeight(30);
    btSetPrior4->setFixedHeight(30);
    btSetPrior5->setFixedHeight(30);
    btResume->setFixedHeight(30);
    btPause->setFixedHeight(30);
    btRemove->setFixedHeight(30);
    btPrintT->setFixedHeight(30);

    //layout adjust
    layout2 = new QGridLayout(0, 2, 5);
    layout2->addWidget(btSetPrior1, 1, 0);
    layout2->addWidget(btSetPrior2, 1, 1);
    layout2->addWidget(btSetPrior3, 1, 2);
    layout2->addWidget(btSetPrior4, 1, 3);
    layout2->addWidget(btSetPrior5, 1, 4);
    layout2->addWidget(btResume, 0, 0);
    layout2->addWidget(btPause, 0, 1);
    layout2->addWidget(btRemove, 0, 2);    
    layout2->addWidget(btPrintT, 0, 3);
    
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
    //### connect(listView, SIGNAL( selectionChanged(QListViewItem *)), this, SLOT(updateSelection( QListViewItem * )));

    //buttons
    connect(btSetPrior1, SIGNAL( clicked() ), this, SLOT( setPriority1() ));
    connect(btSetPrior2, SIGNAL( clicked() ), this, SLOT( setPriority2() ));
    connect(btSetPrior3, SIGNAL( clicked() ), this, SLOT( setPriority3() ));
    connect(btSetPrior4, SIGNAL( clicked() ), this, SLOT( setPriority4() ));
    connect(btSetPrior5, SIGNAL( clicked() ), this, SLOT( setPriority5() ));

    connect(btResume, SIGNAL( clicked() ), this, SLOT( resume() ));
    connect(btPause, SIGNAL( clicked() ), this, SLOT( pause() ));
    connect(btRemove, SIGNAL( clicked() ), this, SLOT( remove() ));
    connect(btPrintT, SIGNAL( clicked() ), this, SLOT( printList() ));
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
    
    TransferList list;
    
    QListViewItemIterator it(listView);
    
    while ( it.current() ) 
        {
        if ( it.current()->isSelected() )
            {
            //sDebug << "###" << endl;
            list.addTransfer( ((TestViewItem *)it.current())->getTransfer() );
        }
        ++it;
    }
    schedSetPriority(list, n);

    sDebugOut << endl;
}

void TestView::resume()
{
    sDebugIn << endl;
    
    TransferList list;
    
    QListViewItemIterator it(listView);
    
    while ( it.current() ) 
        {
        if ( it.current()->isSelected() )
            {
            //sDebug << "###" << endl;
            list.addTransfer( ((TestViewItem *)it.current())->getTransfer() );
        }
        ++it;
    }
    schedSetCommand(list, CmdResume);

    sDebugOut << endl;
}

void TestView::pause()
{
    sDebugIn << endl;
    
    TransferList list;
    
    QListViewItemIterator it(listView);
    
    while ( it.current() ) 
        {
        if ( it.current()->isSelected() )
            {
            //sDebug << "###" << endl;
            list.addTransfer( ((TestViewItem *)it.current())->getTransfer() );
        }
        ++it;
    }
    schedSetCommand(list, CmdPause);

    sDebugOut << endl;
}

void TestView::remove()
{
    sDebugIn << endl;
    
    TransferList list;
    
    QListViewItemIterator it(listView);
    
    while ( it.current() ) 
        {
        if ( it.current()->isSelected() )
            {
            //sDebug << "###" << endl;
            list.addTransfer( ((TestViewItem *)it.current())->getTransfer() );
        }
        ++it;
    }
    schedDelItems(list);

    sDebugOut << endl;
}

void TestView::printList()
{
    schedDebugOperation(OpPrintTransferList);
}

void TestView::schedulerCleared()
{

}

void TestView::schedulerAddedItems( TransferList list)
{
    sDebugIn << endl;
    
    TransferList::iterator it;
    TransferList::iterator endList = list.end();
    
    for(it = list.begin(); it != endList; ++it)
        {
	//sDebug << "item" << endl;
        listView->insertItem(new TestViewItem(listView, *it));
    }
    
    sDebugOut << endl;
}

void TestView::schedulerRemovedItems( TransferList list)
{
    sDebugIn << endl;
    
    TransferList::iterator it = list.begin();
    TransferList::iterator endList = list.end();
    
    
    for(; it != endList; ++it)
        {
        //sDebug << "-----" << endl;
        QListViewItemIterator itemIter(listView);
        while(itemIter.current())
            {
            //sDebug << ":::::" << endl;
            
            if(((TestViewItem*)itemIter.current())->getTransfer() == *it)
                delete(itemIter.current());
            
            itemIter++;
        }
    }
    
    listView->sort();
    
    sDebugOut << endl;
}

void TestView::schedulerChangedItems( TransferList list)
{
    sDebugIn << endl;
    
    TransferList::iterator it = list.begin();
    TransferList::iterator endList = list.end();
    
    
    for(; it != endList; ++it)
        {
        //sDebug << "-----" << endl;
        QListViewItemIterator itemIter(listView);
        while(itemIter.current())
            {
            //sDebug << ":::::" << endl;
            
            ((TestViewItem *)itemIter.current())->update(*it);
            
            itemIter++;
        }
    }
    
    listView->sort();
    
    sDebugOut << endl;
}

void TestView::schedulerStatus( GlobalStatus * )
{

}


#include "testview.moc"
