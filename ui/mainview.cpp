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
#include <kmimetype.h>
#include <kiconeffect.h>

#include "ui/mainview.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "core/model.h"

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
    kdDebug() << "TransferGroupItem::addedTransferEvent" << endl;
    kdDebug() << " source = " << transfer->source().url() << endl;

    new TransferItem(this, transfer, findTransferItem(after));

    setVisible(true);
}

void TransferGroupItem::removedTransferEvent(TransferHandler * transfer)
{
    delete(findTransferItem(transfer));
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

void TransferGroupItem::deletedEvent(TransferGroupHandler * group)
{

}

bool TransferGroupItem::acceptDrop ( const QMimeSource * mime ) const
{
    return true;
}

void TransferGroupItem::dropped ( QDropEvent * e )
{
    kdDebug() << "TransferGroupItem::dropped" << endl;
}

void TransferGroupItem::updateContents(bool updateAll)
{
//     TransferGroupHandler::ChangesFlags groupFlags = m_group->changesFlags(this);
// 
//     if( updateAll || (groupFlags & TransferGroup::Gc_TotalSize) )
//     {
//         kdDebug() << "TransferGroupItem::updateContents (" << (groupFlags & TransferGroup::Gc_TotalSize) << ")" << endl;
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

TransferItem * TransferGroupItem::findTransferItem( TransferHandler * transfer )
{
    if(!transfer)
        return 0;

    QListViewItemIterator it( this );

    for ( ; it.current() ; ++it )
    {
        TransferItem * ti = static_cast<TransferItem *>(it.current());
        if( ti->transfer() == transfer )
            return ti;
    }
    return 0;
}

TransferItem::TransferItem(TransferGroupItem * parent, TransferHandler * transfer, QListViewItem * after)
    : QListViewItem(parent, after),
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
    updateContents();
}

void TransferItem::deleteEvent(TransferHandler * transfer)
{
    delete(this);
}

bool TransferItem::acceptDrop ( const QMimeSource * mime ) const
{
    return true;
}

void TransferItem::dropped ( QDropEvent * e )
{
    kdDebug() << "TransferItem::dropped" << endl;
}

void TransferItem::updateContents(bool updateAll)
{
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    kdDebug() << " TransferFlags = " << transferFlags << endl;

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
        setPixmap(0, KMimeType::pixmapForURL( m_transfer->source(), 0, KIcon::Desktop, 16, 0, 0L));
    }

    if(updateAll || (transferFlags & Transfer::Tc_Status) )
    {
//         kdDebug() << "UPDATE:  status" << endl;
        setText( 1, m_transfer->statusText() );
        setPixmap( 1, m_transfer->statusPixmap() );
    }

    if(updateAll || (transferFlags & Transfer::Tc_TotalSize) )
    {
//         kdDebug() << "UPDATE:  totalSize" << endl;
        kdDebug() << "totalSize = " << m_transfer->totalSize();
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
            if(m_transfer->status() == Job::Running)
                setText(4, i18n("Stalled") );
            else
                setText(4, "" );
        }
        else
            setText(4, i18n("%1/s").arg(KIO::convertSize( speed )) );
    }

    if(updateAll || (transferFlags & Transfer::Tc_Selection) )
    {
        kdDebug() << "UPDATE:  selection    " << m_transfer->isSelected() << endl;
        QListViewItem::setSelected( m_transfer->isSelected() );
    }

    m_transfer->resetChangesFlags(this);
}

void TransferItem::setSelected(bool s)
{
    kdDebug() << "TransferItem::setSelected  -> " << isSelected()
              << "  " << m_transfer->isSelected() << endl;

    m_transfer->setSelected( s );
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
    : KListView( parent, name ), 
      m_popup(0)
{
    setSorting(-1);
    setAllColumnsShowFocus(true);
    setSelectionMode(QListView::Extended);
    setDragEnabled(true);
    setAcceptDrops(true);

    addColumn(i18n("File"), 200);
    addColumn(i18n("Status"), 120);
    addColumn(i18n("Size"), 80);
    addColumn(i18n("Progress"), 80);
    addColumn(i18n("Speed"), 80);
    connect ( this, SIGNAL(rightButtonClicked ( QListViewItem *, const QPoint &, int )), this, SLOT(slotRightButtonClicked(QListViewItem * , const QPoint &, int )) );

    Model::addObserver(this);
}

MainView::~MainView()
{
}

void MainView::addedTransferGroupEvent(TransferGroupHandler * group)
{
    kdDebug() << "MainView::addedTransferGroupEvent" << endl;

    TransferGroupItem * newGroupItem = new TransferGroupItem(this, group);
    newGroupItem->setVisible(false);
}

void MainView::contentsDropEvent ( QDropEvent * e )
{
    kdDebug() << "MainView::contentsDropEvent" << endl;

    cleanDropVisualizer();

    QValueList<TransferHandler *> transfers = Model::selectedTransfers();

    QValueList<TransferHandler *>::iterator it = transfers.end();
    QValueList<TransferHandler *>::iterator itBegin = transfers.begin();

    QListViewItem * parent;
    QListViewItem * after;

    findDrop(e->pos(), parent, after);

    kdDebug() << "parent=" << parent << "  " << "after=" << after << endl;

    //The item has been dropped outside the available groups
    if(parent==0)
        return;

    TransferGroupHandler * destGroup;
    destGroup = static_cast<TransferGroupItem *>(parent)->group();

    kdDebug() << "destGroup = " << destGroup << endl;

    TransferHandler * destTransfer;
    if(after)
        destTransfer = static_cast<TransferItem *>(after)->transfer();
    else
        destTransfer = 0;

    if(destTransfer)
    {
        kdDebug() << "Item dropped on the transfer:" << endl;
        kdDebug() << "(" << destTransfer->source().url() << ")" << endl;
    }
    else
    {
        kdDebug() << "destTransfer == NULL" << endl;
    }

    destGroup->move(transfers, destTransfer);
}

/*void MainView::contentsDragMoveEvent(QDragMoveEvent * event)
{
    kdDebug() << "contentsDragMoveEvent" << endl;
}
*/
void MainView::slotRightButtonClicked( QListViewItem * /*item*/, const QPoint & pos, int column )
{
    QValueList<TransferHandler *> selectedTransfers = Model::selectedTransfers();

    if( selectedTransfers.empty() )
        return;

    if(m_popup)
    {
        delete(m_popup);
        m_popup = 0;
    }

    m_popup = selectedTransfers.first()->popupMenu(selectedTransfers);
    m_popup->popup( pos );
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
