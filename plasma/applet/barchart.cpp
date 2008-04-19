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

#include <QHBoxLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>

#include <KIcon>
#include <KLocale>

BarChart::BarChart(Plasma::Applet *parent)
    : TransferGraph(parent),
    m_actualPage(0)
{
    m_totalSizeLabel = 0;
    m_layout = static_cast <QGraphicsLinearLayout *> (parent->layout());
    if (m_layout)
    {
        // Pager layout and next, previous buttons
        QHBoxLayout *pager_layout = new QHBoxLayout();

        m_pageLabel = new QLabel();

        m_previousPageButton = new QPushButton(KIcon("go-previous"), "");
        m_nextPageButton = new QPushButton(KIcon("go-next"), "");
        m_previousPageButton->setEnabled(false);
        m_nextPageButton->setEnabled(false);

        pager_layout->addWidget(m_previousPageButton);
        pager_layout->addWidget(m_nextPageButton);

        // Total size
        m_totalSizeLabel = new QLabel(0);
        m_totalSizeLabel->setAlignment(Qt::AlignRight);

        m_verticalLayout = new QVBoxLayout();
        m_verticalLayout->addWidget(m_pageLabel);
        m_verticalLayout->addLayout(pager_layout);
        m_verticalLayout->addWidget(m_totalSizeLabel);

        m_mainWidget = new QWidget();
        m_mainWidget->setLayout(m_verticalLayout);
        m_mainWidget->setStyleSheet("background-color: transparent; color: white");

        m_proxyMainWidget = new QGraphicsProxyWidget(parent);
        m_proxyMainWidget->setWidget(m_mainWidget);
        m_layout->addItem(m_proxyMainWidget);

        // connect the clicked signal of the next and previous buttons
        QObject::connect(m_previousPageButton, SIGNAL(clicked()), SLOT(previousPage()));
        QObject::connect(m_nextPageButton, SIGNAL(clicked()), SLOT(nextPage()));
    }
}

BarChart::~BarChart()
{
    m_proxyMainWidget->setWidget(0);

    delete m_pageLabel;
    delete m_totalSizeLabel;
    delete m_nextPageButton;
    delete m_previousPageButton;

    foreach(const QString &key, m_progressBars.keys()) {
        delete m_progressBars[key];
    }

    m_layout->removeItem(m_proxyMainWidget);

    delete m_proxyMainWidget;
    delete m_mainWidget;
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
            QProgressBar *bar = new QProgressBar();
            bar->setFormat(m_transfers[key].toList().at(0).toString() + " %v%");
           // bar->setMinimumSize(QSizeF(100, 20));

            m_progressBars [key] = bar;
            m_verticalLayout->insertWidget(0, bar);
        }
        // set the progress bar opacity to 1 if the transfer is active
        // qreal opacity = (m_transfers [key].toList().at(3).toUInt() == 1) ? 1.0 : 0.6;

        //m_progressBars [key]->setOpacity(opacity);
        m_progressBars [key]->setValue(m_transfers[key].toList().at(1).toString().toInt());
        totalSize += m_transfers[key].toList().at(2).toInt();
    }

    m_totalSizeLabel->setText(i18n("Total size: %1", KGlobal::locale()->formatByteSize(totalSize)));
    m_pageLabel->setText(i18n("Showing %1-%2 of %3 transfers",
        m_actualPage * MAX_DOWNLOADS_PER_PAGE, limit, m_transfers.size()));

    // remove the progressbars for the deleted transfers
    foreach(const QString &key, m_progressBars.keys()) {
        if(!m_transfers.keys().contains(key)) {
            QProgressBar *bar = m_progressBars [key];
            m_progressBars.remove(key);
            m_verticalLayout->removeWidget(bar);
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
}

void BarChart::clear()
{
    foreach(const QString &key, m_progressBars.keys()) {
        QProgressBar *bar = m_progressBars [key];
        m_verticalLayout->removeWidget(bar);
        m_progressBars.remove(key);
        delete bar;
    }
}
