/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QPainter>
#include <QMouseEvent>
#include <QModelIndex>

#include <kdebug.h>
#include <kmenu.h>

#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfertreemodel.h"
#include "ui/transfersviewdelegate.h"

TransfersViewDelegate::TransfersViewDelegate()
    : QItemDelegate(), m_popup(0)
{

}

// void TransfersViewDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
// {
//     painter->drawLine(0,0,100, 300);
// }

bool TransfersViewDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(event);

    if(mouseEvent)
    {
        if(mouseEvent->button() == Qt::RightButton)
        {
//             kDebug(5001) << "TransfersViewDelegate::editorEvent() -> rightClick" << endl;

            if(m_popup)
            {
                delete(m_popup);
                m_popup = 0;
            }

            TransferTreeModel * transferTreeModel = static_cast<TransferTreeModel *>(model);

            if(transferTreeModel->isTransferGroup(index))
            {
//                 kDebug(5001) << "isTransferGroup = true" << endl;
            }
            else
            {
//                 kDebug(5001) << "isTransferGroup = false" << endl;

                TransferHandler * transferHandler = static_cast<TransferHandler *>(index.internalPointer());

                m_popup = transferHandler->popupMenu(KGet::selectedTransfers());
            }

            if(m_popup)
                m_popup->popup( mouseEvent->globalPos() );
        }
    }

    return false;
}

#include "transfersviewdelegate.moc"
