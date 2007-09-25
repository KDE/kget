/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <QObject>
#include <QVariant>
#include <QPainter>

#include <KLocale>
#include <KIcon>

#include "transfergraph.h"

TransferGraph::TransferGraph(QObject *parent)
{
    Q_UNUSED(parent)
}

void TransferGraph::setTransfers(const QVariantMap &transfers) 
{
    m_transfers = transfers;
}

QSizeF TransferGraph::contentSizeHint()
{
    return QSizeF(TRANSFER_APPLET_WIDTH, m_transfers.size() * TRANSFER_LINE_HEIGHT + TRANSFER_MARGIN);
}

void TransferGraph::drawTitle(QPainter *p, const QRect &contentsRect)
{
    // draw the kget icon
    p->drawPixmap(contentsRect.x() + HORIZONTAL_MARGIN, contentsRect.y() + 10,
            KIcon("kget").pixmap(20, 20));
    // draw the title 
    p->drawText(contentsRect.x() + HORIZONTAL_MARGIN + 30, contentsRect.y() + 10,
            contentsRect.width() - 100, TRANSFER_LINE_HEIGHT - 10,
            Qt::AlignLeft, i18n("KGet downloads"));
    // draw a line under the title
    p->drawLine(contentsRect.x() + HORIZONTAL_MARGIN + 30, contentsRect.y() + TRANSFER_LINE_HEIGHT,
            contentsRect.width() - HORIZONTAL_MARGIN, contentsRect.y() + TRANSFER_LINE_HEIGHT);
}
