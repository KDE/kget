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

#include "barapplet/kgetbarapplet.h"
#include "barapplet/kgetbarapplet_p.h"
#include "common/kgetappletutils.h"

#include <QVBoxLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsLayoutItem>
#include <QGraphicsWidget>
#include <QPainter>
#include <QHBoxLayout>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStyleOptionGraphicsItem>

#include <KDebug>
#include <KIcon>

#include <plasma/svg.h>
#include <plasma/applet.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/pushbutton.h>

const int TOP_MARGIN = 60;
const int SPACING = 4;
const int MARGIN = 20;
const int MAX_DOWNLOADS_PER_PAGE = 5;

KGetBarApplet::Private::Private(QGraphicsWidget *parent) : QGraphicsWidget(parent),
    m_actualPage(0)
{
    m_verticalLayout = new QGraphicsLinearLayout(Qt::Vertical, this);

    // Pager layout and next, previous buttons
    QGraphicsLinearLayout *pager_layout = new QGraphicsLinearLayout(Qt::Horizontal, m_verticalLayout);

    m_previousPageButton = new Plasma::PushButton(this);
    m_previousPageButton->setIcon(KIcon("go-previous"));
    m_previousPageButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    m_nextPageButton = new Plasma::PushButton(this);
    m_nextPageButton->setIcon(KIcon("go-next"));
    m_nextPageButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    m_previousPageButton->setEnabled(false);
    m_nextPageButton->setEnabled(false);

    pager_layout->addItem(m_previousPageButton);
    pager_layout->addItem(m_nextPageButton);

    m_verticalLayout->addItem(pager_layout);

    // Total size
    m_totalSizeLabel = new Plasma::Label(this);
    m_totalSizeLabel->setAlignment(Qt::AlignRight);

    m_verticalLayout->addItem(m_totalSizeLabel);

    m_pageLabel = new Plasma::Label(this);

    m_verticalLayout->addItem(m_pageLabel);

    setLayout(m_verticalLayout);

    // connect the clicked signal of the next and previous buttons
    connect(m_previousPageButton, SIGNAL(clicked()), SLOT(previousPage()));
    connect(m_nextPageButton, SIGNAL(clicked()), SLOT(nextPage()));
}

KGetBarApplet::Private::~Private()
{
}

void KGetBarApplet::Private::addTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    m_transfers << transfers;
}

void KGetBarApplet::Private::removeTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    foreach (OrgKdeKgetTransferInterface* transfer, transfers)
        m_transfers.removeAll(transfer);
}

void KGetBarApplet::Private::nextPage()
{
    if (m_transfers.size() >= ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE)) {
        m_actualPage ++;

        clear();
        populate();
    }
}

void KGetBarApplet::Private::previousPage()
{
    if (m_actualPage > 0) {
        m_actualPage --;

        clear();
        populate();
    }
}

void KGetBarApplet::Private::populate()
{
    int totalSize = 0;
    int limit = m_transfers.size() < ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE) ? 
            m_transfers.size() :
            ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE);

    for(int i = (m_actualPage * MAX_DOWNLOADS_PER_PAGE); i < limit; i++) {
        OrgKdeKgetTransferInterface* transfer = m_transfers.at(i);
        QString fileName = KUrl(transfer->source().value()).fileName();
        kDebug() << fileName;

        Item *item = 0;
        foreach (Item * i, m_items) {
            if (i->transfer == transfer) {
                item = i;
                break;
            }
        }

        if (!item) {
            kDebug() << "Create new Item";
            item = new Item;
            item->proxy = new QGraphicsProxyWidget(this);
            item->progressBar = new QProgressBar();
            item->proxy->setWidget(item->progressBar);
            item->progressBar->setFormat(fileName + " %v%");
            item->transfer = transfer;

            m_items.append(item);
            m_verticalLayout->insertItem(0, item->proxy);
        }

        item->progressBar->setValue(transfer->percent().value());
        totalSize += transfer->totalSize().value();
    }

    m_pageLabel->setText(i18n("Showing %1-%2 of %3 transfers",
                         m_actualPage * MAX_DOWNLOADS_PER_PAGE, limit, m_transfers.size()));

    // remove the progressbars for the deleted transfers
    foreach (Item * item, m_items) {
        if (!m_transfers.contains(item->transfer)) {
            m_verticalLayout->removeItem(item->proxy);
            item->proxy->deleteLater();
            item->progressBar->deleteLater();
            m_items.removeAll(item);
            delete item;
        }
    }

    // activate or deactivate the navigation buttons
    m_nextPageButton->setEnabled(m_transfers.size() > ((m_actualPage + 1)* MAX_DOWNLOADS_PER_PAGE));

    m_previousPageButton->setEnabled(m_actualPage > 0);
}

void KGetBarApplet::Private::clear()
{
    foreach (Item * item, m_items) {
        m_verticalLayout->removeItem(item->proxy);
        item->proxy->deleteLater();
        item->progressBar->deleteLater();
        delete item;
    }
    m_items.clear();
}

KGetBarApplet::KGetBarApplet(QObject *parent, const QVariantList &args) 
        : KGetApplet(parent, args)
{
}

KGetBarApplet::~KGetBarApplet()
{
    delete d;
}

void KGetBarApplet::init()
{
    d = new KGetBarApplet::Private(this);
    connect(this, SIGNAL(transfersAdded(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(addTransfers(QList<OrgKdeKgetTransferInterface*>)));
    connect(this, SIGNAL(transfersRemoved(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(removeTransfers(QList<OrgKdeKgetTransferInterface*>)));
    connect(this, SIGNAL(update()), d, SLOT(populate()));
    setDataWidget(d);

    KGetApplet::init();
}

#include "kgetbarapplet.moc"
#include "kgetbarapplet_p.moc"
