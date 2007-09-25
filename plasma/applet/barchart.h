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

#ifndef KGET_BARCHART_H
#define KGET_BARCHART_H

#include "transfergraph.h"

#include <QColor>

static const float PERCENT_OPACITY = 0.6;
static const QColor PERCENT_BACKGROUND_COLOR = QColor(222, 222, 222);
static const QColor PERCENT_COLOR = QColor(40, 0, 0);
static const QColor PERCENT_COLOR2 = QColor(200, 20, 20);
static const int TRANSFER_SIZE_WIDTH = 100;
static const QColor TRANSFER_NAME_COLOR = QColor(33, 33, 33);


class BarChart : public TransferGraph
{
    Q_OBJECT
public:
    BarChart(QObject *parent = 0);

    void paint(QPainter *p, const QRect &contentsRect);
    QSizeF contentSizeHint();
};

#endif
