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

#include "piechart/kgetpiechart.h"
#include "piechart/kgetpiechart_p.h"
#include "common/kgetappletutils.h"

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

const static int TOP_MARGIN = 55;
const static int LEFT_MARGIN = 15;
const static int SPACING = 4;
const static int MARGIN = 20;
const static int MAX_DOWNLOADS_PER_PAGE = 5;

KGetPieChart::Private::Private(QGraphicsWidget *parent) : QGraphicsWidget(parent),
    m_data(),
    m_colors("Oxygen.colors"),
    m_totalSize(0),
    m_needsRepaint(0)
{
     setCacheMode(QGraphicsItem::DeviceCoordinateCache, QSize(300, 360));
}

KGetPieChart::Private::~Private()
{
}

void KGetPieChart::Private::setTransfers(const QList<OrgKdeKgetTransferInterface*> &transfers)
{
    m_needsRepaint = false;
    m_totalSize = 0;

    foreach (OrgKdeKgetTransferInterface* key, m_data.keys()) {
        if (!transfers.contains(key)) {
            m_data.remove(key);
            m_needsRepaint = true;
        }
    }

    foreach(OrgKdeKgetTransferInterface* transfer, transfers) {
        double length = transfer->totalSize().value();
        double activeLength = transfer->percent().value() * length / 100;
        bool active = transfer->status() == 0;//0 == Job::Running

        if (length > 0) {
            if (!m_data.contains(transfer)) {
                m_data[transfer] = PrivateData();
            }


            if (length != m_data[transfer].length || activeLength != m_data[transfer].activeLength) {
                m_needsRepaint = true;
            }

            m_data[transfer].name = KUrl(transfer->source().value()).fileName();
            m_data[transfer].length = length;
            m_data[transfer].activeLength = activeLength;
            m_data[transfer].isActive = active;

            m_totalSize += length;
        }
    }

    if (m_needsRepaint) {
        update();
    }
}

void KGetPieChart::Private::paint(QPainter  *p, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(widget)

    if (m_needsRepaint || true) {
        kDebug () << "Child needs repaint" ;
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::TextAntialiasing);

        // the rect of the pie chart
        // we draw the pie chart with width = height
    //             QRect rect = QRect(chartGeometry().width()/2 - chartGeometry().height()/2, 0, 
    //                         chartGeometry().height(), chartGeometry().height());
        int size = option->rect.height() / 2;
        QRect rect = QRect(option->rect.x() + option->rect.width() / 2 - size / 2, 
                        option->rect.y(),
                        size, 
                        size);

        int angle = 90 * 16; // 0

        QPen totalPen;
        QPen activePen;

        // total pen
        totalPen.setWidth(1);
        totalPen.setColor(Qt::darkGray);
        totalPen.setStyle(Qt::SolidLine);

        // active transfer pen
        activePen.setWidth(1);
        activePen.setColor(Qt::white);
        activePen.setStyle(Qt::SolidLine);

        for(int i = 0; i < m_data.size(); i++) {
            QBrush brush(m_colors.color(i*6 + 4));

            PrivateData portion = m_data [m_data.keys().at(i)];

            float pieOpacity = 0.67;
            float activePieOpacity = 0.84;
            int current_angle = angle;

            p->save();

            if (portion.isActive) {
                p->setPen(activePen);
            }
            else {
                p->setPen(totalPen);
            }

            p->setOpacity(pieOpacity);

            int totalPercent = (int) roundNumber(portion.length * 100/ m_totalSize);
            angle = paintPieData(p, rect, angle, totalPercent, brush);
            p->restore();

            p->save();
            // draw the active length
            p->setOpacity(activePieOpacity);
            p->setPen(Qt::NoPen);
            int activePercent = (int) roundNumber(portion.activeLength * 100/ m_totalSize);
            paintPieData(p, QRect(rect.x() + 15, rect.y() + 15, rect.width() - 30, rect.height() - 30),
                            current_angle, activePercent, brush);
            drawLegend(m_data.keys().at(i), p, option, m_colors.color(i*6 + 4), i);
            p->restore();
        }

        p->restore();
    }

    m_needsRepaint = false;
}

void KGetPieChart::Private::update()
{
    m_needsRepaint = true;
    QGraphicsWidget::update();
}

int KGetPieChart::Private::paintPieData(QPainter *p, const QRect &rect, int angle, int percent, const QBrush &brush)
{
    int end_angle = -1 * percent * 36/10 * 16;

    p->setBrush(brush);
    p->drawPie(rect, angle, end_angle);

    return end_angle + angle;
}

// Draw the graph legend with the names of the data
void KGetPieChart::Private::drawLegend(OrgKdeKgetTransferInterface* transfer, QPainter *p, const QStyleOptionGraphicsItem *option, const QColor &color, int count)
{
    int textLength = option->rect.width() - 100;
    int y = option->rect.height() / 2 + 10;
    p->save();
    p->setPen(Qt::NoPen);
    p->setBrush(QBrush(color));
    p->drawRoundRect(QRect(LEFT_MARGIN, count * 20 + y, 10, 10));
    p->setPen(Qt::SolidLine);
    p->setPen(Qt::white);

    // the data name
    p->drawText(QRect(LEFT_MARGIN + 14, count * 20 + y - 5, textLength, 20), Qt::AlignLeft,
                    p->fontMetrics().elidedText(KUrl(transfer->source().value()).fileName(), Qt::ElideLeft, textLength));

    // the data percent
    p->drawText(QRect(LEFT_MARGIN + 14 + textLength, count * 20 + y - 5, 150, 20),
                    Qt::AlignLeft, KGlobal::locale()->formatByteSize(m_data[transfer].length));

    p->restore();
}


KGetPieChart::KGetPieChart(QObject *parent, const QVariantList &args) 
        : KGetApplet(parent, args),
        m_errorWidget(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setBackgroundHints(Applet::DefaultBackground);

    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/kget");
}

KGetPieChart::~KGetPieChart()
{
    delete m_errorWidget;
    delete d;
}

void KGetPieChart::init()
{
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setSpacing(SPACING);
    m_layout->setContentsMargins(MARGIN, TOP_MARGIN, MARGIN, MARGIN);
    m_layout->setOrientation(Qt::Vertical);

    d = new KGetPieChart::Private(this);
    m_layout->addItem(d);

    setLayout(m_layout);

    resize(QSize(300, 360));

    KGetApplet::init();
}

void KGetPieChart::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        if (d) {
            d->update();
        }
    }
}

void KGetPieChart::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    if(formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
        KGetAppletUtils::paintTitle(p, m_theme, contentsRect);
    }
}

void KGetPieChart::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if(data["error"].toBool()) {
        if (!m_errorWidget) {
            delete d;
            d = 0;
            m_layout->removeAt(0);

            m_errorWidget = KGetAppletUtils::createErrorWidget(data["errorMessage"].toString(), this);
            m_layout->addItem(m_errorWidget);
        }
    }
    else if(!data["error"].toBool()) {
        if (m_errorWidget) {
            delete m_errorWidget;
            m_errorWidget = 0;

            d = new KGetPieChart::Private(this); 

            m_layout->removeAt(0);
            m_layout->addItem(d);
        }

        setTransfers(data["transfers"].toMap());
        d->setTransfers(m_transfers);
    }
}

#include "kgetpiechart.moc"
#include "kgetpiechart_p.moc"
