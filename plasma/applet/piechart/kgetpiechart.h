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
#ifndef KGETPIECHART_H
#define KGETPIECHART_H

#include "common/kgetapplet.h"
#include <plasma/dataengine.h>

class QGraphicsLinearLayout;
class QGraphicsWidget;

namespace Plasma {
    class Svg;
}

class KGetPieChart : public KGetApplet
{
    Q_OBJECT
public:
    KGetPieChart(QObject *parent, const QVariantList &args);
    ~KGetPieChart();

    void init();

private:
    class Data;
    class Item;
    class PieChart;
    class PrivateData;
    class Private;
    Private *d;
};

K_EXPORT_PLASMA_APPLET(kgetpiechart, KGetPieChart)

#endif
