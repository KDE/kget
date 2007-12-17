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
#include <plasma/widgets/pushbutton.h>
#include <plasma/widgets/progressbar.h>
//#include <plasma/widgets/layoutanimator.h>

#include <KLocale>
//#include <QTimeLine>

BarChart::BarChart(Plasma::Applet *parent, Plasma::BoxLayout *mainlayout)
    : TransferGraph(parent),
    m_actualPage(0)
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
    m_layout->addItem(m_titleLabel);

    // the progress bars layout
    m_progressBarsLayout = new Plasma::VBoxLayout(m_layout);
    m_progressBarsLayout->setMargin(0);
    m_layout->addItem(m_progressBarsLayout);

    // Pager layout and next, previous buttons
    m_pagerLayout = new Plasma::HBoxLayout(m_layout);
    m_pagerLayout->setMargin(0);
    m_pageLabel = new Plasma::Label(m_applet);
    m_pageLabel->setPen(QPen(Qt::gray));
    m_pageLabel->setAlignment(Qt::AlignRight);
    m_previousPageButton = new Plasma::PushButton(KIcon("go-previous"), "", m_applet);
    m_nextPageButton = new Plasma::PushButton(KIcon("go-next"), "", m_applet);
    m_previousPageButton->setEnabled(false);
    m_nextPageButton->setEnabled(false);

    m_pagerLayout->addItem(m_pageLabel);
    m_pagerLayout->addItem(m_previousPageButton);
    m_pagerLayout->addItem(m_nextPageButton);

    m_layout->addItem(m_pagerLayout);

    // Total size
    m_totalSizeLabel = new Plasma::Label(m_applet);
    m_totalSizeLabel->setPen(QPen(Qt::white));
    m_totalSizeLabel->setAlignment(Qt::AlignRight);
    m_layout->addItem(m_totalSizeLabel);

    // connect the clicked signal of the next and previous buttons
    QObject::connect(m_previousPageButton, SIGNAL(clicked()), SLOT(previousPage()));
    QObject::connect(m_nextPageButton, SIGNAL(clicked()), SLOT(nextPage()));
}

BarChart::~BarChart()
{
    delete m_titleLabel;
    delete m_pageLabel;
    delete m_totalSizeLabel;
    delete m_nextPageButton;
    delete m_previousPageButton;

    foreach(const QString &key, m_progressBars.keys()) {
        delete m_progressBars[key];
    }

    delete m_pagerLayout;
    delete m_progressBarsLayout;
}

void BarChart::setTransfers(const QVariantMap &transfers)
{
    if(transfers.keys() != m_transfers.keys()) {
        clear();
    }

    TransferGraph::setTransfers(transfers);
    populate();
}

void BarChart::nextPage()
{
    if(m_transfers.size() >= ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE)) {
        m_actualPage ++;

        clear();
        populate();
    }
}

void BarChart::previousPage()
{
    if(m_actualPage > 0) {
        m_actualPage --;

        clear();
        populate();
    }
}

void BarChart::populate()
{
    int totalSize = 0;
    int limit = m_transfers.size() < ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE) ? 
            m_transfers.size() :
            ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE);

    for(int i = (m_actualPage * MAX_DOWNLOADS_PER_PAGE); i < limit; i++) {
        QString key = m_transfers.keys().at(i);

        if(m_progressBars.count(key) <= 0) {
            Plasma::ProgressBar *bar = new Plasma::ProgressBar(m_applet);
            bar->setFormat(m_transfers[key].toList().at(0).toString() + " %v%");
            bar->setMinimumSize(QSizeF(100, 20));
            m_progressBars [key] = bar;

            m_progressBarsLayout->addItem(bar);
        }
        // set the progress bar opacity to 1 if the transfer is active
        qreal opacity = (m_transfers [key].toList().at(3).toUInt() == 1) ? 1.0 : 0.6;

        m_progressBars [key]->setOpacity(opacity);
        m_progressBars [key]->setValue(m_transfers[key].toList().at(1).toString().toInt());
        totalSize += m_transfers[key].toList().at(2).toInt();
    }

    m_totalSizeLabel->setText(i18n("Total size: %1", KGlobal::locale()->formatByteSize(totalSize)));
    m_pageLabel->setText(i18n("Showing %1-%2 of %3 transfers",
        m_actualPage * MAX_DOWNLOADS_PER_PAGE, limit, m_transfers.size()));

    // remove the progressbars for the deleted transfers
    foreach(QString key, m_progressBars.keys()) {
        if(!m_transfers.keys().contains(key)) {
            Plasma::ProgressBar *bar = m_progressBars [key];
            m_progressBars.remove(key);
            m_progressBarsLayout->removeItem(bar);
            delete bar;
        }
    }

    // activate or deactivate the navigation buttons
    if(m_transfers.size() > ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE))
        m_nextPageButton->setEnabled(true);
    else
        m_nextPageButton->setEnabled(false);

    if(m_actualPage > 0)
        m_previousPageButton->setEnabled(true);
    else
        m_previousPageButton->setEnabled(false);

    m_layout->updateGeometry();
    m_applet->updateGeometry();
}

void BarChart::clear()
{
    foreach(QString key, m_progressBars.keys()) {
        Plasma::ProgressBar *bar = m_progressBars [key];
        m_progressBarsLayout->removeItem(bar);
        m_progressBars.remove(key);
        delete bar;
    }
}
