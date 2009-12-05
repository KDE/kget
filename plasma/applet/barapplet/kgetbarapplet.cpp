/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>                 *
 *   Copyright (C) 2009 by Matthias Fuchs <mat69@gmx.net>                  *
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

#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QProgressBar>

#include <KDebug>

#include <Plasma/ScrollWidget>


KGetBarApplet::Private::Private(QGraphicsWidget *parent)
  : QGraphicsWidget(parent)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, this);

    m_scrollWidget = new Plasma::ScrollWidget();
    m_containerWidget = new QGraphicsWidget(m_scrollWidget);
    m_scrollWidget->setWidget(m_containerWidget);
    m_containerLayout = new QGraphicsLinearLayout(Qt::Vertical, m_containerWidget);

    layout->addItem(m_scrollWidget);

    setLayout(layout);
}

KGetBarApplet::Private::~Private()
{
    foreach (Item *item, m_items) {
        delete item;
    }
}

void KGetBarApplet::Private::addTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    foreach (OrgKdeKgetTransferInterface *transfer, transfers) {
        if (m_transfers.contains(transfer)) {
            continue;
        }

        m_transfers.append(transfer);

        QString fileName = KUrl(transfer->source().value()).fileName();
        kDebug() << fileName;

        Item *item = new Item;
        item->proxy = new QGraphicsProxyWidget();
        item->progressBar = new QProgressBar();
        item->proxy->setWidget(item->progressBar);
        item->progressBar->setFormat(fileName + " %v%");
        item->transfer = transfer;

        m_items.append(item);

        //running downloads are prepended, others appended
        if (transfer->percent() == 100) {
            m_containerLayout->addItem(item->proxy);
        } else {
            m_containerLayout->insertItem(0, item->proxy);
        }

        item->progressBar->setValue(transfer->percent().value());
    }
}

void KGetBarApplet::Private::slotUpdate()
{
    //update percent and name (file could be renamed)
    foreach (Item *item, m_items) {
        OrgKdeKgetTransferInterface *transfer = item->transfer;
        QString fileName = KUrl(transfer->source().value()).fileName();

        item->progressBar->setFormat(fileName + " %v%");
        item->progressBar->setValue(transfer->percent());
    }
}

void KGetBarApplet::Private::removeTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    foreach (OrgKdeKgetTransferInterface *transfer, transfers) {
        m_transfers.removeAll(transfer);
    }

    // remove the progressbars for the deleted transfers
    foreach (Item *item, m_items) {
        if (!m_transfers.contains(item->transfer)) {
            m_containerLayout->removeItem(item->proxy);
            item->proxy->deleteLater();
            item->progressBar->deleteLater();
            m_items.removeAll(item);
            delete item;
        }
    }
}

KGetBarApplet::KGetBarApplet(QObject *parent, const QVariantList &args) 
        : KGetApplet(parent, args)
{
}

KGetBarApplet::~KGetBarApplet()
{
}

void KGetBarApplet::init()
{
    d = new KGetBarApplet::Private(this);
    connect(this, SIGNAL(transfersAdded(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(addTransfers(QList<OrgKdeKgetTransferInterface*>)));
    connect(this, SIGNAL(transfersRemoved(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(removeTransfers(QList<OrgKdeKgetTransferInterface*>)));
    connect(this, SIGNAL(update()), d, SLOT(slotUpdate()));
    setDataWidget(d);

    KGetApplet::init();
}

#include "kgetbarapplet.moc"
#include "kgetbarapplet_p.moc"
