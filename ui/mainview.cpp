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

#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kio/global.h>
#include <kimageeffect.h>
#include <kiconeffect.h>

#include "mainview.h"
#include "core/transferlist.h"
#include "core/transfer.h"

MainViewGroupItem::MainViewGroupItem(MainView * parent, Group * g)
    : QListViewItem(parent),
      view(parent),
      group(g),
      m_topGradient( 0 ),
      m_bottomGradient( 0 )
{
    setOpen(true);
    setHeight(30);
    // force gradient pixmaps building
    updatePixmaps();
}

MainViewGroupItem::~MainViewGroupItem()
{
    delete m_topGradient;
    delete m_bottomGradient;
}

void MainViewGroupItem::updateContents(bool updateAll)
{
    Group::Info info = group->info();
    Group::GroupChanges groupFlags = group->changesFlags(view);

    if( updateAll || (groupFlags & Group::Gc_TotalSize) )
    {
        kdDebug() << "MainViewGroupItem::updateContents (" << (groupFlags & Group::Gc_TotalSize) << ")" << endl;
        setText(1, info.name);
        setText(3, KIO::convertSize(info.totalSize));
    }

    group->resetChangesFlags(view);
}

void MainViewGroupItem::updatePixmaps()
{
    delete m_topGradient;
    m_topGradient = new QPixmap( KImageEffect::gradient(
            QSize( 1, 8 ),
            view->palette().active().background().light(110),
            view->palette().active().background(),
            KImageEffect::VerticalGradient ) );

    delete m_bottomGradient;
    m_bottomGradient = new QPixmap( KImageEffect::gradient(
            QSize( 1, 5 ),
            view->palette().active().background().dark(150),
            view->palette().active().base(),
            KImageEffect::VerticalGradient ) );
}

void MainViewGroupItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int /*align*/)
{
    //I must set the height because it gets overwritten when changing fonts
    setHeight(30);

    //QListViewItem::paintCell(p, cg, column, width, align);

    p->fillRect(0,0,width, 4, cg.brush(QColorGroup::Base));
    p->fillRect(0,12,width, height()-4-8-5, cg.brush(QColorGroup::Background));
    p->drawTiledPixmap(0,4, width, 8, *m_topGradient);
    p->drawTiledPixmap(0,height()-5, width, 5, *m_bottomGradient);
    p->setPen(QPen(cg.brush(QColorGroup::Background).color().dark(130),1));
    p->drawLine(0,4,width, 4);

    p->setPen(cg.brush(QColorGroup::Foreground).color());

    if(column == 0)
    {
        p->drawPixmap(3, 4, SmallIcon("folder", 22));
        QFont f(p->font());
        f.setBold(true);
        p->setFont( f );
        p->drawText(30,7,width, height()-7, Qt::AlignLeft,
                     group->info().name);
    }
    else if(column == 2)
    {
        Group::Info info = group->info();
        p->drawText(0,7,width, height()-7, Qt::AlignLeft, 
                        KIO::convertSize(info.totalSize));
    }
    else if(column == 3)
    {
        Group::Info info = group->info();
        p->drawText(0,2,width, height()-4, Qt::AlignCenter, 
                        QString::number(info.percent) + "%");
    }
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
    
    Transfer::TransferChanges transferFlags = transfer->changesFlags(view);

//     kdDebug() << "FLAGS before reset" << transfer->gettransferFlags(view) << endl;
        
    if(updateAll)
    {
//         kdDebug() << "UPDATE:  name" << endl;
        setText(0, info.src.fileName());
    }
    
    if(updateAll || (transferFlags & Transfer::Tc_Priority) )
    {
//         kdDebug() << "UPDATE:  priority" << endl;                
        
        bool colorizeIcon = true;
        QColor saturatedColor;
        
        switch(info.priority)
        {
            case 1:
                saturatedColor = QColor(Qt::yellow);
                break;
            case 2: 
                saturatedColor = QColor(Qt::red);
                break;
            case 3:
                colorizeIcon = false; 
                setPixmap(0, SmallIcon("kget"));
                break;
            case 4: 
                saturatedColor = QColor(Qt::green);
                break;
            case 5: 
                saturatedColor = QColor(Qt::gray);
                break;
            case 6: 
                colorizeIcon = false;
                setPixmap(0, SmallIcon("stop"));
                break;
        }
        if(colorizeIcon)
        {
            QImage priorityIcon = SmallIcon("kget").convertToImage();
            // eros: this looks cool with dark red blue or green but sucks with
            // other colors (such as kde default's pale pink..). maybe the effect
            // or the blended color has to be changed..
            int hue, sat, value;
            saturatedColor.getHsv( &hue, &sat, &value );
            saturatedColor.setHsv( hue, (sat + 510) / 3, value );
            KIconEffect::colorize( priorityIcon, saturatedColor, 0.9 );
            
            setPixmap(0, QPixmap(priorityIcon));
        }
    }
    
    if(updateAll || (transferFlags & Transfer::Tc_Status) )
    {
//         kdDebug() << "UPDATE:  status" << endl;
        switch(info.status)
        {
            case Transfer::St_Trying:
                setText(1, i18n("Connecting..."));
                setPixmap(1, SmallIcon("connect_creating"));
                break;
            case Transfer::St_Running:
                setText(1, i18n("Downloading..."));
                setPixmap(1, SmallIcon("tool_resume"));
                break;
            case Transfer::St_Delayed:
                setText(1, i18n("Delayed"));
                setPixmap(1, SmallIcon("tool_timer"));
                break;
            case Transfer::St_Stopped:
                setText(1, i18n("Stopped"));
                setPixmap(1, SmallIcon("stop"));
                break;
            case Transfer::St_Aborted:
                setText(1, i18n("Aborted"));
                setPixmap(1, SmallIcon("stop"));
                break;
            case Transfer::St_Finished:
                setText(1, i18n("Completed"));
                setPixmap(1, SmallIcon("ok"));
                break;
        }
    }
    
    if(updateAll || (transferFlags & Transfer::Tc_TotalSize) )
    {
//         kdDebug() << "UPDATE:  totalSize" << endl;
        if (info.totalSize != 0)
            setText(2, KIO::convertSize(info.totalSize));
        else
            setText(2, i18n("not available", "n/a"));
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
                setText(4, i18n("Stalled") );
            else
                setText(4, "" );
        }
        else
            setText(4, i18n("%1/s").arg(KIO::convertSize(info.speed)) );
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

    if(column == 3)
    {
        Transfer::Info info = transfer->info();
        int rectWidth = (int)((width-6) * info.percent / 100);

        p->setPen(cg.background().dark());
        p->drawRect(2,2,width-4, height()-4);
        
        p->setPen(cg.background());
        p->fillRect(3,3,width-6, height()-6, cg.brush(QColorGroup::Background));
        
        p->setPen(cg.brush(QColorGroup::Highlight).color().light(105));
        p->drawRect(3,3,rectWidth, height()-6);
        p->fillRect(4,4,rectWidth-2, height()-8, cg.brush(QColorGroup::Highlight));
        
        p->setPen(cg.foreground());
        p->drawText(2,2,width-4, height()-4, Qt::AlignCenter, 
                    QString::number(info.percent) + "%");
    }
}



MainView::MainView( QWidget * parent, const char * name )
    : KListView( parent, name )
{
    setAllColumnsShowFocus(true);
    setSelectionMode(QListView::Extended);
    setSorting(0);

    addColumn(i18n("File"), 200);
    addColumn(i18n("Status"), 120);
    addColumn(i18n("Size"), 80);
    addColumn(i18n("Progress"), 80);
    addColumn(i18n("Speed"), 80);
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
        
        QMap<QString, MainViewGroupItem *>::iterator g = groupsMap.find(group);
        
        if(g != groupsMap.end())
        {
            (*g)->insertItem(newItem = new MainViewItem(this, *it));
            (*g)->setVisible(true);
        }
        else
            newItem = new MainViewItem(this, *it);

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

    //Now we hide the groups that don't contain any transfer
    QMap<QString, MainViewGroupItem *>::iterator it2 = groupsMap.begin();
    QMap<QString, MainViewGroupItem *>::iterator it2End = groupsMap.end();
    
    for(; it2 != it2End; ++it2)
    {
        if( (*it2)->childCount() == 0 )
            (*it2)->setVisible(false);
    }

}

void MainView::schedulerChangedItems( const TransferList& list )
{
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();

    for(; it != endList; ++it)
    {
        MainViewItem * vItem = transfersMap[*it];

        //The transfer's group has changed.
        if( (*it)->changesFlags(this)==Transfer::Tc_Group )
        {
            //The new group name
            QString gName = (*it)->info().group;
            QListViewItem * oldGroup = transfersMap[*it]->parent();

            //Delete the old MainViewItem.
            delete(vItem);

            if( (oldGroup) && (oldGroup->childCount() == 0) )
                oldGroup->setVisible(false);

            //Create a new MainViewItem which belongs to the new group
            QMap<QString, MainViewGroupItem *>::iterator g = groupsMap.find(gName);
            MainViewItem * newItem;

            if(g != groupsMap.end())
            {
                (*g)->insertItem(newItem = new MainViewItem(this, *it));
                (*g)->setVisible(true);
            }
            else
                newItem = new MainViewItem(this, *it);

            transfersMap[*it] = newItem;

            newItem->updateContents(true);
        }
        else
            vItem->updateContents();
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

        newItem->setVisible(false);
    }
}

