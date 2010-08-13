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

#include "piechart/kgetpiechart.h"
#include "piechart/kgetpiechart_p.h"
#include "common/kgetappletutils.h"

#include "../../../core/transferhandler.h"

#include <QVBoxLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsLayoutItem>
#include <QGraphicsWidget>
#include <QPainter>
#include <QHBoxLayout>
#include <QGraphicsProxyWidget>
#include <QStyleOptionGraphicsItem>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KGlobal>

#include <plasma/svg.h>
#include <plasma/applet.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>
#include <Plasma/Label>
#include <Plasma/ScrollWidget>

const float KGetPieChart::PieChart::PIE_OPACITY = 0.67f;
const float KGetPieChart::PieChart::ACTIVE_PIE_OPACITY = 0.84f;

KGetPieChart::PieChart::PieChart(QHash<OrgKdeKgetTransferInterface*, Data> *data, KIO::filesize_t totalSize, QGraphicsWidget *parent)
  : QGraphicsWidget(parent),
    m_data(data),
    m_totalSize(totalSize)
{
    setMinimumSize(QSizeF(80, 80));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    m_totalPen.setWidth(1);
    m_totalPen.setColor(Qt::darkGray);
    m_totalPen.setStyle(Qt::SolidLine);

    m_activePen.setWidth(1);
    m_activePen.setColor(Qt::white);
    m_activePen.setStyle(Qt::SolidLine);
}

KGetPieChart::PieChart::~PieChart()
{
}

void KGetPieChart::PieChart::setTotalSize(KIO::filesize_t totalSize)
{
    m_totalSize = totalSize;
    m_angles.clear();
    update();
}

void KGetPieChart::PieChart::createAngles()
{
    m_angles.clear();

    if (m_totalSize) {
        QHash<OrgKdeKgetTransferInterface*, Data>::const_iterator it;
        QHash<OrgKdeKgetTransferInterface*, Data>::const_iterator itEnd = m_data->constEnd();
        int startAngle = 90 * 16;
        for (it = m_data->constBegin(); it != itEnd; ++it) {
            const int span = (-1 * ((it.value().size * 360 * 16) / m_totalSize));
            m_angles[it.key()] = qMakePair(startAngle, span);

            startAngle += span;
        }
    }
}

void KGetPieChart::PieChart::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    kDebug() << "Repaint";

    if (m_totalSize && (m_angles.count() != m_data->count())) {
        createAngles();
    }

    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::TextAntialiasing);

    int size = option->rect.height();
    if (option->rect.width() < size) {
        size = option->rect.width();
    }
    const QRect rect = QRect(option->rect.x() + option->rect.width() / 2 - size / 2,
                             option->rect.y(),
                             size,
                             size);

    QHash<OrgKdeKgetTransferInterface*, Data>::const_iterator it;
    QHash<OrgKdeKgetTransferInterface*, Data>::const_iterator itEnd = m_data->constEnd();
    for (it = m_data->constBegin(); it != itEnd; ++it) {
        OrgKdeKgetTransferInterface *transfer = it.key();
        const Data data = it.value();
        QPair<int, int> angles = m_angles[transfer];
        const QBrush brush(data.color);

        p->setBrush(brush);

        if (data.isFinished) {
            p->setPen(m_totalPen);
        } else {
            p->setPen(m_activePen);
        }

        p->setOpacity(PIE_OPACITY);
        p->drawPie(rect, angles.first, angles.second);

        p->setOpacity(ACTIVE_PIE_OPACITY);
        if (m_totalSize && !data.isFinished) {
            angles.second = (-1 * ((data.downloadedSize * 360 * 16) / m_totalSize));
        }
        p->drawPie(QRect(rect.x() + 15, rect.y() + 15, rect.width() - 30, rect.height() - 30), angles.first, angles.second);
    }
}

KGetPieChart::Item::Item(QGraphicsWidget *parent)
  : QGraphicsWidget(parent)
{
    m_colorLabel = new Plasma::Label();
    m_colorLabel->nativeWidget()->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_name = new Plasma::Label();
    m_name->nativeWidget()->setWordWrap(false);
    m_name->nativeWidget()->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    m_sizeLabel = new Plasma::Label();
    m_sizeLabel->nativeWidget()->setWordWrap(false);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->addItem(m_colorLabel);
    layout->addItem(m_name);
    layout->addItem(m_sizeLabel);

    setLayout(layout);
}

KGetPieChart::Item::~Item()
{
}

void KGetPieChart::Item::setSize(KIO::filesize_t size)
{
    m_sizeLabel->setText(KGlobal::locale()->formatByteSize(size));
}

void KGetPieChart::Item::setName(const QString &name)
{
    m_name->setText(name);
}

void KGetPieChart::Item::setColor(const QColor &color)
{
    QPixmap pixmap(10, 10);
    pixmap.fill(QColor(color));
    m_colorLabel->nativeWidget()->setPixmap(pixmap);
}

