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

#include "piegraph.h"
#include "piechartwidget.h"

#include <QVariant>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>

#include <math.h>

PieGraph::PieGraph(QGraphicsWidget *parent)
    : TransferGraph(0)
{
    m_layout = static_cast <QGraphicsLinearLayout *> (parent->layout());
    if (m_layout)
    {
        m_chart = new PieChartWidget(parent);

        m_layout->addItem(m_chart);
   }
}

PieGraph::~PieGraph()
{
    m_layout->removeItem(m_chart);
    delete m_chart;
}

void PieGraph::setTransfers(const QVariantMap &transfers)
{
    m_chart->setTransfers(transfers);
}

