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

#include "plasma-kget.h"

#include <KDialog>
#include <KDebug>
#include <KIcon>

#include <QApplication>
#include <QPainter>
#include <QFontMetrics>

#include <plasma/svg.h>
#include <plasma/applet.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>

#include "transfergraph.h"
#include "barchart.h"
#include "piegraph.h"
#include "speedgraph.h"

PlasmaKGet::PlasmaKGet(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args),
                            m_error(false),
                            m_errorMessage(QString()),
                            m_size(QSizeF()),
                            m_dialog(0),
                            m_updatePaint(false)
{
    setHasConfigurationInterface(true);
    setDrawStandardBackground(true);

    m_transferGraph = 0;
    // the default transfer graph
    KConfigGroup cg = config();
    loadTransferGraph(cg.readEntry("graphType", QVariant(PlasmaKGet::BarChartType)).toUInt());

    m_engine = dataEngine("kget");
    m_engine->connectSource("KGet", this, 2000);
    m_engine->setProperty("refreshTime", cg.readEntry("refreshTime", (uint) 1000));
}

PlasmaKGet::~PlasmaKGet()
{}

QSizeF PlasmaKGet::contentSizeHint() const
{
    return m_size;
}

void PlasmaKGet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option,
                                const QRect &contentsRect)
{
    Q_UNUSED(option)
    kDebug() << "Update the graph " << endl;
    if(m_error) {
        p->save();
        p->setPen(Qt::white);
        p->drawText(contentsRect, Qt::AlignCenter, m_errorMessage);

        p->drawPixmap( contentsRect.x()+13, contentsRect.y() / 2 + 10,
                                                KIcon("dialog-warning").pixmap(30));
        p->restore();
    }
    else {
        m_transferGraph->paint(p, contentsRect);
    }

}

void PlasmaKGet::constraintsUpdated()
{
    prepareGeometryChange();
    QSizeF newSize;

    if(m_error) {
        newSize = QSizeF(300, 100);
    }
    else {
        newSize = m_transferGraph->contentSizeHint();
    }

    if((size() != newSize)) {
        m_size = newSize;
        resize(m_size);
    }

    if(m_updatePaint) {
        update();
    }

    m_updatePaint = false;
}

void PlasmaKGet::updated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if(m_error != data["error"].toBool() || m_transferGraph->transfers() != data["transfers"].toMap()) {
        m_updatePaint = true;
    }
    m_error = data["error"].toBool();
    m_errorMessage = data["errorMessage"].toString();
    m_transferGraph->setTransfers(data["transfers"].toMap());

    constraintsUpdated();
}

void PlasmaKGet::showConfigurationInterface()
{
    if(m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure KGet plasmoid"));

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);

        connect(m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        ui.graphType->addItem(i18n("Bar chart"), QVariant(PlasmaKGet::BarChartType));
        ui.graphType->addItem(i18n("Pie graph"), QVariant(PlasmaKGet::PieGraphType));
        ui.graphType->addItem(i18n("Speed graph"), QVariant(PlasmaKGet::SpeedGraphType));
    }
    m_dialog->show();
}

void PlasmaKGet::configAccepted()
{
    KConfigGroup cg = config();

    loadTransferGraph(ui.graphType->itemData(ui.graphType->currentIndex()).toUInt());
    m_engine->setProperty("refreshTime", ui.refreshTime->value());

    constraintsUpdated();
    cg.config()->sync();
}

void PlasmaKGet::loadTransferGraph(uint type)
{
    if(m_transferGraph) {
        delete m_transferGraph;
    }

    switch(type)
    {
        case PlasmaKGet::PieGraphType :
            m_transferGraph = new PieGraph();
            break;
        case PlasmaKGet::SpeedGraphType :
            m_transferGraph = new SpeedGraph();
            break;
        case PlasmaKGet::BarChartType :
        default:
            m_transferGraph = new BarChart();
    }
}

#include "plasma-kget.moc"
