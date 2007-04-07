/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QPainter>

#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kio/global.h>
#include <kimageeffect.h>

#include "ui/mainview.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "core/kget.h"

TransferGroupItem::TransferGroupItem(MainView * parent, TransferGroupHandler * group)
    : Q3ListViewItem(parent),
      m_group(group),
      m_view(parent),
      m_topGradient( 0 ),
      m_bottomGradient( 0 )
{
    setOpen(true);
    setHeight(30);
    // force gradient pixmaps building
    updatePixmaps();
    updateContents(true);

    group->addObserver(this);
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

void TransferGroupItem::addedTransferEvent(TransferHandler * transfer, TransferHandler * after)
{
    kDebug(5001) << "TransferGroupItem::addedTransferEvent" << endl;
    kDebug(5001) << " source = " << transfer->source().url() << endl;

    new TransferItem(this, transfer, findTransferItem(after));

    setVisible(true);
}

void TransferGroupItem::movedTransferEvent(TransferHandler * transfer, TransferHandler * after)
{
    TransferItem * ti = findTransferItem(transfer);

    if(after)
        ti->moveItem(findTransferItem(after));
    else
    {
        //Move to the top of the list
        ti->moveItem(firstChild());
        firstChild()->moveItem(ti);
    }
}

void TransferGroupItem::deleteEvent(TransferGroupHandler * group)
{

}

void TransferGroupItem::updateContents(bool updateAll)
{
//     TransferGroupHandler::ChangesFlags groupFlags = m_group->changesFlags(this);
// 
//     if( updateAll || (groupFlags & TransferGroup::Gc_TotalSize) )
//     {
//         kDebug(5001) << "TransferGroupItem::updateContents (" << (groupFlags & TransferGroup::Gc_TotalSize) << ")" << endl;
//         setText(1, m_group->name());
//         if(m_group->totalSize() != 0)
//             setText(3, KIO::convertSize(m_group->totalSize()));
//         else
//             setText(3, i18n("not available", "n/a"));
//     }

//TODO here (or in paintCell()?) we need to reset the flags!
//    m_group->resetChangesFlags(this);
}

void TransferGroupItem::updatePixmaps()
{
    delete m_topGradient;
    m_topGradient = new QPixmap( QPixmap::fromImage( KImageEffect::gradient(
            QSize( 1, 8 ),
            m_view->palette().color(QPalette::Active, QPalette::Background).light(110),
            m_view->palette().color(QPalette::Active, QPalette::Background),
            KImageEffect::VerticalGradient ) ) );

    delete m_bottomGradient;
    m_bottomGradient = new QPixmap( QPixmap::fromImage( KImageEffect::gradient(
            QSize( 1, 5 ),
            m_view->palette().color(QPalette::Active, QPalette::Background).dark(150),
            m_view->palette().color(QPalette::Active, QPalette::Base),
            KImageEffect::VerticalGradient ) ) );
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
                    QString::number(m_group->percent()) + '%');
    }
}

TransferItem * TransferGroupItem::findTransferItem( TransferHandler * transfer )
{
    if(!transfer)
        return 0;

    Q3ListViewItemIterator it( this );

    for ( ; it.current() ; ++it )
    {
        TransferItem * ti = static_cast<TransferItem *>(it.current());
        if( ti->transfer() == transfer )
            return ti;
    }
    return 0;
}

TransferItem::TransferItem(TransferGroupItem * parent, TransferHandler * transfer, Q3ListViewItem * after)
    : Q3ListViewItem(parent, after),
      m_transfer(transfer),
      m_view(parent->view())
{
    setDragEnabled(true);
    setDropEnabled(true);
    updateContents(true);

    transfer->addObserver(this);
}

void TransferItem::transferChangedEvent(TransferHandler * transfer)
{
    kDebug(5001) << "TransferItem::transferChangedEvent" << endl;
    updateContents();
}

void TransferItem::deleteEvent(TransferHandler * transfer)
{
    kDebug(5001) << "TransferItem::deleteEvent" << endl;
    delete(this);
}

void TransferItem::updateContents(bool updateAll)
{
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    kDebug(5001) << " TransferFlags = " << transferFlags << endl;

    if(updateAll)
    {
        setText(0, m_transfer->source().fileName());
    }

    if( updateAll )
    {
/*        bool colorizeIcon = true;

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
        }*/
        setPixmap(0, KIO::pixmapForUrl( m_transfer->source(), 0, K3Icon::Desktop, 16 ));
    }

    if(updateAll || (transferFlags & Transfer::Tc_Status) )
    {
//         kDebug(5001) << "UPDATE:  status" << endl;
        setText( 1, m_transfer->statusText() );
        setPixmap( 1, m_transfer->statusPixmap() );
    }

    if(updateAll || (transferFlags & Transfer::Tc_TotalSize) )
    {
//         kDebug(5001) << "UPDATE:  totalSize" << endl;
        kDebug(5001) << "totalSize = " << m_transfer->totalSize() << endl;
        if (m_transfer->totalSize() != 0)
            setText(2, KIO::convertSize( m_transfer->totalSize() ));
        else
            setText(2, i18nc("not available", "n/a"));
    }

    if(updateAll || (transferFlags & Transfer::Tc_Speed) )
    {
//         kDebug(5001) << "UPDATE:  speed" << endl;
        int speed = m_transfer->speed();

        if(speed==0)
        {
            if(m_transfer->status() == Job::Running)
                setText(4, i18n("Stalled"));
            else
                setText(4, QString());
        }
        else
            setText(4, i18n("%1/s", KIO::convertSize(speed)));
    }

    if(updateAll || (transferFlags & Transfer::Tc_Selection) )
    {
        kDebug(5001) << "UPDATE:  selection    " << m_transfer->isSelected() << endl;
        Q3ListViewItem::setSelected( m_transfer->isSelected() );
    }

    m_transfer->resetChangesFlags(this);
}

