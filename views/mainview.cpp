/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qstring.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qfont.h>
#include <qimage.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kio/global.h>
#include <kimageeffect.h>

#include "mainview.h"
#include "transferlist.h"
#include "transfer.h"

MainViewGroupItem::MainViewGroupItem(MainView * parent, Group * g)
    : QListViewItem(parent),
      group(g)
{
    setOpen(true);
}

void MainViewGroupItem::updateContents(bool updateAll)
{
    Group::Info info = group->info();
    
    if(updateAll)
    {
        setText(1, info.name);
        setText(3, KIO::convertSize(info.totalSize));
    }
}

void MainViewGroupItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    //QListViewItem::paintCell(p, cg, column, width, align);
    
/*    QPixmap * topLeftPix = new QPixmap(
        KImageEffect::gradient( QSize( width, height() ),
        cg.brush(QColorGroup::Background).color(), 
        cg.brush(QColorGroup::Background).color().light(110),
        KImageEffect::VerticalGradient ) );*/
        

//     p->drawPixmap(0,0, *topLeftPix);
    p->fillRect(0,0,width, height(), cg.brush(QColorGroup::Background));
    
   
    if(column == 0)
    {        
         p->drawPixmap(3, 0, SmallIcon("package"));
    }
    else if(column == 1)
    {
        QFont f(p->font());
        f.setBold(true);
        p->setFont( f );
        p->setPen(QPen(cg.brush(QColorGroup::Background).color().dark(110),2));
        p->drawRoundRect(0,0,width, height(), 15, 100);
        p->setPen(cg.brush(QColorGroup::Foreground).color());
        p->drawText(0,0,width, height(), Qt::AlignCenter, 
                group->info().name);
    }
    else if(column == 3)    
    {
        Group::Info info = group->info();
        p->drawText(2,2,width, height()-4, Qt::AlignLeft, 
                        KIO::convertSize(info.totalSize));
    }
    else if(column == 4)
    {
        Group::Info info = group->info();
/*        float rectWidth = (width-4) * info.percent / 100;
        p->fillRect(2,2,rectWidth, height()-4, cg.brush(QColorGroup::Highlight).color().dark(115));
        p->setPen(cg.foreground());
        p->drawRect(2,2,rectWidth, height()-4);*/
        p->drawText(2,2,width, height()-4, Qt::AlignCenter, 
                        QString().setNum(info.percent) + "%");
    }
    
/*    switch (column)
    {
        case 0:
            p->fillRect(3,3,width-3, height()-3, QColor(174, 174, 220).dark(20));
            break;
        case 5:
            p->fillRect(0,3,width-3, height()-3, QColor(174, 174, 220).dark(20));
            break;
        default:
            p->fillRect(0,3,width, height()-3, QColor(174, 174, 220).dark(20));
            break;
            
    }
*/    
} 



MainViewItem::MainViewItem(MainView * parent, Transfer * t)
    : QListViewItem(parent),
     view(parent),
     transfer(t)
{
    
}

