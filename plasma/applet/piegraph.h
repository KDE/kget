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

#include "transfergraph.h"
#include "piechartwidget.h"

namespace Plasma {
    class Applet;
    class BoxLayout;
}

class PieGraph : public TransferGraph
{
    Q_OBJECT

public:
    PieGraph(Plasma::Applet *parent, Plasma::BoxLayout *main_layout);
    ~PieGraph();

    void setTransfers(const QVariantMap &percents);

private slots:
    void updateGeometry();

private:
    Plasma::BoxLayout *m_layout;
    PieChartWidget *m_chart;

};

#endif
