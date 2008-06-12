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

#include "piechartwidget.h"

#include <QGraphicsWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KDebug>
#include <KLocale>
#include <KGlobal>

const static int LEFT_MARGIN = 15;

PieChartWidget::PieChartWidget(QGraphicsWidget *parent) : QGraphicsWidget(parent),
    m_data(),
    m_transfers(),
    m_colors("Oxygen.colors"),
    m_totalSize(0),
    m_needsRepaint(0)
{
     setCacheMode(QGraphicsItem::DeviceCoordinateCache, QSize(300, 360));
}

PieChartWidget::~PieChartWidget()
{
}

void PieChartWidget::setTransfers(const QVariantMap &transfers)
{
    m_needsRepaint = false;
    m_totalSize = 0;

    foreach (const QString &key, m_transfers.keys()) {
        if (!transfers.contains(key)) {
            m_data.remove(key);
            m_needsRepaint = true;
        }
    }

    m_transfers = transfers;

    foreach(const QString &name, transfers.keys()) {
        QVariantList attributes = transfers[name].toList();
        double length = attributes[2].toDouble();
        double activeLength = attributes[1].toInt() * attributes[2].toDouble() / 100;
        bool active = attributes[3].toBool();

        if (length > 0) {
            if (!m_data.contains(name)) {
                m_data [name] = PrivateData();
            }


            if (length != m_data[name].length || activeLength != m_data[name].activeLength) {
                m_needsRepaint = true;
            }

            m_data [name].name = name;
            m_data [name].length = length;
            m_data [name].activeLength = activeLength;
            m_data [name].isActive = active;

            m_totalSize += length;
        }
    }

    if (m_needsRepaint) {
        update();
    }
}

void PieChartWidget::paint(QPainter  *p, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(widget)

    if (m_needsRepaint || true) {
        kDebug () << "Child needs repaint" ;
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::TextAntialiasing);

        // the rect of the pie chart
        // we draw the pie chart with width = height
    //             QRect rect = QRect(chartGeometry().width()/2 - chartGeometry().height()/2, 0, 
    //                         chartGeometry().height(), chartGeometry().height());
        int size = option->rect.height() / 2;
        QRect rect = QRect(option->rect.x() + option->rect.width() / 2 - size / 2, 
                        option->rect.y(),
                        size, 
                        size);

        int angle = 90 * 16; // 0

        QPen totalPen;
        QPen activePen;

        // total pen
        totalPen.setWidth(1);
        totalPen.setColor(Qt::darkGray);
        totalPen.setStyle(Qt::SolidLine);

        // active transfer pen
        activePen.setWidth(1);
        activePen.setColor(Qt::white);
        activePen.setStyle(Qt::SolidLine);

        for(int i = 0; i < m_data.size(); i++) {
            QBrush brush(m_colors.color(i*6 + 4));

            PrivateData portion = m_data [m_data.keys().at(i)];

            float pieOpacity = 0.67;
            float activePieOpacity = 0.84;
            int current_angle = angle;

            p->save();

            if (portion.isActive) {
                p->setPen(activePen);
            }
            else {
                p->setPen(totalPen);
            }

            p->setOpacity(pieOpacity);

            int totalPercent = (int) roundNumber(portion.length * 100/ m_totalSize);
            angle = paintPieData(p, rect, angle, totalPercent, brush);
            p->restore();

            p->save();
            // draw the active length
            p->setOpacity(activePieOpacity);
            p->setPen(Qt::NoPen);
            int activePercent = (int) roundNumber(portion.activeLength * 100/ m_totalSize);
            paintPieData(p, QRect(rect.x() + 15, rect.y() + 15, rect.width() - 30, rect.height() - 30),
                            current_angle, activePercent, brush);
            drawLegend(m_data.keys().at(i), p, option, m_colors.color(i*6 + 4), i);
            p->restore();
        }

        p->restore();
    }

    m_needsRepaint = false;
}

void PieChartWidget::update()
{
    m_needsRepaint = true;
    QGraphicsWidget::update();
}

int PieChartWidget::paintPieData(QPainter *p, const QRect &rect, int angle, int percent, const QBrush &brush)
{
    int end_angle = -1 * percent * 36/10 * 16;

    p->setBrush(brush);
    p->drawPie(rect, angle, end_angle);

    return end_angle + angle;
}

// Draw the graph legend with the names of the data
void PieChartWidget::drawLegend(const QString &name, QPainter *p, const QStyleOptionGraphicsItem *option, const QColor &color, int count)
{
    int textLength = option->rect.width() - 100;
    int y = option->rect.height() / 2 + 10;
    p->save();
    p->setPen(Qt::NoPen);
    p->setBrush(QBrush(color));
    p->drawRoundRect(QRect(LEFT_MARGIN, count * 20 + y, 10, 10));
    p->setPen(Qt::SolidLine);
    p->setPen(Qt::white);

    // the data name
    p->drawText(QRect(LEFT_MARGIN + 14, count * 20 + y - 5, textLength, 20), Qt::AlignLeft,
                    p->fontMetrics().elidedText(name, Qt::ElideLeft, textLength));

    // the data percent
    p->drawText(QRect(LEFT_MARGIN + 14 + textLength, count * 20 + y - 5, 150, 20),
                    Qt::AlignLeft, KGlobal::locale()->formatByteSize(m_data [name].length));

    p->restore();
}

#include "piechartwidget.moc"
