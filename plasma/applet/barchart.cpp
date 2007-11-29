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

#include "barchart.h"
#include "transfergraph.h"

#include <plasma/widgets/label.h>
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/progressbar.h>
//#include <plasma/widgets/layoutanimator.h>

#include <KLocale>
//#include <QTimeLine>

BarChart::BarChart(Plasma::Applet *parent, Plasma::BoxLayout *mainlayout)
    : TransferGraph(parent)
{
    m_titleLabel = 0;
    m_totalSizeLabel = 0;
    m_layout = mainlayout;

/*
    // Layout animator
    Plasma::LayoutAnimator *animator = new Plasma::LayoutAnimator();
    QTimeLine * timeLine = new QTimeLine();

    animator->setTimeLine(timeLine);
    animator->setEffect(Plasma::LayoutAnimator::InsertedState, Plasma::LayoutAnimator::FadeInMoveEffect);
    animator->setEffect(Plasma::LayoutAnimator::StandardState, Plasma::LayoutAnimator::MoveEffect);
    animator->setEffect(Plasma::LayoutAnimator::RemovedState, Plasma::LayoutAnimator::FadeOutMoveEffect);
    m_layout->setAnimator(animator);
*/
   // Title
    m_titleLabel = new Plasma::Label(m_applet);
    m_titleLabel->setText("KGet downloads");
    m_titleLabel->setPen(QPen(Qt::white));
    m_titleLabel->setAlignment(Qt::AlignLeft);

    // Total size
    m_totalSizeLabel = new Plasma::Label(m_applet);
    m_totalSizeLabel->setPen(QPen(Qt::white));
    m_totalSizeLabel->setAlignment(Qt::AlignRight);
}

BarChart::~BarChart()
{
    delete m_titleLabel;
    delete m_totalSizeLabel;

    foreach(const QString &key, m_progressBars.keys()) {
        delete m_progressBars[key];
    }
}

void BarChart::setTransfers(const QVariantMap &transfers)
{

    TransferGraph::setTransfers(transfers);
    uint totalSize = 0;

    foreach(QString key, m_transfers.keys()) {
        if(m_progressBars.count(key) <= 0) {
            Plasma::ProgressBar *bar = new Plasma::ProgressBar(m_applet);
            bar->setFormat(m_transfers[key].toList().at(0).toString() + " %v%");
            m_progressBars [key] = bar;

            m_layout->addItem(bar);
        }
        m_progressBars [key]->setValue(m_transfers[key].toList().at(1).toString().toInt());
        totalSize += m_transfers[key].toList().at(2).toInt();
    }

    m_totalSizeLabel->setText(i18n("Total size: %1", KGlobal::locale()->formatByteSize(totalSize)));
    if(m_layout->indexOf(m_totalSizeLabel) != m_layout->count() - 1) {
        m_layout->removeItem(m_totalSizeLabel);
        m_layout->insertItem(m_layout->count(), m_totalSizeLabel);
    }

    // remove the progressbars for the deleted transfers
    foreach(QString key, m_progressBars.keys()) {
        if(!m_transfers.keys().contains(key)) {
            Plasma::ProgressBar *bar = m_progressBars [key];
            m_progressBars.remove(key);
            m_layout->removeItem(bar);
            delete bar;
        }
    }

    m_applet->updateGeometry();
}
