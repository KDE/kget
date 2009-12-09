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

#include "../../../core/transferhandler.h"

#include <QProgressBar>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>

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
}

void KGetBarApplet::Private::addTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    foreach (OrgKdeKgetTransferInterface *transfer, transfers) {
        if (m_items.contains(transfer)) {
            continue;
        }

        connect(transfer, SIGNAL(transferChangedEvent(int)), this, SLOT(slotUpdateTransfer(int)));

        QString fileName = KUrl(transfer->dest().value()).fileName();
        kDebug() << fileName;

        Item *item = new Item;
        item->proxy = new QGraphicsProxyWidget();
        item->progressBar = new QProgressBar();
        item->proxy->setWidget(item->progressBar);
        item->progressBar->setFormat(fileName + " %v%");
        m_items[transfer] = item;

        //running downloads are prepended, others appended
        if (transfer->percent() == 100) {
            m_containerLayout->addItem(item->proxy);
        } else {
            m_containerLayout->insertItem(0, item->proxy);
        }

        item->progressBar->setValue(transfer->percent().value());
    }
}

void KGetBarApplet::Private::removeTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    // remove the progressbars for the deleted transfers
    QHash<OrgKdeKgetTransferInterface*, Item*>::iterator it;
    QHash<OrgKdeKgetTransferInterface*, Item*>::iterator itEnd = m_items.end();
    for (it = m_items.begin(); it != itEnd; ) {
        OrgKdeKgetTransferInterface *transfer = it.key();
        if (transfers.contains(transfer)) {
            Item *item = it.value();
            it = m_items.erase(it);

            m_containerLayout->removeItem(item->proxy);
            item->proxy->deleteLater();
            item->progressBar->deleteLater();
            delete item;
        } else {
            ++it;
        }
    }
}

void KGetBarApplet::Private::slotUpdateTransfer(int transferChange)
{
    OrgKdeKgetTransferInterface *transfer = qobject_cast<OrgKdeKgetTransferInterface*>(QObject::sender());

    if (transfer && m_items.contains(transfer)) {
        Item *item = m_items[transfer];

        if (transferChange & Transfer::Tc_Percent) {
            item->progressBar->setValue(transfer->percent());
        }
        if (transferChange & Transfer::Tc_FileName) {
            const QString fileName = KUrl(transfer->dest().value()).fileName();
            item->progressBar->setFormat(fileName + " %v%");
            item->progressBar->setValue(transfer->percent());
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
    setDataWidget(d);
    connect(this, SIGNAL(transfersAdded(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(addTransfers(QList<OrgKdeKgetTransferInterface*>)));
    connect(this, SIGNAL(transfersRemoved(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(removeTransfers(QList<OrgKdeKgetTransferInterface*>)));

    KGetApplet::init();
}

#include "kgetbarapplet.moc"
#include "kgetbarapplet_p.moc"