void TransferItem::setSelected(bool s)
{
    kDebug(5001) << "TransferItem::setSelected  -> " << isSelected()
              << "  " << m_transfer->isSelected() << endl;

    m_transfer->setSelected( s );
}

void TransferItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    Q3ListViewItem::paintCell(p, cg, column, width, align);

    if(column == 3)
    {
        int rectWidth = (int)((width-6) * m_transfer->percent() / 100);
        int height = this->height();

        p->setPen(cg.color(QPalette::Background).dark());
        p->drawRect(2,2,width-5, height-5);

        p->setPen(cg.color(QPalette::Background));
        p->fillRect(3,3,width-6, height-6, cg.brush(QColorGroup::Background));

        p->setPen(cg.brush(QColorGroup::Highlight).color().light(105));
        if(rectWidth-1 >= 0)
            p->drawRect(3,3,rectWidth-1, height-7);
        if(rectWidth-2 >= 0)
            p->fillRect(4,4,rectWidth-2, height-8, cg.brush(QColorGroup::Highlight));

        p->setPen(cg.color(QPalette::Foreground));
        p->drawText(2,2,width-4, height-4, Qt::AlignCenter, 
                    QString::number(m_transfer->percent()) + '%');
    }
}

MainView::MainView( QWidget * parent )
    : K3ListView( parent ),
      m_popup(0)
{
    setSorting(-1);
    setAllColumnsShowFocus(true);
    setSelectionMode(Q3ListView::Extended);
    setDragEnabled(true);
    setAcceptDrops(true);

    addColumn(i18n("File"), 200);
    addColumn(i18n("Status"), 120);
    addColumn(i18n("Size"), 80);
    addColumn(i18n("Progress"), 80);
    addColumn(i18n("Speed"), 80);
    connect ( this, SIGNAL(rightButtonClicked ( Q3ListViewItem *, const QPoint &, int )), this, SLOT(slotRightButtonClicked(Q3ListViewItem * , const QPoint &, int )) );

    KGet::addObserver(this);
}

MainView::~MainView()
{
}

void MainView::addedTransferGroupEvent(TransferGroupHandler * group)
{
    kDebug(5001) << "MainView::addedTransferGroupEvent" << endl;

    TransferGroupItem * newGroupItem = new TransferGroupItem(this, group);
    newGroupItem->setVisible(false);
}

void MainView::contentsDropEvent ( QDropEvent * e )
{
    kDebug(5001) << "MainView::contentsDropEvent" << endl;

    cleanDropVisualizer();

    QList<TransferHandler *> transfers = KGet::selectedTransfers();

    QList<TransferHandler *>::iterator it = transfers.end();
    QList<TransferHandler *>::iterator itBegin = transfers.begin();

    Q3ListViewItem * parent;
    Q3ListViewItem * after;

    findDrop(e->pos(), parent, after);

    kDebug(5001) << "parent=" << parent << "  " << "after=" << after << endl;

    //The item has been dropped outside the available groups
    if(parent==0)
        return;

    TransferGroupHandler * destGroup;
    destGroup = static_cast<TransferGroupItem *>(parent)->group();

    kDebug(5001) << "destGroup = " << destGroup << endl;

    TransferHandler * destTransfer;
    if(after)
        destTransfer = static_cast<TransferItem *>(after)->transfer();
    else
        destTransfer = 0;

    if(destTransfer)
    {
        kDebug(5001) << "Item dropped on the transfer:" << endl;
        kDebug(5001) << "(" << destTransfer->source().url() << ")" << endl;
    }
    else
    {
        kDebug(5001) << "destTransfer == NULL" << endl;
    }

    destGroup->move(transfers, destTransfer);
}

void MainView::slotRightButtonClicked( Q3ListViewItem * item, const QPoint & pos, int column )
{
    if(m_popup)
    {
        delete(m_popup);
        m_popup = 0;
    }

    if(dynamic_cast<TransferItem *> (item))
    {
        //Transfer item
        QList<TransferHandler *> selectedTransfers = KGet::selectedTransfers();

        if( selectedTransfers.empty() )
            return;

        m_popup = selectedTransfers.first()->popupMenu(selectedTransfers);
    }
    else if(TransferGroupItem * tg = dynamic_cast<TransferGroupItem *> (item))
    {
        //TransferGroup item
        m_popup = tg->group()->popupMenu();
    }
    else return;

    m_popup->popup( pos );
}

void MainView::paletteChange()
{
    Q3ListViewItemIterator it(this);

    for(;*it != 0; it++)
    {
        TransferGroupItem * groupItem;
        if(( groupItem = dynamic_cast<TransferGroupItem *>(*it) ) )
            groupItem->updatePixmaps( );
    }
}

#include "mainview.moc"