void MainViewItem::updateContents(bool updateAll)
{
    Transfer::Info info=transfer->info();
    
    transfer->changesFlags(view);
    
    Transfer::TransferChanges transferFlags = transfer->changesFlags(view);

//     kdDebug() << "FLAGS before reset" << transfer->gettransferFlags(view) << endl;
        
    if(updateAll)
    {
//         kdDebug() << "UPDATE:  name" << endl;
        setText(1, info.src.fileName());
    }
    
    if(updateAll || (transferFlags & Transfer::Tc_Priority) )
    {
//         kdDebug() << "UPDATE:  priority" << endl;                
//         setText(0, QString().setNum(info.priority));
        switch(info.priority)
        {
            case 1: 
//                 setText(0, i18n("Highest") );
                setPixmap(0, SmallIcon("2uparrow") ); 
                break;
            case 2: 
//                 setText(0, i18n("High") );
                setPixmap(0, SmallIcon("1uparrow") ); 
                break;
            case 3: 
//                 setText(0, i18n("Normal") );
                setPixmap(0, SmallIcon("1rightarrow") ); 
                break;
            case 4: 
//                 setText(0, i18n("Low") );
                setPixmap(0, SmallIcon("1downarrow") ); 
                break;
            case 5: 
//                 setText(0, i18n("Lowest") );
                setPixmap(0, SmallIcon("2downarrow") ); 
                break;
            case 6: 
//                 setText(0, i18n("Highest") );
                setPixmap(0, SmallIcon("stop") ); 
                break;
        }
    }
    
    if(updateAll || (transferFlags & Transfer::Tc_Status) )
    {
//         kdDebug() << "UPDATE:  status" << endl;
        switch(info.status)
        {
            case Transfer::St_Trying:
                setText(2, i18n("Connecting..."));
                setPixmap(2, SmallIcon("connect_creating"));
                break;
            case Transfer::St_Running:
                setText(2, i18n("Downloading..."));
                setPixmap(2, SmallIcon("tool_resume"));
                break;
            case Transfer::St_Delayed:
                setText(2, i18n("Delayed"));
                setPixmap(2, SmallIcon("tool_timer"));
                break;
            case Transfer::St_Stopped:
                setText(2, i18n("Stopped"));
                setPixmap(2, SmallIcon("stop"));
                break;
            case Transfer::St_Aborted:
                setText(2, i18n("Aborted"));
                setPixmap(2, SmallIcon("stop"));
                break;
            case Transfer::St_Finished:
                setText(2, i18n("Completed"));
                setPixmap(2, SmallIcon("ok"));
                break;
        }
    }
    
    if(updateAll || (transferFlags & Transfer::Tc_TotalSize) )
    {
//         kdDebug() << "UPDATE:  totalSize" << endl;
        if (info.totalSize != 0)
            setText(3, KIO::convertSize(info.totalSize));
        else
            setText(3, i18n("unknown"));
    }
                
    if(updateAll || (transferFlags & Transfer::Tc_Percent) )
    {
//         kdDebug() << "UPDATE:  percent" << endl;
        
    }
            
    if(updateAll || (transferFlags & Transfer::Tc_Speed) )
    {
//         kdDebug() << "UPDATE:  speed" << endl;
        if(info.speed==0)
        {
            if(info.status == Transfer::St_Running)
                setText(5, i18n("Stalled") );
            else
                setText(5, "" );
        }
        else
            setText(5, i18n("%1/s").arg(KIO::convertSize(info.speed)) );
    }
        
/*    if(updateAll || (transferFlags & Transfer::Tc_Log) )
    {
        
    }*/
            
    transfer->resetChangesFlags(view);        
//     kdDebug() << "FLAGS after reset" << transfer->gettransferFlags(view) << endl;
}

void MainViewItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    QListViewItem::paintCell(p, cg, column, width, align);

    if(column == 4)
    {
        Transfer::Info info = transfer->info();
        float rectWidth = (width-4) * info.percent / 100;
        
        p->fillRect(2,2,rectWidth, height()-4, cg.brush(QColorGroup::Highlight));
        p->setPen(cg.foreground());
        p->drawRect(2,2,rectWidth, height()-4);
        p->drawText(2,2,width, height()-4, Qt::AlignCenter, 
                    QString().setNum(info.percent) + "%");
    }
}



MainView::MainView( QWidget * parent, const char * name )
    : KListView( parent, name )
{
    setAllColumnsShowFocus(true);
    setSelectionMode(QListView::Extended);
    setSorting(0);
    
    addColumn("Priority", 60);
    addColumn("File", 150);
    addColumn("Status", 120);
    addColumn("Size", 80);    
    addColumn("Progress", 80);    
    addColumn("Speed", 80);    
    connect ( this, SIGNAL(rightButtonClicked ( QListViewItem *, const QPoint &, int )), this, SLOT(slotRightButtonClicked(QListViewItem * , const QPoint &, int )) );
}

MainView::~MainView()
{

}

void MainView::schedulerCleared()
{

}

void MainView::schedulerAddedItems( const TransferList& list )
{
    kdDebug() << "MainView::schedulerAddedItems" << endl;

    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        QString group = (*it)->info().group;
        MainViewItem * newItem;
        
        kdDebug() << "groupsMap.size=" << groupsMap.size() << endl;
        
        if(groupsMap.contains(group))
        {
            groupsMap[group]->insertItem(newItem = new MainViewItem(this, *it));
            kdDebug() << "Trovato gruppo " << endl;   
        }
        else
        {
            newItem = new MainViewItem(this, *it);
            kdDebug() << "Non trovato gruppo " << endl;   
        }
        transfersMap[*it] = newItem;

        newItem->updateContents(true);
    }
}

void MainView::schedulerRemovedItems( const TransferList& list )
{
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        delete(transfersMap[*it]); 
    }
}

