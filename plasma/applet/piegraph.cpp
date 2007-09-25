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

#include <QObject>
#include <QMap>
#include <QSizeF>
#include <QPainter>

#include <math.h>

#include "piegraph.h"

PieGraph::PieGraph(QObject *parent) 
    : TransferGraph(parent),
        m_totalFiles(0)
{
}

void PieGraph::paint(QPainter *p, const QRect &contentsRect)
{
    Q_UNUSED(contentsRect)
    double totalSize = 0;
    m_totalFiles = 0;

    p->setPen(Qt::white);
    TransferGraph::drawTitle(p, contentsRect);

    QMap<QString, double> downloads;

    // search for the downloads with size attribute not empty
    foreach(QString url, m_transfers.keys()) {
        QVariantList attributes = m_transfers[url].toList();
        if (QString::compare(attributes[2].toString(), "") != 0 && 
                            attributes[2].toString().toDouble() > 0) {
            m_totalFiles ++;
            downloads.insert(url, attributes[2].toString().toDouble());
            totalSize = totalSize + downloads.value(url, 0);
        }
    }
    p->save();

    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::white);

    // piegraph rect
    QRect rect((int) (contentsRect.width() / 2) - (TRANSFER_PIEGRAPH_WIDTH / 2),
                50, TRANSFER_PIEGRAPH_WIDTH, TRANSFER_PIEGRAPH_HEIGHT);

    int angle = 90 * 16; // 0
    int i = 0;

    foreach(QString url, downloads.keys()) {
        QString name = m_transfers[url].toList().at(0).toString();
        QBrush brush(MyColors[i]);
        QRect pie = rect;
        QColor totalColor = MyColors[i];

        float totalPercentOpacity = 0.17;
        float downloadPercentOpacity = 0.37;

        // the download is active
        if (m_transfers[url].toList().at(3).toUInt() == 1) {
            pie = QRect(rect.x() - 2, rect.y() - 2, TRANSFER_PIEGRAPH_WIDTH + 4, TRANSFER_PIEGRAPH_HEIGHT + 4);
            totalPercentOpacity = 1;
            downloadPercentOpacity = 1;

            QRadialGradient myRadial(pie.x(), pie.y(), 170, 200, 200);
            myRadial.setCenter(150, 150);
            myRadial.setFocalPoint(150, 150);
            myRadial.setColorAt(0, Qt::black);
            myRadial.setColorAt(1, MyColors[i]);
            brush = QBrush(myRadial);
        }


        int current_angle = angle;
        int totalPercent = (int) round(downloads[url] * 100/ totalSize);
        int downloadPercent = (int) round((downloads[url] * m_transfers[url].toList().at(1).toInt() / 100) 
                    * 100 / totalSize);

        // draw the total percent of all downloads of this transfer
        p->save();
        p->setOpacity(totalPercentOpacity);
        angle = drawPie(p, pie, angle, totalPercent, brush);
        p->restore();

        // draw the percent of this download
        p->save();
        // p->setPen(Qt::NoPen);
        p->setOpacity(downloadPercentOpacity);
        drawPie(p, pie, current_angle, downloadPercent, QBrush(totalColor));
        p->restore();

        drawLegend(p, 200 + (TRANSFER_LINE_HEIGHT * i),
                    name, m_transfers[url].toList().at(1).toInt(), downloads[url], MyColors[i]);
        i++;
    }
    p->restore();
}

QSizeF PieGraph::contentSizeHint()
{
    return QSizeF(TransferGraph::contentSizeHint().width(),
                    TransferGraph::contentSizeHint().height() + 230);
}

void PieGraph::drawLegend(QPainter *p, int y, const QString &name, int percent, double size, const QColor &color)
{
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


int PieGraph::drawPie(QPainter *p, const QRect &rect, int angle, int percent, const QBrush &brush)
{
    p->save();

    int end_angle = -1 * percent * 36/10 * 16;

    p->setBrush(brush);

    p->drawPie(rect, angle, end_angle);

    p->restore();

    return end_angle + angle;
}

