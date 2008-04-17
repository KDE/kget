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

#include <KIcon>
#include <KDebug>
#include <KLocale>
#include <KGlobal>

#include <QVariant>
#include <QGraphicsLinearLayout>

#include <math.h>

PieGraph::PieGraph(Plasma::Applet *parent)
    : TransferGraph(parent)
{
    m_layout = static_cast <QGraphicsLinearLayout *> (parent->layout());
    if (m_layout)
    {/*
        m_chart = new PieChartWidget(parent);
        m_layout->addItem(m_chart);

        QObject::connect(m_chart, SIGNAL(geometryChanged()), SLOT(updateGeometry()));*/
    }
}

PieGraph::~PieGraph()
{
    /*m_layout->removeItem(m_chart);
    delete m_chart;*/
}

void PieGraph::updateGeometry()
{/*
    kDebug() << "About to update the widget geometry " << endl;
    m_applet->updateGeometry();*/
}

void PieGraph::setTransfers(const QVariantMap &transfers)
{/*
    // drop the deleted transfers
    foreach (const QString &key, m_transfers.keys()) {
        if (!transfers.contains(key)) {
            m_chart->removeData(key);
        }
    }

    TransferGraph::setTransfers(transfers);

    m_chart->clear();

    foreach(const QString &name, transfers.keys()) {
        QVariantList attributes = transfers[name].toList();

        m_chart->addData(name, attributes[2].toDouble(),
                        attributes[1].toInt() * attributes[2].toDouble() / 100,
                        attributes[3].toBool());
    }

    m_chart->updateView();*/
}
