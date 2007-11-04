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

#ifndef KGET_PIEGRAPH_H
#define KGET_PIEGRAPH_H

#include <QColor>
#include "transfergraph.h"

#define TRANSFER_PIEGRAPH_WIDTH 140
#define TRANSFER_PIEGRAPH_HEIGHT 140

namespace Plasma {
    class Applet;
};
class QBrush;

class PieGraph : public TransferGraph
{
    Q_OBJECT

public:
    PieGraph(Plasma::Applet *parent);
    ~PieGraph();

    void paint(QPainter *p, const QRect &contentsRect);
    QSizeF contentSizeHint();

private:
    int drawPie(QPainter *, const QRect &, int angle, int percent, const QBrush &brush);
    void drawLegend(QPainter *p, int y, const QString &name, int percent, double size, const QColor &color);

    int m_totalFiles;
};

#endif
