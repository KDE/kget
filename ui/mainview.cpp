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
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"

TransferGroupItem::TransferGroupItem(MainView * parent, TransferGroupHandler * group)
    : QListViewItem(parent),
      m_group(group),
      m_view(parent),
      m_topGradient( 0 ),
      m_bottomGradient( 0 )
{
    setOpen(true);
    setHeight(30);
    // force gradient pixmaps building
    updatePixmaps();
}

TransferGroupItem::~TransferGroupItem()
{
    delete m_topGradient;
    delete m_bottomGradient;
}

void TransferGroupItem::groupChangedEvent(TransferGroupHandler * group)
{
    updateContents();
}

void TransferGroupItem::addedTransferEvent(TransferHandler * transfer)
{
    new TransferItem(this, transfer);
}

void TransferGroupItem::removedTransferEvent(TransferHandler * transfer)
{

}

void TransferGroupItem::deletedEvent(TransferGroupHandler * group)
{
    
}

void TransferGroupItem::updateContents(bool updateAll)
{
    TransferGroupHandler::ChangesFlags groupFlags = m_group->changesFlags(this);

    if( updateAll || (groupFlags & TransferGroupHandler::Gc_TotalSize) )
    {
        kdDebug() << "TransferGroupItem::updateContents (" << (groupFlags & TransferGroupHandler::Gc_TotalSize) << ")" << endl;
        setText(1, m_group->name());
        setText(3, KIO::convertSize(m_group->totalSize()));
    }

    m_group->resetChangesFlags(this);
}

void TransferGroupItem::updatePixmaps()
{
    delete m_topGradient;
    m_topGradient = new QPixmap( KImageEffect::gradient(
            QSize( 1, 8 ),
            m_view->palette().active().background().light(110),
            m_view->palette().active().background(),
            KImageEffect::VerticalGradient ) );

    delete m_bottomGradient;
    m_bottomGradient = new QPixmap( KImageEffect::gradient(
            QSize( 1, 5 ),
            m_view->palette().active().background().dark(150),
            m_view->palette().active().base(),
            KImageEffect::VerticalGradient ) );
}

void TransferGroupItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int /*align*/)
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
                    m_group->name());
    }
    else if(column == 2)
    {
        p->drawText(0,7,width, height()-7, Qt::AlignLeft, 
                    KIO::convertSize(m_group->totalSize()));
    }
    else if(column == 3)
    {
        p->drawText(0,2,width, height()-4, Qt::AlignCenter, 
                    QString::number(m_group->percent()) + "%");
    }
}

TransferItem::TransferItem(TransferGroupItem * parent, TransferHandler * transfer)
    : QListViewItem(parent),
      m_transfer(transfer),
      m_view(parent->view())
{
}

