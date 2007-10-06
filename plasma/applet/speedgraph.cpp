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

#include <KIcon>
#include <KDebug>
#include <KLocale>
#include <KGlobal>

#include <QObject>
#include <QMap>
#include <QSizeF>
#include <QPainter>
#include <QQueue>

#include "speedgraph.h"
#include "transfergraph.h"

SpeedGraph::SpeedGraph(QObject *parent)
    : TransferGraph(parent),
    m_maxSpeed(1000)
{
}

void SpeedGraph::paint(QPainter *p, const QRect &contentsRect)
{
    p->save();
    p->setPen(Qt::white);
    p->setRenderHint(QPainter::Antialiasing);

    TransferGraph::drawTitle(p, contentsRect);

    // draw the y axis 0..maxSpeed + 10
    p->setPen(Qt::white);
    p->drawLine(QPoint(TRANSFER_MARGIN + 40, VERTICAL_MARGIN),
                QPoint(TRANSFER_MARGIN + 40, VERTICAL_MARGIN + GRAPH_HEIGHT));
    // draw the x axis
    p->drawLine(QPoint(TRANSFER_MARGIN + 40, VERTICAL_MARGIN + GRAPH_HEIGHT),
                QPoint(contentsRect.width() - TRANSFER_MARGIN + 2, VERTICAL_MARGIN + GRAPH_HEIGHT));

    // draw the x line items
    for(int i=0; i<MAX_ITEMS; i++) {
        p->drawLine(QPoint(i*((contentsRect.width() - TRANSFER_MARGIN - 40 ) / MAX_ITEMS) + TRANSFER_MARGIN + 40,
                                                        VERTICAL_MARGIN + GRAPH_HEIGHT - 5),
                    QPoint(i*((contentsRect.width() - TRANSFER_MARGIN - 40 ) / MAX_ITEMS) + TRANSFER_MARGIN + 40,
                                                        VERTICAL_MARGIN + GRAPH_HEIGHT + 5)) ;
        p->save();
        p->setOpacity(0.3);
        p->drawLine(QPoint(i*((contentsRect.width() - TRANSFER_MARGIN - 40) / MAX_ITEMS) + TRANSFER_MARGIN + 40,
                                                        VERTICAL_MARGIN),
                    QPoint(i*((contentsRect.width() - TRANSFER_MARGIN - 40) / MAX_ITEMS) + TRANSFER_MARGIN + 40,
                                                        VERTICAL_MARGIN + GRAPH_HEIGHT - 5)) ;
        p->restore();
    }

    // draw the y line items
    for(int i=0; i<MAX_ITEMS; i++) {
        p->drawLine(QPoint(TRANSFER_MARGIN - 5 + 40, VERTICAL_MARGIN +
                                            (i * (GRAPH_HEIGHT)/ MAX_ITEMS)),
                    QPoint(TRANSFER_MARGIN + 5 + 40, VERTICAL_MARGIN +
                                            (i * (GRAPH_HEIGHT)/ MAX_ITEMS)));
        p->save();
        p->setOpacity(0.3);
        p->drawText(TRANSFER_MARGIN - 28, VERTICAL_MARGIN + 5 +
                        (i * (GRAPH_HEIGHT)/ MAX_ITEMS),
                        KGlobal::locale()->formatByteSize(((MAX_ITEMS - i) * m_maxSpeed/ MAX_ITEMS)));
        p->drawLine(QPoint(TRANSFER_MARGIN + 5 + 40, VERTICAL_MARGIN +
                                            (i * (GRAPH_HEIGHT)/ MAX_ITEMS)),
                    QPoint(contentsRect.width() - TRANSFER_MARGIN + 2, VERTICAL_MARGIN + 
                                            (i * (GRAPH_HEIGHT)/ MAX_ITEMS)));
        p->restore();
    }

    // draw a line before the legends
    p->drawLine(contentsRect.x() + HORIZONTAL_MARGIN, contentsRect.y() + GRAPH_HEIGHT + 80,
            contentsRect.width() - HORIZONTAL_MARGIN, contentsRect.y() + GRAPH_HEIGHT + 80);
    // draw the transfers
    int i=0;
    foreach(QString key, m_data.keys()) {
        drawTimeline(m_data[key], p, contentsRect, MyColors[i]);
        drawLegend(m_transfers[key].toList(), p, MyColors[i], GRAPH_HEIGHT + 100 + i*30); 
        i++;
    }

    p->restore();
}

QSizeF SpeedGraph::contentSizeHint()
{
    return QSizeF(TransferGraph::contentSizeHint().width(),
                    TransferGraph::contentSizeHint().height() + GRAPH_HEIGHT + TRANSFER_MARGIN * 4);
}

void SpeedGraph::setTransfers(const QVariantMap &percents)
{
    TransferGraph::setTransfers(percents);
    prepareData();
}

void SpeedGraph::drawLegend(const QVariantList &transfer, QPainter *p, const QColor &color, uint y)
{
    uint percent = transfer.at(1).toUInt();
    QString name = transfer.at(0).toString();
    uint size = transfer.at(2).toString().toUInt();

    p->save();
    p->setPen(Qt::NoPen);
    p->setBrush(QBrush(color)); 
    p->drawRoundRect(QRect(10, y + 5, 10, 10));
    p->setPen(Qt::SolidLine);
    p->setPen(Qt::white);

    // the download name
    p->drawText(QRect(23, y + 1, 180, 20), Qt::AlignLeft, name);

    // the download size
    p->drawText(QRect(220, y + 1, 78, 20), Qt::AlignLeft,
                "[" + KGlobal::locale()->formatByteSize(size) + "]");

    // the download percent
    if(percent >= 100) {
        p->drawPixmap(300, y -2 , 20, 20, KIcon("ok").pixmap(20));
    }
    else {
        p->setOpacity(0.5);
        p->drawText(QRect(300, y + 1, 40, 20), Qt::AlignLeft, QString::number(percent) + "%");
    }
    p->restore();
}

void SpeedGraph::drawTimeline(QQueue <uint> *transfer, QPainter *p, const QRect &contentsRect, const QColor &color)
{
    p->save();
    p->setPen(color);

    QPoint *previous = NULL;

    for(int i=0;i<transfer->size();i++) {
        uint x = i * ((contentsRect.width() - TRANSFER_MARGIN - 40) / MAX_ITEMS) + TRANSFER_MARGIN + 40;
        uint y = transfer->at(transfer->size() - 1 - i) * GRAPH_HEIGHT / m_maxSpeed;

        p->drawPoint(x, VERTICAL_MARGIN + GRAPH_HEIGHT - y);
        if(previous != NULL) {
            p->drawLine(x, VERTICAL_MARGIN + GRAPH_HEIGHT - y, previous->x(), previous->y());
        }
        previous = new QPoint(x, VERTICAL_MARGIN + GRAPH_HEIGHT - y);
    }

    p->restore();
}

void SpeedGraph::prepareData()
{
    foreach(QString key, m_transfers.keys()) {
        QVariantList attributes = m_transfers[key].toList();

        if (m_transfers[key].toList().at(3).toUInt() == 1) { // is active
            uint speed = attributes[4].toUInt();
            if(!m_data.contains(key)) {
                m_data[key] = new QQueue <uint> ();
            }

            if(m_data[key]->size() >= MAX_ITEMS) {
                m_data[key]->dequeue();
            }

            m_maxSpeed = (speed > m_maxSpeed) ? speed : m_maxSpeed; // update the max speed
            m_data[key]->enqueue(speed);
        }
    }
}