void MainView::schedulerChangedItems( const TransferList& list )
{
    //kdDebug() << "CHANGED_ITEMS!! " << endl;
    
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        transfersMap[*it]->updateContents();
    }
}

void MainView::schedulerAddedGroups( const GroupList& list ) 
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        MainViewGroupItem * newItem = new MainViewGroupItem(this, *it);
        groupsMap[(*it)->info().name] = newItem;
        
        newItem->updateContents(true);
    }
}

void MainView::schedulerRemovedGroups( const GroupList& list)
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        delete(groupsMap[(*it)->info().name]);
    }

}

void MainView::schedulerChangedGroups( const GroupList& list)
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        groupsMap[(*it)->info().name]->updateContents();
    }
}

void MainView::schedulerStatus( GlobalStatus * )
{
}

TransferList MainView::getSelectedList()
{
    TransferList list;

    QListViewItemIterator it(this);
    
    for(;*it != 0; it++)
    {
        MainViewItem * item;
        
        if( isSelected(*it) &&  (item = dynamic_cast<MainViewItem*>(*it)) )
            list.addTransfer( item->getTransfer() );
    }
    
    return list;
}


void MainView::slotNewTransfer()
{
    schedNewURLs( KURL(), QString::null );
}

void MainView::slotResumeItems()
{
    TransferList tl = getSelectedList();
    schedSetCommand( tl, CmdResume );
}

void MainView::slotStopItems()
{
    TransferList tl = getSelectedList();
    schedSetCommand( tl, CmdPause );
}

void MainView::slotRemoveItems()
{
    TransferList tl = getSelectedList();
    schedDelItems( tl );
}

void MainView::slotSetPriority1()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 1 );
}

void MainView::slotSetPriority2()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 2 );
}

void MainView::slotSetPriority3()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 3 );
}

void MainView::slotSetPriority4()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 4 );
}

void MainView::slotSetPriority5()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 5 );
}

void MainView::slotSetPriority6()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 6 );
}

void MainView::slotSetGroup()
{
    TransferList tl = getSelectedList();
    schedSetGroup( tl, "" );
}

void MainView::slotRightButtonClicked( QListViewItem * item, const QPoint & pos, int column )
{
    KPopupMenu * popup = new KPopupMenu( this );
    
    // insert title
    QString t1 = i18n("Transfer operations Menu");
    QString t2 = i18n("KGet Menu");
    popup->insertTitle( column!=-1 ? t1 : t2 );
    
    // add menu entries
    if ( column!=-1 )
    {   // menu over an item
        popup->insertItem( SmallIcon("down"), i18n("Resume"), this, SLOT(slotResumeItems()) );
        popup->insertItem( SmallIcon("stop"), i18n("Stop"), this, SLOT(slotStopItems()) );
        popup->insertItem( SmallIcon("remove"), i18n("Remove"), this, SLOT(slotRemoveItems()) );
        
        KPopupMenu * subPrio = new KPopupMenu( popup );
        subPrio->insertItem( SmallIcon("2uparrow"), i18n("highest"), this,  SLOT( slotSetPriority1() ) );
        subPrio->insertItem( SmallIcon("1uparrow"), i18n("high"), this,  SLOT( slotSetPriority2() ) );
        subPrio->insertItem( SmallIcon("1rightarrow"), i18n("normal"), this,  SLOT( slotSetPriority3() ) );
        subPrio->insertItem( SmallIcon("1downarrow"), i18n("low"), this,  SLOT( slotSetPriority4() ) );
        subPrio->insertItem( SmallIcon("2downarrow"), i18n("lowest"), this,  SLOT( slotSetPriority5() ) );
        subPrio->insertItem( SmallIcon("stop"), i18n("do now download"), this,  SLOT( slotSetPriority6() ) );
        popup->insertItem( i18n("Set priority"), subPrio );
                
        KPopupMenu * subGroup = new KPopupMenu( popup );
        //for loop inserting all existant groups
        subGroup->insertItem( i18n("new ..."), this,  SLOT( slotSetGroup() ) );	
        popup->insertItem( SmallIcon("folder"), i18n("Set group ..."), subGroup );
    }
    else
    	// menu on empty space
	    popup->insertItem( i18n("New transfer ..."), this, SLOT(slotNewTransfer()) );

    // show popup
    popup->popup( pos );
}



#include "mainview.moc"