void TransferItem::updateContents(bool updateAll)
{
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if(updateAll)
    {
        setText(0, m_transfer->source().fileName());
    }

    if( updateAll )
    {
        bool colorizeIcon = true;

        QColor saturatedColor = QColor(Qt::red);

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

    if(updateAll || (transferFlags & TransferHandler::Tc_Status) )
    {
//         kdDebug() << "UPDATE:  status" << endl;
        setText( 1, m_transfer->statusText() );
        setPixmap( 1, m_transfer->statusPixmap() );
    }

    if(updateAll || (transferFlags & Transfer::Tc_TotalSize) )
    {
//         kdDebug() << "UPDATE:  totalSize" << endl;
        if (m_transfer->totalSize() != 0)
            setText(2, KIO::convertSize( m_transfer->totalSize() ));
        else
            setText(2, i18n("not available", "n/a"));
    }

    if(updateAll || (transferFlags & Transfer::Tc_Speed) )
    {
//         kdDebug() << "UPDATE:  speed" << endl;
        int speed = m_transfer->speed();

        if(speed==0)
        {
            if(m_transfer->jobStatus() == Job::Running)
                setText(4, i18n("Stalled") );
            else
                setText(4, "" );
        }
        else
            setText(4, i18n("%1/s").arg(KIO::convertSize( speed )) );
    }

    m_transfer->resetChangesFlags(this);
}

void TransferItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    QListViewItem::paintCell(p, cg, column, width, align);

    if(column == 3)
    {
        int rectWidth = (int)((width-6) * m_transfer->percent() / 100);

        p->setPen(cg.background().dark());
        p->drawRect(2,2,width-4, height()-4);

        p->setPen(cg.background());
        p->fillRect(3,3,width-6, height()-6, cg.brush(QColorGroup::Background));

        p->setPen(cg.brush(QColorGroup::Highlight).color().light(105));
        p->drawRect(3,3,rectWidth, height()-6);
        p->fillRect(4,4,rectWidth-2, height()-8, cg.brush(QColorGroup::Highlight));

        p->setPen(cg.foreground());
        p->drawText(2,2,width-4, height()-4, Qt::AlignCenter, 
                    QString::number(m_transfer->percent()) + "%");
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

void MainView::addedTransferGroupEvent(TransferGroupHandler * group)
{
    TransferGroupItem * newGroupItem = new TransferGroupItem(this, group);
    newGroupItem->setVisible(false);
}

QValueList<TransferHandler *> MainView::getSelectedList()
{
    QValueList<TransferHandler *> list;

    QListViewItemIterator it(this);

    for(;*it != 0; it++)
    {
        TransferItem * item;

        if( isSelected(*it) &&  (item = dynamic_cast<TransferItem *>(*it)) )
            list.append( item->transfer() );
    }

    return list;
}


void MainView::slotNewTransfer()
{
//    schedNewURLs( KURL(), QString::null );
}

void MainView::slotResumeItems()
{
    QValueList <TransferHandler *> selItems = getSelectedList();
//    schedSetCommand( selItems, CmdResume );
}

void MainView::slotStopItems()
{
    QValueList <TransferHandler *> selItems = getSelectedList();
//     schedSetCommand( selItems, CmdPause );
}

void MainView::slotRemoveItems()
{
    QValueList <TransferHandler *> selItems = getSelectedList();
//     schedDelItems( selItems );
}

void MainView::slotSetPriority( int id )
{
    QValueList <TransferHandler *> selItems = getSelectedList();
//     schedSetPriority( selItems, id );
}

void MainView::slotSetGroup( int id )
{
/*    QValueList <TransferHandler *> selItems = getSelectedList();

    QMap<QString, TransferGroupItem *>::iterator it = m_groupsMap.begin();
    QMap<QString, TransferGroupItem *>::iterator itEnd = m_groupsMap.end();

    for(int i=0; it!=itEnd && i<id; ++it, ++i);

    //Now the iterator points to the selected group
    schedSetGroup( tl, (*it)->group()->info().name );
    kdDebug() << "Move to group: " << id+1 << " / " << m_groupsMap.size() << " " <<
            (*it)->group()->info().name << endl;*/
}

void MainView::slotRightButtonClicked( QListViewItem * /*item*/, const QPoint & pos, int column )
{
/*    if (!ac) return;

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
        QMap<QString, TransferGroupItem *>::iterator it = m_groupsMap.begin();
        QMap<QString, TransferGroupItem *>::iterator itEnd = m_groupsMap.end();

        for(int i=0; it != itEnd; ++it, ++i)
        {
            subGroup->insertItem( SmallIcon("folder"),
                                    (*it)->group()->info().name, 
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
    popup->popup( pos );*/
}

void MainView::setupActions( KActionCollection * a )
{
    ac = a;
}

void MainView::paletteChange()
{
    QListViewItemIterator it(this);

    for(;*it != 0; it++)
    {
        TransferGroupItem * groupItem;
        if( groupItem = dynamic_cast<TransferGroupItem *>(*it) )
            groupItem->updatePixmaps( );
    }
}

#include "mainview.moc"