void MainView::schedulerRemovedGroups( const GroupList& list)
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();

    for(; it != endList; ++it)
    {
        groupsMap.remove((*it)->info().name);
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

void MainView::slotSetPriority( int id )
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, id );
}

void MainView::slotSetGroup( int id )
{
    TransferList tl = getSelectedList();

    QMap<QString, MainViewGroupItem *>::iterator it = groupsMap.begin();
    QMap<QString, MainViewGroupItem *>::iterator itEnd = groupsMap.end();

    for(int i=0; it!=itEnd && i<id; ++it, ++i);

    //Now the iterator points to the selected group
    schedSetGroup( tl, (*it)->getGroup()->info().name );
    kdDebug() << "Move to group: " << id+1 << " / " << groupsMap.size() << " " <<
            (*it)->getGroup()->info().name << endl;
}

void MainView::slotRightButtonClicked( QListViewItem * /*item*/, const QPoint & pos, int column )
{
    if (!ac) return;

    KPopupMenu * popup = new KPopupMenu( this );
    
    TransferList t = getSelectedList();
    // insert title
    QString t1 = i18n("%n download", "%n downloads", t.count());
    QString t2 = i18n("KGet");
    popup->insertTitle( column!=-1 ? t1 : t2 );
    
    // add menu entries
    if ( column!=-1 )
    {   // menu over an item
        popup->insertItem( SmallIcon("down"), i18n("R&esume"), this, SLOT(slotResumeItems()) );
        popup->insertItem( SmallIcon("stop"), i18n("&Stop"), this, SLOT(slotStopItems()) );
        popup->insertItem( SmallIcon("editdelete"), i18n("&Remove"), this, SLOT(slotRemoveItems()) );
        
        KPopupMenu * subPrio = new KPopupMenu( popup );
        subPrio->insertItem( SmallIcon("2uparrow"), i18n("highest"), this,  SLOT( slotSetPriority(int) ), 0, 1);
        subPrio->insertItem( SmallIcon("1uparrow"), i18n("high"), this,  SLOT( slotSetPriority(int) ), 0, 2);
        subPrio->insertItem( SmallIcon("1rightarrow"), i18n("normal"), this,  SLOT( slotSetPriority(int) ), 0, 3);
        subPrio->insertItem( SmallIcon("1downarrow"), i18n("low"), this,  SLOT( slotSetPriority(int) ), 0, 4);
        subPrio->insertItem( SmallIcon("2downarrow"), i18n("lowest"), this,  SLOT( slotSetPriority(int) ), 0, 5);
        subPrio->insertItem( SmallIcon("stop"), i18n("do now download"), this,  SLOT( slotSetPriority(int) ), 0, 6 );
        popup->insertItem( i18n("Set &priority"), subPrio );
                
        KPopupMenu * subGroup = new KPopupMenu( popup );
        //for loop inserting all existant groups
        QMap<QString, MainViewGroupItem *>::iterator it = groupsMap.begin();
        QMap<QString, MainViewGroupItem *>::iterator itEnd = groupsMap.end();
        
        for(int i=0; it != itEnd; ++it, ++i)
        {
            subGroup->insertItem( SmallIcon("folder"),
                                    (*it)->getGroup()->info().name, 
                                    this,  SLOT( slotSetGroup( int ) ),
                                    0, i);
        }
        //subGroup->insertItem( i18n("new ..."), this,  SLOT( slotSetGroup() ) );	
        popup->insertItem( SmallIcon("folder"), i18n("Set &group"), subGroup );
    }
    else
        // menu on empty space
        ac->action("open_transfer")->plug(popup);

    // show popup
    popup->popup( pos );
}

void MainView::setupActions( KActionCollection * a )
{
    ac = a;
}

void MainView::paletteChange( const QPalette & )
{
    QMap<QString, MainViewGroupItem *>::iterator it = groupsMap.begin(), end = groupsMap.end();
    for ( ; it != end; ++it )
        (*it)->updatePixmaps();
}

#include "mainview.moc"
