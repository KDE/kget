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

#include <QVBoxLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsLayoutItem>
#include <QPainter>

#include <KDialog>
#include <KDebug>
#include <KIcon>
#include <KConfigDialog>

#include <plasma/svg.h>
#include <plasma/applet.h>
#include <plasma/theme.h>
#include <plasma/dataengine.h>

#include "transfergraph.h"
#include "barchart.h"
#include "errorgraph.h"
#include "speedgraph.h"
#include "piegraph.h"
#include "panelgraph.h"

#define TOP_MARGIN 60
#define MARGIN 10
#define SPACING 4

PlasmaKGet::PlasmaKGet(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args),
                            m_dialog(0),
                            m_errorMessage(QString()),
                            m_error(false),
                            m_graphType(0)
{
    setHasConfigurationInterface(true);
    setBackgroundHints(Applet::DefaultBackground);

    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/kget");
}

PlasmaKGet::~PlasmaKGet()
{
    delete m_transferGraph;
}

void PlasmaKGet::init()
{
    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setOrientation(Qt::Vertical);
    m_layout->setContentsMargins(MARGIN, TOP_MARGIN, MARGIN, MARGIN);
    m_layout->setSpacing(SPACING);

    if(formFactor() == Plasma::Vertical || formFactor() == Plasma::Horizontal) {
        setMaximumSize(QSize(80, 80));
        m_layout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    }
    else {
        setMinimumSize(QSize(240, 190));
    }
    m_transferGraph = 0;
    KConfigGroup cg = config();

    m_engine = dataEngine("kget");
    if (m_engine) {
        m_engine->connectSource("KGet", this);
        m_engine->setProperty("refreshTime", cg.readEntry("refreshTime", (uint) 4000));
    }
    else {
        kDebug() << "KGet Engine could not be loaded";
    }

    setLayout(m_layout);
}

void PlasmaKGet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    if(formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);

        m_theme->paint(p, QRect(contentsRect.x() + MARGIN +10, 
                                contentsRect.y() + MARGIN + 10, 111, 35), "title");
        m_theme->paint(p, QRect(contentsRect.x() + MARGIN + 10, 
                                contentsRect.y() + MARGIN + 45, 
                                contentsRect.width() - (MARGIN + 10) * 2, 1), "line");
    }
}

void PlasmaKGet::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if(data["error"].toBool() && !m_error) {
        m_errorMessage = data["errorMessage"].toString();
        loadTransferGraph(PlasmaKGet::ErrorGraphType);
    }
    else if(!data["error"].toBool()) {
        loadTransferGraph(config().readEntry("graphType", QVariant(PlasmaKGet::BarChartType)).toUInt());

        if(m_transferGraph && m_transferGraph->transfers() != data["transfers"].toMap()) {
            m_transferGraph->setTransfers(data["transfers"].toMap());
        }
    }

    m_error = data["error"].toBool();
}

void PlasmaKGet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(0);
    ui.setupUi(widget);

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.graphType->addItem(i18n("Bar Chart"), QVariant(PlasmaKGet::BarChartType));
    ui.graphType->addItem(i18n("Pie Graph"), QVariant(PlasmaKGet::PieGraphType));
    ui.graphType->addItem(i18n("Speed Graph"), QVariant(PlasmaKGet::SpeedGraphType));
    parent->addPage(widget, i18n("Configure KGet-Plasmoid"));
}

void PlasmaKGet::configAccepted()
{
    KConfigGroup cg = config();

    cg.config()->sync();
    loadTransferGraph(ui.graphType->itemData(ui.graphType->currentIndex()).toUInt());
    m_engine->setProperty("refreshTime", ui.refreshTime->value());
    cg.writeEntry("graphType", ui.graphType->itemData(ui.graphType->currentIndex()).toUInt());
}

void PlasmaKGet::loadTransferGraph(uint type)
{
    // QSizeF size = geometry().size();

    if(formFactor() == Plasma::Horizontal || formFactor() == Plasma::Vertical) {
        type = PlasmaKGet::PanelGraphType;
    }

    if(type != m_graphType) {
        delete m_transferGraph;

        switch(type)
        {
            case PlasmaKGet::ErrorGraphType :
                m_transferGraph = new ErrorGraph(this, m_errorMessage);
                break;
            case PlasmaKGet::PieGraphType :
                m_transferGraph = new PieGraph(this);
                break;
            case PlasmaKGet::SpeedGraphType :
                m_transferGraph = new SpeedGraph(this);
                break;
            case PlasmaKGet::PanelGraphType :
                m_transferGraph = new PanelGraph(this);
                break;
            case PlasmaKGet::BarChartType :
            default:
                m_transferGraph = new BarChart(this);
        }

        resize(QSize(m_layout->geometry().width(), m_layout->geometry().height()));
        // updateGeometry();
        m_graphType = type;
    }
//    resize(size);
}

#include "plasma-kget.moc"