KGetPieChart::Private::Private(QGraphicsWidget *parent) : QGraphicsWidget(parent),
    m_colors("Oxygen.colors"),
    m_totalSize(0),
    m_piechart(0)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, this);

    m_piechart = new KGetPieChart::PieChart(&m_data, m_totalSize);
    layout->insertItem(0, m_piechart);
    m_scrollWidget = new Plasma::ScrollWidget();
    m_scrollWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_containerWidget = new QGraphicsWidget(m_scrollWidget);
    m_scrollWidget->setWidget(m_containerWidget);
    m_containerLayout = new QGraphicsLinearLayout(Qt::Vertical, m_containerWidget);

    layout->addItem(m_scrollWidget);

    setLayout(layout);
}

KGetPieChart::Private::~Private()
{
}

void KGetPieChart::Private::addTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    foreach (OrgKdeKgetTransferInterface *transfer, transfers) {
        if (m_items.contains(transfer)) {
            continue;
        }

        connect(transfer, SIGNAL(transferChangedEvent(int)), this, SLOT(slotUpdateTransfer(int)));

        QString fileName = KUrl(transfer->dest().value()).fileName();
        kDebug() << fileName;

        Data data;
        data.name = fileName;
        data.isFinished = (transfer->status() == Transfer::Finished);
        data.size = transfer->totalSize();
        data.downloadedSize = transfer->downloadedSize();
        m_data[transfer] = data;

        Item *item = new Item;
        item->setName(data.name);
        item->setSize(data.size);
        m_items[transfer] = item;

        m_totalSize += data.size;

        //running downloads are prepended, others appended
        if (data.isFinished) {
            m_containerLayout->addItem(item);
        } else {
            m_containerLayout->insertItem(0, item);
        }
    }

    updateTransfers();
}

void KGetPieChart::Private::removeTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    // remove the progressbars for the deleted transfers
    QHash<OrgKdeKgetTransferInterface*, Item*>::iterator it;
    QHash<OrgKdeKgetTransferInterface*, Item*>::iterator itEnd = m_items.end();
    for (it = m_items.begin(); it != itEnd; ) {
        OrgKdeKgetTransferInterface *transfer = it.key();
        if (transfers.contains(transfer)) {
            Item *item = it.value();
            it = m_items.erase(it);

            m_totalSize -= m_data[transfer].size;
            m_data.remove(transfer);

            m_containerLayout->removeItem(item);
            item->deleteLater();
        } else {
            ++it;
        }
    }

    updateTransfers();
}

void KGetPieChart::Private::slotUpdateTransfer(int transferChange)
{
    OrgKdeKgetTransferInterface *transfer = qobject_cast<OrgKdeKgetTransferInterface*>(QObject::sender());

    bool updatePieSize = false;
    bool updatePie = false;
    if (transfer && m_items.contains(transfer)) {
        Item *item = m_items[transfer];

        if (transferChange & Transfer::Tc_Status) {
            m_data[transfer].isFinished = (transfer->status() == Transfer::Finished);
            updatePie = true;
        }
        if (transferChange & Transfer::Tc_FileName) {
            m_data[transfer].name = KUrl(transfer->dest().value()).fileName();
            item->setName(m_data[transfer].name);
        }
        if (transferChange & Transfer::Tc_TotalSize) {
            m_totalSize -= m_data[transfer].size;
            KIO::filesize_t newSize = transfer->totalSize();
            m_totalSize += newSize;
            m_data[transfer].size = newSize;

            item->setSize(m_data[transfer].size);

            updatePieSize = true;
        }
        if (transferChange & Transfer::Tc_DownloadedSize) {
            m_data[transfer].downloadedSize = transfer->downloadedSize();
            updatePie = true;
        }
    }

    if (updatePieSize) {
        m_piechart->setTotalSize(m_totalSize);
    } else if (updatePie) {
        m_piechart->update();
    }
}
void KGetPieChart::Private::updateTransfers()
{
    if (m_items.isEmpty()) {
        return;
    }

    int step = m_colors.count() / m_items.count();
    //there are more items than colors
    if (!step) {
        step = 1;
    }

    int i = 0;
    QHash<OrgKdeKgetTransferInterface*, Item*>::const_iterator it;
    QHash<OrgKdeKgetTransferInterface*, Item*>::const_iterator itEnd = m_items.constEnd();
    for (it = m_items.constBegin(); it != itEnd; ++it) {
        m_data[it.key()].color = m_colors.color(i * 6 + 4);
        it.value()->setColor(m_data[it.key()].color);
        ++i;
    }

    m_piechart->setTotalSize(m_totalSize);
}


KGetPieChart::KGetPieChart(QObject *parent, const QVariantList &args) 
        : KGetApplet(parent, args)
{
    setMinimumSize(QSizeF(150, 250));
}

KGetPieChart::~KGetPieChart()
{
    delete d;
}

void KGetPieChart::init()
{
    d = new KGetPieChart::Private(this);
    setDataWidget(d);
    connect(this, SIGNAL(transfersAdded(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(addTransfers(QList<OrgKdeKgetTransferInterface*>)));
    connect(this, SIGNAL(transfersRemoved(QList<OrgKdeKgetTransferInterface*>)), d, SLOT(removeTransfers(QList<OrgKdeKgetTransferInterface*>)));

    KGetApplet::init();
}

#include "kgetpiechart.moc"
#include "kgetpiechart_p.moc"
