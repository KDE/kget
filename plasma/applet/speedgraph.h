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

#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include "transfergraph.h"
#include <QMap>
#include <QQueue>

static const int MAX_ITEMS = 10;
static const int GRAPH_HEIGHT = 200;


class SpeedGraph : public TransferGraph
{
    Q_OBJECT

public:
    SpeedGraph(QObject *parent = 0);

    void paint(QPainter *p, const QRect &contentsRect);
    QSizeF contentSizeHint();
    void setTransfers(const QVariantMap &percents);

private:
    void drawLegend(const QVariantList &transfer, QPainter *p, const QColor &color, uint y);
    void drawTimeline(QQueue <uint> *transfer, QPainter *p, const QRect &contentsRect, const QColor &color);
    void prepareData();

    QMap <QString, QQueue <uint> *> m_data;
    uint m_maxSpeed;
};

#endif
