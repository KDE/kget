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
#include <plasma/layouts/boxlayout.h>

#include "transfergraph.h"
#include "barchart.h"
#include "errorgraph.h"
#include "speedgraph.h"
#include "piegraph.h"

PlasmaKGet::PlasmaKGet(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args),
                            m_dialog(0),
                            m_errorMessage(QString()),
                            m_error(false),
                            m_graphType(0)
{
    setHasConfigurationInterface(true);
    setDrawStandardBackground(true);
}

PlasmaKGet::~PlasmaKGet()
{
    prepareGeometryChange();

    delete m_transferGraph;
}

void PlasmaKGet::init()
{
    m_layout = new Plasma::VBoxLayout(this);
    // setLayout(m_layout);

    //setMinimumSize(QSize(330, 100));

    m_transferGraph = 0;
    KConfigGroup cg = config();

    m_engine = dataEngine("kget");
    if (m_engine) {
      m_engine->connectSource("KGet", this);
      m_engine->setProperty("refreshTime", cg.readEntry("refreshTime", (uint) 2000));
    }
    else {
      kDebug()<<"KGet Engine could not be loaded";
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

        if(m_transferGraph->transfers() != data["transfers"].toMap()) {
            m_transferGraph->setTransfers(data["transfers"].toMap());
        }
    }

    m_error = data["error"].toBool();
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

    cg.config()->sync();
    loadTransferGraph(ui.graphType->itemData(ui.graphType->currentIndex()).toUInt());
    m_engine->setProperty("refreshTime", ui.refreshTime->value());
    cg.writeEntry("graphType", ui.graphType->itemData(ui.graphType->currentIndex()).toUInt());
}

void PlasmaKGet::loadTransferGraph(uint type)
{
    if(type != m_graphType) {
        prepareGeometryChange();
        delete m_transferGraph;

        switch(type)
        {
            case PlasmaKGet::ErrorGraphType :
                m_transferGraph = new ErrorGraph(this, m_layout, m_errorMessage);
                break;
            case PlasmaKGet::PieGraphType :
                m_transferGraph = new PieGraph(this, m_layout);
                break;
            case PlasmaKGet::SpeedGraphType :
                m_transferGraph = new SpeedGraph(this, m_layout);
                break;
            case PlasmaKGet::BarChartType :
            default:
                m_transferGraph = new BarChart(this, m_layout);
        }

        m_graphType = type;
    }
}

#include "plasma-kget.moc"
